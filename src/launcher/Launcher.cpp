//
// Created by alexis on 8/30/18.
//

// system includes
extern "C" {
    #include "unistd.h"
    #include <appimage/appimage.h>
    #include <appimage/info.h>
}

// library includes
#include <QFile>
#include <QDebug>
#include <QMessageBox>
#include <QTemporaryDir>
#include <QTemporaryFile>

// local includes
#include "../shared.h"
#include "Launcher.h"


const QString& Launcher::getAppImagePath() const {
    return appImagePath;
}

void Launcher::setAppImagePath(const QString& appImagePath) {
    // error check before value is stored
    // throws exceptions in case of errors
    Launcher::validateAppImage(appImagePath);

    Launcher::appImagePath = appImagePath;

    // also update AppImage type to not have to read the file more than once to get that data
    Launcher::appImageType = appimage_get_type(appImagePath.toStdString().c_str(), false);
}

const std::vector<char*>& Launcher::getArgs() const {
    return args;
}

void Launcher::setArgs(const std::vector<char*>& args) {
    Launcher::args = args;
}

int Launcher::getAppImageType() const {
    return appImageType;
}

void Launcher::validateAppImage(const QString& appImagePath) {
    if (appImagePath.isEmpty())
        throw ValueError("AppImage path must not be empty");

    if (!QFile::exists(appImagePath))
        throw FileNotFoundError(appImagePath.toStdString());

    const auto appImageType = appimage_get_type(appImagePath.toStdString().c_str(), false);

    if (appImageType < 1)
        throw InvalidAppImageError("Types < 0 don't exist");

    if (appImageType > 2)
        throw UnsupportedTypeError("Types > 2 don't exist or aren't supported yet");
}

void Launcher::validateAppImage() const {
    Launcher::validateAppImage(appImagePath);
}

void Launcher::executeAppImage() {
    // needs to be converted to std::string to be able to use c_str()
    // when using QString and then .toStdString().c_str(), the std::string instance will be an rvalue, and the
    // pointer returned by c_str() will be invalid
    auto x = appImagePath.toStdString();
    auto fullPathToAppImage = QFileInfo(appImagePath).absoluteFilePath();

    // first of all, chmod +x the AppImage file, otherwise execv() will complain
    if (!makeExecutable(fullPathToAppImage))
        throw ExecutionFailedError(QObject::tr("Could not make AppImage executable: %1").arg(appImagePath).toStdString());

    // build path to AppImage runtime
    // as it might error, check before fork()ing to be able to display an error message beforehand
    auto exeDir = QFileInfo(QFile("/proc/self/exe").symLinkTarget()).absoluteDir().absolutePath();

    if (appImageType == 1) {
        QFile appImage(QString::fromStdString(fullPathToAppImage.toStdString()));

        if (!appImage.open(QIODevice::ReadOnly))
            throw ExecutionFailedError(QObject::tr("Failed to open AppImage for reading: %1").arg(appImagePath).toStdString());

        // copy AppImage to temp dir
        QTemporaryDir tempDir("/tmp/AppImageLauncher-type1-XXXXXX");

        if (!tempDir.isValid())
            throw ExecutionFailedError(QObject::tr("Unable to create temporary directory %1").arg(tempDir.path()).toStdString());

        tempDir.setAutoRemove(true);

        // TODO: Fin a better solution for this.
        // copy AppImage to temporary directory to prevent an execution loop, i.e., when AIL would call
        // the AppImage again, due to binfmt, the system would call AIL.
        // see: https://github.com/TheAssassin/AppImageLauncher/issues/3

        auto tempAppImagePath = QDir(tempDir.path()).absoluteFilePath(QFileInfo(appImage).fileName());
        if (!appImage.copy(tempAppImagePath))
            throw ExecutionFailedError(QObject::tr("Failed to create temporary copy of type 1 AppImage").toStdString());

        QFile tempAppImage(tempAppImagePath);

        if (!tempAppImage.open(QFile::ReadWrite))
            throw ExecutionFailedError(QObject::tr("Failed to open temporary AppImage copy for writing").toStdString());

        // nuke magic bytes
        if (!tempAppImage.seek(8))
            throw ExecutionFailedError(QObject::tr("Failed to remove magic bytes from temporary AppImage copy").toStdString());

        if (tempAppImage.write(QByteArray(3, '\0')) != 3)
            throw ExecutionFailedError(QObject::tr("Failed to remove magic bytes from temporary AppImage copy").toStdString());

        auto tempAppImageFileName = tempAppImage.fileName();

        // actually _write_ changes
        tempAppImage.close();

        makeExecutable(tempAppImageFileName);

        // need a char pointer instead of a const one, therefore can't use .c_str()
        std::vector<char> argv0Buffer(tempAppImageFileName.size() + 1, '\0');
        strcpy(argv0Buffer.data(), tempAppImageFileName.toStdString().c_str());

        // copy args before pushing nullptr to avoid modifications
        auto execArgs = args;

        // args need to be null terminated for execv
        execArgs.push_back(nullptr);

        execv(tempAppImageFileName.toStdString().c_str(), execArgs.data());

        const auto& error = errno;
        qCritical() << QObject::tr("execv() failed: %1", "error message").arg(strerror(error));
    } else if (appImageType == 2) {
        // use external runtime _without_ magic bytes to run the AppImage
        // the original idea was to use /lib64/ld-linux-x86_64.so to run AppImages, but it did complain about some
        // violated ELF data, and refused to run the AppImage
        // alternatively, the AppImage would have to be mounted by this application, and AppRun would need to be called
        // however, this requires some process management (e.g., killing all processes inside the AppImage and also
        // the FUSE "mount" process, when this application is killed...)
        setenv("TARGET_APPIMAGE", fullPathToAppImage.toStdString().c_str(), true);

        // suppress desktop integration script
        setenv("DESKTOPINTEGRATION", "AppImageLauncher", true);

        // first attempt: find runtime in expected installation directory
        auto pathToRuntime = exeDir.toStdString() + "/../lib/appimagelauncher/runtime";

        // next method: find runtime in expected build location
        if (!QFile(QString::fromStdString(pathToRuntime)).exists()) {
            pathToRuntime = exeDir.toStdString() + "/../lib/AppImageKit/src/runtime";
        }

        // if it can't be found in either location, display error and exit
        if (!QFile(QString::fromStdString(pathToRuntime)).exists())
            throw ExecutionFailedError(QObject::tr("runtime not found: no such file or directory: %1").arg(QString::fromStdString(pathToRuntime)).toStdString());

        // need a char pointer instead of a const one, therefore can't use .c_str()
        std::vector<char> argv0Buffer(appImagePath.toStdString().size() + 1, '\0');
        strcpy(argv0Buffer.data(), appImagePath.toStdString().c_str());

        // copy args before pushing nullptr to avoid modifications
        auto execArgs = args;

        // args need to be null terminated for execv
        execArgs.push_back(nullptr);

        execv(pathToRuntime.c_str(), execArgs.data());

        const auto& error = errno;
        qWarning() << QObject::tr("execv() failed: %1").arg(strerror(error));
    }
}

bool Launcher::shouldBeIgnored() {
    bool result =
            isAMountOrExtractOperation() &&
            // check for X-AppImage-Integrate=false
            appimage_shall_not_be_integrated(appImagePath.toStdString().c_str()) &&
            // ignore terminal apps (fixes #2)
            appimage_is_terminal_app(appImagePath.toStdString().c_str()) &&
            // AppImages in AppImages are not supposed to be integrated
            appImagePath.startsWith("/tmp/.mount_");

    return result;
}

bool Launcher::isAMountOrExtractOperation() const {
    // type 2 specific checks
    if (appImageType == 2) {
        // check parameters
        {
            for (auto& i : args) {
                QString arg = i;

                // reserved argument space
                const QString prefix = "--appimage-";

                if (arg.startsWith(prefix)) {
                    // don't annoy users who try to mount or extract AppImages
                    if (arg == prefix + "mount" || arg == prefix + "extract") {
                        return true;
                    }
                }
            }
        }
    }
    return false;
}

void Launcher::setIntegrationManager(AppImageDesktopIntegrationManager* integrationManager) {
    Launcher::integrationManager = integrationManager;
}

void Launcher::integrateAppImage() {
    integrationManager->integrateAppImage(appImagePath);
    appImagePath = integrationManager->buildDeploymentPath(appImagePath);
}

void Launcher::overrideAppImageIntegration() {
    auto targetPath = integrationManager->buildDeploymentPath(appImagePath);
    trashBin->disposeAppImage(targetPath);

    integrateAppImage();
}

void Launcher::setTrashBin(TrashBin* trashBin) {
    Launcher::trashBin = trashBin;
}

nlohmann::json Launcher::getAppImageInfo() {
    auto rawJson = appimage_extract_info(appImagePath.toStdString().c_str());
    nlohmann::json info = nlohmann::json::parse(rawJson);
    return info;
}

QIcon Launcher::getAppImageIcon() {
    QTemporaryFile temporaryFile;
    if (temporaryFile.open()) {

        appimage_extract_appinamge_icon_file(appImagePath.toStdString().c_str(), temporaryFile.fileName().toStdString().c_str());
        QIcon icon(temporaryFile.fileName());
        return icon;
    }
    return QIcon();
}

bool Launcher::isAppImageExecutable() {
    QFileInfo fileInfo(appImagePath);
    return fileInfo.isExecutable();
}

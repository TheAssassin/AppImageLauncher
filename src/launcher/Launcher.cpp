#include "unistd.h"

#include <QFile>
#include <QDebug>
#include <QMessageBox>
#include <QTemporaryDir>
#include <QTemporaryFile>
#include <appimage/appimage.h>
#include <appimage/info.h>

#include "Launcher.h"


void Launcher::setAppImagePath(const QString& appImagePath) {
    Launcher::appImagePath = appImagePath;
}

void Launcher::setArgs(const std::vector<char*>& args) {
    Launcher::args = args;
}


void Launcher::inspectAppImageFile() {
    if (appImagePath.isEmpty())
        throw AppImageFilePathNotSet();

    if (!QFile::exists(appImagePath))
        throw AppImageFileNotExists(appImagePath.toStdString());

    validateAppImageType();     // @throws InvalidAppImageFile, UnsuportedAppImageType
}

void Launcher::validateAppImageType() {
    appImageType = appimage_get_type(appImagePath.toStdString().c_str(), false);
    if (appImageType < 1)
        throw InvalidAppImageFile("");

    if (appImageType > 2)
        throw UnsuportedAppImageType("");
}

void Launcher::executeAppImage() {
    // needs to be converted to std::string to be able to use c_str()
    // when using QString and then .toStdString().c_str(), the std::string instance will be an rvalue, and the
    // pointer returned by c_str() will be invalid
    auto x = appImagePath.toStdString();
    auto fullPathToAppImage = QFileInfo(appImagePath).absoluteFilePath();

    // first of all, chmod +x the AppImage file, otherwise execv() will complain
    QFile appImageFile(fullPathToAppImage);
    if (!appImageFile.setPermissions(QFileDevice::ReadOwner | QFileDevice::ExeOwner))
        throw ExecutionFailed(QObject::tr("Unable to make the AppImage file executable: %1")
                                  .arg(appImagePath).toStdString());

    // build path to AppImage runtime
    // as it might error, check before fork()ing to be able to display an error message beforehand
    auto exeDir = QFileInfo(QFile("/proc/self/exe").symLinkTarget()).absoluteDir().absolutePath();

    // suppress desktop integration script
    setenv("DESKTOPINTEGRATION", "AppImageLauncher", true);

    if (appImageType == 1) {
        QFile appImage(QString::fromStdString(fullPathToAppImage.toStdString()));

        if (!appImage.open(QIODevice::ReadOnly))
            throw ExecutionFailed(QObject::tr("Not enough privileges to read the AppImage file: %1")
                                      .arg(appImagePath).toStdString());

        // copy AppImage to temp dir
        QTemporaryDir tempDir("/tmp/AppImageLauncher-type1-XXXXXX");

        if (!tempDir.isValid())
            throw ExecutionFailed(QObject::tr("Unable to create temporary directory %1")
                                      .arg(tempDir.path()).toStdString());

        tempDir.setAutoRemove(true);

        // TODO: Fin a better solution for this.
        // copy AppImage to temporary directory to prevent an execution loop, i.e., when AIL would call
        // the AppImage again, due to binfmt, the system would call AIL.
        // see: https://github.com/TheAssassin/AppImageLauncher/issues/3

        auto tempAppImagePath = QDir(tempDir.path()).absoluteFilePath(QFileInfo(appImage).fileName());
        if (!appImage.copy(tempAppImagePath))
            throw ExecutionFailed(QObject::tr("Failed to create temporary copy of type 1 AppImage").toStdString());

        QFile tempAppImage(tempAppImagePath);
        tempAppImage.setPermissions(QFileDevice::ReadOwner | QFileDevice::WriteOwner | QFileDevice::ExeOwner);
        if (!tempAppImage.open(QFile::ReadWrite))
            throw ExecutionFailed(QObject::tr("Failed to open temporary AppImage copy for writing").toStdString());

        // nuke magic bytes
        if (!tempAppImage.seek(8))
            throw ExecutionFailed(
                QObject::tr("Failed to remove magic bytes from temporary AppImage copy").toStdString());

        if (tempAppImage.write(QByteArray(3, '\0')) != 3)
            throw ExecutionFailed(
                QObject::tr("Failed to remove magic bytes from temporary AppImage copy").toStdString());

        auto tempAppImageFileName = tempAppImage.fileName();

        // actually _write_ changes
        tempAppImage.close();

        tryToMakeAppImageFileExecutable(tempAppImageFileName);

        // need a char pointer instead of a const one, therefore can't use .c_str()
        std::vector<char> argv0Buffer(tempAppImageFileName.size() + 1, '\0');
        strcpy(argv0Buffer.data(), tempAppImageFileName.toStdString().c_str());

        std::vector<char*> args;

        args.push_back(argv0Buffer.data());

        // copy arguments
        for (int i = 1; i < args.size(); i++) {
            args.push_back(args[i]);
        }

        // args need to be null terminated
        args.push_back(nullptr);

        execv(tempAppImageFileName.toStdString().c_str(), args.data());

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

        // first attempt: find runtime in expected installation directory
        auto pathToRuntime = exeDir.toStdString() + "/../lib/appimagelauncher/runtime";

        // next method: find runtime in expected build location
        if (!QFile(QString::fromStdString(pathToRuntime)).exists()) {
            pathToRuntime = exeDir.toStdString() + "/../../lib/AppImageKit/src/runtime";
        }

        // if it can't be found in either location, display error and exit
        if (!QFile(QString::fromStdString(pathToRuntime)).exists())
            throw ExecutionFailed(QObject::tr("runtime not found: no such file or directory: %1").arg(
                QString::fromStdString(pathToRuntime)).toStdString());

        // need a char pointer instead of a const one, therefore can't use .c_str()
        std::vector<char> argv0Buffer(appImagePath.toStdString().size() + 1, '\0');
        strcpy(argv0Buffer.data(), appImagePath.toStdString().c_str());

        std::vector<char*> args;

        args.push_back(argv0Buffer.data());

        // copy arguments
        for (int i = 1; i < args.size(); i++) {
            args.push_back(args[i]);
        }

        // args need to be null terminated
        args.push_back(nullptr);

        execv(pathToRuntime.c_str(), args.data());

        const auto& error = errno;
        throw ExecutionFailed(
            QObject::tr("execv() failed: %1").arg(QString::fromStdString(strerror(error))).toStdString());
    }
}

void Launcher::tryToMakeAppImageFileExecutable(const QString& path) const {
    QFile appImageFile(path);
    if (!appImageFile.setPermissions(QFileDevice::ReadOwner | QFileDevice::ExeOwner))
        throw ExecutionFailed(QObject::tr("Unable to make the AppImage file executable: %1")
                                  .arg(path).toStdString());
}

bool Launcher::isCandidateForDesktopIntegration() {
    if (isAMountOrExtractOperation())
        return false;

    // check for X-AppImage-Integrate=false
    if (appimage_shall_not_be_integrated(appImagePath.toStdString().c_str()) > 0)
        return false;

    // ignore terminal apps (fixes #2)
    if (appimage_is_terminal_app(appImagePath.toStdString().c_str()) > 0)
        return false;

    // AppImages in AppImages are not supposed to be integrated
    if (appImagePath.startsWith("/tmp/.mount_"))
        return false;

    return true;
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
    appImagePath = AppImageDesktopIntegrationManager::buildDeploymentPath(appImagePath);
}

void Launcher::overrideAppImageIntegration() {
    auto targetPath = AppImageDesktopIntegrationManager::buildDeploymentPath(appImagePath);
    trashBin->disposeAppImage(targetPath);

    integrateAppImage();
}

void Launcher::setTrashBin(TrashBin* trashBin) {
    Launcher::trashBin = trashBin;
}

nlohmann::json Launcher::getAppImageInfo() {
    auto rawJson = appimage_extract_info(appImagePath.toStdString().c_str());
    nlohmann::json info = nlohmann::json::parse(rawJson);
    if (info.find("file") == info.end())
        throw InvalidAppImageFile("Unable to extract AppImage info.");
    return info;
}

QIcon Launcher::getAppImageIcon() {
    QTemporaryFile temporaryFile;
    if (temporaryFile.open()) {

        appimage_extract_icon_file(appImagePath.toStdString().c_str(),
                                   temporaryFile.fileName().toStdString().c_str());
        QIcon icon(temporaryFile.fileName());
        return icon;
    }
    return QIcon();
}

bool Launcher::isAppImageExecutable() {
    QFileInfo fileInfo(appImagePath);
    return fileInfo.isExecutable();
}


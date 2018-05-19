// library headers
#include <QDir>
#include <QLibraryInfo>
#include <QString>

// local headers
#include "translationmanager.h"

TranslationManager::TranslationManager(QApplication& app) : app(app) {
    // set up translations
    auto qtTranslator = new QTranslator();
    qtTranslator->load("qt_" + QLocale::system().name(), QLibraryInfo::location(QLibraryInfo::TranslationsPath));
    app.installTranslator(qtTranslator);

    // first we need to find the translation directory
    // if this is run from the source tree, we try a path that can only work within the repository
    // then, we try the expected install location relative to the main binary
    auto translationDir = QString(CMAKE_PROJECT_SOURCE_DIR) + "/resources/l10n";

    if (!QDir(translationDir).exists()) {
        const auto binaryDirPath = app.applicationDirPath();

        translationDir = binaryDirPath + "/../share/appimagelauncher/l10n";
    }

    const auto systemLocale = QLocale::system().name();

    // we're using primarily short names for translations, so we should load these translations as well
    const auto shortSystemLocale = systemLocale.split('_')[0];

    auto myappTranslator = new QTranslator();
    myappTranslator->load(translationDir + "/ui." + systemLocale + ".qm");
    myappTranslator->load(translationDir + "/ui." + shortSystemLocale + ".qm");
    app.installTranslator(myappTranslator);

    // store translators in list so they won't be deleted
    installedTranslators.push_back(qtTranslator);
    installedTranslators.push_back(myappTranslator);
}

TranslationManager::~TranslationManager() {
    for (auto& translator : installedTranslators) {
        delete translator;
        translator = nullptr;
    }
}

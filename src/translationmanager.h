#include <QApplication>
#include <QTranslator>
#include <QList>

/*
 * Installs translations for AppImageLauncher UIs in a Qt application.
 *
 * You need to keep instances of this alive until the application is terminated.
 */
class TranslationManager {
private:
    const QApplication& app;
    QList<QTranslator*> installedTranslators;

public:
    explicit TranslationManager(QApplication& app);
    ~TranslationManager();

public:
    // get translation dir
    static QString getTranslationDir();
};

#pragma once

// library headers
#include <QObject>
#include <QSettings>
#include <QTimer>
#include <QLoggingCategory>

// local headers
#include "worker.h"
#include "types.h"
#include "filesystemwatcher.h"

namespace appimagelauncher::daemon {

    Q_DECLARE_LOGGING_CATEGORY(daemonCat)

    class Daemon : public QObject {
        Q_OBJECT

    public:
        explicit Daemon(QObject* parent = nullptr);
        QDirSet watchedDirectories() const;
        bool startWatching();

    public slots:
        void slotStopWatching();

    private:
        void initialSearchForAppImages(const QDirSet& dirsToSearch);

        QSettings *_settings;
        Worker* _worker;
        FileSystemWatcher *_watcher;

        QTimer* _updateWatchedDirsTimer;
    };

} // namespace

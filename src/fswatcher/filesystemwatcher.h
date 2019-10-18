// system includes
#include <algorithm>
#include <memory>
#include <unordered_set>

// library includes
#include <QDir>
#include <QObject>
#include <QSet>
#include <QString>
#include <QThread>

// local includes
#include "types.h"

#pragma once

class FileSystemWatcherError : public std::runtime_error {
public:
    explicit FileSystemWatcherError(const QString& message) : std::runtime_error(message.toStdString().c_str()) {};
};

class FileSystemWatcher : public QObject {
    Q_OBJECT

private:
    class PrivateData;
    std::shared_ptr<PrivateData> d;

public:
    explicit FileSystemWatcher(const QDir& directory);
    explicit FileSystemWatcher(const QDirSet& paths);
    FileSystemWatcher();

public slots:
    bool startWatching();
    bool stopWatching();
    void readEvents();
    bool updateWatchedDirectories(QDirSet watchedDirectories);

public:
    QDirSet directories();

signals:
    void fileChanged(QString path);
    void fileRemoved(QString path);
    void newDirectoriesToWatch(QDirSet set);
    void directoriesToWatchDisappeared(QDirSet set);
};

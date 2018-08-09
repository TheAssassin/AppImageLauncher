// system includes
#include <memory>

// library includes
#include <QObject>
#include <QString>
#include <QThread>

#pragma once

class FileSystemWatcherError : public std::runtime_error {
public:
    FileSystemWatcherError(const QString& message) : std::runtime_error(message.toStdString().c_str()) {};
};

class FileSystemWatcher : public QObject {
    Q_OBJECT

private:
    class PrivateData;
    std::shared_ptr<PrivateData> d;

public:
    explicit FileSystemWatcher(const QString& path);
    explicit FileSystemWatcher(const QStringList& paths);
    FileSystemWatcher();

public:
    bool startWatching();
    bool stopWatching();

public slots:
    void readEventsForever();

public:
    QStringList directories();

signals:
    void fileCreated(QString path);
    void fileModified(QString path);
    void fileDeleted(QString path);
};

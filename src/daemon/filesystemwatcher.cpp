// system includes
#include <iostream>
#include <map>
#include <unistd.h>

// library includes
#include <QDir>
#include <QStringList>
#include <QThread>
#include <sys/inotify.h>

// local includes
#include "filesystemwatcher.h"

class INotifyEvent {
public:
    uint32_t mask;
    QString path;

public:
    INotifyEvent(uint32_t mask, QString path) : mask(mask), path(std::move(path)) {}
};

class FileSystemWatcher::PrivateData {
public:
    enum EVENT_TYPES {
        fileCreationEvents = IN_CREATE | IN_MOVED_TO,
        fileModificationEvents = IN_CLOSE_WRITE | IN_MODIFY,
        fileDeletionEvents = IN_DELETE | IN_MOVED_FROM,
    };

public:
    QStringList watchedDirectories;

private:
    int fd = -1;
    std::map<int, QString> watchFdMap;

public:
    // reads events from the inotify fd and emits the correct signals
    std::vector<INotifyEvent> readEventsFromFd() {
        // read raw bytes into buffer
        // this is necessary, as the inotify_events have dynamic sizes
        static const auto bufSize = 4096;
        char buffer[bufSize] __attribute__ ((aligned(8)));

        const auto rv = read(fd, buffer, bufSize);
        const auto error = errno;

        if (rv == 0) {
            throw FileSystemWatcherError("read() on inotify FD must never return 0");
        }

        if (rv == -1) {
            // we're using a non-blocking inotify fd, therefore, if errno is set to EAGAIN, we just didn't find any
            // new events
            // this is not an error case
            if (error == EAGAIN)
                return {};

            throw FileSystemWatcherError(QString("Failed to read from inotify fd: ") + strerror(error));
        }

        // read events into vector
        std::vector<INotifyEvent> events;

        for (char* p = buffer; p < buffer + rv;) {
            // create inotify_event from current position in buffer
            auto* currentEvent = (struct inotify_event*) p;

            // initialize new INotifyEvent with the data from the currentEvent
            QString relativePath(currentEvent->name);
            auto directoryPath = watchFdMap[currentEvent->wd];
            events.emplace_back(currentEvent->mask, directoryPath + "/" + relativePath);

            // update current position in buffer
            p += sizeof(struct inotify_event) + currentEvent->len;
        }

        return events;
    }

    PrivateData() : watchedDirectories() {
        fd = inotify_init1(IN_NONBLOCK);
        if (fd < 0) {
            auto error = errno;
            throw FileSystemWatcherError(strerror(error));
        }
    };

    bool startWatching() {
        static const auto mask = fileCreationEvents | fileModificationEvents | fileDeletionEvents;

        for (const auto& directory : watchedDirectories) {
            const int watchFd = inotify_add_watch(fd, directory.toStdString().c_str(), mask);

            if (watchFd == -1) {
                const auto error = errno;
                std::cerr << "Failed to start watching: " << strerror(error) << std::endl;
                return false;
            }

            watchFdMap[watchFd] = directory;
        }

        return true;
    }

    bool stopWatching() {
        while (!watchFdMap.empty()) {
            const auto pair = *(watchFdMap.begin());
            const auto watchFd = pair.first;

            if (inotify_rm_watch(fd, watchFd) == -1) {
                const auto error = errno;
                std::cerr << "Failed to stop watching: " << strerror(error) << std::endl;
                return false;
            }

            watchFdMap.erase(watchFd);
        }

        return true;
    }
};

FileSystemWatcher::FileSystemWatcher() {
    d = std::make_shared<PrivateData>();
}

FileSystemWatcher::FileSystemWatcher(const QString& path) : FileSystemWatcher() {
    if (!QDir(path).exists())
        QDir().mkdir(path);
    d->watchedDirectories.append(path);
}

FileSystemWatcher::FileSystemWatcher(const QStringList& paths) : FileSystemWatcher() {
    d->watchedDirectories.append(paths);
}

QStringList FileSystemWatcher::directories() {
    return d->watchedDirectories;
}

bool FileSystemWatcher::startWatching() {
    return d->startWatching();
}

bool FileSystemWatcher::stopWatching() {
    return d->stopWatching();
}

void FileSystemWatcher::readEventsForever() {
    while (true) {
        auto events = d->readEventsFromFd();

        if (events.empty()) {
            QThread::msleep(100);
            continue;
        }

        for (const auto& event : events) {
            const auto mask = event.mask;
            if (mask & d->fileCreationEvents) {
                emit fileCreated(event.path);
            } else if (mask & d->fileDeletionEvents) {
                emit fileDeleted(event.path);
            } else if (mask & d->fileModificationEvents) {
                emit fileModified(event.path);
            }
        }
    }
}

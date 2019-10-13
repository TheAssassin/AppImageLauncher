// system includes
#include <iostream>
#include <map>
#include <unistd.h>

// library includes
#include <QDebug>
#include <QDir>
#include <QTimer>
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
        // events that indicate file creations, modifications etc.
        fileChangeEvents = IN_CLOSE_WRITE | IN_MOVE,
        // events that indicate a file removal from a directory, e.g., deletion or moving to another location
        fileRemovalEvents = IN_DELETE | IN_MOVED_FROM,
    };

    // tracks whether the watcher is running
    bool isRunning;

public:
    QDirSet watchedDirectories;
    QTimer eventsLoopTimer;

private:
    int inotifyFd = -1;
    std::map<int, QDir> watchFdMap;

public:
    // reads events from the inotify fd and emits the correct signals
    std::vector<INotifyEvent> readEventsFromFd() {
        // read raw bytes into buffer
        // this is necessary, as the inotify_events have dynamic sizes
        static const auto bufSize = 4096;
        char buffer[bufSize] __attribute__ ((aligned(8)));

        const auto rv = read(inotifyFd, buffer, bufSize);
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
            auto directory = watchFdMap[currentEvent->wd];
            events.emplace_back(currentEvent->mask, directory.absolutePath() + "/" + relativePath);

            // update current position in buffer
            p += sizeof(struct inotify_event) + currentEvent->len;
        }

        return events;
    }

    PrivateData() : isRunning(false), watchedDirectories() {
        inotifyFd = inotify_init1(IN_NONBLOCK);
        if (inotifyFd < 0) {
            auto error = errno;
            throw FileSystemWatcherError(strerror(error));
        }
    };

    bool startWatching(const QDir& directory) {
        if (isRunning) {
            qDebug() << "tried to start file system watcher while it's running already";
            return true;
        }

        isRunning = true;

        static const auto mask = fileChangeEvents | fileRemovalEvents;

        if (!directory.exists()) {
            qDebug() << "Warning: directory " << directory.absolutePath() << " does not exist, skipping";
            return true;
        }

        const int watchFd = inotify_add_watch(inotifyFd, directory.absolutePath().toStdString().c_str(), mask);

        if (watchFd == -1) {
            const auto error = errno;
            std::cerr << "Failed to start watching: " << strerror(error) << std::endl;
            return false;
        }

        watchFdMap[watchFd] = directory;
        eventsLoopTimer.start();

        return true;
    }

    bool startWatching() {
        if (!isRunning) {
            qDebug() << "tried to stop file system watcher while stopped";
            return true;
        }

        for (const auto& directory : watchedDirectories) {
            if (!startWatching(directory))
                return false;
        }

        isRunning = false;

        return true;
    }

    bool stopWatching(int watchFd) {
        // no matter whether the watch removal succeeds, retrying to remove the watch won't help
        // therefore, we can remove the file descriptor from the map in any case
        watchFdMap.erase(watchFd);

        std::cout << "stop watching watchfd " << watchFd << std::endl;

        if (inotify_rm_watch(inotifyFd, watchFd) == -1) {
            const auto error = errno;
            std::cerr << "Failed to stop watching: " << strerror(error) << std::endl;
            return false;
        }

        return true;
    }

    bool stopWatching() {
        while (!watchFdMap.empty()) {
            const auto pair = *(watchFdMap.begin());
            const auto watchFd = pair.first;

            if (!stopWatching(watchFd)) {
                std::cerr << "Warning: Failed to stop watching on file descriptor " << watchFd << std::endl;
            }
        }

        eventsLoopTimer.stop();

        return true;
    }
};

FileSystemWatcher::FileSystemWatcher() {
    d = std::make_shared<PrivateData>();

    d->eventsLoopTimer.setInterval(100);
    connect(&d->eventsLoopTimer, &QTimer::timeout, this, &FileSystemWatcher::readEvents);
}

FileSystemWatcher::FileSystemWatcher(const QDir& path) : FileSystemWatcher() {
    updateWatchedDirectories(QDirSet{{path}});
}

FileSystemWatcher::FileSystemWatcher(const QDirSet& paths) : FileSystemWatcher() {
    updateWatchedDirectories(paths);
}

QDirSet FileSystemWatcher::directories() {
    return d->watchedDirectories;
}

bool FileSystemWatcher::startWatching() {
    return d->startWatching();
}

bool FileSystemWatcher::stopWatching() {
    return d->stopWatching();
}

void FileSystemWatcher::readEvents() {
    auto events = d->readEventsFromFd();

    for (const auto& event : events) {
        const auto mask = event.mask;

        if (mask & d->fileChangeEvents) {
            emit fileChanged(event.path);
        } else if (mask & d->fileRemovalEvents) {
            emit fileRemoved(event.path);
        }
    }
}

bool FileSystemWatcher::updateWatchedDirectories(QDirSet watchedDirectories) {
    // the list may contain entries for directories which don't exist already, therefore we have to remove those first
    // so when they'll be created, we'll notice
    {
        // erase-remove doesn't work with sets apparently (see https://stackoverflow.com/a/26833313)
        // therefore we use a simple custom algorithm
        auto it = watchedDirectories.begin();
        while (it != watchedDirectories.end()) {
            if (!it->exists()) {
                it = watchedDirectories.erase(it);
            } else {
                ++it;
            }
        }
    }

    // first, we calculate which directores are new to be watched
    QDirSet newDirectories;

    std::set_difference(
        watchedDirectories.begin(), watchedDirectories.end(),
        d->watchedDirectories.begin(), d->watchedDirectories.end(),
        std::inserter(newDirectories, newDirectories.end()),
        QDirComparator{}
    );

    emit newDirectoriesToWatch(newDirectories);

    // now we can update the internal state
    d->watchedDirectories = watchedDirectories;

    // if the watching hasn't been started yet, we shouldn't start/stop any watches
    // unfortunately we need an extra variable to track this...
    if (!d->isRunning)
        return true;

    if (!stopWatching())
        return false;

    if (!startWatching())
        return false;

    return true;
}

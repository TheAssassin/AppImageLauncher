// system includes
#include <iostream>
#include <map>
#include <unistd.h>

// library includes
#include <QDebug>
#include <QDir>
#include <QMutex>
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
    QMutex* mutex;

private:
    int inotifyFd = -1;
    std::map<int, QDir> watchFdMap;

public:
    // reads events from the inotify fd and emits the correct signals
    std::vector<INotifyEvent> readEventsFromFd() {
        // we don't want to read events in parallel
        QMutexLocker lock{mutex};

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

    PrivateData() : isRunning(false), watchedDirectories(), mutex(new QMutex) {
        inotifyFd = inotify_init1(IN_NONBLOCK);

        if (inotifyFd < 0) {
            auto error = errno;
            throw FileSystemWatcherError(QString("Failed to initialize inotify, reason: ") + strerror(error));
        }
    };

    // caution: method is not threadsafe!
    bool startWatching(const QDir& directory) {
        static const auto mask = fileChangeEvents | fileRemovalEvents;

        qDebug() << "start watching directory " << directory;

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
        QMutexLocker lock{mutex};

        for (const auto& directory : watchedDirectories) {
            if (!startWatching(directory))
                return false;
        }

        return true;
    }

    bool startWatching(const QDirSet& directories) {
        QMutexLocker lock{mutex};

        for (const auto& directory : directories) {
            if (!startWatching(directory)) {
                return false;
            }
        }

        return true;
    }

    // caution: method is not threadsafe!
    bool stopWatching(int watchFd) {
        // no matter whether the watch removal succeeds, retrying to remove the watch won't help
        // therefore, we can remove the file descriptor from the map in any case
        watchFdMap.erase(watchFd);

        qDebug() << "stop watching watchfd " << watchFd;

        if (inotify_rm_watch(inotifyFd, watchFd) == -1) {
            const auto error = errno;
            std::cerr << "Failed to stop watching: " << strerror(error) << std::endl;
            return false;
        }

        return true;
    }

    bool stopWatching() {
        QMutexLocker lock{mutex};

        while (!watchFdMap.empty()) {
            const auto pair = *(watchFdMap.begin());
            const auto watchFd = pair.first;

            if (!stopWatching(watchFd)) {
                std::cerr << "Warning: Failed to stop watching on file descriptor " << watchFd << std::endl;
            }
        }

        return true;
    }

    bool stopWatching(const QDirSet& directories) {
        QMutexLocker lock{mutex};

        for (const auto& directory : directories) {
            for (const auto& pair : watchFdMap) {
                if (pair.second == directory) {
                    if (!stopWatching(pair.first)) {
                        return false;
                    }
                }
            }

            // reaching the following line means that we couldn't find the requested path in the fd map
            return false;
        }

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
    QMutexLocker lock{d->mutex};

    return d->watchedDirectories;
}

bool FileSystemWatcher::startWatching() {
    {
        QMutexLocker lock{d->mutex};

        if (d->isRunning) {
            qDebug() << "tried to start file system watcher while it's running already";
            return true;
        }
    }

    auto rv = d->startWatching();

    {
        QMutexLocker lock{d->mutex};

        if (rv)
            d->isRunning = true;
    }

    return rv;
}

bool FileSystemWatcher::stopWatching() {
    {
        QMutexLocker lock{d->mutex};

        if (!d->isRunning) {
            qDebug() << "tried to stop file system watcher while stopped";
            return true;
        }
    }

    const auto rv = d->stopWatching();

    {
        QMutexLocker lock{d->mutex};

        if (rv) {
            d->isRunning = false;

            // we can stop reporting events now, I guess
            d->eventsLoopTimer.stop();
        }
    }

    return rv;
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

    auto setDifference = [](const QDirSet& toExamine, const QDirSet& toSearchFor) -> QDirSet {
        QDirSet results;

        // QDir behaves weirdly with STL algorithm comparisons etc.
        // therefore we implement this difference algorithm all by ourselves to make sure it works correctly
        for (const auto& examined : toExamine) {
            bool found = false;

            for (const auto& searched : toSearchFor) {
                if (searched == examined) {
                    found = true;
                    break;
                }
            }

            if (!found) {
                results.insert(examined);
            }
        }

        return results;
    };

    // first, we calculate which directores are new to be watched
    QDirSet newDirectories = setDifference(watchedDirectories, d->watchedDirectories);

    // to stop watching with a fine granularity, we also need to know which directories have been removed
    QDirSet disappearedDirectories = setDifference(d->watchedDirectories, watchedDirectories);

    {
        QMutexLocker lock{d->mutex};

        // now we can update the internal state
        d->watchedDirectories = watchedDirectories;

        // if the watching hasn't been started yet, we shouldn't start/stop any watches
        // unfortunately we need an extra variable to track this...
        if (!d->isRunning)
            return true;
    }

    // we must run both stop and start methods, so we cannot directly return false if either fails
    // also, this makes sure the signals are sent even in case either of the following methods fails
    bool rv = true;
    rv = rv && d->stopWatching(disappearedDirectories);
    rv = rv && d->startWatching(newDirectories);

    // send out the signals for further handling by users of a fs watcher instance
    emit newDirectoriesToWatch(newDirectories);
    emit directoriesToWatchDisappeared(disappearedDirectories);

    return rv;
}

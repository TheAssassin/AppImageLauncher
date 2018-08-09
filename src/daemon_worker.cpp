// system includes
#include <atomic>
#include <iostream>
#include <deque>

// library includes
#include <QFile>
#include <QMutex>
#include <QObject>
#include <QTimer>
#include <appimage/appimage.h>

// local includes
#include "daemon_worker.h"
#include "shared.h"

class Worker::PrivateData {
public:
    std::atomic<bool> timerActive;

    static constexpr int TIMEOUT = 15 * 1000;

    QMutex mutex;

    enum OP_TYPE {
        INTEGRATE = 0,
        UNINTEGRATE = 1,
    };

    // std::set is unordered, therefore using std::deque to keep the order of the operations
    std::deque<std::pair<QString, OP_TYPE>> deferredOperations;

public:
    PrivateData() : timerActive(false) {}

public:
    // in addition to a simple duplicate check, this function is context sensitive
    // it starts with the last element, and checks for duplicates until an opposite action is found
    // for instance, when the element shall integrated, it will check for duplicates until an unintegration operation
    // is found
    bool isDuplicate(std::pair<QString, OP_TYPE> operation) {
        for (auto it = deferredOperations.rbegin(); it != deferredOperations.rend(); ++it) {
            if ((*it).first == operation.first) {
                // if operation type is different, then the operation is new, and should be added to the list
                // if it is equal, it's a duplicate
                // in either case, the loop can be aborted here
                return (*it).second == operation.second;
            }
        }

        return false;
    }
};

Worker::Worker() {
    d = std::make_shared<PrivateData>();

    connect(this, &Worker::startTimer, this, &Worker::startTimerIfNecessary);
}

void Worker::executeDeferredOperations() {
    d->mutex.lock();

    std::cout << "Executing deferred operations" << std::endl;

    bool cleanUpAfterwards = false;

    while (!d->deferredOperations.empty()) {
        auto entry = d->deferredOperations.front();
        d->deferredOperations.pop_front();

        const auto& path = entry.first;
        const auto& type = entry.second;

        const auto exists = QFile::exists(path);
        const auto appImageType = appimage_get_type(path.toStdString().c_str(), false);
        const auto isAppImage = 0 < appImageType && appImageType <= 2;

        if (type == d->INTEGRATE) {
            std::cout << "Integrating: " << path.toStdString() << std::endl;

            if (!exists) {
                std::cout << "ERROR: file does not exist, cannot integrate" << std::endl;
                continue;
            }

            if (!isAppImage) {
                std::cout << "ERROR: not an AppImage, skipping" << std::endl;
                continue;
            }

            // check for X-AppImage-Integrate=false
            if (appimage_shall_not_be_integrated(path.toStdString().c_str())) {
                std::cout << "WARNING: AppImage shall not be integrated, skipping" << std::endl;
                continue;
            }

            if (!integrateAppImage(path, path)) {
                std::cout << "ERROR: Failed to register AppImage in system" << std::endl;
                continue;
            }
        } else if (type == d->UNINTEGRATE) {
            cleanUpAfterwards = true;
        }
    }

    if (cleanUpAfterwards) {
        std::cout << "Cleaning up old desktop integration files" << std::endl;
        if (!cleanUpOldDesktopIntegrationResources(true)) {
            std::cout << "Failed to clean up old desktop integration files" << std::endl;
        }
    }

    // make sure the icons in the launcher are refreshed
    std::cout << "Updating desktop database and icon caches" << std::endl;
    if (!updateDesktopDatabaseAndIconCaches())
        std::cout << "Failed to update desktop database and icon caches" << std::endl;

    std::cout << "Done" << std::endl;

    // while unlocking would be possible before the cleanup, this allows for a more consistent console output
    d->mutex.unlock();
};

void Worker::scheduleForIntegration(const QString& path) {
    d->mutex.lock();

    auto entry = std::make_pair(path, d->INTEGRATE);
    if (!d->isDuplicate(entry)) {
        std::cout << "Scheduling for (re-)integration: " << path.toStdString() << std::endl;
        d->deferredOperations.push_back(entry);
        emit startTimer();
    }

    d->mutex.unlock();
}

void Worker::scheduleForUnintegration(const QString& path) {
    d->mutex.lock();

    auto entry = std::make_pair(path, d->UNINTEGRATE);
    if (!d->isDuplicate(entry)) {
        std::cout << "Scheduling for unintegration: " << path.toStdString() << std::endl;
        d->deferredOperations.push_back(entry);
        emit startTimer();
    }

    d->mutex.unlock();
}

void Worker::startTimerIfNecessary() {
    if (!d->timerActive) {
        d->timerActive = true;
        auto* timer = new QTimer();
        timer->setSingleShot(true);
        timer->setInterval(d->TIMEOUT);
        connect(timer, &QTimer::timeout, [this, timer]() {
            d->timerActive = false;
            executeDeferredOperations();
            delete timer;
        });
        timer->start();
    }
}

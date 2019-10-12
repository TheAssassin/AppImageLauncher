// system includes
#include <atomic>
#include <iostream>
#include <deque>

// library includes
#include <QDebug>
#include <QFile>
#include <QObject>
#include <QSysInfo>
#include <QTimer>
#include <QThreadPool>
#include <QMutexLocker>
#include <appimage/appimage.h>

// local includes
#include "worker.h"
#include "shared.h"

enum OP_TYPE {
    INTEGRATE = 0,
    UNINTEGRATE = 1,
};

typedef std::pair<QString, OP_TYPE> Operation;

class Worker::PrivateData {
public:
    QTimer deferredOperationsTimer;

    static constexpr int TIMEOUT = 15 * 1000;

    // std::set is unordered, therefore using std::deque to keep the order of the operations
    std::deque<Operation> deferredOperations;

    class OperationTask : public QRunnable {
    private:
        Operation operation;
        std::shared_ptr<QMutex> mutex;

    public:
        OperationTask(const Operation& operation, std::shared_ptr<QMutex> mutex) : operation(operation),
                                                                                   mutex(std::move(mutex)) {}

        void run() override {
            const auto& path = operation.first;
            const auto& type = operation.second;

            const auto exists = QFile::exists(path);
            const auto appImageType = appimage_get_type(path.toStdString().c_str(), false);
            const auto isAppImage = 0 < appImageType && appImageType <= 2;

            if (type == INTEGRATE) {
                {   // Scope for Output Mutex Locker
                    QMutexLocker mutexLocker(mutex.get());
                    std::cout << "Integrating: " << path.toStdString() << std::endl;

                    if (!exists) {
                        std::cout << "ERROR: file does not exist, cannot integrate" << std::endl;
                        return;
                    }

                    if (!isAppImage) {
                        std::cout << "ERROR: not an AppImage, skipping" << std::endl;
                        return;
                    }
                }

                // check for X-AppImage-Integrate=false
                if (appimage_shall_not_be_integrated(path.toStdString().c_str())) {
                    QMutexLocker mutexLocker(mutex.get());
                    std::cout << "WARNING: AppImage shall not be integrated, skipping" << std::endl;
                    return;
                }

                if (!installDesktopFileAndIcons(path)) {
                    QMutexLocker mutexLocker(mutex.get());
                    std::cout << "ERROR: Failed to register AppImage in system" << std::endl;
                    return;
                }
            } else if (type == UNINTEGRATE) {
                // nothing to do
            }
        }
    };

public:
    PrivateData() {
        deferredOperationsTimer.setSingleShot(true);
        deferredOperationsTimer.setInterval(TIMEOUT);
    }

public:
    // in addition to a simple duplicate check, this function is context sensitive
    // it starts with the last element, and checks for duplicates until an opposite action is found
    // for instance, when the element shall integrated, it will check for duplicates until an unintegration operation
    // is found
    bool isDuplicate(Operation operation) {
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

    connect(this, &Worker::startTimer, this, &Worker::startTimerIfNecessary, Qt::QueuedConnection);
    connect(&d->deferredOperationsTimer, &QTimer::timeout, this, &Worker::executeDeferredOperations);
}

void Worker::executeDeferredOperations() {
    if (d->deferredOperations.empty()) {
        qDebug() << "No deferred operations to execute";
        return;
    }

    std::cout << "Executing deferred operations" << std::endl;

    auto outputMutex = std::make_shared<QMutex>();

    while (!d->deferredOperations.empty()) {
        auto operation = d->deferredOperations.front();
        d->deferredOperations.pop_front();
        QThreadPool::globalInstance()->start(new PrivateData::OperationTask(operation, outputMutex));
    }

    // wait until all AppImages have been integrated
    QThreadPool::globalInstance()->waitForDone();

    std::cout << "Cleaning up old desktop integration files" << std::endl;
    if (!cleanUpOldDesktopIntegrationResources(true)) {
        std::cout << "Failed to clean up old desktop integration files" << std::endl;
    }

    // make sure the icons in the launcher are refreshed
    std::cout << "Updating desktop database and icon caches" << std::endl;
    if (!updateDesktopDatabaseAndIconCaches())
        std::cout << "Failed to update desktop database and icon caches" << std::endl;

    std::cout << "Done" << std::endl;
}

void Worker::scheduleForIntegration(const QString& path) {
    auto operation = std::make_pair(path, INTEGRATE);
    if (!d->isDuplicate(operation)) {
        std::cout << "Scheduling for (re-)integration: " << path.toStdString() << std::endl;
        d->deferredOperations.push_back(operation);
        emit startTimer();
    }

}

void Worker::scheduleForUnintegration(const QString& path) {
    auto operation = std::make_pair(path, UNINTEGRATE);
    if (!d->isDuplicate(operation)) {
        std::cout << "Scheduling for unintegration: " << path.toStdString() << std::endl;
        d->deferredOperations.push_back(operation);
        emit startTimer();
    }
}

void Worker::startTimerIfNecessary() {
    if (!d->deferredOperationsTimer.isActive())
        QMetaObject::invokeMethod(&d->deferredOperationsTimer, "start");
}

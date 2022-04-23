// system includes
#include <memory>

// library includes
#include <QObject>
#include <QLoggingCategory>

#pragma once

namespace appimagelauncher::daemon {

    Q_DECLARE_LOGGING_CATEGORY(workerCat)

    class Worker : public QObject {
        Q_OBJECT

    private:
        class PrivateData;
        std::shared_ptr<PrivateData> d = nullptr;

    public:
        explicit Worker(QObject* parent = nullptr);

    signals:
        void startTimer();

    public slots:
        void scheduleForIntegration(const QString& path);
        void scheduleForUnintegration(const QString& path);

    public slots:
        void executeDeferredOperations();

    private slots:
        void startTimerIfNecessary();
    };

}

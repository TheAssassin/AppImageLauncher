// system includes
#include <memory>

// library includes
#include <QObject>
#include <QTimer>
#include <QSharedPointer>
#include <AppImageDesktopIntegrationManager.h>

#pragma once

class Worker : public QObject {
Q_OBJECT

    class PrivateData;

    static constexpr int TIMEOUT = 15 * 1000;
    bool isTimerActive{false};

    std::shared_ptr<PrivateData> d = nullptr;
    QSharedPointer<AppImageDesktopIntegrationManager> integrationManager;
public:
    Worker();

    void setIntegrationManager(QSharedPointer<AppImageDesktopIntegrationManager> integrationManager);

signals:
    void startTimer();

public slots:
    void scheduleForIntegration(const QString& path);

    void scheduleForUnintegration(const QString& path);

    void executeDeferredOperations();

private slots:
    void startTimerIfNecessary();

    void handleTimerTimeout();
};

// system includes
#include <memory>

// library includes
#include <QObject>

#pragma once

class Worker : public QObject {
    Q_OBJECT

private:
    class PrivateData;
    std::shared_ptr<PrivateData> d = nullptr;

public:
    Worker();

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

//
// Created by alexis on 8/30/18.
//

#ifndef APPIMAGELAUNCHER_LAUNCHER_H
#define APPIMAGELAUNCHER_LAUNCHER_H

#include <QString>
#include <vector>

class Launcher {
    QString appImagePath;
    std::vector<char *> args{};
    int appImageType{-1};

public:
    const QString &getAppImagePath() const;

    void setAppImagePath(const QString &appImagePath);

    const std::vector<char *> &getArgs() const;

    void setArgs(const std::vector<char *> &args);

    int getAppImageType() const;

    void inspectAppImageFile();

    bool shouldBeIgnored();

    bool isIntegrationRequired();

    void integrateAppImage();

    void executeAppImage();

private:
    void validateAppImageType();

    bool isAMountOrExtractOperation() const;
};

/* Exceptions that can be thrown from the Launcher methods. */

class AppImageFilePathNotSet : public std::runtime_error {
public:
    explicit AppImageFilePathNotSet(const std::string &what) : runtime_error(what) {}
};

class AppImageFileNotExists : public std::runtime_error {
public:
    explicit AppImageFileNotExists(const std::string &what) : runtime_error(what) {}
};

class InvalidAppImageFile : public std::runtime_error {
public:
    explicit InvalidAppImageFile(const std::string &what) : runtime_error(what) {}
};

class IntegrationFailed : public std::runtime_error {
public:
    explicit IntegrationFailed(const std::string &what) : runtime_error(what) {}
};

class UnsuportedAppImageType : public std::runtime_error {
public:
    explicit UnsuportedAppImageType(const std::string &what) : runtime_error(what) {}
};

class ExecutionFailed : public std::runtime_error {
public:
    explicit ExecutionFailed(const std::string &what) : runtime_error(what) {}
};

class DeploymentFailed : public std::runtime_error {
public:
    explicit DeploymentFailed(const std::string &what) : runtime_error(what) {}
};


#endif //APPIMAGELAUNCHER_LAUNCHER_H

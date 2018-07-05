// system includes
#include <QString>

#pragma once

class TrashBin {
    private:
        class PrivateData;
        PrivateData* d;

    public:
        TrashBin();

    public:
        QString path();

    public:
        // move AppImage into trash bin directory
        bool disposeAppImage(const QString& pathToAppImage);

        // check all AppImages in trash bin whether they can be removed
        // this function should be called regularly to make sure the files in the trash bin are cleaned up as soon
        // as possible
        bool cleanUp();
};

#include "types.h"

QDebug operator<<(QDebug debug, const QDirSet& set) {
    QDebugStateSaver saver(debug);
    for (const auto& item : set) {
        debug << item;
    }
    return debug;
}

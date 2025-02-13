#pragma once

// system headers
#include <set>

// library headers
#include <QDebug>
#include <QDir>

struct QDirComparator {
public:
    size_t operator()(const QDir& a, const QDir& b) const {
        return a.absolutePath() < b.absolutePath();
    }
};

typedef std::set<QDir, QDirComparator> QDirSet;


QDebug operator<<(QDebug debug, const QDirSet &set);

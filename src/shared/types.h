#pragma once

// system headers
#include <unordered_set>

// library headers
#include <QDir>

struct QDirHash {
public:
    size_t operator()(const QDir& dir) const {
        return qt_hash(dir.absolutePath());
    }
};

struct QDirComparator {
public:
    size_t operator()(const QDir& a, const QDir& b) const {
        return a.absolutePath() < b.absolutePath();
    }
};

typedef std::unordered_set<QDir, QDirHash, QDirComparator> QDirSet;

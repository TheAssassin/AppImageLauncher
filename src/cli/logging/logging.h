#pragma once

// system headers
#include <string.h>

// library headers
#include <QIODevice>
#include <QTextStream>
#include <QDebug>

// wrapper for stdout
#define qout() QTextStream(stdout, QIODevice::WriteOnly)

// wrapper for stderr
#define qerr() QTextStream(stderr, QIODevice::WriteOnly)

// Qt6 moved endl to Qt namespace
using Qt::endl;

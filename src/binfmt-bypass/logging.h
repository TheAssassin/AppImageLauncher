#pragma once

// system headers
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifndef COMPONENT_NAME
#define COMPONENT_NAME "unknown"
#endif

static int v_log_message(const char* const format, va_list args) {
    static const char prefix[] = "[appimage-binfmt-bypass/" COMPONENT_NAME "] ";

    char* patched_format = (char*) (malloc(strlen(format) + strlen(prefix) + 1));
    strcpy(patched_format, prefix);
    strcat(patched_format, format);

    return vfprintf(stderr, patched_format, args);
}

static int v_log_message_prefix(const char* const prefix, const char* const format, va_list args) {
    char* patched_format = (char*) (malloc(strlen(format) + strlen(prefix) + 2 + 1));
    strcpy(patched_format, prefix);
    strcat(patched_format, ": ");
    strcat(patched_format, format);

    return v_log_message(patched_format, args);
}

static int log_message(const char* const format, ...) {
    va_list args;
    va_start(args, format);

    int result = v_log_message(format, args);

    va_end(args);

    return result;
}

static void log_debug(const char* const format, ...) {
    if (getenv("DEBUG") == NULL) {
        return;
    }

    va_list args;
    va_start(args, format);

    v_log_message_prefix("DEBUG", format, args);

    va_end(args);
}

static void log_error(const char* const format, ...) {
    va_list args;
    va_start(args, format);

    v_log_message_prefix("ERROR", format, args);

    va_end(args);
}


static void log_warning(const char* const format, ...) {
    va_list args;
    va_start(args, format);

    v_log_message_prefix("WARNING", format, args);

    va_end(args);
}

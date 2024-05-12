#ifndef LOGGER_H // !LOGGER_H
#define LOGGER_H

#include <android/log.h>

#ifndef LOGTAG
#define LOGTAG "Logger"
#endif

#define LogInfo(formatter, ...) __android_log_print(ANDROID_LOG_INFO, LOGTAG, formatter __VA_OPT__(, ) __VA_ARGS__)
#define LogDebug(formatter, ...) __android_log_print(ANDROID_LOG_DEBUG, LOGTAG, formatter __VA_OPT__(, ) __VA_ARGS__)
#define LogError(formatter, ...) __android_log_print(ANDROID_LOG_ERROR, LOGTAG, formatter __VA_OPT__(, ) __VA_ARGS__)

#endif // !LOGGER_H
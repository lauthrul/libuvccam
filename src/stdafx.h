#pragma once

#define LOG_DEBUG(fmt, ...)     _tprintf(fmt, ## __VA_ARGS__)
#define LOG_INFO(fmt, ...)      _tprintf(fmt, ## __VA_ARGS__)
#define LOG_WARN(fmt, ...)      _tprintf(fmt, ## __VA_ARGS__)
#define LOG_ERROR(fmt, ...)     _tprintf(fmt, ## __VA_ARGS__)

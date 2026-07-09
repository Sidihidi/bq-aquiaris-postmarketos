/* shim bionic->musl: cutils/log.h -> stderr */
#ifndef _SHIM_CUTILS_LOG_H
#define _SHIM_CUTILS_LOG_H
#include <stdio.h>
#ifndef LOG_TAG
#define LOG_TAG "mnl"
#endif
#define __SHIMLOG(lvl, ...) do { fprintf(stderr, "[" LOG_TAG " " lvl "] " __VA_ARGS__); fputc('\n', stderr); } while (0)
#define ALOGV(...) __SHIMLOG("V", __VA_ARGS__)
#define ALOGD(...) __SHIMLOG("D", __VA_ARGS__)
#define ALOGI(...) __SHIMLOG("I", __VA_ARGS__)
#define ALOGW(...) __SHIMLOG("W", __VA_ARGS__)
#define ALOGE(...) __SHIMLOG("E", __VA_ARGS__)
#define LOGV ALOGV
#define LOGD ALOGD
#define LOGI ALOGI
#define LOGW ALOGW
#define LOGE ALOGE
#define XLOGV ALOGV
#define XLOGD ALOGD
#define XLOGI ALOGI
#define XLOGW ALOGW
#define XLOGE ALOGE
#define SLOGD ALOGD
#define SLOGE ALOGE
#define ALOG_ASSERT(cond, ...) do {} while (0)
#define LOG_PRI(pri, tag, ...) __SHIMLOG("P", __VA_ARGS__)
#define android_printLog(prio, tag, ...) __SHIMLOG("A", __VA_ARGS__)
#endif

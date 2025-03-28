/*
 * Copyright The async-profiler authors
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef _OS_H
#define _OS_H

#include <signal.h>
#include <stddef.h>
#include <sys/types.h>
#include "arch.h"


typedef void (*SigAction)(int, siginfo_t*, void*);
typedef void (*SigHandler)(int);
typedef void (*TimerCallback)(void*);

// Interrupt threads with this signal. The same signal is used inside JDK to interrupt I/O operations.
const int WAKEUP_SIGNAL = SIGIO;

enum ThreadState {
    THREAD_UNKNOWN,
    THREAD_RUNNING,
    THREAD_SLEEPING
};


class ThreadList {
  protected:
    u32 _index;
    u32 _count;

    ThreadList() : _index(0), _count(0) {
    }

  public:
    virtual ~ThreadList() {}

    u32 index() const { return _index; }
    u32 count() const { return _count; }

    bool hasNext() const {
        return _index < _count;
    }

    virtual int next() = 0;
    virtual void update() = 0;
};


// W^X memory support
class JitWriteProtection {
  private:
    u64 _prev;
    bool _restore;

  public:
    JitWriteProtection(bool enable);
    ~JitWriteProtection();
};


class OS {
  public:
    static const size_t page_size;
    static const size_t page_mask;

    static u64 nanotime();
    static u64 micros();
    static u64 processStartTime();
    static void sleep(u64 nanos);
    static u64 overrun(siginfo_t* siginfo);

    static u64 hton64(u64 x);
    static u64 ntoh64(u64 x);

    static int getMaxThreadId();
    static int processId();
    static int threadId();
    static const char* schedPolicy(int thread_id);
    static bool threadName(int thread_id, char* name_buf, size_t name_len);
    static ThreadState threadState(int thread_id);
    static u64 threadCpuTime(int thread_id);
    static ThreadList* listThreads();

    static bool isLinux();
    static bool isMusl();

    static SigAction installSignalHandler(int signo, SigAction action, SigHandler handler = NULL);
    static SigAction replaceCrashHandler(SigAction action);
    static int getProfilingSignal(int mode);
    static bool sendSignalToThread(int thread_id, int signo);

    static void* safeAlloc(size_t size);
    static void safeFree(void* addr, size_t size);

    static bool getCpuDescription(char* buf, size_t size);
    static int getCpuCount();
    static u64 getProcessCpuTime(u64* utime, u64* stime);
    static u64 getTotalCpuTime(u64* utime, u64* stime);

    static int createMemoryFile(const char* name);
    static void copyFile(int src_fd, int dst_fd, off_t offset, size_t size);
    static void freePageCache(int fd, off_t start_offset);
};

#define INT64_FORMAT             "%"          PRId64
#define PTR_FORMAT               "0x%016"     PRIxPTR
#define DDD_FMT_I INT64_FORMAT
#define DDD_FMT_P PTR_FORMAT
#define DDD_FMT_S "%s"
#define DDD_CST_I(v) uint64_t(v)
#define DDD_CST_P(v) ((intptr_t) (v))
#define DDD_CST_S(v) (v)

#define DDD_T_1(f1, v1)                                 \
                             printf("DDD %s:%d "                                   \
                             #v1 ":" DDD_FMT_##f1      \
                             "\n",                  \
                             __func__,                  \
                             __LINE__,                  \
                             DDD_CST_##f1(v1))
#define DDD_T_2(f1, v1, f2, v2)                         \
                             printf("DDD %s:%d "               \
                             #v1 ":" DDD_FMT_##f1 " "   \
                             #v2 ":" DDD_FMT_##f2      \
                             "\n",                  \
                             __func__,                  \
                             __LINE__,                  \
                             DDD_CST_##f1(v1),          \
                             DDD_CST_##f2(v2))

#define DDD_T_3(f1, v1, f2, v2, f3, v3)                 \
                             printf("DDD %s:%d "               \
                             #v1 ":" DDD_FMT_##f1 " "   \
                             #v2 ":" DDD_FMT_##f2 " "   \
                             #v3 ":" DDD_FMT_##f3      \
                             "\n",                  \
                             __func__,                  \
                             __LINE__,                  \
                             DDD_CST_##f1(v1),          \
                             DDD_CST_##f2(v2),          \
                             DDD_CST_##f3(v3))

#define DDD_T_4(f1, v1, f2, v2, f3, v3, f4, v4)         \
                             printf("DDD %s:%d "               \
                             #v1 ":" DDD_FMT_##f1 " "   \
                             #v2 ":" DDD_FMT_##f2 " "   \
                             #v3 ":" DDD_FMT_##f3 " "   \
                             #v4 ":" DDD_FMT_##f4      \
                             "\n",                  \
                             __func__,                  \
                             __LINE__,                  \
                             DDD_CST_##f1(v1),          \
                             DDD_CST_##f2(v2),          \
                             DDD_CST_##f3(v3),          \
                             DDD_CST_##f4(v4))

#include <inttypes.h>

#define DDD_T_NUM_ARGS(...) DDD_T_NUM_ARGS_IMPL(__VA_ARGS__, 5, 4, 3, 2, 1, 0)
#define DDD_T_NUM_ARGS_IMPL(_1, _2, _3, _4, _5, N, ...) N

#define DDD_T_NUM_ARG_PAIRS(...) DDD_T_NUM_ARG_PAIRS_IMPL(__VA_ARGS__, 6, 6, 5, 5, 4, 4, 3, 3, 2, 2, 1, 1, 0, 0)
#define DDD_T_NUM_ARG_PAIRS_IMPL(_1, _1b, _2, _2b, _3, _3b, _4, _4b, _5, _5b, _6, _6b, N, ...) N

#define DDD_T_PASTE(a, b) a##b
#define DDD_T_MACRO_CONCAT(a, b) DDD_T_PASTE(a, b)
#define DDD_T(...) DDD_T_MACRO_CONCAT(DDD_T_, DDD_T_NUM_ARG_PAIRS(__VA_ARGS__))(__VA_ARGS__)

#define DDD_D_1(f1, v1)                                 \
                             printf("DDD %s:%d "               \
                             #v1 ":" DDD_FMT_##f1      \
                             "\n",                  \
                             __func__,                  \
                             __LINE__,                  \
                             DDD_CST_##f1(v1))
#define DDD_D_2(f1, v1, f2, v2)                         \
                             printf("DDD %s:%d "               \
                             #v1 ":" DDD_FMT_##f1 " "   \
                             #v2 ":" DDD_FMT_##f2      \
                             "\n",                  \
                             __func__,                  \
                             __LINE__,                  \
                             DDD_CST_##f1(v1),          \
                             DDD_CST_##f2(v2))

#define DDD_D_3(f1, v1, f2, v2, f3, v3)                 \
                             printf("DDD %s:%d "               \
                             #v1 ":" DDD_FMT_##f1 " "   \
                             #v2 ":" DDD_FMT_##f2 " "   \
                             #v3 ":" DDD_FMT_##f3      \
                             "\n",                  \
                             __func__,                  \
                             __LINE__,                  \
                             DDD_CST_##f1(v1),          \
                             DDD_CST_##f2(v2),          \
                             DDD_CST_##f3(v3))

#define DDD_D_4(f1, v1, f2, v2, f3, v3, f4, v4)         \
                             printf("DDD %s:%d "               \
                             #v1 ":" DDD_FMT_##f1 " "   \
                             #v2 ":" DDD_FMT_##f2 " "   \
                             #v3 ":" DDD_FMT_##f3 " "   \
                             #v4 ":" DDD_FMT_##f4      \
                             "\n",                  \
                             __func__,                  \
                             __LINE__,                  \
                             DDD_CST_##f1(v1),          \
                             DDD_CST_##f2(v2),          \
                             DDD_CST_##f3(v3),          \
                             DDD_CST_##f4(v4))

#define DDD_D_NUM_ARGS(...) DDD_D_NUM_ARGS_IMPL(__VA_ARGS__, 5, 4, 3, 2, 1, 0)
#define DDD_D_NUM_ARGS_IMPL(_1, _2, _3, _4, _5, N, ...) N

#define DDD_D_NUM_ARG_PAIRS(...) DDD_D_NUM_ARG_PAIRS_IMPL(__VA_ARGS__, 6, 6, 5, 5, 4, 4, 3, 3, 2, 2, 1, 1, 0, 0)
#define DDD_D_NUM_ARG_PAIRS_IMPL(_1, _1b, _2, _2b, _3, _3b, _4, _4b, _5, _5b, _6, _6b, N, ...) N

#define DDD_D_PASTE(a, b) a##b
#define DDD_D_MACRO_CONCAT(a, b) DDD_D_PASTE(a, b)
#define DDD_D(...) DDD_D_MACRO_CONCAT(DDD_D_, DDD_D_NUM_ARG_PAIRS(__VA_ARGS__))(__VA_ARGS__)

void DDD_print_stack();

#endif // _OS_H

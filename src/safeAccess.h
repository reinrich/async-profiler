/*
 * Copyright The async-profiler authors
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef _SAFEACCESS_H
#define _SAFEACCESS_H

#include <stdint.h>
#include "arch.h"


#ifdef __clang__
#  define NOINLINE __attribute__((noinline))
#else
#  define NOINLINE __attribute__((noinline,noclone))
#endif


class SafeAccess {
  public:
    NOINLINE __attribute__((aligned(16)))
    static void* load(void** ptr) {
        return *ptr;
    }

    NOINLINE __attribute__((aligned(16)))
    static u32 load32(u32* ptr, u32 default_value) {
        return *ptr;
    }

    NOINLINE __attribute__((aligned(16)))
    static void* loadPtr(void** ptr, void* default_value) {
        return *ptr;
    }

    static uintptr_t skipLoad(uintptr_t pc) {
        if ((pc - (uintptr_t)load) < 16) {
#if defined(__x86_64__)
            return *(u16*)pc == 0x8b48 ? 3 : 0;  // mov rax, [reg]
#elif defined(__i386__)
            return *(u8*)pc == 0x8b ? 2 : 0;     // mov eax, [reg]
#elif defined(__arm__) || defined(__thumb__)
            return (*(instruction_t*)pc & 0x0e50f000) == 0x04100000 ? 4 : 0;  // ldr r0, [reg]
#elif defined(__aarch64__)
            return (*(instruction_t*)pc & 0xffc0001f) == 0xf9400000 ? 4 : 0;  // ldr x0, [reg]
#else
            return sizeof(instruction_t);
#endif
        }
        return 0;
    }

    static uintptr_t skipLoadArg(uintptr_t pc) {
#if defined(__aarch64__)
        if ((pc - (uintptr_t)load32) < 16 || (pc - (uintptr_t)loadPtr) < 16) {
            return 4;
        }
#endif
        return 0;
    }

    static const void* lastJavaPCFromStack(uintptr_t lastJavaSP, uintptr_t topSP) {
      const void* lastJavaPC = NULL;
#if defined(__aarch64__)
        // If not stored in the frame anchor, the last java pc is always stored in the stack before setting the last java sp.
        lastJavaPC = ((const void**)lastJavaSP)[-1];
#elif defined(__PPC64__) && (__BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__)
        // Rely on the vm to find th last java pc. Failure means the attempt would be unsafe.
#else
        if (lastJavaSP < topSP) {
            // The callee of the last java frame is already on stack. This means
            // the call instruction stored the last java pc on stack.
            lastJavaPC = ((const void**)lastJavaSP)[-1];
        }
#endif
        return lastJavaPC;
    }
};

#endif // _SAFEACCESS_H

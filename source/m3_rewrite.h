#ifndef m3_validate_h
#define m3_validate_h

#include "wasm3.h"
#include "m3_core.h"


// offsets encoded as pointers have their top bit set

#if __SIZEOF_POINTER__ != 8
    #error "todo portable macros"
#endif

#define TO_OFF(TYPE, PTR, BASE)                                                 \
    ( (!(PTR) || ((uintptr_t)(PTR) & (1ULL << 63)))                             \
        ? (PTR)                                                                 \
        : (TYPE)(((u8*)(PTR) - (u8*)(BASE)) + (u8*)((uintptr_t)1ULL << 63)) )

#define TO_PTR(TYPE, PTR, BASE)                                                 \
    ( (!(PTR) || !((uintptr_t)(PTR) & (1ULL << 63)))                            \
        ? (PTR)                                                                 \
        : (TYPE)(((u8*)(PTR) - (u8*)((uintptr_t)1ULL << 63)) + (u8*)(BASE)) )

#define REWRITE(TYPE, PTR, BASE, IS_STORE)  \
    do {                                    \
        if (IS_STORE)                       \
        {                                   \
            PTR = TO_OFF(TYPE, PTR, BASE);  \
        }                                   \
        else                                \
        {                                   \
            PTR = TO_PTR(TYPE, PTR, BASE);  \
        }                                   \
    } while (0)

#define REWRITE_F(TYPE, PTR, BASE, IS_STORE, FUNC)  \
    do {                                            \
        if (IS_STORE)                               \
        {                                           \
            TYPE off = TO_OFF(TYPE, PTR, BASE);     \
            if (off != (PTR))                       \
            {                                       \
                TYPE ptr_tmp = PTR;                 \
                PTR = off;                          \
                FUNC(ptr_tmp, BASE);                \
            }                                       \
        }                                           \
        else                                        \
        {                                           \
            TYPE ptr = TO_PTR(TYPE, PTR, BASE);     \
            if (ptr != (PTR))                       \
            {                                       \
                PTR = ptr;                          \
                FUNC(ptr, BASE);                    \
            }                                       \
        }                                           \
    } while (0)

#define NULLIFY_STORE(PTR, IS_STORE)                \
    do {                                            \
        if (IS_STORE)                               \
        {                                           \
            (PTR) = NULL;                           \
        }                                           \
    } while (0)

void m3_RewritePointersRuntime(IM3Runtime runtime, u8* base, bool is_subtract);

#endif // m3_validate_h

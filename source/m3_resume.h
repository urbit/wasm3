#ifndef m3_resume_h
#define m3_resume_h

#include "wasm3.h"
#include "m3_core.h"
#include "m3_exec.h"
#include "m3_rewrite.h"

#define r_pop_suspend(TYPE)                             \
    ( *(TYPE*)(                                         \
            runtime->stack_suspend                      \
            + (runtime->edge_suspend -= slots_of(TYPE)) \
            ) )
            
#define r_peek_suspend(TYPE)                                                    \
  (*(TYPE*)(runtime->stack_suspend + (runtime->edge_suspend - slots_of(TYPE))))  

static inline void*
r_pop_suspend_ptr(IM3Runtime runtime, u8* base)
{
    void* offset = r_pop_suspend(void*);
    return TO_PTR(void*, offset, base);
}

#define r_push_suspend(TYPE, VAL)                                 \
    do {                                                          \
        if (runtime->stack_suspend)                               \
        {                                                         \
            u32 edge = runtime->edge_suspend;                     \
            if (edge + slots_of(TYPE) > runtime->size_suspend)    \
            {                                                     \
                return m3Err_trapStackOverflow;                   \
            }                                                     \
            *(TYPE*)(runtime->stack_suspend + edge) = (VAL);      \
            runtime->edge_suspend += slots_of(TYPE);              \
        }                                                         \
    } while (0)

#define r_push_suspend_ptr(VAL, BASE)                             \
    r_push_suspend(void*, TO_OFF(void*, (void*)VAL, BASE))

M3Result m3_Resume(IM3Runtime runtime);

#endif // m3_resume_h
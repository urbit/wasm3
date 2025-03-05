#ifndef m3_resume_h
#define m3_resume_h

#include "wasm3.h"
#include "m3_core.h"
#include "m3_exec.h"
#include "m3_exception.h"
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

#endif // m3_resume_h
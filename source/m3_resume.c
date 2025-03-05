#include "m3_env.h"

#include "m3_resume.h"

M3Result
m3_Resume(IM3Runtime runtime)
{
    if (!runtime->stack_suspend)
    {
        return m3Err_none;
    }
    
    M3Result result = m3Err_none;
    
    while (runtime->edge_suspend)
    {
        SuspendTag t = r_peek_suspend(SuspendTag);
        switch (t)
        {
            default:
            {
                return m3Err_SuspensionError;
            }
            case m3_st_External:
            {
                result = runtime->resume_external(result, runtime);
                break;
            }
            case m3_st_Call:
            {
                // case of non-blocking unsuccessful result:
                // just pop the frame and go on, "returning" the result
                // to the next frame
                //
                if (result
                    && result != m3Err_ComputationBlock
                    && result != m3Err_SuspensionError)
                {
                    u8* base = runtime->base;
                    u8* base_pc = runtime->base_transient;

                    r_pop_suspend(SuspendTag);
                    r_pop_suspend(f64);
                    r_pop_suspend(m3reg_t);
                    r_pop_suspend_ptr(runtime, base);
                    r_pop_suspend_ptr(runtime, base_pc);
                    r_pop_suspend_ptr(runtime, base);
                }
                // either new block or suspension error:
                // leave everything as is, return
                //
                else if (result)
                {
                    return result;
                }
                // pop the frame, treat the call, "return" the result
                // to the next frame
                //
                else
                {
                    u8* base = runtime->base;
                    u8* base_pc = runtime->base_transient;
                    
                    r_pop_suspend(SuspendTag);
                    f64 _fp0 = r_pop_suspend(f64);
                    m3reg_t _r0 = r_pop_suspend(m3reg_t);
                    m3stack_t _sp = r_pop_suspend_ptr(runtime, base);
                    pc_t _pc = r_pop_suspend_ptr(runtime, base_pc);
                    IM3Memory memory = r_pop_suspend_ptr(runtime, base);

                    M3MemoryHeader* _mem = memory->mallocated;
                    result = nextOpImpl();
                }
                break;
            }
            case m3_st_Call_indirect:
            {
                if (result
                    && result != m3Err_ComputationBlock
                    && result != m3Err_SuspensionError)
                {
                    u8* base = runtime->base;
                    u8* base_pc = runtime->base_transient;

                    r_pop_suspend(SuspendTag);
                    r_pop_suspend(f64);
                    r_pop_suspend(m3reg_t);
                    r_pop_suspend_ptr(runtime, base);
                    r_pop_suspend_ptr(runtime, base_pc);
                    r_pop_suspend_ptr(runtime, base);
                }
                else if (result)
                {
                    return result;
                }
                else
                {
                    u8* base = runtime->base;
                    u8* base_pc = runtime->base_transient;

                    r_pop_suspend(SuspendTag);
                    f64 _fp0 = r_pop_suspend(f64);
                    m3reg_t _r0 = r_pop_suspend(m3reg_t);
                    m3stack_t _sp = r_pop_suspend_ptr(runtime, base);
                    pc_t _pc = r_pop_suspend_ptr(runtime, base_pc);
                    IM3Memory memory = r_pop_suspend_ptr(runtime, base);
                    
                    M3MemoryHeader* _mem = memory->mallocated;
                    result = nextOpImpl();
                }
                break;
            }
            case m3_st_CallRaw:
            {
                if (result != m3Err_ComputationBlock
                    && result != m3Err_SuspensionError)
                {
                    // we always restore backed up stack if we are not blocked
                    //
                    r_pop_suspend(SuspendTag);
                    void* stack_backup = r_pop_suspend_ptr(
                        runtime,
                        runtime->base
                    );
                    runtime->stack = stack_backup;
                }
                else
                {
                    return result;
                }
                break;
            }
            case m3_st_Entry:
            {
                // trivial frame, nothing to handle
                //
                if (result != m3Err_ComputationBlock
                    && result != m3Err_SuspensionError)
                {
                    r_pop_suspend(SuspendTag);
                }
                else
                {
                    return result;
                }
                break;
            }
            case m3_st_Loop:
            {
                if (result == m3Err_ComputationBlock
                    || result == m3Err_SuspensionError)
                {
                    return result;
                }
                else
                {
                    // non empty result in case of op_Loop
                    // may also be loop's label
                    //
                    u8* base = runtime->base;
                    u8* base_pc = runtime->base_transient;

                    r_pop_suspend(SuspendTag);
                    IM3Memory memory = r_pop_suspend_ptr(runtime, base);
                    m3stack_t _sp = r_pop_suspend_ptr(runtime, base);
                    pc_t _pc = r_pop_suspend_ptr(runtime, base_pc);

                    M3MemoryHeader* _mem = memory->mallocated;
                    m3ret_t r = result;

                    // while instead of do..while because we might
                    // already be done with the loop
                    //
                    while (r == _pc)
                    {
                        r = ((IM3Operation)(*_pc))(_pc + 1, _sp, _mem, 0, 0);
                        _mem = memory->mallocated;
                    }

                    result = r;
                }
                break;
            }
            case m3_st_m3_Call:
            {
                if (result
                    && result != m3Err_ComputationBlock
                    && result != m3Err_SuspensionError)
                {
                    u8* base = runtime->base;
                    u8* base_pc = runtime->base_transient;

                    r_pop_suspend(SuspendTag);
                    r_pop_suspend_ptr(runtime, base);

                    runtime->lastCalled = NULL;
                }
                else if (result)
                {
                    return result;
                }
                else
                {
                    u8* base = runtime->base;
                    u8* base_pc = runtime->base_transient;

                    r_pop_suspend(SuspendTag);
                    IM3Function i_function = r_pop_suspend_ptr(runtime, base);
                    runtime->lastCalled = i_function;
                }
                break;
            }
        }
    }

    return result;
}
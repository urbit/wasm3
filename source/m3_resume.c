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
    
    while (runtime->edge_suspend
        && result != m3Err_ComputationBlock
        && result != m3Err_SuspensionError
    )
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
                if (result)
                {
                    u8* base = runtime->base;
                    u8* base_pc = runtime->base_transient;

                    r_pop_suspend(SuspendTag);
                    r_pop_suspend(f64);
                    r_pop_suspend(m3reg_t);
                    r_pop_suspend_ptr(runtime, base);
                    r_pop_suspend_ptr(runtime, base_pc);
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

                    M3MemoryHeader* _mem = runtime->memory.mallocated;
                    result = nextOpImpl();
                }
                break;
            }
            case m3_st_Call_indirect:
            {
                if (result)
                {
                    u8* base = runtime->base;
                    u8* base_pc = runtime->base_transient;

                    r_pop_suspend(SuspendTag);
                    r_pop_suspend(f64);
                    r_pop_suspend(m3reg_t);
                    r_pop_suspend_ptr(runtime, base);
                    r_pop_suspend_ptr(runtime, base_pc);
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
                    
                    M3MemoryHeader* _mem = runtime->memory.mallocated;
                    result = nextOpImpl();
                }
                break;
            }
            case m3_st_CallRaw:
            {
                // we always restore backed up stack if we are not blocked
                //
                r_pop_suspend(SuspendTag);
                void* stack_backup = r_pop_suspend_ptr(
                    runtime,
                    runtime->base
                );
                runtime->stack = stack_backup;
                break;
            }
            case m3_st_Loop:
            {
                // non empty result in case of op_Loop
                // may also be loop's label
                //
                u8* base = runtime->base;
                u8* base_pc = runtime->base_transient;

                r_pop_suspend(SuspendTag);
                m3stack_t _sp = r_pop_suspend_ptr(runtime, base);
                pc_t _pc = r_pop_suspend_ptr(runtime, base_pc);

                M3MemoryHeader* _mem = runtime->memory.mallocated;
                m3ret_t r = result;
                
                //  we might get blocked again in the loop, so restore the
                //  frame if reenter
                //
                if (r == _pc)
                {
                    r_push_suspend_ptr(_pc, base_pc);
                    r_push_suspend_ptr(_sp, base);
                    r_push_suspend(SuspendTag, m3_st_Loop);
                    
                    do
                    {
                        r = ((IM3Operation)(*_pc))(_pc + 1, _sp, _mem, 0, 0);
                        M3MemoryHeader* _mem = runtime->memory.mallocated;
                    }
                    while (r == _pc);
                    

                    //  optz: we might know that we are done with the loop,
                    //  pop the frame immediately if so
                    //
                    result = r;
                    if (result != m3Err_ComputationBlock 
                        && result != m3Err_SuspensionError)
                    {
                        r_pop_suspend(SuspendTag);
                        r_pop_suspend_ptr(runtime, base);
                        r_pop_suspend_ptr(runtime, base_pc);
                    }
                }

                result = r;
                break;
            }
            case m3_st_m3_Call:
            {
                if (result)
                {
                    u8* base = runtime->base;
                    u8* base_pc = runtime->base_transient;

                    r_pop_suspend(SuspendTag);
                    r_pop_suspend_ptr(runtime, base);

                    runtime->lastCalled = NULL;
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
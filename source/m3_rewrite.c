#include "m3_env.h"

#include "m3_rewrite.h"

// base pointer must be 16 byte alligned

#if (d_m3EnableCodePageRefCounting)
#   define nullify_codepagerefs NULLIFY_STORE(f->codePageRefs, IS_STORE)
#else
#   define nullify_codepagerefs do {} while (0)
#endif

static void
RewritePointersFunction(IM3Function f, u8* base, bool is_store)
{
    REWRITE(IM3Module, f->module, base, is_store);
    REWRITE(char*, f->import.moduleUtf8, base, is_store);
    REWRITE(char*, f->import.fieldUtf8, base, is_store);
    REWRITE(bytes_t, f->wasm, base, is_store);
    REWRITE(bytes_t, f->wasmEnd, base, is_store);

    if (!is_store)
    {
        REWRITE(cstr_t*, f->names, base, is_store);
    }
    for (u32 i = 0; i < f->numNames; i++)
    {
        REWRITE(cstr_t, f->names[i], base, is_store);
    }
    if (is_store)
    {
        REWRITE(cstr_t*, f->names, base, is_store);
    }

    REWRITE(IM3FuncType, f->funcType, base, is_store);
    NULLIFY_STORE(f->compiled, is_store);
    nullify_codepagerefs;
    REWRITE(void*, f->constants, base, is_store);
}

static void
RewritePointersDataSegment(M3DataSegment* d, u8* base, bool is_store)
{
    REWRITE(u8*, d->initExpr, base, is_store);
    REWRITE(u8*, d->data, base, is_store);
}


static void
RewritePointersGlobal(IM3Global g, u8* base, bool is_store)
{
    REWRITE(char*, g->import.fieldUtf8, base, is_store);
    REWRITE(char*, g->import.moduleUtf8, base, is_store);
    REWRITE(cstr_t, g->name, base, is_store);
    REWRITE(bytes_t, g->initExpr, base, is_store);
}

static void
RewritePointersFuncType(IM3FuncType t, u8* base, bool is_store)
{
    REWRITE_F(IM3FuncType,
        t->next,
        base,
        is_store,
        RewritePointersFuncType);
}

static void
RewritePointersMemoryHeader(M3MemoryHeader* h, u8* base, bool is_store)
{
    REWRITE(IM3Runtime, h->runtime, base, is_store);
    REWRITE(void*, h->maxStack, base, is_store);
}

static void
RewritePointersMemory(M3Memory* m, u8* base, bool is_store)
{
    // REWRITE_F(M3MemoryHeader *, m->mallocated, base, is_store,
    //     RewritePointersMemoryHeader);
    do {
        if (is_store)
        {
            M3MemoryHeader * off = TO_OFF(M3MemoryHeader *, m->mallocated, base);
            if (off != (m->mallocated))
            {
                M3MemoryHeader * ptr_tmp = m->mallocated;
                m->mallocated = off;
                RewritePointersMemoryHeader(ptr_tmp, base, is_store);
            }
        }
        else
        {
            M3MemoryHeader * ptr = TO_PTR(M3MemoryHeader *, m->mallocated, base);
            if (ptr != (m->mallocated))
            {
                m->mallocated = ptr;
                RewritePointersMemoryHeader(ptr, base, is_store);
            }
        }
    } while (0);
}


static void
RewritePointersModule(IM3Module m, u8* base, bool is_store)
{
    /* do not enter runtime, environment, they called us */
    REWRITE(IM3Runtime, m->runtime, base, is_store);
    REWRITE(IM3Environment, m->environment, base, is_store);
    REWRITE(bytes_t, m->wasmStart, base, is_store);
    REWRITE(bytes_t, m->wasmEnd, base, is_store);
    REWRITE(cstr_t, m->name, base, is_store);

    if (!is_store)
    {
        REWRITE(IM3FuncType*, m->funcTypes, base, is_store);
    }
    for (u32 i = 0; i < m->numFuncTypes; i++)
    {
        /* do not enter funcTypes, they are handled in environment */
        REWRITE(IM3FuncType, m->funcTypes[i], base, is_store);
    }
    if (is_store)
    {
        REWRITE(IM3FuncType*, m->funcTypes, base, is_store);
    }

    if (!is_store)
    {
        REWRITE(M3Function *, m->functions, base, is_store);
    }
    for (u32 i = 0; i < m->numFunctions; i++)
    {
        RewritePointersFunction(&m->functions[i],base, is_store);
    }
    if (is_store)
    {
        REWRITE(M3Function *, m->functions, base, is_store);
    }
    
    if (!is_store)
    {
        REWRITE(M3DataSegment *, m->dataSegments, base, is_store);
    }
    for (u32 i = 0; i < m->numDataSegments; i++)
    {
        RewritePointersDataSegment(
            &m->dataSegments[i],
            base,
            is_store
        );
    }
    if (is_store)
    {
        REWRITE(M3DataSegment *, m->dataSegments, base, is_store);
    }

    if (!is_store)
    {
        REWRITE(M3Global *, m->globals, base, is_store);
    }
    for (u32 i = 0; i < m->numGlobals; i++)
    {
        RewritePointersGlobal(&m->globals[i], base, is_store);
    }
    if (is_store)
    {
        REWRITE(M3Global *, m->globals, base, is_store);
    }

    REWRITE(bytes_t, m->elementSection, base, is_store);
    REWRITE(bytes_t, m->elementSectionEnd, base, is_store);

    if (!is_store)
    {
        REWRITE(IM3Function*, m->table0, base, is_store);
    }
    for (u32 i = 0; i < m->table0Size; i++)
    {
        REWRITE(IM3Function, m->table0[i], base, is_store);
    }
    if (is_store)
    {
        REWRITE(IM3Function*, m->table0, base, is_store);
    }

    REWRITE_F(IM3Module,
        m->next,
        base,
        is_store,
        RewritePointersModule);
}

static void
RewritePointersEnvironment(IM3Environment e, u8* base, bool is_store)
{
    REWRITE_F(IM3FuncType, e->funcTypes, base, is_store,
        RewritePointersFuncType);
    for (u32 i = 0; i < c_m3Type_unknown; i++)
    {
        /* retFuncTypes "point" to funcTypes, do not enter */
        REWRITE(IM3FuncType, e->retFuncTypes[i], base, is_store);
    }
        NULLIFY_STORE(e->pagesReleased, is_store);
}

void
m3_RewritePointersRuntime(IM3Runtime r, u8* base, bool is_store)
{
    if (is_store)
    {
        /* reset compilation, code pages must be freed           */
        /* this is because code pages contain function pointers, */
        /* and preserving them does not seem feasible            */
        /**/
        memset(&r->compilation, 0, sizeof(r->compilation));
        /* ErrorInfo contains pointers to static storage */
        /**/
        memset(&r->error, 0, sizeof(r->error));
    }
    // REWRITE_F(IM3Environment, r->environment, base, is_store,
    //     RewritePointersEnvironment);
    do {
        if (is_store)
        {
            IM3Environment off = TO_OFF(IM3Environment, r->environment, base);
            if (off != (r->environment))
            {
                IM3Environment ptr_tmp = r->environment;
                r->environment = off;
                RewritePointersEnvironment(ptr_tmp, base, is_store);
            }
        }
        else
        {
            IM3Environment ptr = NULL;
            ptr = TO_PTR(IM3Environment, r->environment, base);
            if (ptr != (r->environment))
            {
                r->environment = ptr;
                RewritePointersEnvironment(ptr, base, is_store);
            }
        }
    } while (0);
    /* Code pages are allocated transiently */
    NULLIFY_STORE(r->pagesOpen, is_store);
    NULLIFY_STORE(r->pagesFull, is_store);
    REWRITE_F(IM3Module, r->modules, base, is_store,
        RewritePointersModule);
    REWRITE(void*, r->stack, base, is_store);
    REWRITE(m3slot_t*, r->stack_suspend, base, is_store);
    NULLIFY_STORE(r->base, is_store);
    NULLIFY_STORE(r->base_transient, is_store);
    /* don't enter runtime->lastCalled, we will get there eventually */
    REWRITE(IM3Function, r->lastCalled, base, is_store);
    /* user is responsible for his data */
    NULLIFY_STORE(r->userdata, is_store);
    NULLIFY_STORE(r->userdata_import, is_store);
    RewritePointersMemory(&r->memory, base, is_store);
}
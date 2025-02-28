#include "m3_env.h"

#include "m3_rewrite.h"

// base pointer must be 16 byte alligned

#if (d_m3EnableCodePageRefCounting)
#   define nullify_codepagerefs NULLIFY_STORE(f->codePageRefs, IS_STORE)
#else
#   define nullify_codepagerefs do {} while (0)
#endif

#define RewritePointersFunction(STORE_OR_LOAD, IS_STORE)                        \
    static void                                                                 \
    RewritePointersFunction##STORE_OR_LOAD(IM3Function f, u8* base)             \
    {                                                                           \
        REWRITE(IM3Module, f->module, base, IS_STORE);                          \
        REWRITE(char*, f->import.moduleUtf8, base, IS_STORE);                   \
        REWRITE(char*, f->import.fieldUtf8, base, IS_STORE);                    \
        REWRITE(bytes_t, f->wasm, base, IS_STORE);                              \
        REWRITE(bytes_t, f->wasmEnd, base, IS_STORE);                           \
        for (u32 i = 0; i < f->numNames; i++)                                   \
        {                                                                       \
            REWRITE(cstr_t, f->names[i], base, IS_STORE);                       \
        }                                                                       \
        REWRITE(cstr_t*, f->names, base, IS_STORE);                             \
        REWRITE(IM3FuncType, f->funcType, base, IS_STORE);                      \
        NULLIFY_STORE(f->compiled, IS_STORE);                                   \
        nullify_codepagerefs;                                                   \
        REWRITE(void*, f->constants, base, IS_STORE);                           \
    }
RewritePointersFunction(Store, 1)
RewritePointersFunction(Load, 0)


#define RewritePointersDataSegment(STORE_OR_LOAD, IS_STORE)                     \
    static void                                                                 \
    RewritePointersDataSegment##STORE_OR_LOAD(M3DataSegment* d, u8* base)       \
    {                                                                           \
        REWRITE(u8*, d->initExpr, base, IS_STORE);                              \
        REWRITE(u8*, d->data, base, IS_STORE);                                  \
    }
RewritePointersDataSegment(Store, 1)
RewritePointersDataSegment(Load, 0)


#define RewritePointersGlobal(STORE_OR_LOAD, IS_STORE)                          \
    static void                                                                 \
    RewritePointersGlobal##STORE_OR_LOAD(IM3Global g, u8* base)                 \
    {                                                                           \
        REWRITE(char*, g->import.fieldUtf8, base, IS_STORE);                    \
        REWRITE(char*, g->import.moduleUtf8, base, IS_STORE);                   \
        REWRITE(cstr_t, g->name, base, IS_STORE);                               \
        REWRITE(bytes_t, g->initExpr, base, IS_STORE);                          \
    }
RewritePointersGlobal(Store, 1)
RewritePointersGlobal(Load, 0)



#define RewritePointersFuncType(STORE_OR_LOAD, IS_STORE)                        \
    static void                                                                 \
    RewritePointersFuncType##STORE_OR_LOAD(IM3FuncType t, u8* base)             \
    {                                                                           \
        REWRITE_F(IM3FuncType,                                                  \
            t->next,                                                            \
            base,                                                               \
            IS_STORE,                                                           \
            RewritePointersFuncType##STORE_OR_LOAD);                            \
    }
RewritePointersFuncType(Store, 1)
RewritePointersFuncType(Load, 0)


#define RewritePointersMemoryHeader(STORE_OR_LOAD, IS_STORE)                    \
    static void                                                                 \
    RewritePointersMemoryHeader##STORE_OR_LOAD(M3MemoryHeader* h, u8* base)     \
    {                                                                           \
        REWRITE(IM3Runtime, h->runtime, base, IS_STORE);                        \
        REWRITE(void*, h->maxStack, base, IS_STORE);                            \
    }
RewritePointersMemoryHeader(Store, 1)
RewritePointersMemoryHeader(Load, 0)


#define RewritePointersMemory(STORE_OR_LOAD, IS_STORE)                          \
    static void                                                                 \
    RewritePointersMemory##STORE_OR_LOAD(M3Memory* m, u8* base)                 \
    {                                                                           \
        REWRITE_F(M3MemoryHeader *, m->mallocated, base, IS_STORE,              \
            RewritePointersMemoryHeader##STORE_OR_LOAD);                        \
    }
RewritePointersMemory(Store, 1)
RewritePointersMemory(Load, 0)


#define RewritePointersModule(STORE_OR_LOAD, IS_STORE)                          \
    static void                                                                 \
    RewritePointersModule##STORE_OR_LOAD(IM3Module m, u8* base)                 \
    {                                                                           \
        /* do not enter runtime, environment, they called us */                 \
        REWRITE(IM3Runtime, m->runtime, base, IS_STORE);                        \
        REWRITE(IM3Environment, m->environment, base, IS_STORE);                \
        REWRITE(bytes_t, m->wasmStart, base, IS_STORE);                         \
        REWRITE(bytes_t, m->wasmEnd, base, IS_STORE);                           \
        REWRITE(cstr_t, m->name, base, IS_STORE);                               \
        for (u32 i = 0; i < m->numFuncTypes; i++)                               \
        {                                                                       \
            /* do not enter funcTypes, they are handled in environment */       \
            REWRITE(IM3FuncType, m->funcTypes[i], base, IS_STORE);              \
        }                                                                       \
        REWRITE(IM3FuncType*, m->funcTypes, base, IS_STORE);                    \
        for (u32 i = 0; i < m->numFunctions; i++)                               \
        {                                                                       \
            RewritePointersFunction##STORE_OR_LOAD(&m->functions[i],base);      \
        }                                                                       \
        REWRITE(M3Function *, m->functions, base, IS_STORE);                    \
        for (u32 i = 0; i < m->numDataSegments; i++)                            \
        {                                                                       \
            RewritePointersDataSegment##STORE_OR_LOAD(                          \
                &m->dataSegments[i],                                            \
                base                                                            \
            );                                                                  \
        }                                                                       \
        REWRITE(M3DataSegment *, m->dataSegments, base, IS_STORE);              \
        for (u32 i = 0; i < m->numGlobals; i++)                                 \
        {                                                                       \
            RewritePointersGlobal##STORE_OR_LOAD(&m->globals[i], base);         \
        }                                                                       \
        REWRITE(M3Global *, m->globals, base, IS_STORE);                        \
        REWRITE(bytes_t, m->elementSection, base, IS_STORE);                    \
        REWRITE(bytes_t, m->elementSectionEnd, base, IS_STORE);                 \
        for (u32 i = 0; i < m->table0Size; i++)                                 \
        {                                                                       \
            REWRITE(IM3Function, m->table0[i], base, IS_STORE);                 \
        }                                                                       \
        REWRITE(IM3Function*, m->table0, base, IS_STORE);                       \
        REWRITE_F(IM3Module,                                                    \
            m->next,                                                            \
            base,                                                               \
            IS_STORE,                                                           \
            RewritePointersModule##STORE_OR_LOAD);                              \
    }
RewritePointersModule(Store, 1)
RewritePointersModule(Load, 0)


#define RewritePointersEnvironment(STORE_OR_LOAD, IS_STORE)                     \
    static void                                                                 \
    RewritePointersEnvironment##STORE_OR_LOAD(IM3Environment e, u8* base)       \
    {                                                                           \
        REWRITE_F(IM3FuncType, e->funcTypes, base, IS_STORE,                    \
            RewritePointersFuncType##STORE_OR_LOAD);                            \
        for (u32 i = 0; i < c_m3Type_unknown; i++)                              \
        {                                                                       \
            /* retFuncTypes "point" to funcTypes, do not enter */               \
            REWRITE(IM3FuncType, e->retFuncTypes[i], base, IS_STORE);           \
        }                                                                       \
            NULLIFY_STORE(e->pagesReleased, IS_STORE);                          \
    }
RewritePointersEnvironment(Store, 1)
RewritePointersEnvironment(Load, 0)


#define RewritePointersRuntime(STORE_OR_LOAD, IS_STORE)                         \
    static void                                                                 \
    RewritePointersRuntime##STORE_OR_LOAD(IM3Runtime r, u8* base)               \
    {                                                                           \
        if (IS_STORE)                                                           \
        {                                                                       \
            /* reset compilation, code pages must be freed           */         \
            /* this is because code pages contain function pointers, */         \
            /* and preserving them does not seem feasible            */         \
            /**/                                                                \
            memset(&r->compilation, 0, sizeof(r->compilation));                 \
            /* ErrorInfo contains pointers to static storage */                 \
            /**/                                                                \
            memset(&r->error, 0, sizeof(r->error));                             \
        }                                                                       \
        REWRITE_F(IM3Environment, r->environment, base, IS_STORE,               \
            RewritePointersEnvironment##STORE_OR_LOAD);                         \
        /* Code pages are allocated transiently */                              \
        NULLIFY_STORE(r->pagesOpen, IS_STORE);                                  \
        NULLIFY_STORE(r->pagesFull, IS_STORE);                                  \
        REWRITE_F(IM3Module, r->modules, base, IS_STORE,                        \
            RewritePointersModule##STORE_OR_LOAD);                              \
        REWRITE(void*, r->stack, base, IS_STORE);                               \
        REWRITE(m3slot_t*, r->stack_suspend, base, IS_STORE);                   \
        NULLIFY_STORE(r->base, IS_STORE);                                       \
        NULLIFY_STORE(r->base_transient, IS_STORE);                             \
        /* don't enter runtime->lastCalled, we will get there eventually */     \
        REWRITE(IM3Function, r->lastCalled, base, IS_STORE);                    \
        /* user is responsible for his data */                                  \
        NULLIFY_STORE(r->userdata, IS_STORE);                                   \
        NULLIFY_STORE(r->userdata_import, IS_STORE);                            \
        RewritePointersMemory##STORE_OR_LOAD(&r->memory, base);                 \
    }
RewritePointersRuntime(Store, 1)
RewritePointersRuntime(Load, 0)


void
m3_RewritePointersRuntime(IM3Runtime r, u8* base, bool is_store)
{
    if (is_store)
    {
        RewritePointersRuntimeStore(r, base);
    }
    else
    {
        RewritePointersRuntimeLoad(r, base);
    }
}
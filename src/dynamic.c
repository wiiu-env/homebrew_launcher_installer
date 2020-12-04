#include <coreinit/dynload.h>
#include <coreinit/debug.h>

#define IMPORT(name) void* addr_##name
#define IMPORT_BEGIN(lib)
#define IMPORT_END()

#include "imports.h"

#undef IMPORT
#undef IMPORT_BEGIN
#undef IMPORT_END

#define IMPORT(name)       do{if(OSDynLoad_FindExport(handle, 0, #name, &addr_##name) < 0)OSFatal("Function " # name " is NULL");} while(0)
#define IMPORT_BEGIN(lib)  OSDynLoad_Acquire(#lib ".rpl", &handle)
/* #define IMPORT_END()       OSDynLoad_Release(handle) */
#define IMPORT_END()

#define EXPORT_VAR(type, var)           type var __attribute__((section(".data")));

EXPORT_VAR(uint32_t *, MEMAllocFromDefaultHeap);
EXPORT_VAR(uint32_t *, MEMAllocFromDefaultHeapEx);
EXPORT_VAR(uint32_t *, MEMFreeToDefaultHeap);

void InitFunctionPointers(void) {
    OSDynLoad_Module handle;
    addr_OSDynLoad_Acquire = (void *) 0x0102A3B4;
    addr_OSDynLoad_FindExport = (void *) 0x0102B828;

    OSDynLoad_Acquire("coreinit.rpl", &handle);

    uint32_t **value = 0;
    OSDynLoad_FindExport(handle, 1, "MEMAllocFromDefaultHeap", (void **) &value);
    MEMAllocFromDefaultHeap = *value;
    OSDynLoad_FindExport(handle, 1, "MEMAllocFromDefaultHeapEx", (void **) &value);
    MEMAllocFromDefaultHeapEx = *value;
    OSDynLoad_FindExport(handle, 1, "MEMFreeToDefaultHeap", (void **) &value);
    MEMFreeToDefaultHeap = *value;

#include "imports.h"

    // override failed __rplwrap_exit find export
    OSDynLoad_FindExport(handle, 0, "exit", (void **) &addr___rplwrap_exit);
}

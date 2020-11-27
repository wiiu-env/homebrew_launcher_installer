#include "elf_abi.h"
#include "../sd_loader/src/common.h"
#include "../sd_loader/sd_loader.h"
#include <stdint.h>

#define OSDynLoad_Acquire ((void (*)(char* rpl, unsigned int *handle))0x0102A3B4)
#define OSDynLoad_FindExport ((void (*)(unsigned int handle, int isdata, char *symbol, void *address))0x0102B828)
#define OSFatal ((void (*)(char* msg))0x01031618)

#define EXPORT_DECL(res, func, ...) res (* func)(__VA_ARGS__);
#define OS_FIND_EXPORT(handle, funcName, func) OSDynLoad_FindExport(handle, 0, funcName, &func)

#define ADDRESS_OSTitle_main_entry_ptr              0x1005E040
#define ADDRESS_main_entry_hook                     0x0101c56c

#define KERN_SYSCALL_TBL_1                          0xFFE84C70 // unknown
#define KERN_SYSCALL_TBL_2                          0xFFE85070 // works with games
#define KERN_SYSCALL_TBL_3                          0xFFE85470 // works with loader
#define KERN_SYSCALL_TBL_4                          0xFFEAAA60 // works with home menu
#define KERN_SYSCALL_TBL_5                          0xFFEAAE60 // works with browser (previously KERN_SYSCALL_TBL)

#define address_LiWaitIopComplete                   0x01010180
#define address_LiWaitIopCompleteWithInterrupts     0x0101006C
#define address_LiWaitOneChunk                      0x0100080C
#define address_PrepareTitle_hook                   0xFFF184E4
#define address_sgIsLoadingBuffer                   0xEFE19E80
#define address_gDynloadInitialized                 0xEFE13DBC

/* assembly functions */
extern void Syscall_0x36(void);
extern void KernelPatches(void);
extern void SCKernelCopyData(unsigned int addr, unsigned int src, unsigned int len);

extern void SC_0x25_KernelCopyData(unsigned int addr, unsigned int src, unsigned int len);


void __attribute__ ((noinline)) kern_write(void *addr, uint32_t value);

typedef struct _private_data_t {
    EXPORT_DECL(void *, MEMAllocFromDefaultHeapEx,int size, int align);
    EXPORT_DECL(void, MEMFreeToDefaultHeap,void *ptr);

    EXPORT_DECL(void*, memcpy, void *p1, const void *p2, unsigned int s);
    EXPORT_DECL(void*, memset, void *p1, int val, unsigned int s);

    EXPORT_DECL(unsigned int, OSEffectiveToPhysical, const void*);
    EXPORT_DECL(void, exit, int);
    EXPORT_DECL(void, DCInvalidateRange, const void *addr, unsigned int length);
    EXPORT_DECL(void, DCFlushRange, const void *addr, unsigned int length);
    EXPORT_DECL(void, ICInvalidateRange, const void *addr, unsigned int length);

    EXPORT_DECL(int, FSInit, void);
    EXPORT_DECL(int, FSAddClientEx, void *pClient, int unk_zero_param, int errHandling);
    EXPORT_DECL(int, FSDelClient, void *pClient);
    EXPORT_DECL(void, FSInitCmdBlock, void *pCmd);
    EXPORT_DECL(int, FSGetMountSource, void *pClient, void *pCmd, int type, void *source, int errHandling);
    EXPORT_DECL(int, FSMount, void *pClient, void *pCmd, void *source, const char *target, uint32_t bytes, int errHandling);
    EXPORT_DECL(int, FSUnmount, void *pClient, void *pCmd, const char *target, int errHandling);
    EXPORT_DECL(int, FSOpenFile, void *pClient, void *pCmd, const char *path, const char *mode, int *fd, int errHandling);
    EXPORT_DECL(int, FSGetStatFile, void *pClient, void *pCmd, int fd, void *buffer, int error);
    EXPORT_DECL(int, FSReadFile, void *pClient, void *pCmd, void *buffer, int size, int count, int fd, int flag, int errHandling);
    EXPORT_DECL(int, FSCloseFile, void *pClient, void *pCmd, int fd, int errHandling);

    EXPORT_DECL(int, SYSRelaunchTitle, int argc, char** argv);
} private_data_t;

static void InstallPatches(private_data_t *private_data);


static void loadFunctionPointers(private_data_t * private_data) {
    unsigned int coreinit_handle;

    OSDynLoad_Acquire("coreinit", &coreinit_handle);

    unsigned int *functionPtr = 0;

    OSDynLoad_FindExport(coreinit_handle, 1, "MEMAllocFromDefaultHeapEx", &functionPtr);
    private_data->MEMAllocFromDefaultHeapEx = (void * (*)(int, int))*functionPtr;
    OSDynLoad_FindExport(coreinit_handle, 1, "MEMFreeToDefaultHeap", &functionPtr);
    private_data->MEMFreeToDefaultHeap = (void (*)(void *))*functionPtr;

    OS_FIND_EXPORT(coreinit_handle, "memcpy", private_data->memcpy);
    OS_FIND_EXPORT(coreinit_handle, "memset", private_data->memset);
    OS_FIND_EXPORT(coreinit_handle, "DCFlushRange", private_data->DCFlushRange);
    OS_FIND_EXPORT(coreinit_handle, "DCInvalidateRange", private_data->DCInvalidateRange);
    OS_FIND_EXPORT(coreinit_handle, "ICInvalidateRange", private_data->ICInvalidateRange);
    OS_FIND_EXPORT(coreinit_handle, "OSEffectiveToPhysical", private_data->OSEffectiveToPhysical);
    OS_FIND_EXPORT(coreinit_handle, "exit", private_data->exit);

    OS_FIND_EXPORT(coreinit_handle, "FSInit", private_data->FSInit);
    OS_FIND_EXPORT(coreinit_handle, "FSAddClientEx", private_data->FSAddClientEx);
    OS_FIND_EXPORT(coreinit_handle, "FSDelClient", private_data->FSDelClient);
    OS_FIND_EXPORT(coreinit_handle, "FSInitCmdBlock", private_data->FSInitCmdBlock);
    OS_FIND_EXPORT(coreinit_handle, "FSGetMountSource", private_data->FSGetMountSource);
    OS_FIND_EXPORT(coreinit_handle, "FSMount", private_data->FSMount);
    OS_FIND_EXPORT(coreinit_handle, "FSUnmount", private_data->FSUnmount);
    OS_FIND_EXPORT(coreinit_handle, "FSOpenFile", private_data->FSOpenFile);
    OS_FIND_EXPORT(coreinit_handle, "FSGetStatFile", private_data->FSGetStatFile);
    OS_FIND_EXPORT(coreinit_handle, "FSReadFile", private_data->FSReadFile);
    OS_FIND_EXPORT(coreinit_handle, "FSCloseFile", private_data->FSCloseFile);

    unsigned int sysapp_handle;
    OSDynLoad_Acquire("sysapp.rpl", &sysapp_handle);
    OS_FIND_EXPORT(sysapp_handle, "SYSRelaunchTitle", private_data->SYSRelaunchTitle);
}

static unsigned int load_elf_image (private_data_t *private_data, unsigned char *elfstart) {
    Elf32_Ehdr *ehdr;
    Elf32_Phdr *phdrs;
    unsigned char *image;
    int i;

    ehdr = (Elf32_Ehdr *) elfstart;

    if(ehdr->e_phoff == 0 || ehdr->e_phnum == 0)
        return 0;

    if(ehdr->e_phentsize != sizeof(Elf32_Phdr))
        return 0;

    phdrs = (Elf32_Phdr*)(elfstart + ehdr->e_phoff);

    for(i = 0; i < ehdr->e_phnum; i++) {
        if(phdrs[i].p_type != PT_LOAD)
            continue;

        if(phdrs[i].p_filesz > phdrs[i].p_memsz)
            continue;

        if(!phdrs[i].p_filesz)
            continue;

        unsigned int p_paddr = phdrs[i].p_paddr;
        image = (unsigned char *) (elfstart + phdrs[i].p_offset);

        private_data->memcpy ((void *) p_paddr, image, phdrs[i].p_filesz);
        private_data->DCFlushRange((void*)p_paddr, phdrs[i].p_filesz);

        if(phdrs[i].p_flags & PF_X)
            private_data->ICInvalidateRange ((void *) p_paddr, phdrs[i].p_memsz);
    }

    //! clear BSS
    Elf32_Shdr *shdr = (Elf32_Shdr *) (elfstart + ehdr->e_shoff);
    for(i = 0; i < ehdr->e_shnum; i++) {
        const char *section_name = ((const char*)elfstart) + shdr[ehdr->e_shstrndx].sh_offset + shdr[i].sh_name;
        if(section_name[0] == '.' && section_name[1] == 'b' && section_name[2] == 's' && section_name[3] == 's') {
            private_data->memset((void*)shdr[i].sh_addr, 0, shdr[i].sh_size);
            private_data->DCFlushRange((void*)shdr[i].sh_addr, shdr[i].sh_size);
        } else if(section_name[0] == '.' && section_name[1] == 's' && section_name[2] == 'b' && section_name[3] == 's' && section_name[4] == 's') {
            private_data->memset((void*)shdr[i].sh_addr, 0, shdr[i].sh_size);
            private_data->DCFlushRange((void*)shdr[i].sh_addr, shdr[i].sh_size);
        }
    }

    return ehdr->e_entry;
}


void KernelWriteU32(uint32_t addr, uint32_t value, private_data_t * pdata) {
    pdata->ICInvalidateRange(&value, 4);
    pdata->DCFlushRange(&value, 4);

    uint32_t dst = (uint32_t) pdata->OSEffectiveToPhysical((void *)addr);
    uint32_t src = (uint32_t) pdata->OSEffectiveToPhysical((void *)&value);

    SC_0x25_KernelCopyData(dst, src, 4);

    pdata->DCFlushRange((void *)addr, 4);
    pdata->ICInvalidateRange((void *)addr, 4);
}

int _start(int argc, char **argv) {
    kern_write((void*)(KERN_SYSCALL_TBL_1 + (0x25 * 4)), (unsigned int)SCKernelCopyData);
    kern_write((void*)(KERN_SYSCALL_TBL_2 + (0x25 * 4)), (unsigned int)SCKernelCopyData);
    kern_write((void*)(KERN_SYSCALL_TBL_3 + (0x25 * 4)), (unsigned int)SCKernelCopyData);
    kern_write((void*)(KERN_SYSCALL_TBL_4 + (0x25 * 4)), (unsigned int)SCKernelCopyData);
    kern_write((void*)(KERN_SYSCALL_TBL_5 + (0x25 * 4)), (unsigned int)SCKernelCopyData);

    kern_write((void*)(KERN_SYSCALL_TBL_1 + (0x36 * 4)), (unsigned int)KernelPatches);
    kern_write((void*)(KERN_SYSCALL_TBL_2 + (0x36 * 4)), (unsigned int)KernelPatches);
    kern_write((void*)(KERN_SYSCALL_TBL_3 + (0x36 * 4)), (unsigned int)KernelPatches);
    kern_write((void*)(KERN_SYSCALL_TBL_4 + (0x36 * 4)), (unsigned int)KernelPatches);
    kern_write((void*)(KERN_SYSCALL_TBL_5 + (0x36 * 4)), (unsigned int)KernelPatches);

    Syscall_0x36();

    private_data_t private_data;
    loadFunctionPointers(&private_data);

    InstallPatches(&private_data);

    unsigned char * pElfBuffer = (unsigned char *) sd_loader; // use this address as temporary to load the elf

    unsigned int mainEntryPoint = load_elf_image(&private_data, pElfBuffer);

    if(mainEntryPoint == 0) {
        OSFatal("failed to load elf");
    }

    //! Install our entry point hook
    unsigned int repl_addr = ADDRESS_main_entry_hook;
    unsigned int jump_addr = mainEntryPoint & 0x03fffffc;

    unsigned int bufferU32 = 0x48000003 | jump_addr;
    KernelWriteU32(repl_addr,bufferU32,&private_data);

    // restart mii maker.
    private_data.SYSRelaunchTitle(0, 0);
    private_data.exit(0);
    return 0;

    //return ((int (*)(int, char **))mainEntryPoint)(argc, argv);
}

/* Write a 32-bit word with kernel permissions */
void __attribute__ ((noinline)) kern_write(void *addr, uint32_t value) {
    asm volatile (
        "li 3,1\n"
        "li 4,0\n"
        "mr 5,%1\n"
        "li 6,0\n"
        "li 7,0\n"
        "lis 8,1\n"
        "mr 9,%0\n"
        "mr %1,1\n"
        "li 0,0x3500\n"
        "sc\n"
        "nop\n"
        "mr 1,%1\n"
        :
        :	"r"(addr), "r"(value)
        :	"memory", "ctr", "lr", "0", "3", "4", "5", "6", "7", "8", "9", "10",
        "11", "12"
    );
}

/* ****************************************************************** */
/*                         INSTALL PATCHES                            */
/* All OS specific stuff is done here                                 */
/* ****************************************************************** */
static void InstallPatches(private_data_t *private_data) {
    OsSpecifics osSpecificFunctions;
    private_data->memset(&osSpecificFunctions, 0, sizeof(OsSpecifics));

    unsigned int bufferU32;
    /* Pre-setup a few options to defined values */
    bufferU32 = 550;
    private_data->memcpy((void*)&OS_FIRMWARE, &bufferU32, sizeof(bufferU32));
    bufferU32 = 0xDEADC0DE;
    private_data->memcpy((void*)&MAIN_ENTRY_ADDR, &bufferU32, sizeof(bufferU32));
    private_data->memcpy((void*)&ELF_DATA_ADDR, &bufferU32, sizeof(bufferU32));
    bufferU32 = 0;
    private_data->memcpy((void*)&ELF_DATA_SIZE, &bufferU32, sizeof(bufferU32));

    osSpecificFunctions.addr_OSDynLoad_Acquire = (unsigned int)OSDynLoad_Acquire;
    osSpecificFunctions.addr_OSDynLoad_FindExport = (unsigned int)OSDynLoad_FindExport;

    osSpecificFunctions.addr_KernSyscallTbl1 = KERN_SYSCALL_TBL_1;
    osSpecificFunctions.addr_KernSyscallTbl2 = KERN_SYSCALL_TBL_2;
    osSpecificFunctions.addr_KernSyscallTbl3 = KERN_SYSCALL_TBL_3;
    osSpecificFunctions.addr_KernSyscallTbl4 = KERN_SYSCALL_TBL_4;
    osSpecificFunctions.addr_KernSyscallTbl5 = KERN_SYSCALL_TBL_5;

    osSpecificFunctions.LiWaitIopComplete = (int (*)(int, int *)) address_LiWaitIopComplete;
    osSpecificFunctions.LiWaitIopCompleteWithInterrupts = (int (*)(int, int *)) address_LiWaitIopCompleteWithInterrupts;
    osSpecificFunctions.addr_LiWaitOneChunk = address_LiWaitOneChunk;
    osSpecificFunctions.addr_PrepareTitle_hook = address_PrepareTitle_hook;
    osSpecificFunctions.addr_sgIsLoadingBuffer = address_sgIsLoadingBuffer;
    osSpecificFunctions.addr_gDynloadInitialized = address_gDynloadInitialized;
    osSpecificFunctions.orig_LiWaitOneChunkInstr = *(unsigned int*)address_LiWaitOneChunk;

    //! pointer to main entry point of a title
    osSpecificFunctions.addr_OSTitle_main_entry = ADDRESS_OSTitle_main_entry_ptr;

    private_data->memcpy((void*)OS_SPECIFICS, &osSpecificFunctions, sizeof(OsSpecifics));
}

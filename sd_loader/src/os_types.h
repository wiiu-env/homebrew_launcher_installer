#ifndef _OS_TYPES_H_
#define _OS_TYPES_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>

/*+----------------------------------------------------------------------------------------------+*/
typedef uint8_t u8;					///< 8bit unsigned integer
typedef uint16_t u16;				///< 16bit unsigned integer
typedef uint32_t u32;				///< 32bit unsigned integer
typedef uint64_t u64;				///< 64bit unsigned integer
/*+----------------------------------------------------------------------------------------------+*/
typedef int8_t s8;					///< 8bit signed integer
typedef int16_t s16;				///< 16bit signed integer
typedef int32_t s32;				///< 32bit signed integer
typedef int64_t s64;				///< 64bit signed integer
/*+----------------------------------------------------------------------------------------------+*/
typedef volatile u8 vu8;			///< 8bit unsigned volatile integer
typedef volatile u16 vu16;			///< 16bit unsigned volatile integer
typedef volatile u32 vu32;			///< 32bit unsigned volatile integer
typedef volatile u64 vu64;			///< 64bit unsigned volatile integer
/*+----------------------------------------------------------------------------------------------+*/
typedef volatile s8 vs8;			///< 8bit signed volatile integer
typedef volatile s16 vs16;			///< 16bit signed volatile integer
typedef volatile s32 vs32;			///< 32bit signed volatile integer
typedef volatile s64 vs64;			///< 64bit signed volatile integer
/*+----------------------------------------------------------------------------------------------+*/
// fixed point math typedefs
typedef s16 sfp16;					///< signed 8:8 fixed point
typedef s32 sfp32;					///< signed 20:8 fixed point
typedef u16 ufp16;					///< unsigned 8:8 fixed point
typedef u32 ufp32;					///< unsigned 24:8 fixed point
/*+----------------------------------------------------------------------------------------------+*/
typedef float f32;
typedef double f64;
/*+----------------------------------------------------------------------------------------------+*/
typedef volatile float vf32;
typedef volatile double vf64;
/*+----------------------------------------------------------------------------------------------+*/

#define OS_MESSAGE_NOBLOCK 0
#define OS_MESSAGE_BLOCK 1

#define OS_EXCEPTION_DSI 2
#define OS_EXCEPTION_ISI 3
#define OS_EXCEPTION_PROGRAM 6
#define OS_EXCEPTION_MODE_THREAD 1
#define OS_EXCEPTION_MODE_GLOBAL_ALL_CORES 4

#define OS_THREAD_ATTR_AFFINITY_NONE    0x0007u        // affinity to run on every core
#define OS_THREAD_ATTR_AFFINITY_CORE0   0x0001u        // run only on core0
#define OS_THREAD_ATTR_AFFINITY_CORE1   0x0002u        // run only on core1
#define OS_THREAD_ATTR_AFFINITY_CORE2   0x0004u        // run only on core2
#define OS_THREAD_ATTR_DETACH           0x0008u        // detached
#define OS_THREAD_ATTR_PINNED_AFFINITY  0x0010u        // pinned (affinitized) to a single core
#define OS_THREAD_ATTR_CHECK_STACK_USE  0x0040u        // check for stack usage
#define OS_THREAD_ATTR_NAME_SENT        0x0080u        // debugger has seen the name
#define OS_THREAD_ATTR_LAST (OS_THREAD_ATTR_DETACH | OS_THREAD_ATTR_PINNED_AFFINITY | OS_THREAD_ATTR_AFFINITY_NONE)

typedef struct OSThread_ OSThread;

typedef struct OSThreadLink_ {
    OSThread *next;
    OSThread *prev;
}  OSThreadLink;

typedef struct OSThreadQueue_ {
    OSThread *head;
    OSThread *tail;
    void *parentStruct;
    u32 reserved;
} OSThreadQueue;

typedef struct OSMessage_ {
    u32 message;
    u32 data0;
    u32 data1;
    u32 data2;
} OSMessage;

typedef struct OSMessageQueue_ {
    u32 tag;
    char *name;
    u32 reserved;

    OSThreadQueue sendQueue;
    OSThreadQueue recvQueue;
    OSMessage *messages;
    int msgCount;
    int firstIndex;
    int usedCount;
} OSMessageQueue;

typedef struct OSContext_ {
    char tag[8];

    u32 gpr[32];

    u32 cr;
    u32 lr;
    u32 ctr;
    u32 xer;

    u32 srr0;
    u32 srr1;

    u32 ex0;
    u32 ex1;

    u32 exception_type;
    u32 reserved;

    double fpscr;
    double fpr[32];

    u16 spinLockCount;
    u16 state;

    u32 gqr[8];
    u32 pir;
    double psf[32];

    u64 coretime[3];
    u64 starttime;

    u32 error;
    u32 attributes;

    u32 pmc1;
    u32 pmc2;
    u32 pmc3;
    u32 pmc4;
    u32 mmcr0;
    u32 mmcr1;
} OSContext;

typedef enum OSExceptionType {
    OS_EXCEPTION_TYPE_SYSTEM_RESET         = 0,
    OS_EXCEPTION_TYPE_MACHINE_CHECK        = 1,
    OS_EXCEPTION_TYPE_DSI                  = 2,
    OS_EXCEPTION_TYPE_ISI                  = 3,
    OS_EXCEPTION_TYPE_EXTERNAL_INTERRUPT   = 4,
    OS_EXCEPTION_TYPE_ALIGNMENT            = 5,
    OS_EXCEPTION_TYPE_PROGRAM              = 6,
    OS_EXCEPTION_TYPE_FLOATING_POINT       = 7,
    OS_EXCEPTION_TYPE_DECREMENTER          = 8,
    OS_EXCEPTION_TYPE_SYSTEM_CALL          = 9,
    OS_EXCEPTION_TYPE_TRACE                = 10,
    OS_EXCEPTION_TYPE_PERFORMANCE_MONITOR  = 11,
    OS_EXCEPTION_TYPE_BREAKPOINT           = 12,
    OS_EXCEPTION_TYPE_SYSTEM_INTERRUPT     = 13,
    OS_EXCEPTION_TYPE_ICI                  = 14,
} OSExceptionType;

typedef int (*ThreadFunc)(int argc, void *argv);

struct OSThread_ {
    OSContext context;

    u32 txtTag;
    u8 state;
    u8 attr;

    short threadId;
    int suspend;
    int priority;

    char _[0x394 - 0x330 - sizeof(OSThreadLink)];
    OSThreadLink linkActive;

    void *stackBase;
    void *stackEnd;

    ThreadFunc entryPoint;

    char _3A0[0x6A0 - 0x3A0];
};

typedef struct _OSCalendarTime {
    int sec;
    int min;
    int hour;
    int mday;
    int mon;
    int year;
    int wday;
    int yday;
    int msec;
    int usec;
} OSCalendarTime;


typedef struct MCPTitleListType {
    u64 titleId;
    u8 unknwn[4];
    s8 path[56];
    u32 appType;
    u8 unknwn1[0x54 - 0x48];
    u8 device;
    u8 unknwn2;
    s8 indexedDevice[10];
    u8 unk0x60;
} MCPTitleListType;

#ifdef __cplusplus
}
#endif

#endif

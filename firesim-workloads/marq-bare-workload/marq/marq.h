#ifndef C_MAR_H
#define C_MAR_H

#include <stdint.h>
#include <stdbool.h>

//#define DEBUG

#ifdef DEBUG
#define MAR_FIFO_DEPTH 4
#else
#define MAR_FIFO_DEPTH 32
#endif

#define CSR_MAR_ENABLE 	  "0xBC0"
#define CSR_MAR_READ_FIFO "0xBD0"
#define BLACKLIST_FIXED   "0xBE0"
#define BLACKLIST_FIFO    "0xBF0"

#define BLACKLIST_SIZE 5
#define MARQ_FLAG 0x10

typedef struct {
    uint64_t pc;
    uint64_t addr;
    uint8_t  flags;
} mar_entry;

typedef enum {
    ld 	  = 0x8,
    st 	  = 0x4,
    amo   = 0x2,
    hella = 0x1
} flags_t;

typedef enum {
    read_success = 0,
    read_end = 1,
    read_failed = 2
} fifo_read_t;

#define read_csr(reg) ({ \
	unsigned long __tmp; \
	asm volatile ("csrr %0, " reg : "=r"(__tmp)); \
	__tmp; })

#define write_csr(reg, val) ({ \
	asm volatile ("csrw " reg ", %0" :: "rK"(val)); })

#define disable_marq_irq (asm volatile ("csrc mie, (1 << 16)"))
#define enable_marq_irq (asm volatile ("csrc mie, (1 << 16)"))

/*
 * Standard function calls invoke the call stack which adds extra
 * memory loads to the code. This triggers our MARQ for api calls
 * which we do not. To solve this, we created guards that perform
 * a safe call/operation. The following is used to create these.
 *
 * How to use: PARAM_<0/1>_SAFE_DECL<_VOID>(<func_name>, <type>, <arg type>, <arg>)
 * 	where choices are 0/1 parameter. void is optional otherwise it
 * 	returns the type. If void function, there is no type param.
 *
 * This goes in the header. The implementation has an IMPL variant.
 */

#define PARAM_0_SAFE_DECL(FUNC_NAME, TYPE) PARAM_1_SAFE_DECL(FUNC_NAME, TYPE,,)
#define PARAM_1_SAFE_DECL(FUNC_NAME, TYPE, ARG_TYPE, ARG) \
    DECL_SAFE(FUNC_NAME, TYPE, ARG_TYPE, ARG) \
	{ 					    \
	    bool prev_enabled = disable_mar(); 	    \
	    TYPE tmp = __##FUNC_NAME(ARG); 	    \
	    if (prev_enabled) enable_mar(); 	    \
	    return tmp; 			    \
	}
#define PARAM_0_SAFE_DECL_VOID(FUNC_NAME) PARAM_1_SAFE_DECL_VOID(FUNC_NAME,,)
#define PARAM_1_SAFE_DECL_VOID(FUNC_NAME, ARG_TYPE, ARG) \
    DECL_SAFE(FUNC_NAME, void, ARG_TYPE, ARG) \
	{ 					    \
	    bool prev_enabled = disable_mar(); 	    \
	    __##FUNC_NAME(ARG); 	    	    \
	    if (prev_enabled) enable_mar(); 	    \
	}

// prototypes the inplementation and creates the signature
#define DECL_SAFE(FUNC_NAME, TYPE, ARG_TYPE, ARG) \
    TYPE __##FUNC_NAME(ARG_TYPE ARG); 	    \
    static inline TYPE FUNC_NAME(ARG_TYPE ARG) \

#define PARAM_0_SAFE_IMPL(FUNC_NAME, TYPE) PARAM_1_SAFE_IMPL(FUNC_NAME, TYPE,,)
#define PARAM_1_SAFE_IMPL(FUNC_NAME, TYPE, ARG_TYPE, ARG) \
    TYPE __##FUNC_NAME(ARG_TYPE ARG)
/* 
 * Enables/disables tracking of memory accesses. Disable returns prev 
 * enabled value 
 */

extern bool enabled;

static inline void enable_mar() 
{
    enabled = true; 
    write_csr(CSR_MAR_ENABLE, 1);
}

static inline bool disable_mar()  
{
    bool prev_enabled; 
    
    write_csr(CSR_MAR_ENABLE, 0);
    prev_enabled = enabled; 
    enabled = false; 
    
    return prev_enabled;
}

#define is_load(entry)  ((entry.flags & ld) != 0)
#define is_store(entry) ((entry.flags & st) != 0)
#define is_amo(entry)   ((entry.flags & amo) != 0)
#define is_hella(entry) ((entry.flags & hella) != 0)

PARAM_1_SAFE_DECL_VOID(blacklist_addr, void*, addr)

/*
 * Reads the oldest entry from the mar fifo or flushes the 
 * entire thing. An addr of 0 means invalid entry.
 */

PARAM_1_SAFE_DECL(read_mar_fifo, int, mar_entry*, entry)
PARAM_0_SAFE_DECL(flush_mar_fifo, int)

#endif

// engler, cs140e: trivial example of how to log GET32.  
//
// part 1: add get32, put32, PUT32.  
//    - you'll have to modify the makefile --- search for GET32 and add there.
//    - simply have put32 call PUT32, get32 call GET32 so its easier to compare output.
//
// part 2: add a simple log for capturing
//
//  without capture it's unsafe to call during UART operations since infinitely
//  recursive and, also, will mess with the UART state.  
//
//  record the calls in a log (just declare a statically sized array until we get malloc)
//  for later emit by trace_dump()
//
// part 3: keep adding more functions!  
//      often it's useful to trace at a higher level, for example, doing 
//      uart_get() vs all the put's/get's it does, since that is easier for
//      a person to descipher.  or, you can do both:
//          -  put a UART_PUTC_START in the log.
//          - record the put/gets as normal.
//          - put a UART_PUTC_STOP in the log.
//  
// XXX: may have to use wdiff or similar to match the outputs up exactly.  or strip out
// spaces.
//

#include "rpi.h"
#include "trace.h"

// gross that these are not auto-consistent with GET32 in rpi.h
unsigned __wrap_GET32(unsigned addr);
unsigned __real_GET32(unsigned addr);
unsigned __wrap_get32(const volatile void *addr);
unsigned __real_get32(const volatile void *addr);
void __wrap_PUT32(unsigned addr, unsigned v);
void __real_PUT32(unsigned addr, unsigned v);
void __wrap_put32(volatile void *addr, unsigned v);
void __real_put32(volatile void *addr, unsigned v);

static int tracing_p = 0; // whether tracing is currently enabled
static int in_trace = 0; // whether we're currently running a trace function, so we can prevent recursion

typedef enum {
    TRACE_OP_GET32,
    TRACE_OP_PUT32,
} trace_op_t;

typedef struct {
    unsigned addr;
    unsigned val;
    trace_op_t op;
} trace_record_t;

enum { NUM_LOG_RECORDS = 1024 };
trace_record_t trace_capture[NUM_LOG_RECORDS];
size_t next_trace_slot = 0;
int capture_p;

void capture_trace(unsigned addr, unsigned val, trace_op_t op) {
    demand(next_trace_slot < NUM_LOG_RECORDS, "trace capture full");
    trace_record_t *slot = &trace_capture[next_trace_slot];
    slot->addr = addr;
    slot->val = val;
    slot->op = op;
    next_trace_slot++;
}

void printk_get32(unsigned addr, unsigned v, int capture) {
    if (capture) {
        capture_trace(addr, v, TRACE_OP_GET32);
    } else {
        printk("\tTRACE:GET32(0x%x)=0x%x\n", addr, v);
    }
}

void printk_put32(unsigned addr, unsigned v, int capture) {
    if (capture) {
        capture_trace(addr, v, TRACE_OP_PUT32);
    } else {
        printk("\tTRACE:PUT32(0x%x)=0x%x\n", addr, v);
    }
}

void printk_trace_record(trace_record_t *record) {
    switch (record->op) {
        case TRACE_OP_GET32: printk_get32(record->addr, record->val, 0); break;
        case TRACE_OP_PUT32: printk_put32(record->addr, record->val, 0); break;
    }
}

void trace_start(int capture) {
    demand(!tracing_p, "trace already running!");
    demand(!in_trace, "invalid in_trace");
    tracing_p = 1; 
    capture_p = capture;
}

// the linker will change all calls to GET32 to call __wrap_GET32
unsigned __wrap_GET32(unsigned addr) { 
    // the linker will change the name of GET32 to __real_GET32,
    // which we can then call ourselves.
    unsigned v = __real_GET32(addr); 

    // doing this print while the UART is busying printing a character
    // will lead to an inf loop since printk will do its own
    // puts/gets.  use <in_trace> to skip our own monitoring calls.
    if(!in_trace && tracing_p) {
        in_trace = 1;
        // match it up with your unix print so you can compare.
        // we have to add a 0x
        printk_get32(addr, v, capture_p);
        in_trace = 0;
    }
    return v;
}

unsigned __wrap_get32(const volatile void *addr) {
    return __wrap_GET32((unsigned) addr);
}

void __wrap_PUT32(unsigned addr, unsigned v) {
    __real_PUT32(addr, v);
    if (!in_trace && tracing_p) {
        in_trace = 1;
        printk_put32(addr, v, capture_p);
        in_trace = 0;
    }
}

void __wrap_put32(volatile void *addr, unsigned v) {
    __wrap_PUT32((unsigned) addr, v);
}

void trace_stop(void) {
    demand(tracing_p, "trace already stopped!\n");
    tracing_p = 0; 
}

void trace_dump(int reset_p) { 
    for (int i = 0; i < next_trace_slot; i++) {
        printk_trace_record(&trace_capture[i]);
    }

    if (reset_p) {
        next_trace_slot = 0;
    }
}

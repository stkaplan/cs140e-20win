/*
 * Simple tracing memory implementation.  Used for cross-checking.
 * 
 * you will implement a simple fake memory that maps addresses to values.  
 * e.g., an array, linked list, hash table of (<addr>, <val>) tuples such 
 * that after you insert some (<addr>, <val>), you can lookup <val> using 
 * <addr> as a key.
 *
 * simplistic assumptions:
 *  - all loads/stores are 32 bits.
 *  - read returns value of last write, or random() if none.
 *  - load and store prints the addr,val
 * 
 * HINT: I woul suggest a fixed size array, that you do a simple linear scan 
 * to insert, and lookup.
 *
 * mildly formal-ish rules for fake memory:
 *  1. no duplicate entries.  
 *       if: 
 *           (<addr>, <val0>) \in memory
 *           (<addr>, <val1>) \in memory
 *      then <val0> == <val1>
 *  2. addresses are persistent: if you ever write <addr>,<val> to memory, 
 *     then <addr> stays there forever.
 *  3. values are persistent until the next write: if you ever write 
 *     (<addr>,<val> ) to memory, then until the next write, 
 *          if (<addr>, <v>) \in memory, then <v> == <val>.
 */
#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "demand.h"
#include "rpi.h"
#include "pi-test.h"

/****************************************************************************
 *          DO NOT CHANGE THIS CODE (your changes go after it)
 *          DO NOT CHANGE THIS CODE (your changes go after it)
 *          DO NOT CHANGE THIS CODE (your changes go after it)
 *          DO NOT CHANGE THIS CODE (your changes go after it)
 *          DO NOT CHANGE THIS CODE (your changes go after it)
 */

typedef struct {
    const volatile void *addr;
    unsigned val;
} mem_t;

typedef struct {
    int op;
    mem_t v;
} log_ent_t;

enum { NUM_MEM_SLOTS = 1024 };
mem_t mem[NUM_MEM_SLOTS];
size_t next_slot = 0;

static mem_t mk_mem(const volatile void *addr, unsigned val) {
    return (mem_t) { .addr = addr, .val = val };
}

// don't change routine so we can compare to everyone.
static void print_write(mem_t *m) {
    printf("\tTRACE:PUT32(%p)=0x%x\n", m->addr, m->val);
}
// don't change routine so we can compare to everyone.
static void print_read(mem_t *m) {
    printf("\tTRACE:GET32(%p)=0x%x\n", m->addr, m->val);
}

unsigned get32(const volatile void *addr) {
    for (size_t i = 0; i < next_slot; i++) {
        if (mem[i].addr == addr) {
            print_read(&mem[i]);
            return mem[i].val;
        }
    }

    demand(next_slot < NUM_MEM_SLOTS, "out of memory slots");
    unsigned val = fake_random();
    mem[next_slot] = mk_mem(addr, val);
    print_read(&mem[next_slot]);
    next_slot++;
    return val;
}

void put32(volatile void *addr, unsigned val) {
    for (size_t i = 0; i < next_slot; i++) {
        if (mem[i].addr == addr) {
            mem[i].val = val;
            print_write(&mem[i]);
            return;
        }
    }

    demand(next_slot < NUM_MEM_SLOTS, "out of memory slots");
    mem[next_slot] = mk_mem(addr, val);
    print_write(&mem[next_slot]);
    next_slot++;
}


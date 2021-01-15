/* engler, cs140e.
 *
 * very simple bootloader.  more robust than xmodem, which seems to 
 * have bugs in terms of recovery with inopportune timeouts.
 */


/**********************************************************************
 * the code below is starter code provided by us.    the code you
 * should modify follows it (`notmain`).
 *
 *      DO NOT MODIFY THE FOLLOWING!!!
 *      DO NOT MODIFY THE FOLLOWING!!!
 *      DO NOT MODIFY THE FOLLOWING!!!
 *      DO NOT MODIFY THE FOLLOWING!!!
 *      DO NOT MODIFY THE FOLLOWING!!!
 */
#include "rpi.h"

#define __SIMPLE_IMPL__
#include "../shared-code/simple-boot.h"         // holds crc32

// blocking calls to send / receive a single byte from the uart.
static void put_byte(unsigned char uc)  { 
    uart_putc(uc); 
}
static unsigned char get_byte(void) { 
    return uart_getc(); 
}

static unsigned get_uint(void) {
	unsigned u = get_byte();
        u |= get_byte() << 8;
        u |= get_byte() << 16;
        u |= get_byte() << 24;
	return u;
}
static void put_uint(unsigned u) {
    put_byte((u >> 0)  & 0xff);
    put_byte((u >> 8)  & 0xff);
    put_byte((u >> 16) & 0xff);
    put_byte((u >> 24) & 0xff);
}

// send the unix side an error code and reboot.
static void die(int code) {
    put_uint(code);
    reboot();
}

// send a string <msg> to the unix side to print.  
// message format:
//  [PRINT_STRING, strlen(msg), <msg>]
//
// DANGER DANGER DANGER!!!  Be careful WHEN you call this!
// DANGER DANGER DANGER!!!  Be careful WHEN you call this!
// DANGER DANGER DANGER!!!  Be careful WHEN you call this!
//
// We do not have interrupts and the UART can only hold 
// 8 bytes before it starts dropping them.   so if you print at the
// same time the UNIX end can send you data, you will likely
// lose some, leading to weird bugs.  If you print, safest is 
// to do right after you have completely received a message.
void putk(const char *msg) {
    put_uint(PRINT_STRING);
    // lame strlen.
    int n;
    for(n = 0; msg[n]; n++)
        ;
    put_uint(n);
    for(n = 0; msg[n]; n++)
        put_byte(msg[n]);
}

void putk_value(unsigned val) {
    put_uint(PRINT_VALUE);
    put_uint(val);
}

// send a <GET_PROG_INFO> message every 300ms.
// 
// NOTE: look at the idiom for comparing the current usec count to 
// when we started.  
void wait_for_data(void) {
    while(1) {
        put_uint(GET_PROG_INFO);

        unsigned s = timer_get_usec();
        // the funny subtraction is to prevent wrapping.
        while((timer_get_usec() - s) < 300*1000) {
            // the UART says there is data: start eating it!
            if(uart_rx_has_data())
                return;
        }
    }
}

/*****************************************************************
 * All the code you write goes below.
 */

// Simple bootloader: put all of your code here: implement steps 2,3,4,5,6
void notmain(void) {
    unsigned op, nbytes, expected_crc;

    uart_init();

    // The Pi has a single byte in the RX queue when first booting. No idea
    // where that is coming from; just ignore it.
    // NOTE: It seems like uart_rx_has_data() doesn't work when called
    // immediately after uart_init(). No idea why. It works if I sleep 300 ms
    // first, but that's fragile. Instead, unconditionally pull 1 byte from RX.
    get_byte();

    // 1. keep sending GET_PROG_INFO until there is data.
    wait_for_data();

    /****************************************************************
     * Add your code below: 2,3,4,5,6
     */


    // 2. expect: [PUT_PROG_INFO, addr, nbytes, cksum] 
    //    we echo cksum back in step 4 to help debugging.
    if ((op = get_uint()) != PUT_PROG_INFO) {
        putk("FATAL ERROR: PUT_PROG_INFO mismatch");
        putk_value(op);
        reboot();
    }
    if ((op = get_uint()) != ARMBASE) {
        putk("FATAL ERROR: ARMBASE mismatch");
        putk_value(op);
        reboot();
    }
    nbytes = get_uint();
    expected_crc = get_uint();

    // 3. If the binary will collide with us, abort. 
    //    you can assume that code must be below where the booloader code
    //    gap starts.
    if (ARMBASE + nbytes > (unsigned) BRANCHTO) {
        putk("FATAL_ERROR: code too large");
        put_uint(BAD_CODE_ADDR);
        reboot();
    }

    // 4. send [GET_CODE, cksum] back.
    put_uint(GET_CODE);
    put_uint(expected_crc);

    // 5. expect: [PUT_CODE, <code>]
    //  read each sent byte and write it starting at 
    //  ARMBASE using PUT8
    if ((op = get_uint()) != PUT_CODE) {
        putk("FATAL ERROR: PUT_CODE mismatch");
        putk_value(op);
        reboot();
    }
    for (int i = 0; i < nbytes; i++) {
        PUT8(ARMBASE+i, get_byte());
    }

    // 6. verify the cksum of the copied code.
    unsigned actual_crc = crc32((void *) ARMBASE, nbytes);
    if (actual_crc != expected_crc) {
        putk("FATAL ERROR: incorrect crc");
        putk_value(expected_crc);
        putk_value(actual_crc);
        put_uint(BAD_CODE_CKSUM);
        reboot();
    }

    /****************************************************************
     * add your code above: don't modify below unless you want to 
     * experiment.
     */

    // 7. no previous failures: send back a BOOT_SUCCESS!
    put_uint(BOOT_SUCCESS);

	// XXX: appears we need these delays or the unix side gets confused.
	// I believe it's b/c the code we call re-initializes the uart; could
	// disable that to make it a bit more clean.
	//
    // XXX: I think we can remove this now; should test.
    delay_ms(500);

    // run what we got.
    BRANCHTO(ARMBASE);

    // should not get back here, but just in case.
    reboot();
}

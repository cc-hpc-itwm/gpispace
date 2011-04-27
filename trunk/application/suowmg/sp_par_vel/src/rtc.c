#include "rtc.h"

/*
 *   rtc timers
 */

static __inline__ unsigned long long get_rtc_res(void);
static __inline__ unsigned long long get_rtc(void);


static __inline__ unsigned long long get_rtc_res(void) {
    static unsigned long long res = 0;
    unsigned long long rtc;

    if (res != 0)
	/* the value is in the cache */
	return res;

    rtc = get_rtc();
    usleep(1000000); /* usleep doesn't work as desired */
    res = get_rtc() - rtc;

    return res;
}


static __inline__ unsigned long long get_rtc(void) {
    unsigned long long rtc;

    do {
	asm volatile ("rdtsc\n\t"
		      "salq $32, %%rdx\n\t"
		      "orq %%rdx, %%rax\n\t"
		      "movq %%rax, %0"
		      : "=r" (rtc)
		      : /* no inputs */
		      : "%rax", "%rdx");
    } while (0);

    return rtc;
}

void get_rtc_(unsigned long long *rtc) {
    *rtc = get_rtc();
}

void get_rtc_res_(unsigned long long *res) {
    *res = get_rtc_res();
}

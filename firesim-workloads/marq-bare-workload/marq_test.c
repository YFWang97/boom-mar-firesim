#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include "marq.h"

int main (void) {
    int a[32];
    register int i;

    printf("Int size is %ld\n", sizeof(int));
    for (i = 0; i < 4; i++)
	printf("C_ADDR: 0x%p\n", (a + i));

    printf("C_ADDR: 0x%p\n", (a + 63));

    blacklist_addr((void *)0x8002af58);
    write_csr("0xBC1", 1);
    enable_mar();
    a[0] = 0;
    a[1] = 1;
    a[2] = 2;
    a[3] = 3;
    for(i = 4; i < 32; i++) {
	a[i] = i;
    }
    disable_mar();
    return 0;
}

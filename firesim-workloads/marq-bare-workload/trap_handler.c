#include <stdint.h>
#include <stdio.h>
#include <marq.h>

void* handle_trap(uint64_t mepc, uint64_t mcause, uint64_t mtval,
		uint64_t sp) {

    uint64_t irq = mcause & ~(1LL << 63);
    if (irq == 16) {
	printf("Flushing mar fifo on interrupt\n");
	flush_mar_fifo();
    }

    return mepc;
}

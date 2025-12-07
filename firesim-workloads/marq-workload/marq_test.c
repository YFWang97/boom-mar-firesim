#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include "boom_csr.h"

#define MAR_FIFO_DEPTH 32

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

#define is_load(entry)  ((entry.flags & ld) != 0)
#define is_store(entry) ((entry.flags & st) != 0)
#define is_amo(entry)   ((entry.flags & amo) != 0)
#define is_hella(entry) ((entry.flags & hella) != 0)

int main (void) {

    static const char filename[] = "/dev/boom_csr";
    int boom_csr_fd;

    if ((boom_csr_fd = open(filename, O_RDWR)) == -1) {
	    fprintf(stderr, "could not open %s\n", filename);
	    return -1;
    }

    boom_csr_op_t vla;

    int a[64];
    register int i;

    printf("Int size is %ld\n", sizeof(int));
    for (i = 0; i < 4; i++)
	printf("C_ADDR: 0x%p\n", (a + i));

    printf("C_ADDR: 0x%p\n", (a + 63));

    vla.id = BOOM_ENABLE;
    if (ioctl(boom_csr_fd, BOOM_CSR_WRITE, &vla)) {
	    perror("ioctl(BOOM_CSR_WRITE) failed");
    }

    a[0] = 0;
    a[1] = 1;
    a[2] = 2;
    a[3] = 3;
    for(i = 4; i < 32; i++) {
	a[i] = i;
    }

    vla.id = BOOM_DISABLE;
    if (ioctl(boom_csr_fd, BOOM_CSR_WRITE, &vla)) {
	    perror("ioctl(BOOM_CSR_WRITE) failed");
    }

    // Original Flush MAR FIFO
    register int access_i_orig, search_range;
    static int access_i = 0;
    fifo_read_t read_status;
    mar_entry entry;

    access_i_orig = access_i;
    search_range = MAR_FIFO_DEPTH;
    do {
        vla.id = BOOM_READ_FIFO;
	if (ioctl(boom_csr_fd, BOOM_CSR_READ, &vla)) {
	    perror("ioctl(BOOM_CSR_READ) failed");
	}
    	entry.addr = vla.data;
	if (ioctl(boom_csr_fd, BOOM_CSR_READ, &vla)) {
	    perror("ioctl(BOOM_CSR_READ) failed");
	}
    	entry.pc = vla.data;
	if (ioctl(boom_csr_fd, BOOM_CSR_READ, &vla)) {
	    perror("ioctl(BOOM_CSR_READ) failed");
	}
    	entry.flags = (uint8_t)vla.data & 0xF;

    	read_status = read_success;
    	if (!entry.addr)
    	    read_status = read_failed;
    	else if (access_i == MAR_FIFO_DEPTH - 1)
    	    read_status = read_end;	
    	access_i = (access_i + 1) % MAR_FIFO_DEPTH;

        fprintf(stdout, "%lx: 0x%lx, LD: %d, ST: %d, AMO: %d, HELLA: %d\n", 
			entry.pc, entry.addr, 
			is_load(entry), is_store(entry), 
			is_amo(entry), is_hella(entry));
    } while (read_status == read_success); 
    return 0;
}

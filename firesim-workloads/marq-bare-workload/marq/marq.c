#include "marq.h"
#include <stdio.h>

static int access_i = 0;
bool enabled = false;

PARAM_1_SAFE_IMPL(blacklist_addr, void, void*, addr)
{
    write_csr(BLACKLIST_FIXED, addr);
}

/* Returns: read from Fifo */
PARAM_1_SAFE_IMPL(read_mar_fifo, int, mar_entry*, entry)
{
    fifo_read_t read_status;

    entry->addr = read_csr(CSR_MAR_READ_FIFO);
    entry->pc = read_csr(CSR_MAR_READ_FIFO);
    entry->flags = (uint8_t)read_csr(CSR_MAR_READ_FIFO) & 0xF;

    read_status = read_success;
    if (!entry->addr)
	read_status = read_failed;
    else if (access_i == MAR_FIFO_DEPTH - 1)
	read_status = read_end;	
    access_i = (access_i + 1) % MAR_FIFO_DEPTH;

    return (int)read_status;
}

/* Returns: Number of entries read */
PARAM_0_SAFE_IMPL(flush_mar_fifo, int)
{
    register int access_i_orig, search_range;
    fifo_read_t read_status;
    mar_entry entry;

    access_i_orig = access_i;
    search_range = MAR_FIFO_DEPTH;
#ifdef DEBUG
    fprintf(stdout, "access_i: %d, FIFO_DEPTH: %d\n", access_i, MAR_FIFO_DEPTH);
#endif

    do {
	read_status = read_mar_fifo(&entry);
        fprintf(stdout, "%lx: 0x%lx, LD: %d, ST: %d, AMO: %d, HELLA: %d\n", 
			entry.pc, entry.addr, 
			is_load(entry), is_store(entry), 
			is_amo(entry), is_hella(entry));
    } while (read_status == read_success); 

    return search_range - access_i_orig;
};


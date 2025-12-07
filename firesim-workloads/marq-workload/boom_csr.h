#ifndef _BOOM_CSR_H
#define _BOOM_CSR_H

#include <linux/ioctl.h>

typedef struct boom_csr_op {
    uint32_t id;
    uint32_t padding;
    uint64_t data;
} boom_csr_op_t;

enum boom_csr_id {
	BOOM_ENABLE = 0,
	BOOM_DISABLE,
	BOOM_MODE_INTERRUPT,
	BOOM_MODE_SAMPLE,
	BOOM_READ_FIFO,
};

#define BOOM_CSR_MAGIC 0xBC

#define BOOM_CSR_WRITE _IOW(BOOM_CSR_MAGIC, 1, boom_csr_op_t) 
#define BOOM_CSR_READ  _IOR(BOOM_CSR_MAGIC, 2, boom_csr_op_t)

#endif

obj-m := boom_csr.o

KDIR := /usr/src/linux-riscv-5.15-headers-5.15.0-1018

module:
	${MAKE} -C ${KDIR} M=$(PWD) modules

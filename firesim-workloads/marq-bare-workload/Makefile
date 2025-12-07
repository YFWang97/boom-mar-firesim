TARGET = riscv64-unknown-elf

GCC = $(TARGET)-gcc

ARCH = rv64imafdc
ABI = lp64d
ARCHFLAGS = -march=$(ARCH) -mabi=$(ABI)

CFLAGS = $(OPT)

CFLAGS += -std=gnu99 -fno-common -fno-builtin-printf -Wall
CFLAGS += $(ARCHFLAGS)
LDFLAGS = -static

ifeq ($(CORE),Boom)
	CFLAGS += -D=BOOM
endif

include ./libgloss.mk

MARQ_DEPS = marq.o trap_handler.o

marq.o: marq/marq.c
	$(GCC) $(CFLAGS) -c marq/marq.c -o marq.o

trap_handler.o: trap_handler.c marq.o

%.o: %.c
	$(GCC) $(CFLAGS) -c $< -o $@ -I./marq

marq_test: marq_test.o $(libgloss) $(MARQ_DEPS)
	$(GCC) $(LDFLAGS) $< $(MARQ_DEPS) -o $@
	cp marq_test marq_test.riscv

clean:
	rm -f *.o
	rm -f *.dump
	rm -f *.riscv
	rm -f marq_test
	rm -rf libgloss

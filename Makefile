obj-m += kprochide.o
kprochide-y := source.o readpid.o

KERNELDIR ?= ~/workspace/buildroot/output/build/linux-4.19.98
PWD       := $(shell pwd)

debug:
	$(MAKE) -C $(KERNELDIR) M=$(PWD) modules
release:
	$(MAKE) -C $(rm -rf *.o *~ core .depend .*.cmd *.ko *.mod.c .tmp_versions debugKERNELDIR) M=$(PWD) modules
clean:
	rm -rf *.o *~ core .depend .*.cmd *.ko *.mod.c .tmp_versions debug

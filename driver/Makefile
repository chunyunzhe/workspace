ifneq ($(KERNELRELEASE),)
	obj-m :=globalmem.o
	#obj-m +=subhello.o
#obj0-m := subhello.o
else
	KERNELDIR ?=/lib/modules/$(shell uname -r)/build
PWD := $(shell pwd)
#all: Hello Subhello
all:globalmem 
#Hello:
	#make -C $(KERNELDIR) M=$(PWD) modules
#Subhello:
	#make -C $(KERNELDIR) M=$(PWD) modules
globalmem:
	make -C $(KERNELDIR) M=$(PWD) modules
endif
clean:
	rm *.o *.ko Module.symvers \
		*.mod.c *.order \

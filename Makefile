KERNEL_SRC:= /lib/modules/$(shell uname -r)/build

obj-m += echo.o
obj-m += hello-usb.o
obj-m += simple.o

all:
	make -C $(KERNEL_SRC) M=$(PWD) modules

clean:
	make -C $(KERNEL_SRC) M=$(PWD) clean

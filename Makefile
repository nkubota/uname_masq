COMMIT_REV ?= $(shell git describe  --always --abbrev=12)
KERNEL_SOURCE_VERSION ?= $(shell uname -r)
KERNEL_TREE ?= /lib/modules/$(KERNEL_SOURCE_VERSION)/build

EXTRA_CFLAGS += -I$(KERNEL_TREE)/include/ -I$(KERNEL_TREE)/include/linux 

obj-m += uname_masq.o

.PHONY: all
all: modules

.PHONY: modules
modules:
	make -C $(KERNEL_TREE) M=$(PWD) modules V=1

.PHONY: modules_install
modules_install: modules
	install -o root -g root -m 0755 -d /lib/modules/$(KERNEL_SOURCE_VERSION)/extra/uname_masq/
	depmod -a

.PHONY: install
install: modules_install

.PHONY: clean
clean:
	make -C $(KERNEL_TREE) M=$(PWD) clean


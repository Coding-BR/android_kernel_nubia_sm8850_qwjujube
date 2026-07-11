# Makefile for MMRM driver
REPOSITORY_ROOT := $(abspath $(CURDIR)/../../../..)
KERNEL_SRC ?= $(REPOSITORY_ROOT)/kernel_platform/common
OUT_DIR ?= $(REPOSITORY_ROOT)/out/mmrm-driver

all:
	$(MAKE) -C $(KERNEL_SRC) M=$(shell pwd) O=$(OUT_DIR) modules

clean:
	$(MAKE) -C $(KERNEL_SRC) M=$(shell pwd) O=$(OUT_DIR) clean

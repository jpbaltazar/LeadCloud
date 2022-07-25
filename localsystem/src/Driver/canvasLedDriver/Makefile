ddname = canvasLedDriver

obj-m := $(ddname).o
#KDIR := /home/pedro/buildroot/buildroot-2020.08/output/build/linux-custom
#KDIR := /lib/modules/$(shell uname -r)/build
KDIR := /home/zed4805/buildroot/buildroot-2021.08/output/build/linux-custom

#ARCH ?= arm
#ARCH ?= x86
ARCH ?= arm64

#CROSS_COMPILE ?= /home/pedro/buildroot/buildroot-2020.08/output/host/bin/arm-buildroot-linux-uclibcgnueabihf-
CROSS_COMPILE ?= /home/zed4805/buildroot/buildroot-2021.08/output/host/bin/aarch64-buildroot-linux-uclibc-

export ARCH CROSS_COMPILE

KBUILD_TARGETS	:= clean help modules modules_install

SSHUSER = root
IP = 10.42.0.66

EXTDIR = /root/Driver#/$(ddname)

.PHONY: all $(KBUILD_TARGETS)

all: modules
$(KBUILD_TARGETS):
	#$(MAKE) -C $(KDIR) M=$(shell pwd) $@ EXTRA_CFLAGS="$(MY_CFLAGS)"
	$(MAKE) -C $(KDIR) M=$(shell pwd) $@

send:
	scp -r ../$(ddname) $(SSHUSER)@$(IP):$(EXTDIR)
DISK		:=	disk.rx2
DISKROOT	:=	diskroot
DISKLAYOUT	:=	disk.lyt

MKDISK		:=	mkdisk/build/mkdisk

MON		:=	mon/build/mon.bin
BOOT		:=	mon/build/boot.bin
NUCLEUS		:=	nucleus/build/nucleus
SYSLIB		:=	syslib/build/syslib
STARTUP		:=	programs/startup/build/startup
LOGON		:=	programs/logon/build/logon
TSO		:=	programs/tso/build/tso
ISPF		:=	programs/ispf/build/ispf
CLOCK		:=	programs/clock/build/clock
CLOCKCFG	:=	programs/clockcfg/build/clockcfg

.PHONY:	all clean modules

all: $(DISK)

$(DISK): modules
	@$(MKDISK) -b $(BOOT) -m $(MON) -l $(DISKLAYOUT) -o $@

modules:
	@$(MAKE) -C mkdisk all --no-print-directory
	@$(MAKE) -C lsi-ld all --no-print-directory
	@$(MAKE) -C lsi-nm all --no-print-directory
	@$(MAKE) -C mon all --no-print-directory
	@$(MAKE) -C nucleus all --no-print-directory
	@$(MAKE) -C syslib all --no-print-directory
	@$(MAKE) -C programs/startup all --no-print-directory
	@$(MAKE) -C programs/logon all --no-print-directory
	@$(MAKE) -C programs/tso all --no-print-directory
	@$(MAKE) -C programs/ispf all --no-print-directory
	@$(MAKE) -C programs/clock all --no-print-directory
	@$(MAKE) -C programs/clockcfg all --no-print-directory
	@$(MAKE) -C programs/sim all --no-print-directory

clean:
	@$(MAKE) -C mkdisk clean --no-print-directory
	@$(MAKE) -C lsi-ld clean --no-print-directory
	@$(MAKE) -C lsi-nm clean --no-print-directory
	@$(MAKE) -C mon clean --no-print-directory
	@$(MAKE) -C nucleus clean --no-print-directory
	@$(MAKE) -C syslib clean --no-print-directory
	@$(MAKE) -C programs/startup clean --no-print-directory
	@$(MAKE) -C programs/logon clean --no-print-directory
	@$(MAKE) -C programs/tso clean --no-print-directory
	@$(MAKE) -C programs/ispf clean --no-print-directory
	@$(MAKE) -C programs/clock clean --no-print-directory
	@$(MAKE) -C programs/clockcfg clean --no-print-directory
	@$(MAKE) -C programs/sim clean --no-print-directory
	@rm -f $(DISK)

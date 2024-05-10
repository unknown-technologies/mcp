DISK		:=	disk.rx2
DISKROOT	:=	diskroot

MKDISK		:=	mkdisk/build/mkdisk

MON		:=	mon/build/mon.bin
BOOT		:=	mon/build/boot.bin
NUCLEUS		:=	nucleus/build/nucleus
SYSLIB		:=	syslib/build/syslib
ISPF		:=	programs/ispf/build/ispf
CLOCK		:=	programs/clock/build/clock
CLOCKCFG	:=	programs/clockcfg/build/clockcfg

.PHONY:	all clean modules

all: $(DISK)

$(DISK): modules
	@cp $(NUCLEUS) $(DISKROOT)/SYS1.NUCLEUS
	@cp $(SYSLIB) $(DISKROOT)/SYS1.SYSLIB
	@cp $(ISPF) $(DISKROOT)/SYS1.ISPF
	@cp $(CLOCK) $(DISKROOT)/SYS1.PROG.CLOCK
	@cp $(CLOCKCFG) $(DISKROOT)/SYS1.PROG.CLOCKCFG
	@$(MKDISK) -b $(BOOT) -m $(MON) -d $(DISKROOT) -o $@

modules:
	@$(MAKE) -C mkdisk all --no-print-directory
	@$(MAKE) -C lsi-ld all --no-print-directory
	@$(MAKE) -C lsi-nm all --no-print-directory
	@$(MAKE) -C mon all --no-print-directory
	@$(MAKE) -C nucleus all --no-print-directory
	@$(MAKE) -C syslib all --no-print-directory
	@$(MAKE) -C programs/ispf all --no-print-directory
	@$(MAKE) -C programs/clock all --no-print-directory
	@$(MAKE) -C programs/clockcfg all --no-print-directory

clean:
	@$(MAKE) -C mkdisk clean --no-print-directory
	@$(MAKE) -C lsi-ld clean --no-print-directory
	@$(MAKE) -C lsi-nm clean --no-print-directory
	@$(MAKE) -C mon clean --no-print-directory
	@$(MAKE) -C nucleus clean --no-print-directory
	@$(MAKE) -C syslib clean --no-print-directory
	@$(MAKE) -C programs/ispf clean --no-print-directory
	@$(MAKE) -C programs/clock clean --no-print-directory
	@$(MAKE) -C programs/clockcfg clean --no-print-directory
	@rm -f $(DISK)
	@rm -f $(DISKROOT)/SYS1.NUCLEUS
	@rm -f $(DISKROOT)/SYS1.SYSLIB
	@rm -f $(DISKROOT)/SYS1.ISPF
	@rm -f $(DISKROOT)/SYS1.PROG.CLOCK
	@rm -f $(DISKROOT)/SYS1.PROG.CLOCKCFG

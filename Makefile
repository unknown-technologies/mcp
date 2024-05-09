DISK		:=	disk.rx2
DISKROOT	:=	diskroot

MKDISK		:=	mkdisk/build/mkdisk

MON		:=	mon/build/mon.bin
BOOT		:=	mon/build/boot.bin
NUCLEUS		:=	nucleus/build/nucleus
SYSLIB		:=	syslib/build/syslib
ISPF		:=	programs/ispf/build/ispf

.PHONY:	all clean modules

all: $(DISK)

$(DISK): modules
	@cp $(NUCLEUS) $(DISKROOT)/SYS1.NUCLEUS
	@cp $(SYSLIB) $(DISKROOT)/SYS1.SYSLIB
	@cp $(ISPF) $(DISKROOT)/SYS1.ISPF
	@$(MKDISK) -b $(BOOT) -m $(MON) -d $(DISKROOT) -o $@

modules:
	@$(MAKE) -C mkdisk all --no-print-directory
	@$(MAKE) -C lsi-ld all --no-print-directory
	@$(MAKE) -C lsi-nm all --no-print-directory
	@$(MAKE) -C mon all --no-print-directory
	@$(MAKE) -C nucleus all --no-print-directory
	@$(MAKE) -C syslib all --no-print-directory
	@$(MAKE) -C programs/ispf all --no-print-directory

clean:
	@$(MAKE) -C mkdisk clean --no-print-directory
	@$(MAKE) -C lsi-ld clean --no-print-directory
	@$(MAKE) -C lsi-nm clean --no-print-directory
	@$(MAKE) -C mon clean --no-print-directory
	@$(MAKE) -C nucleus clean --no-print-directory
	@$(MAKE) -C syslib clean --no-print-directory
	@$(MAKE) -C programs/ispf clean --no-print-directory
	@rm -f $(DISK)

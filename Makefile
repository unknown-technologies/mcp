DISK		:=	disk.rx2
DISKROOT	:=	diskroot

MKDISK		:=	mkdisk/build/mkdisk

MON		:=	mon/build/mon.bin
BOOT		:=	mon/build/boot.bin
NUCLEUS		:=	nucleus/build/nucleus

.PHONY:	all clean modules

all: $(DISK)

$(DISK): modules
	@cp $(NUCLEUS) $(DISKROOT)/SYS1.NUCLEUS
	@$(MKDISK) -b $(BOOT) -m $(MON) -d $(DISKROOT) -o $@

modules:
	@$(MAKE) -C mkdisk all --no-print-directory
	@$(MAKE) -C lsi-ld all --no-print-directory
	@$(MAKE) -C mon all --no-print-directory
	@$(MAKE) -C nucleus all --no-print-directory

clean:
	@$(MAKE) -C mkdisk clean --no-print-directory
	@$(MAKE) -C lsi-ld clean --no-print-directory
	@$(MAKE) -C mon clean --no-print-directory
	@$(MAKE) -C nucleus clean --no-print-directory
	@rm -f $(DISK)

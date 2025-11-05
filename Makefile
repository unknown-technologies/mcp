DISK		:=	disk.rx2
DISKROOT	:=	diskroot
DISKLAYOUT	:=	disk.lyt

MKDISK		:=	mkdisk/build/mkdisk

MON		:=	mon/build/mon.lda
BOOT		:=	mon/build/boot.lda

.PHONY:	all clean modules

all: $(DISK)

$(DISK): modules
	@$(MKDISK) -b $(BOOT) -m $(MON) -c $(DISKLAYOUT) -l MCP11 -o $@

modules:
	@$(MAKE) -C mkdisk all --no-print-directory
	@$(MAKE) -C lsi-ld all --no-print-directory
	@$(MAKE) -C lsi-nm all --no-print-directory
	@$(MAKE) -C lsi-ldadump all --no-print-directory
	@$(MAKE) -C mon all --no-print-directory
	@$(MAKE) -C nucleus all --no-print-directory
	@$(MAKE) -C modules/loadlib all --no-print-directory
	@$(MAKE) -C modules/autolib all --no-print-directory
	@$(MAKE) -C modules/fsinit all --no-print-directory
	@$(MAKE) -C modules/fslocate all --no-print-directory
	@$(MAKE) -C modules/fslist all --no-print-directory
	@$(MAKE) -C lib/syslib all --no-print-directory
	@$(MAKE) -C lib/ispf all --no-print-directory
	@$(MAKE) -C lib/tso all --no-print-directory
	@$(MAKE) -C programs/startup all --no-print-directory
	@$(MAKE) -C programs/logon all --no-print-directory
	@$(MAKE) -C programs/tso all --no-print-directory
	@$(MAKE) -C programs/ispf all --no-print-directory
	@$(MAKE) -C programs/clock all --no-print-directory
	@$(MAKE) -C programs/workspace all --no-print-directory
	@$(MAKE) -C programs/calc all --no-print-directory
	@$(MAKE) -C programs/xcalc all --no-print-directory
	@$(MAKE) -C programs/system all --no-print-directory
	@$(MAKE) -C programs/settings all --no-print-directory
	@$(MAKE) -C programs/sim all --no-print-directory
	@$(MAKE) -C commands/shell all --no-print-directory
	@$(MAKE) -C commands/tsologon all --no-print-directory
	@$(MAKE) -C commands/logoff all --no-print-directory
	@$(MAKE) -C commands/system all --no-print-directory
	@$(MAKE) -C commands/showlog all --no-print-directory
	@$(MAKE) -C commands/listcat all --no-print-directory
	@$(MAKE) -C commands/listds all --no-print-directory
	@$(MAKE) -C commands/printds all --no-print-directory
	@$(MAKE) -C commands/write all --no-print-directory
	@$(MAKE) -C commands/help all --no-print-directory

clean:
	@$(MAKE) -C mkdisk clean --no-print-directory
	@$(MAKE) -C lsi-ld clean --no-print-directory
	@$(MAKE) -C lsi-nm clean --no-print-directory
	@$(MAKE) -C lsi-ldadump clean --no-print-directory
	@$(MAKE) -C mon clean --no-print-directory
	@$(MAKE) -C nucleus clean --no-print-directory
	@$(MAKE) -C modules/loadlib clean --no-print-directory
	@$(MAKE) -C modules/autolib clean --no-print-directory
	@$(MAKE) -C modules/fsinit clean --no-print-directory
	@$(MAKE) -C modules/fslocate clean --no-print-directory
	@$(MAKE) -C modules/fslist clean --no-print-directory
	@$(MAKE) -C lib/syslib clean --no-print-directory
	@$(MAKE) -C lib/ispf clean --no-print-directory
	@$(MAKE) -C lib/tso clean --no-print-directory
	@$(MAKE) -C programs/startup clean --no-print-directory
	@$(MAKE) -C programs/logon clean --no-print-directory
	@$(MAKE) -C programs/tso clean --no-print-directory
	@$(MAKE) -C programs/ispf clean --no-print-directory
	@$(MAKE) -C programs/clock clean --no-print-directory
	@$(MAKE) -C programs/workspace clean --no-print-directory
	@$(MAKE) -C programs/calc clean --no-print-directory
	@$(MAKE) -C programs/xcalc clean --no-print-directory
	@$(MAKE) -C programs/system clean --no-print-directory
	@$(MAKE) -C programs/settings clean --no-print-directory
	@$(MAKE) -C programs/sim clean --no-print-directory
	@$(MAKE) -C commands/shell clean --no-print-directory
	@$(MAKE) -C commands/tsologon clean --no-print-directory
	@$(MAKE) -C commands/logoff clean --no-print-directory
	@$(MAKE) -C commands/system clean --no-print-directory
	@$(MAKE) -C commands/showlog clean --no-print-directory
	@$(MAKE) -C commands/listcat clean --no-print-directory
	@$(MAKE) -C commands/listds clean --no-print-directory
	@$(MAKE) -C commands/printds clean --no-print-directory
	@$(MAKE) -C commands/write clean --no-print-directory
	@$(MAKE) -C commands/help clean --no-print-directory
	@rm -f $(DISK)

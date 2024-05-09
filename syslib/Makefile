#!/bin/make
export	PREFIX	:=	pdp11-aout-
export	CC	:=	$(PREFIX)gcc
#export	LD	:=	$(CC)
export	OBJCOPY	:=	$(PREFIX)objcopy
export	NM	:=	$(PREFIX)nm
export	SIZE	:=	$(PREFIX)size

#-------------------------------------------------------------------------------
.SUFFIXES:
#-------------------------------------------------------------------------------
TARGET		:=	syslib
INCLUDES	:=	include ../common/include
SOURCES		:=	src src/libgcc ../common/src
BUILD		:=	build

ARCHFLAGS	:=	-m40 -msoft-float -mno-split -Wa,-mkd11f -Wa,-meis
OPTFLAGS	:=	-Os

CFLAGS		:=	$(OPTFLAGS) -Wall -std=c99 \
			-fwrapv -fwrapv-pointer -fzero-initialized-in-bss \
			$(ARCHFLAGS) -nostdlib -ffreestanding \
			-DREVISION=\"$(REVISION)\" \
			-DCOMMIT=\"$(COMMIT)\" \
			-DBUILDDATE=\"$(BUILDDATE)\" \
			-DBUILDTIME=\"$(BUILDTIME)\" \
			$(INCLUDE)

LDFLAGS		:=	$(OPTFLAGS) -Wl,-x -Wl,--gc-sections \
			-fwrapv -fwrapv-pointer -fzero-initialized-in-bss \
			$(ARCHFLAGS) -nostdlib -ffreestanding \
			-N -e _start

CFILES		:=	$(foreach dir,$(SOURCES),$(notdir $(wildcard $(dir)/*.c)))
SFILES		:=	$(foreach dir,$(SOURCES),$(notdir $(wildcard $(dir)/*.s)))

ifneq ($(BUILD),$(notdir $(CURDIR)))
#-------------------------------------------------------------------------------
export	LD	:=	$(CURDIR)/../lsi-ld/build/lsi-ld
export	DEPSDIR	:=	$(CURDIR)/$(BUILD)
export	OFILES	:=	$(SFILES:.s=.o) $(CFILES:.c=.o)
export	VPATH	:=	$(foreach dir,$(SOURCES),$(CURDIR)/$(dir)) $(CURDIR)
export	INCLUDE	:=	$(foreach dir,$(INCLUDES),-I$(CURDIR)/$(dir)) \
				-I$(CURDIR)/$(BUILD)

export	REVISION :=	$(shell git rev-list --count HEAD)
export	COMMIT   :=	$(shell git rev-parse --short HEAD)
export	BUILDDATE:=	$(shell date -u +"%Y-%m-%d")
export	BUILDTIME:=	$(shell date -u +"%H:%M:%S")

.PHONY: $(BUILD) clean all

$(BUILD):
	@[ -d $@ ] || mkdir -p $@
	@$(MAKE) --no-print-directory -C $(BUILD) -f $(CURDIR)/Makefile

all: $(BUILD)

clean:
	@echo "[CLEAN]"
	@rm -rf $(BUILD) $(OFILES)

else

#-------------------------------------------------------------------------------
# main target
#-------------------------------------------------------------------------------
.PHONY: all

all: $(TARGET)

%.o %.s: %.c
	@echo "[CC]    $(notdir $@)"
	@$(CC) -MMD -MP -MF $(DEPSDIR)/$*.d $(CFLAGS) -c $< -o $@
	@$(CC) -S $(CFLAGS) -fverbose-asm -o $(@:.o=.s) $< # create assembly file

%.o: %.s
	@echo "[AS]    $(notdir $@)"
	@$(CC) -MMD -MP -MF $(DEPSDIR)/$*.d $(CFLAGS) -c $< -o $@

$(TARGET).aout: $(OFILES)
	@echo "[LD]    $(notdir $@)"
	@$(LD) $(LDFLAGS) $(OFILES) -o $@ -Wl,-Map=$(@:.aout=.map) -lgcc

#$(TARGET): $(TARGET).aout
#	@echo "[LDA]   $(notdir $@)"
#	@aout2lda --aout $< --lda $@ --data-align 2 --text 0x800

$(TARGET): $(OFILES)
	@echo "[LD]    $(notdir $@)"
	@$(LD) -l -o $@ -m $@.map $(OFILES)

-include $(DEPSDIR)/*.d

#-------------------------------------------------------------------------------
endif
#-------------------------------------------------------------------------------

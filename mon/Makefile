#!/bin/make
export	AS	:=	lsi-as

#-------------------------------------------------------------------------------
.SUFFIXES:
#-------------------------------------------------------------------------------
SOURCES		:=	src
BUILD		:=	build

SFILES		:=	$(foreach dir,$(SOURCES),$(notdir $(wildcard $(dir)/*.s)))

ifneq ($(BUILD),$(notdir $(CURDIR)))
#-------------------------------------------------------------------------------
export	DEPSDIR	:=	$(CURDIR)/$(BUILD)
export	BINFILES:=	$(SFILES:.s=.bin)
export	VPATH	:=	$(foreach dir,$(SOURCES),$(CURDIR)/$(dir)) $(CURDIR)
export	OUTPUT	:=	$(CURDIR)/$(TARGET)

.PHONY: $(BUILD) clean all

$(BUILD):
	@[ -d $@ ] || mkdir -p $@
	@$(MAKE) --no-print-directory -C $(BUILD) -f $(CURDIR)/Makefile

clean:
	@echo "[CLEAN]"
	@rm -rf $(BUILD) $(BINFILES)

all: $(BUILD)

else

#-------------------------------------------------------------------------------
# main target
#-------------------------------------------------------------------------------
.PHONY: all

all: $(BINFILES)

%.bin: %.s
	@echo "[AS]    $(notdir $@)"
	@$(AS) -q $< $@

#-------------------------------------------------------------------------------
endif
#-------------------------------------------------------------------------------

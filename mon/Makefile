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
export	LDAFILES:=	$(SFILES:.s=.lda)
export	VPATH	:=	$(foreach dir,$(SOURCES),$(CURDIR)/$(dir)) $(CURDIR)
export	OUTPUT	:=	$(CURDIR)/$(TARGET)

export	LDADUMP	:=	$(CURDIR)/../lsi-ldadump/build/lsi-ldadump

.PHONY: $(BUILD) clean all

$(BUILD):
	@[ -d $@ ] || mkdir -p $@
	@$(MAKE) --no-print-directory -C $(BUILD) -f $(CURDIR)/Makefile

clean:
	@echo "[CLEAN]"
	@rm -rf $(BUILD) $(BINFILES) $(LDAFILES)

all: $(BUILD)

else

#-------------------------------------------------------------------------------
# main target
#-------------------------------------------------------------------------------
.PHONY: all

all: $(BINFILES) $(LDAFILES)

%.lda: %.s
	@echo "[AS]    $(notdir $@)"
	@$(AS) -q $< $@

%.bin: %.lda
	@echo "[DUMP]  $(notdir $@)"
	@$(LDADUMP) -q $< $@

#-------------------------------------------------------------------------------
endif
#-------------------------------------------------------------------------------

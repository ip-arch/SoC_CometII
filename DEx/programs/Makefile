###############################################################################
# Makefile for exercise in 17th PARTHENON Workshop
# (GNU make is required)
# Usage:
#      To compile all .sc files   : make
#      To compile .sc to .asm     : make <xxxx.hex>
#      To write .hex file to FPGA : make <xxxx.write>
#          The contents of xxx.hex is stored in instruction memory (imem)
#      To read .hex file from FPGA: make FILE=<xxx.hex> <xxx.hex>
#          The contents of data memory (dmem) is stored in xxx.hex
###############################################################################
# 2010-09-12 : Initial version by Y.Okuyama
# 2016-03-21 : Modified for CASLII-SoC by T.Suzuki
###############################################################################
COMET2DIR	= ../../comet2
CASL2CDIR	= ../../casl2c
CASLCOMPDIR	= $(CASL2CDIR)/caslcomp
CASLASMDIR	= $(CASL2CDIR)/caslasm
CASL2C		= $(CASL2CDIR)/casl2c
CASLCOMP	= $(CASLCOMPDIR)/caslcomp
CASLASM		= $(CASLASMDIR)/caslasm
HEX2INTEL	= $(COMET2DIR)/hex2intel.py
INPUTS		= $(wildcard *.c2c)
OUTPUTS		= $(INPUTS:.c2c=.hex)
FILE=

all: $(OUTPUTS) $(CASLCOMP) $(CASLASM)

tools: $(CASLCOMP) $(CASLASM)
clean-tools:
	(cd $(CASLASMDIR); make clean)
	cd $(CASLCOMDIR); make clean
	-rm  $(CASLCOMP).exe $(CASLASM).exe

$(CASLASM):
	cd $(CASLASMDIR); make

$(CASLCOMP):
	cd $(CASLCOMPDIR); make

$(FILE):
	quratus_stp -t ./mem_read dmem $(FILE)

download:
	cd ../rtl; make download
clean:
	rm -f *.asm *.cas *.hex *.raw *~

################################
# suffix rules
################################
.SUFFIXES:  .hex .cas .c2c .raw .write

.c2c.cas:$(CASLCOMP)
	$(CASLCOMP) $< | tee $@
.cas.raw:$(CASLASM)
	$(CASLASM) -compiled $< -o $@
.raw.hex:$(HEX2INTEL)
	python3 $(HEX2INTEL) $< $@
.hex.write:
	quartus_stp -t ./mem_write cmem $<

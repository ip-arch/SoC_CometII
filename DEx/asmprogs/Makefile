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
###############################################################################
#ALTERADIR	= /cygdrive/c/altera/13.0sp1/quartus/bin
#PATH		= /bin:/usr/local/bin:$(ALTERADIR)
COMET2DIR		= ../../comet2
COMET2ASMDIR	= ../../casl2c/caslasm
#SNXC		= $(SNXCDIR)/snxc
COMET2ASM	= $(COMET2ASMDIR)/caslasm
HEX2INTEL	= $(COMET2DIR)/hex2intel.py
INPUTS		= $(wildcard *.cas)
OUTPUTS		= $(INPUTS:.cas=.hex)
FILE=


all: $(OUTPUTS) #$(SNXC) $(SNXASM)

tools: $(COMET2ASM)# $(SNXASM)
clean-tools:
#	(cd $(SNXASMDIR); make clean)
#	cd $(SNXCDIR); make clean
	-rm  $(SNXC) $(SNXASM)

$(SNXASM):
	cd $(SNXASMDIR); make

$(SNXC):
	cd $(SNXCDIR); make

download:
	cd ../rtl; make download
clean:
	rm -f *.asm *.hex *~

################################
# suffix rules
################################
.SUFFIXES:  .hex .cas .sc .raw .write

.sc.asm:$(SNXC)
	$(SNXC) < $< | tee $@
.cas.raw:$(COMET2ASM)
	$(COMET2ASM) -compiled $< -o $@
.raw.hex:$(HEX2INTEL)
	python3 $(HEX2INTEL)  $< $@
.hex.write:
	quartus_stp -t ./mem_write cmem $<

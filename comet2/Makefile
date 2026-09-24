###############################################################################
# Makefile for COMETII simulation
# makefile for cometII RTL by N.Shimizu
# makefile for casl2 tool chain by T.Suzuki
# Usage:
# make TARGET=target
# Set target c2c/cas file to $(TARGET) without file extention
# This file make $(TARGET).mem and system.vvp
###############################################################################
# 23rd Mar. 2016 : Initial version by Y.Okuyama
###############################################################################
COMET2DIR	= ./
CASL2CDIR	= ../casl2c

CASLCOMPDIR	= $(CASL2CDIR)/caslcomp
CASLASMDIR	= $(CASL2CDIR)/caslasm

CASL2C		= $(CASL2CDIR)/casl2c
CASLCOMP	= $(CASLCOMPDIR)/caslcomp.exe
CASLASM		= $(CASLASMDIR)/caslasm.exe

HEX2INTEL	= $(COMET2DIR)/hex2intel.py
# for system
SRC=comet2.nsl comet2.h system.nsl
TARGET=test01
VERI=system.v
VCD=system.vcd
VVP=system.vvp
CLEAN=system.v system.vvp system.vcd $(TARGET).mem $(TARGET).mem $(TARGET)*~ comet2.mem

all: $(VCD)

clean:
	-rm -f $(CLEAN)

$(VCD): $(VVP) $(TARGET).mem
	cp $(TARGET).mem comet2.mem
	vvp $(VVP)

$(CASLASM):
	cd $(CASLASMDIR); make

$(CASLCOMP):
	cd $(CASLCOMPDIR); make

$(VERI): $(SRC)
	nsl2vl -verisim2 -target system system.nsl

$(VVP): $(VERI)
	iverilog system.v -o $(VVP)

wave: $(VCD)
	gtkwave system.vcd system.sig

clean-tools:
	(cd $(CASLASMDIR); make clean)
	cd $(CASLCOMDIR); make clean
	-rm  $(CASLCOMP) $(CASLASM)

################################
# suffix rules
################################
.SUFFIXES: .cas .c2c .mem

.cas.mem: $(CASLASM)
	#**** Type make in casl2c dir ****
	$(CASLASM) -compiled $< -o $@

.c2c.cas: $(CASLCOMP)
	$(CASLCOMP) $< | tee $@

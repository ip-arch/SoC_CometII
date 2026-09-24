################################################################################
# Makefile : sof file generation using Quartus II
# Usage:
#		make compile for synthesis all files
#       make download for download .sof file to FPGA board
################################################################################
# 2011-09-12,13 Initial version by Y.Okuyama (based on PICO's makefile)
# 2012-10-08    CABLE is modified for linux environment
# 2013-03-20    Adopted for snx2013
# 2015-08-13    Adopted for comet2
################################################################################

ifndef SRCDIR
SRCDIR	= .
endif
#ALTERADIR	= /cygdrive/c/altera/13.0sp1/quartus/bin
#PATH		= $(ALTERADIR):/bin:/usr/local/bin
CPUDIR		= $(SRCDIR)/../../comet2
WORKDIR		= synth
VPATH		= $(SRCDIR):$(SRCDIR)/MEM:$(SRCDIR)/LCD:$(SRCDIR)/VGA:$(CPUDIR)

DESIGN		= DE0
NSL2VL    	= nsl2vl
NSLFLAGS  	= -O2 -neg_res -I$(SRCDIR) -I$(SRCDIR)/MEM -I$(SRCDIR)/LCD -I$(SRCDIR)/VGA -I$(CPUDIR) -I$(SRCDIR)/rtl/../../comet2
MKPROJ		= $(SRCDIR)/mkproj-$(DESIGN).tcl
#Q2SH		= $(ALTERADIR)/quartus_sh
#Q2PGM		= $(ALTERADIR)/quartus_pgm
Q2SH		= quartus_sh
Q2PGM		= quartus_pgm
CABLE		= "USB-Blaster"
PMODE		= JTAG

SRCS		= $(DESIGN).nsl HEX.nsl LED.nsl key.nsl sw.nsl \
		  comet2.nsl VGA_ctrl.nsl ps2in.nsl timer.nsl
VFILES 		= $(SRCS:%.nsl=%.v)
LIBS		= cmem.v \
		  LCD_controller.v dpram8x32.v \
		  RAM2P_16x4K.v ROM_8x4K.v
MIFS		= VGA/VRAM.mif VGA/CGROM.mif
RESULT		= result.txt

########

all:
	@if [ ! -d $(WORKDIR) ]; then \
		echo mkdir $(WORKDIR); \
		mkdir $(WORKDIR); \
	fi
	( cd $(WORKDIR); make -f ../Makefile SRCDIR=.. compile )

########

.SUFFIXES: .v .nsl

.nsl.v:
	$(NSL2VL) $(NSLFLAGS) $< -o $@

$(DESIGN).qsf: $(VFILES) $(LIBS)
	$(Q2SH) -t $(MKPROJ) -project $(DESIGN) $^

$(DESIGN).sof: $(DESIGN).qsf $(MIFS)
	$(Q2SH) --flow compile $(DESIGN)

########

compile: $(DESIGN).sof
	@echo "**** $(DESIGN).fit.summary" | tee -a $(RESULT)
	@cat $(DESIGN).fit.summary | tee -a $(RESULT)
	@echo "**** $(DESIGN).sta.rpt" | tee -a $(RESULT)
	@grep -2 "Info: Fmax" $(DESIGN).sta.rpt | tee -a $(RESULT)

download: config-n

config: all
	$(Q2PGM) -c $(CABLE) -m $(PMODE) -o "p;$(WORKDIR)/$(DESIGN).sof"
config-n: # without re-compil
	$(Q2PGM) -c $(CABLE) -m $(PMODE) -o "p;$(WORKDIR)/$(DESIGN).sof"

clean:
	rm -rf - $(WORKDIR)

########

$(DESIGN).v	: $(DESIGN).nsl comet2.h cmem.inc \
		  HEX.inc LED.inc key.inc sw.inc \
		  LCD_controller.inc VGA_ctrl.inc
HEX.v		: HEX.inc HEX.nsl
LED.v		: LED.inc LED.nsl
key.v		: key.inc key.nsl
sw.v		: sw.inc sw.nsl
VGA_ctrl.v	: VGA_ctrl.inc VGA_ctrl.nsl

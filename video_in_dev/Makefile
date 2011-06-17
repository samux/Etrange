SOFT       = soft/soft.elf

PLATFORM_DESC    =  platform_desc
SOCLIB_INCLUDES  =  -I. -I./display -I./wb_slave -I./video_gen -I./wb_master_module -I./dummy_wb_master -I./video_out -I./hdl/include

SOCLIB_CC_ARGS   = -j4  $(SOCLIB_INCLUDES) -t sccom

SOCLIB_CC = soclib-cc

SOCLIB = $(shell $(SOCLIB_CC) --getpath)
export SOCLIB

#Verilog compilation
VLOG = vlog +acc
VLOG_TOP = video_in.sv video_out.sv
VLOG_IN = video_in_read.sv video_in_store.sv fifo.sv ram.sv video_in.sv video_out_read.sv video_out_gen.sv
VLOG_SRC = $(VLOG_IN) $(VLOG_TOP)
VLOG_DAT = $(patsubst %.sv,work/%/_primary.dat,$(VLOG_SRC))

#SystemC wrappers for verilog modules
WRAP_DIR = hdl/include
WRAPPERS = $(patsubst %.sv, $(WRAP_DIR)/%.h, $(VLOG_TOP))
SCGENMOD = scgenmod

#wraps wires to bool and vectors to sc_uijnt
# 'scgenmod -help' for more options
SCGENMOD_OPT = -sc_uint -bool

vpath %.sv hdl/SystemVerilog

.PHONY: all soft soclibcomp vlogcompile soft_clean 

all:  soft soclibcomp vlogcompile

sim:
	vsim sc_main

work: 
	vlib work

$(WRAP_DIR):
	mkdir $@

soclibcomp: $(PLATFORM_DESC) $(WRAPPERS) | work
	$(SOCLIB_CC) -P $(SOCLIB_CC_ARGS) -p $<

vlogcompile: $(VLOG_DAT)

work/%/_primary.dat:%.sv | work
	$(VLOG) $<

$(WRAP_DIR)/%.h:work/%/_primary.dat | $(WRAP_DIR)
	$(SCGENMOD) $(SCGENMOD_OPT) $* > $@

soft:
	$(MAKE) -C soft

clean: soft_clean
	$(SOCLIB_CC) -P $(SOCLIB_CC_ARGS) $(SOCLIB_CC_ADD_ARGS) -x -o $(SIM_BINARY)
	rm -f tty.log
	rm -f vcd_traces.vcd
	rm -f $(OUTPUT_LOG)
	rm -rf work
	rm -f transcript vsim.wlf
	rm -rf $(WRAP_DIR)

soft_clean:
	$(MAKE) -C soft clean


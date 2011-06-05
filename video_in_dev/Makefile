SIM_BINARY = simulation.x
SOFT       = soft/soft.elf
OUTPUT_LOG = simu.log

PLATFORM_DESC    =  platform_desc
SOCLIB_INCLUDES  =  -I. -I./video_in -I./display -I./wb_slave -I./video_gen -I./wb_master_module -I./dummy_wb_master -I./video_out -I./video_calc
SOCLIB_CC_ARGS   = -j4 -p $(PLATFORM_DESC) $(SOCLIB_INCLUDES)

SOCLIB_CC = soclib-cc

SOCLIB = $(shell $(SOCLIB_CC) --getpath)
export SOCLIB

.PHONY: all soft simlog soft_clean $(SIM_BINARY)


all: test_soclib $(SIM_BINARY) soft top.cpp

soft: top.cpp
	$(MAKE) -C soft

test_soclib: top.cpp
	@test -z "$(SOCLIB)" && (\
	echo "You must have soclib-cc in your $$PATH" ; exit 1 ) || exit 0
	@test ! -z "$(SOCLIB)"

$(SIM_BINARY): $(PLATFORM_DESC)
	$(SOCLIB_CC) -P $(SOCLIB_CC_ARGS) $(SOCLIB_CC_ADD_ARGS) -o $@

simlog: $(SIM_BINARY) soft
	set -o pipefail ; SOCLIB_TTY=TERM ./$(SIM_BINARY) $(SIMULATION_ARGS) < /dev/null 2>&1 | tee $(OUTPUT_LOG)


clean: soft_clean
	$(SOCLIB_CC) -P $(SOCLIB_CC_ARGS) $(SOCLIB_CC_ADD_ARGS) -x -o $(SIM_BINARY)
	rm -f tty.log
	rm -f vcd_traces.vcd
	rm -f $(OUTPUT_LOG)
	rm -f *~

soft_clean:
	$(MAKE) -C soft clean


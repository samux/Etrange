SOCLIB_PATH = /comelec/softs/opt/www.soclib.fr/distrib/soclib

.PHONY: all sim clean

all: lib.done system.done

clean:
	rm -f *.done
	rm -rf work
	rm -f transcript  vsim.wlf


lib.done:
	vlib work
	@touch $@

display.done : ../display/display.cpp
	sccom -DSOCLIB_MODULE_DEBUG=1 `sdl-config --cflags` -I../display -ggdb -DSOCLIB_MODE_DEBUG -Wall  -DSOCLIB -I$(SOCLIB_PATH)/soclib/lib/include  -c ../display/display.cpp
	@touch $@

video.done : ../video_gen/video_gen.cpp
	sccom -DSOCLIB_MODULE_DEBUG=1 -I../video_gen -ggdb -DSOCLIB_MODE_DEBUG -Wall  -DSOCLIB -I$(SOCLIB_PATH)/soclib/lib/include  -c ../video_gen/video_gen.cpp
	@touch $@

top.done : ../top.cpp
	sccom `sdl-config --cflags` -I../video_gen -I../display -O2 -DSOCLIB_MODE_RELEASE -Wall  -DSOCLIB -I$(SOCLIB_PATH)/soclib/lib/include  -c  ../top.cpp
	@touch $@

system.done :display.done video.done  top.done
	sccom -link -lpng `sdl-config --libs`
	@touch $@


sim:system.done
	vsim -novopt sc_main

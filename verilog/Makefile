SRC = ram.sv test_fifo.sv fifo.sv


all: work $(SRC)
	vlog +$^ 
	vsim

work:
	vlib work

clean:
	rm -r work

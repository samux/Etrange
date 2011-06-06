parameter ADDR_SIZE = 4;
parameter DATA_SIZE = 8;

module ram (
		input wire clk,
		//Port 1, read/write 
		input wire [DATA_SIZE-1:0] data_in_A,
		output reg [DATA_SIZE-1:0] data_out_A,
		input wire [ADDR_SIZE-1:0] addr_A,
		input wire w_e_A,
		);


reg  [DATA_SIZE-1:0] internal_data[2**ADDR_SIZE-1:0];


always_ff@(posedge clk) 
	begin
		data_out_A <= internal_data[addr_A];
		if (w_e_A)
			internal_data[addr_A] = data_in_A;
	end

endmodule;


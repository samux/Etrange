parameter ADDR_SIZE = 16;
parameter DATA_SIZE = 32;

module ram (
	wire clk,
	//Port 1, read/write 
	wire data_in_A[DATA_SIZE-1:0],
	reg data_out_A[DATA_SIZE-1:0],
	wire addr_A[ADDR_SIZE -1:0],
	wire w_e_A,
	wire r_e_A,
	//Port 2, read only
	reg data_out_B[DATA_SIZE:0],
	wire addr_B[ADDR_SIZE-1:0],
	wire r_e_B
);


reg [2**ADDR_SIZE-1:0] internal_data[DATA_SIZE-1:0];

always_ff@(posedge clk) 
begin
	if (w_e_A)
		internal_data[addr_A] = data_in_A;
	data_out_A = internal_data[addr_A];
	data_out_B = internal_data[aadr_B];
	end
end

endmodule;
	

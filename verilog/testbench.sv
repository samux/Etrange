parameter ADDR_SIZE = 16;
parameter DATA_SIZE = 32;

module testbench;
	
	reg clk;
	reg data_in_A[DATA_SIZE-1:0];
	wire data_out_A[DATA_SIZE-1:0];
	wire data_out_B[DATA_SIZE-1:0];
	reg addr_A[ADDR_SIZE-1:0];
	reg addr_B[ADDR_SIZE-1:0];

	reg w_e_A;
	reg r_e_A;
	reg r_e_B;


module ram (
	.clk(clk),
	.data_in_A(data_in_A),
	.data_out_A(data_out_A),
	.w_e_A(w_e_A),
	.r_e_A(r_e_A),
	.data_out_B(data_out_B),
	.addr_B(addr_B),
	.r_e_B(r_e_B)
	);







initial
begin
	clk = 0;
	data_in_A = 245;
	w_e_A = 0;
	r_e_A = 0;
	r_e_B = 0;
	addr_A = 0;
	addr_B = 0;

	#10
	w_e_A = 1;
	r_e_A = 1;
	r_e_B = 1;

	#50
	data_in_A = 420;
	addr_B = addr_B + 1;

	#100
	$stop();



always
begin
	#5 clk = ~clk;
end
	

endmodule

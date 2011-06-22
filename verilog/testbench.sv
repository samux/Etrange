parameter ADDR_SIZE = 16;
parameter DATA_SIZE = 32;
module testbench();
	reg clk;
	reg nRST;
	reg [DATA_SIZE-1:0] data_in_A;
	wire [DATA_SIZE-1:0] data_out_A;
	wire [DATA_SIZE-1:0] data_out_B;
	reg [ADDR_SIZE-1:0] addr_A;
	reg [ADDR_SIZE-1:0] addr_B;
	reg w_e_A;
	
ram ram (
	.clk(clk),
	.data_in_A(data_in_A),
	.data_out_A(data_out_A),
	.w_e_A(w_e_A),
	.data_out_B(data_out_B),
	.addr_B(addr_B),
	.addr_A(addr_A)
	);






initial
begin
	clk = 1;
	nRST = 0;
	data_in_A = 245;
	w_e_A = 0;
	addr_A = 0;
	addr_B = 0;
	#10
	nRST = 1;
	#10
	nRST = 0;

	#10
	w_e_A = 1;

	#10
	w_e_A = 0;
	
	#50
	data_in_A = 420;
	addr_B = addr_B + 1;
	#100
	
	$stop();
end
///////////////////////////////////////
//			Génération de l'horloge
////////////////////////////////////////
always
begin
	#5 clk = ~clk;
end
	

endmodule

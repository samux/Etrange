parameter ADDR_SIZE = 4;
parameter DATA_SIZE = 32;
module test_fifo();
	reg clk;
	reg nRST;
	reg [DATA_SIZE-1:0] data_in;
	wire [DATA_SIZE-1:0] data_out;
	reg w_e;
	reg r_ack;
	
fifo fifo (
	.clk(clk),	
	.nRST(nRST),
	.data_in(data_in),
	.data_out(data_out),
	.w_e(w_e),
	.r_ack(r_ack)
	);




always_ff @(posedge clk or negedge nRST)
if (nRST)
	data_in = 0;
else
	data_in = data_in + 1;

initial
begin
	clk = 1;
	nRST = 0;
	w_e = 0;
	r_ack = 0;

	//Reset
	#10
	nRST = 1;
	#10
	nRST = 0;

	//Test en ecriture
	#10
	w_e = 1;

	#200
	w_e = 0;

	//Test en lecture
	for (integer i = 0; i<30; i++)
	begin
		#10
		r_ack = 0;
		#10
		r_ack = 1;
	end

	#50
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

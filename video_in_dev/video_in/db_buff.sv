//Double buffer, chaque buffer est de taille
//NB_PACK
//Fonctionnement --> 
//w_nb vaut 0 quand on écrit dans la première ram
//et 1 quand on écrit dans la seconde
parameter DATA_SIZE = 8;
parameter NB_PACK_ADDR //TODO compléter/ voir comment faire
					// Nombre de pixels dans le pack : 2**NB_PACK_ADDR
					// des modules verilog parametrables




module db_buffer (
		wire clk,
		wire nRST,
		wire [DATA_SIZE-1:0] data_in,
		wire w_e,
		wire r_e,
		wire [DATA_SIZE-1:0] data_out,
		reg w_nb
	);

wire [DATA_SIZE-1:0] data_out_0;
wire [DATA_SIZE-1:0] data_out_1;

wire [NB_PACK_ADDR-1:0] addr_0;
wire [NB_PACK_ADDR-1:0] addr_1;

reg [NB_PACK_ADDR-1:0] addr_in;
reg [NB_PACK_ADDR-1:0] addr_out;


assign data_out = (w_nb)?data_out_1:data_out_0;
assign {addr_0, addr_1} = (w_nb)?{addr_out, addr_in}:{addr_in, addr_out};


ram ram0 (
	.clk(clk),
	.data_in_A(data_in),
	.data_out_A(data_out_0),
	.addr_A(addr_0),
	.w_e_A(w_e_0)
	);

ram ram1 (
	.clk(clk),
	.data_in_A(data_in),
	.data_out_A(data_out_1),
	.addr_A(addr_1),
	.w_e_A(w_e_1)
	);


always_ff @(posedge clk or negedge nRST)
if (~nRST)
	begin
		w_nb = 0;
	end

		









endmodule

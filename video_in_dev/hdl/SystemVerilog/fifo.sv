parameter ADDR_SIZE = 6;
parameter DATA_SIZE = 32;
parameter NB_PACK = 16;//TODO remplir et voir comment
					// Faire des modules verilog paramêtrables

module fifo (
	input wire clk,
	input wire nRST,
	input wire [DATA_SIZE-1:0] data_in,
	input wire w_e,
	input wire r_ack,
	output reg [DATA_SIZE-1:0] data_out,
	output wire nb_pack_available
	);

reg [ADDR_SIZE-1:0] addr_first;
reg [ADDR_SIZE-1:0] addr_last;
reg [ADDR_SIZE:0] fifo_cnt; //Pas addr_size -1 car fifo_cnt peut valoir 2**ADDR_SIZE

wire full;
wire empty;

//////////////////////////////////
//	RAM
///////////////////////////////////

ram ram (
	.clk(clk),
	.data_in_A(data_in),
	.data_out_B(data_out),
	.addr_A(addr_last),
	.addr_B(addr_first),
	.w_e_A(w_e & ~full)
	);

assign nb_pack_available = (fifo_cnt > NB_PACK);
assign empty = (fifo_cnt == 0);
assign full = (fifo_cnt == 2**ADDR_SIZE);

/////////////////////////////////
// Calcul des addresses
/////////////////////////////////
always_ff@(posedge clk or negedge nRST)
if (~nRST)
	begin
	addr_last <= 0;
	addr_first <= 0;
	fifo_cnt = 0;
	end
else
	begin
	if (w_e & ~full)
		begin
			addr_last <=  addr_last + 1;	
			fifo_cnt = fifo_cnt + 1;
		end
	if (r_ack)
		begin
			addr_first <= addr_first + 1;
			fifo_cnt = fifo_cnt - 1;
		end
	end

endmodule;

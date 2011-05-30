parameter ADDR_SIZE = 16;
parameter DATA_SIZE = 32;

module fifo (
	input wire clk,
	input wire nRST,
	input wire [DATA_SIZE-1:0] data_in,
	input wire w_e,
	input wire r_e,
	output reg [DATA_SIZE-1:0] data_out
	);

reg [DATA_SIZE-1:0] addr_first;
reg [DATA_SIZE-1:0] addr_last;
wire [DATA_SIZE-1:0] addr_first_next;
wire [DATA_SIZE-1:0] addr_last_next;

reg full;
reg empty;

//////////////////////////////////
//	RAM
///////////////////////////////////

ram ram (
	.clk(clk),
	.data_in_A(data_in),
	.data_out_B(data_out),
	.addr_A(addr_first),
	.addr_B(addr_last),
	.w_e_A(w_e)
	);

assign addr_last_next = (w_e & ~full)?addr_last + 1:addr_last;
assign addr_first_next = (r_e & ~empty)?addr_first + 1:addr_first;


/////////////////////////////////
// Calcul des addresses
///////////////////////////////
always_ff@(posedge clk or negedge nRST)
if (nRST)
	begin
	addr_last <= 0;
	addr_first <= 0;
	full <= 0;
	empty <= 1;
	end
else
	begin
	addr_last <= addr_last_next;
	addr_first <= addr_firs_next;
	if (w_e)
		begin
			full <= (addr_last_next==addr_first_next);
			empty <= 0;
		end
		else if (r_e)
		begin
			empty <= (addr_last_next==addr_first_next);
			full <= 0;
		end
	end

endmodule;

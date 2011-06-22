parameter p_WIDTH = 640;
parameter p_HEIGHT = 480;
parameter TIME_BREAK = 160;
parameter IMAGE_TIME_BREAK = 40*(160+640);

module video_out_gen (
	input wire clk,
	input wire clk_out,
	input wire nRST,
	
	output reg r_ack,
	input wire [7:0] pixel_in,
	input wire nb_pack_available,

	output reg [7:0] pixel_out,
	output reg frame_valid,
	output reg line_valid
	);

//Génération de ack court pour la fifo
//à partir des ack long de la machine à état
reg r_ack_slow;
//Détection des fronts descendants de clk_out
reg old_clk_out;

always_ff @(posedge clk)
	old_clk_out <= clk_out;

//On met r_ack à 1 pendant un coup de clk
//sur front descendant de clk_out
always_ff @(posedge clk)
	if (old_clk_out & ~clk_out & r_ack_slow)
		r_ack <= 1;
	else
		r_ack <= 0;

//Compteur de ligne
reg [9:0] pixel_l;

//Compteur de colonne
reg [9:0] pixel_c;

//Compteur pour la durée du break entre
//lignes et entre image
reg [9:0] line_break;
reg [15:0] image_break;

enum logic [1:0] {WAIT, GEN, BREAK} state, next_state;

always_ff @(posedge clk_out or negedge nRST)
if (~nRST)
	state <= WAIT;
else 
	state <= next_state;

//Les compteurs 
always_ff @(posedge clk_out or negedge nRST)
if (~nRST)
begin
	pixel_l <= 0;
	pixel_c <= 0;
	line_break <= 0;
	image_break <= 0;
end
else
case (state)
	WAIT:
		begin
			pixel_l <= 0;
			pixel_c <= 0;
			line_break <= 0;
			image_break <= image_break + 1;
		end
	GEN:
		begin
			pixel_c <= pixel_c + 1;
			line_break <= 0;
		end
	BREAK:
		begin
			line_break <= line_break + 1;
			image_break <= 0;
			pixel_c <= 0;
			if (line_break == 0) pixel_l <= pixel_l + 1;
		end
endcase


//Calcul combinatoire de l'état suivant
always_comb 
case (state)
	WAIT:
		if (nb_pack_available && image_break == IMAGE_TIME_BREAK) next_state <= GEN;
		else next_state <= WAIT;
	GEN: 
		if (pixel_c == (p_WIDTH-1)) 
			if (pixel_l == (p_HEIGHT-1))
				next_state <= WAIT;
			else
				next_state <= BREAK;
		else next_state <= GEN;
	BREAK:
		if (line_break == TIME_BREAK) 
			next_state <= GEN;
		else 
			next_state <= BREAK;
endcase

always_comb
case (state)
	WAIT:
		begin
			frame_valid <= 0;
			line_valid <= 0;
			r_ack_slow <= 0;
		end
	GEN:
		begin
			frame_valid <= 1;
			line_valid <= 1;
			pixel_out <= pixel_in;
			//Ce ack dure 3 coups de clock système, ce qui est trop 
			//long 
			r_ack_slow <= 1; 
		end
	BREAK:
		begin
			frame_valid <= 1;
			line_valid <= 0;
			r_ack_slow <= 0;
		end
endcase

endmodule


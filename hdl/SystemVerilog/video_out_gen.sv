/*
*	Caroline Kéramsi
*	Projet Etrange INF342
*	TélécomParisTech
*/

/*
*	Ce module prend les pixels dans la fifo
*	et générè le flux vidéo
*/

module video_out_gen 
	#(
	parameter WIDTH = 640;
	parameter HEIGHT = 480;
	//Temps de pause entre 2 lignes
	parameter TIME_BREAK = 160;
	//Temps de pause entre 2 images
	parameter IMAGE_TIME_BREAK = 40*(160+640);
	)
	(
	input wire clk,
	input wire clk_out,
	input wire nRST,

	//Lecture dans la fifo
	output reg r_ack,
	input wire [7:0] pixel_in,
	input wire nb_pack_available,

	//Signaux vidéo
	output reg [7:0] pixel_out,
	output reg frame_valid,
	output reg line_valid
	);

/**********************************************
*	Génération de ack court pour la fifo à 
*	partir des ack long de la machine à états
***********************************************/

///////Détection des fronts descendants de clk_out///////////
reg old_clk_out;

always_ff @(posedge clk)
	old_clk_out <= clk_out;

//On met r_ack à 1 pendant un coup de clk sur front descendant de clk_out//
reg r_ack_slow;

always_ff @(posedge clk)
	if (old_clk_out & ~clk_out & r_ack_slow)
		r_ack <= 1;
	else
		r_ack <= 0;


/*****************************************
*	Machine à états pour la génération
*	des signaux Vidéo et de r_ack_slow
*****************************************/

//Compteurs pour la position dans l'image
reg [9:0] pixel_l;
reg [9:0] pixel_c;

//Compteur pour la durée du break entre les lignes et entre les images
reg [9:0] line_break;
reg [15:0] image_break;

/////////////////États//////////////////////////////
enum logic [1:0] {WAIT, GEN, BREAK} state, next_state;

///////////////Mise à jour séquentiel de l'état/////////
always_ff @(posedge clk_out or negedge nRST)
if (~nRST)
	state <= WAIT;
else 
	state <= next_state;

////////////////Gestion séquentiel des compteurs////////
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


/////////////Calcul combinatoire de l'état suivant/////////////////////
always_comb 
case (state)
	WAIT:
		if (nb_pack_available && image_break == IMAGE_TIME_BREAK) next_state <= GEN;
		else next_state <= WAIT;
	GEN: 
		if (pixel_c == (WIDTH-1)) 
			if (pixel_l == (HEIGHT-1))
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

///////////////Calcul combinatoire des sorties/////////////////////////
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


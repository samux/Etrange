/* 
*  Caroline Kéramsi
*  Projet Etrange INF342
*  Telecom ParisTech
*/

/* 
*	Ce module prend en entrée un flux vidéo 
*	et stocke l'image correspondante
*	en RAM
*/




module video_in
	(
	//horloge système, 100 MHz
	input wire clk,
	//horloge du flux vidéo, 25 MHz
	input wire clk_in,		
	input wire reset_n,	

	//Signaux du flux vidéo
	input wire line_valid,
	input wire frame_valid,
	input wire [7:0] pixel_in,

	//Interruption vers le processeur
	output wire interrupt,

	//Connexion avec le module wishbone slave
	input wire [31:0] wb_reg_data,
	input wire [31:0] wb_reg_ctr,

	//Signaux wishbone master
	output wire p_wb_STB_O,
	output wire p_wb_CYC_O,
	output wire p_wb_LOCK_O, 
	output wire [3:0] p_wb_SEL_O,
	output wire p_wb_WE_O,
	output wire [31:0] p_wb_ADR_O,
	input wire p_wb_ACK_I,
	output wire [31:0] p_wb_DAT_O
	);

/*
*	On génère un reset si video_in_store a passé
*	new_addr à 1 pour assurer la synchronisation
*	entre video_in_store et video_in_read
*/

wire nRST_im;

wire new_addr;
assign nRST_im = ~(new_addr || ~reset_n);





/*
*	video_in_read lit le flux video entrant, 
*	regroupe les pixels par paquet de 32 bits 
*	et les pose dans la fifo
*/

wire w_e;
wire [31:0] pixels_fifo_in;

video_in_read video_in_read (	
	//Signaux reliés aux signaux d'entrée
	.clk_in(clk_in), 
	.clk(clk), 
	.nRST(nRST_im),
	.line_valid(line_valid),
	.frame_valid(frame_valid),
	.pixel_in(pixel_in),

	//Signaux reliés à la fifo
	.w_e(w_e),
	.pixels_out(pixels_fifo_in),
	.new_addr(new_addr)
);


/*
*	Fifo
*/

//Signaux fifo <--> video_in_store
wire r_ack;
wire [31:0] pixels_fifo_out;
wire nb_pack_available;

fifo #(.NB_PACK(4)) fifo 
	(
	.clk(clk),
	.nRST(nRST_im),
	.data_in(pixels_fifo_in),
	.data_out(pixels_fifo_out),
	.w_e(w_e),
	.r_ack(r_ack),
	//Indique si la fifo contient au moins NB_PACK pixels
	.nb_pack_available(nb_pack_available)
	);

/*
*	Video_in_store récupère les paquets 
*	de pixels de la fifo et les stocke 
*	en RAM par wishbone
*/

video_in_store #(.NB_PACK(16)) video_in_store (
	.clk(clk),
	.nRST(reset_n),
	.new_addr(new_addr),

	//Interruption processeur
	.interrupt(interrupt),

	//Connexion avec la fifo
	.r_ack(r_ack),
	.data_fifo(pixels_fifo_out),
	.nb_pack_available(nb_pack_available),
	
	//Connexion avec le module wishbone slave
	.wb_reg_ctr(wb_reg_ctr),
	.wb_reg_data(wb_reg_data),

	//Signaux wisbone master
	.p_wb_STB_O(p_wb_STB_O),
	.p_wb_CYC_O(p_wb_CYC_O),
	.p_wb_LOCK_O(p_wb_LOCK_O),
	.p_wb_WE_O(p_wb_WE_O),
	.p_wb_SEL_O(p_wb_SEL_O),
	.p_wb_ADR_O(p_wb_ADR_O),
	.p_wb_ACK_I(p_wb_ACK_I),
	.p_wb_DAT_O(p_wb_DAT_O),
	.p_wb_ERR_I(p_wb_ERR_I)
	);

endmodule

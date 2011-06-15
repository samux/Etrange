module video_in (
	input wire clk, //horloge interne
	input wire clk_in, //horloge entrée à 25MHz
	input wire reset_n,
	input wire line_valid,
	input wire frame_valid,
	input wire [7:0] pixel_in,
	output wire interrupt,
	//Connexion avec le 
	//module wishbone slave
	input wire [31:0] wb_reg_data,
	input wire [31:0] wb_reg_ctr,
	//Signaux wisbone master
	output wire p_wb_STB_O,
	output wire p_wb_CYC_O,
	output wire p_wb_LOCK_O,
	output wire [3:0] p_wb_SEL_O,
	output wire [31:0] p_wb_ADR_O,
	input wire p_wb_ACK_I,
	output wire [31:0] p_wb_DAT_O,
	input wire p_wb_ERR_I
	);

//Signaux pour faire le lien video_in_read <---> fifo
wire w_e;
wire r_ack;
wire [31:0] pixels_fifo_in;
wire nb_pack_available;
//Rmque : Si la fifo est pleine, on ne peut rien y faire
//Le module video_in_read ne peut pas attendre et ne vérifie
//Donc pas le signal full de la fifo


//Lit le flux video entrant, regroupe
//les pixels par paquet de 32 bits et les
//pose dans la fifo
video_in_read video_in_read (	
	//Signaux directement reliés aux signaux d'entrée
	.clk(clk_in),
	.nRST(reset_n),
	.line_valid(line_valid),
	.frame_valid(frame_valid),
	.pixel_in(pixel_in),

	//Signaux reliés à la fifo
	.w_e(w_e),
	.pixels_out(pixels_fifo_in)
);


fifo fifo(
	.clk(clk),
	.nRST(reset_n),
	.data_in(pixels_fifo_in),
	.w_e(w_e),
	//TO DO data_out n'est relié à rien pour l'instant. Il faut 
	//changer ça
	.nb_pack_available(nb_pack_available)
	);

//Récupère les paquets de pixels de la fifo
//et les stocke en RAM par wishbone
video_in_store video_in_store (
	.clk(clk),
	.nRST(reset_n),

	//Interruption processeur
	.interrupt(interrupt),

	//Connexion avec la fifo
	.r_ack(r_ack),
	.data_fifo(data_fifo_out),
	

	//Connexion avec le 
	//module wishbone slave
	.wb_reg_ctr(wb_reg_ctr),
	.wb_reg_data(wb_reg_data),
	//Signaux wisbone master
	.p_wb_STB_O(p_wb_STB_O),
	.p_wb_CYC_O(p_wb_CYC_O),
	.p_wb_LOCK_O(p_wb_LOCK_O),
	.p_wb_SEL_O(p_wb_SEL_O),
	.p_wb_ADR_O(p_wb_ADR_O),
	.p_wb_ACK_I(p_wb_ACK_I),
	.p_wb_DAT_O(p_wb_DAT_O),
	.p_wb_ERR_I(p_wb_ERR_I)
	);
		



endmodule

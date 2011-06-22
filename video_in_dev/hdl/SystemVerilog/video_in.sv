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
	output wire p_wb_WE_O,
	output wire [31:0] p_wb_ADR_O,
	input wire p_wb_ACK_I,
	output wire [31:0] p_wb_DAT_O
	);

//Signaux pour faire le lien video_in_read <---> fifo
wire w_e;
wire r_ack;
wire [31:0] pixels_fifo_in;
wire [31:0] pixels_fifo_out;
wire nb_pack_available;
//Rmque : Si la fifo est pleine, on ne peut rien y faire
//Le module video_in_read ne peut pas attendre et ne vérifie
//Donc pas le signal full de la fifo

//Reset de video_in_read et fifo
wire new_addr;
wire nRST_im;
assign nRST_im = ~(new_addr || ~reset_n);

//Lit le flux video entrant, regroupe
//les pixels par paquet de 32 bits et les
//pose dans la fifo
video_in_read video_in_read (	
	//Signaux directement reliés aux signaux d'entrée
	.clk_in(clk_in), //clk pour les signaux d'entree
	.clk(clk), //clk du system
	.nRST(nRST_im),
	.line_valid(line_valid),
	.frame_valid(frame_valid),
	.pixel_in(pixel_in),

	//Signaux reliés à la fifo
	.w_e(w_e),
	.pixels_out(pixels_fifo_in),
	.new_addr(new_addr)
);


fifo fifo(
	.clk(clk),
	.nRST(nRST_im),
	.data_in(pixels_fifo_in),
	.data_out(pixels_fifo_out),
	.w_e(w_e),
	.r_ack(r_ack),
	//TO DO data_out n'est relié à rien pour l'instant. Il faut 
	//changer ça
	.nb_pack_available(nb_pack_available)
	);

//Récupère les paquets de pixels de la fifo
//et les stocke en RAM par wishbone
video_in_store video_in_store (
	.clk(clk),
	.nRST(reset_n),
	.new_addr(new_addr),

	//Interruption processeur
	.interrupt(interrupt),

	//Connexion avec la fifo
	.r_ack(r_ack),
	.data_fifo(pixels_fifo_out),
	.nb_pack_available(nb_pack_available),
	

	//Connexion avec le 
	//module wishbone slave
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

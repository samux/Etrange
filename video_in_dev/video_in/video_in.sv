module video_in (
	input wire clk,
	input wire nRST,
	input wire line_valid,
	input wire frame_valid,
	input wire pixel_in,
	output wire interrupt,
	//TO DO Rajouter les signaux wishbone
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
	.clk(clk),
	.nRST(nRST),
	.line_valid(line_valid),
	.frame_valid(frame_valid),
	.pixel_in(pixel_in),

	//Signaux reliés à la fifo
	.w_e(w_e),
	.pixel_out(pixels_fifo_in)
);


fifo fifo(
	.clk(clk),
	.nRST(nRST),
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
	.nRST(nRST),

	//Interruption processeur
	.interrupt(interrupt),

	//Connexion avec la fifo
	.r_ack(r_ack),
	.data_fifo(data_fifo_out),
	

	//Connexion avec le 
	//module wishbone slave
	.wb_reg(wb_reg),
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

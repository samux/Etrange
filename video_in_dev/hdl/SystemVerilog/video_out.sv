module video_out (
		input wire clk, //horloge système à 100MHz
		input wire clk_out, //horloge pour les sorties à 25MHz
		input wire nRST,

		//Connexion avec le module wishbone
		//slave pour l'obtention des adresses
		input wire [31:0] wb_reg_data,
		input wire [31:0] wb_reg_ctr,

		//Interruption de video_out 
		output reg interrupt,

		//TODO REMPLACER LES WIRE par REG
		//Signaux video
		
		output wire line_valid,
		output wire frame_valid,
		output wire [7:0] pixel_out,
		

		//Signaux wishbone master pour
		//la lecture en RAM
		output wire p_wb_STB_O,
		output wire p_wb_CYC_O,
		output wire p_wb_LOCK_O, 
		output wire [3:0] p_wb_SEL_O,
		output wire p_wb_WE_O,
		output wire [31:0] p_wb_ADR_O,
		input wire p_wb_ACK_I,
		input wire [31:0] p_wb_DAT_I
		);

//Signaux video_out_read <-> fifo
wire full;
wire w_e;
wire [7:0] pixel_fifo_in;

//Signaux video_out_gen <-> fifo
wire nb_pack_available;
wire r_ack = 0;
wire [7:0] pixel_fifo_out;

//Ce module va lire une image en RAM
//et la place dans la fifo
video_out_read video_out_read (
		.clk(clk),
		.nRST(nRST),
		//Pour la gestion des adresses
		.wb_reg_data(wb_reg_data),
		.wb_reg_ctr(wb_reg_ctr),
		.interrupt(interrupt),
		//Wishbone pour la lecture en ram
		.p_wb_STB_O(p_wb_STB_O),
		.p_wb_CYC_O(p_wb_CYC_O),
		.p_wb_LOCK_O(p_wb_LOCK_O),
		.p_wb_SEL_O(p_wb_SEL_O),
		.p_wb_WE_O(p_wb_WE_O),
		.p_wb_ADR_O(p_wb_ADR_O),
		.p_wb_ACK_I(p_wb_ACK_I),
		.p_wb_DAT_I(p_wb_DAT_I),

		//Pour écrire dans la fifo
		.full(full),
		.w_e(w_e),
		.pixel_out(pixel_fifo_in)
		);

//Ce module prend les pixels dans la
//fifo et génère line_valid / frame_valid
// et pixel_out
video_out_gen video_out_gen (
	.clk(clk),
	.clk_out(clk_out),
	.nRST(nRST),

	//Communication avec la fifo
	.r_ack(r_ack),
	.nb_pack_available(nb_pack_available),
	.pixel_in(pixel_fifo_out),

	//Signaux de sortie
	.pixel_out(pixel_out),
	.frame_valid(frame_valid),
	.line_valid(line_valid)


	);

fifo fifo_out (
	.clk(clk),
	.nRST(nRST),
	.data_in(pixel_fifo_in),
	.data_out(pixel_fifo_out),
	.nb_pack_available(nb_pack_available),
	.full(full),
	.w_e(w_e),
	.r_ack(r_ack)
	 );


endmodule

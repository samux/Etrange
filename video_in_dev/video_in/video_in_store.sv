parameter p_WIDTH = 640;
parameter p_HEIGHT = 480;
parameter ADDR_SIZE //TODO A definir
parameter NB_PACK //TODO A définir

module video_in_store (
		input wire clk,
		input wire nRST,
		input wire [1:0] wb_reg,
		input wire nb_pack_available,
		output reg data_fifo,
		output reg w_e,
		output reg interrupt,
		//Signaux wishbone
		output reg p_wb_STB_O,
		output reg p_wb_CYC_O,
		output reg p_wb_LOCK_O,
		output wire [3:0] p_wb_SEL_O
		output reg p_wb_WE_O,
		output reg [31:0] p_wb_ADR_O,
		input wire [31:0] p_wb_ADR_I,
		input wire p_wb_ACK_I,
		output reg [31:0] p_wb_DAT_O,
		input wire p_wb_ERR_I,
		//Rmque il n'y a pas p_wb_RTY, je sais 
		//pas à quoi ça sert

		);

assign p_wb_SEL_O = 0xf;


//Indique si l'on attend une adresse 
//du processeur ou si on l'a déjà
reg stockage_ok;

//Compteurs pour la position 
//dans l'image
reg [9:0] pixel_c;
reg [9:0] pixel_l;

//Adresse en Ram du début de l'image
reg [31:0] deb_im;

//Compteur pour les pixels qu'il faut aller chercher
//en RAM ATTENTION à la taille du compteur
reg [7:0] counter;


//On va faire une machine à états 
// WAIT_ADDR :
//		-on attend l'adresse de la part du
//		du processeur.
//		On sait qu'on a reçue grâce à
//		new_addr.
//WAIT_PACK_AVB : 
//		-on attend que nb_pack pixels soit disponible
//		dans la fifo. Attention la fifo stocke
//		déjà des mots de 32 bits
//WAIT_ACK : 
//		-on attend le ack du wishbone
//STORE :
//		-on stocke les nb_pack pixels dans la fifo
// 

//On va défnir une fonction CleanWb pour nettoyer
//le wishbone

//TRANSITIONS 
//Un reset nous ramène à l'état WAIT_ADDR
//Si new_addr WAIT_ADDR -> WAIT_PACK_AVB
//Si nb_pack_available WAIT_PACK_AVB -> ST //XXX TO BE CONTINUED

//Lecture de l'adresse envoyée par le processeur
always_ff @(posedge clk or negedge nRST)
if (~nRST)
	begin
		stockage_ok <= 0;
		interupt <= 0;
		deb_im <= wb_reg[0];
	end
else
	begin
		//Cas où on attend l'adresse venue
		//du processeur
		if (pixel_l == 0 && pixel_c == 0)
			//Lecture de l'adresse
			if (wb_reg[1] != 0 )
				begin
					deb_im <= wb_reg[0];
					stockage_ok <= 1;
					interrupt <= 0;
				end
	end
end


//On vérifie si la fifo est pleine ou pas
//et on stocke en RAM
always_ff @(posedge clk or negedge nRST)
if (~nRST)
	begin
		pixel_c <= 0;
		pixel_l <= 0;
		//Reset du wishbone
		p_wb_STB_O <= 0;
		p_wb_CYC_O <= 0;
		p_wb_LOCK_O <= 0;
		p_wb_WE_O <= 0;
		p_wb_ADR_O <= 0;
		p_wb_DAT_O <= 0;
	end
else
	r_ack <= 0;
	if ((nb_pack_available || nb_stock) && stockage_ok)
		begin
			//On stocke en RAM à l'aide du wishbone
			
			//Lecture de la valeur dans la fifo
			//Et écriture sur le bus wishbone
			p_wb_DAT_O <= data_fifo;
			r_ack <= 1;
			p_wb_ADR_O <= deb_im + pixel_l*p_WIDTH + pixel_c;
			p_wb_STB_O <= 1;
			p_wb_CYC_O <= 1;
			p_wb_WE_O w <= 1;

			//On attend le ack



		end


	

endmodule

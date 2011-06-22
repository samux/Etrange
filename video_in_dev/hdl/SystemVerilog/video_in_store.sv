
//TODO ATTENTION A BIEN SYNCHRONISER LA FIFO
//SUR les interruptions du processeur
//L'interruption vers le processeur doit durer
//au moins 3 cycles

module video_in_store (
		input wire clk,
		input wire nRST,
		input wire [31:0] wb_reg_ctr,
		input wire [31:0] wb_reg_data,
		input wire nb_pack_available,
		input wire [31:0] data_fifo,
		output reg r_ack,
		output reg interrupt,
		//new_addr vaut 1 pendant un cycle d'horloge lorsqu'une 
		//nouvelle addresse a été fournie par le processeur
		//Ce signal déclenche un reset des autres modules
		output wire new_addr, 
		//Signaux wishbone
		output reg p_wb_STB_O,
		output reg p_wb_CYC_O,
		output reg p_wb_LOCK_O,
		output wire [3:0] p_wb_SEL_O,
		output reg p_wb_WE_O,
		output reg [31:0] p_wb_ADR_O,
		input wire p_wb_ACK_I,
		output reg [31:0] p_wb_DAT_O,
		input wire p_wb_ERR_I
		//Rmque il n'y a pas p_wb_RTY, je sais 
		//pas à quoi ça sert
		);

assign p_wb_SEL_O = 4'hf;

parameter p_WIDTH = 640;
parameter p_HEIGHT = 480;
parameter NB_PACK_STORE = 16; //TODO À définir correctement


//Indique si l'on attend une adresse 
//du processeur ou si on l'a déjà
reg stockage_ok;



//Adresse en Ram du début de l'image
reg [31:0] deb_im;

//Compteurs pour la position 
//dans l'image
reg [19:0] pixel_count;

//Compteur pour les pixels qu'il faut aller stocker
//en RAM ATTENTION à la taille du compteur
reg [7:0] counter_pack;

reg old_wb_reg_ctr_0;

//Détection d'un front montant de wb_reg[1] ie, le processeur
//a écrit une nouvelle adresse dans wb_reg[0]
assign new_addr = (~old_wb_reg_ctr_0 && wb_reg_ctr[0]);

always_ff @(posedge clk)
	old_wb_reg_ctr_0 <= wb_reg_ctr[0];




//On va faire une machine à états 
// WAIT_ADDR :
//		-on attend l'adresse de la part du
//		du processeur.
//		On sait qu'on l'a reçue grâce à
//		new_addr.
//WAIT_PACK_AVB : 
//		-on attend que nb_pack pixels soit disponibles
//		dans la fifo. Attention la fifo stocke
//		déjà des mots de 32 bits
//WAIT_ACK :
//		-on attend le ack du wishbone
//STORE :
//		-on stocke les nb_pack pixels dans la fifo
//IMAGE_PROCESSED:
//		-on a fini de stocker une image on passe interrupt
//		à 1 pendant un coup d'horloge

enum logic [2:0] {WAIT_ADDR, WAIT_PACK_AVB, WAIT_ACK,BREAK, STORE, IMAGE_PROCESSED} state, next_state;



//On va défnir une fonction CleanWb pour nettoyer
//le wishbone

//TRANSITIONS 
//Un reset nous ramène à l'état WAIT_ADDR
//Si new_addr WAIT_ADDR -> WAIT_PACK_AVB
//Si nb_pack_available WAIT_PACK_AVB -> STORE
//Si le compteur de pixel à stocker atteint 0 STORE -> WAIT_ACK
//Une fois que l'on a reçu le ack:
//En fonction des compteurs soit en : 
//	-STORE
//	-WAIT_PACK_AVB
//	-WAIT_ADDR

//Calcul combinatoire de l'état suivant
//en fonction de l'état courant et des entrées

//3 coups d'horloge au moins pour l'interruption
reg [1:0] int_cnt;

always_comb
begin
	next_state <= state; 
	case (state)
		WAIT_ADDR:
			if (new_addr) next_state <= WAIT_PACK_AVB;
		WAIT_PACK_AVB:
			if (nb_pack_available) next_state <= STORE;
		STORE:
			next_state <= WAIT_ACK;
		BREAK:
			if (pixel_count == p_WIDTH * p_HEIGHT)
			//Cas de fin d'image
				next_state <= IMAGE_PROCESSED;
			//Cas de fin d'un paquet mais pas d'une image
			else if (counter_pack == 0)
				next_state <= WAIT_PACK_AVB;
			else next_state <= STORE;
		WAIT_ACK:
			if (p_wb_ACK_I) 
				next_state <= BREAK;
		IMAGE_PROCESSED:
			if (int_cnt == 3)
				next_state <= WAIT_ADDR;
			
	endcase
end

//Partie séquentielle : mis à jour de l'état
always_ff @(posedge clk or negedge nRST)
if (~nRST)
	state <= WAIT_ADDR;
else
	state <= next_state;

//Calcul des sorties et incrémentations
//des compteurs
always_ff @(posedge clk or negedge nRST)
if (~nRST)
	begin
		interrupt <= 0;
		deb_im <= wb_reg_data;
		pixel_count <= 0;
		counter_pack <= NB_PACK_STORE/4;
		p_wb_STB_O <= 0;
		p_wb_CYC_O <= 0;
		p_wb_LOCK_O <= 0;
		p_wb_WE_O <= 0;
		r_ack <= 0;
		int_cnt <= 0;
	end
else
	begin
	r_ack <= 0;
	case (state)
		//On ne sort rien sur le wishbone
		//On met à jour l'adresse
		WAIT_ADDR:
			begin
				p_wb_STB_O <= 0;
				p_wb_CYC_O <= 0;
				p_wb_LOCK_O <= 0;
				p_wb_WE_O <= 0;
				deb_im <= wb_reg_data;
				interrupt <= 0;
			end
		//On ne sort rien sur le wishbone
		//On maintient le compteur counter_pack
		//à la valeur NB_PACK
		//On acquitte la lecture de l'adresse
		WAIT_PACK_AVB:
			begin
				p_wb_STB_O <= 0;
				p_wb_CYC_O <= 0;
				p_wb_LOCK_O <= 0;
				p_wb_WE_O <= 0;
				counter_pack <= NB_PACK_STORE;
			end
		//On lit une valeur dans la fifo
		//et on positionne les sorties wishbone 
		//pour écrire cette valeur en RAM
		STORE:
			begin
				r_ack <= 1;
				p_wb_DAT_O <= data_fifo;
				p_wb_ADR_O <= deb_im + pixel_count;
				p_wb_STB_O <= 1;
				p_wb_CYC_O <= 1;
				p_wb_WE_O <= 1;
				counter_pack <= counter_pack - 1;
				pixel_count <= pixel_count + 4;
			end
		//On reste dans cet état où il
		//ne se passe rien de nouveau jusqu'à réception
		//du ack du wishbone
		WAIT_ACK:
			begin
			end
		BREAK:
			begin
				p_wb_STB_O <= 0;
				p_wb_CYC_O <= 0;
			end
		IMAGE_PROCESSED:
			begin
				interrupt <= 1;
				int_cnt <= int_cnt + 1;
				pixel_count <= 0;
			end

		endcase
				

				
	end			
				







//Lecture de l'adresse envoyée par le processeur
always_ff @(posedge clk or negedge nRST)
if (~nRST)
	begin
		stockage_ok <= 0;
		interrupt <= 0;
		deb_im <= wb_reg_data;
	end
else
	begin
		//Cas où on attend l'adresse venue
		//du processeur
		if (pixel_count == 0)
			//Lecture de l'adresse
			if (new_addr != 0 )
				begin
					deb_im <= wb_reg_data;
					stockage_ok <= 1;
					interrupt <= 0;
				end
	end

endmodule

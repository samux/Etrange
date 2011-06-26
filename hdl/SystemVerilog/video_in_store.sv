/* 
*  Caroline Kéramsi
*  Projet Etrange INF342
*  Telecom ParisTech
*/

module video_in_store
		#(parameter WIDTH = 640,
		parameter HEIGHT = 480,
		parameter NB_PACK = 16
		)
		(
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
		output reg [3:0] p_wb_SEL_O,
		output reg p_wb_WE_O,
		output reg [31:0] p_wb_ADR_O,
		input wire p_wb_ACK_I,
		output reg [31:0] p_wb_DAT_O,
		input wire p_wb_ERR_I
		);


/*****************************************************
*
*		Gestion de l'adresse de l'image en RAM
*
******************************************************/

//////////////Variable de stockage de l'adresse//////////
reg [31:0] deb_im;

//////////////Détection d'une nouvelle adresse///////////
reg old_wb_reg_ctr_0;

assign new_addr = (~old_wb_reg_ctr_0 && wb_reg_ctr[0]);

always_ff @(posedge clk)
	old_wb_reg_ctr_0 <= wb_reg_ctr[0];

/******************************************************
*
*		Machine à états
*
******************************************************/

///////////////Etats///////////////////////////////////
/* 

WAIT_ADDR :
		On attend l'adresse de la part du du processeur.
		On détecte sa réception grâce à new_addr.
		
WAIT_PACK_AVB : 
		On attend que NB_PACK pixels soient disponibles dans la fifo. 
		Attention la fifo stocke déjà des mots de 32 bits (4 pixels).

BREAK : 
		Entre deux stockages au sein d'un même paquet 
		On repositionne p_wb_DAT_O à une nouvelle valeur

WAIT_ACK :
		On attend le ack du wishbone

IMAGE_PROCESSED:
		On a fini de stocker une image on maintient interrupt à 1 
		pendant 3 coups d'horloge (comptés avec int_cnt)
*/


enum logic [2:0] {WAIT_ADDR, WAIT_PACK_AVB, WAIT_ACK, BREAK, IMAGE_PROCESSED} state;

////////////////Compteurs////////////////////////////////////
//Pour maintenir l'interruption pendant 3 cycles d'horloge
reg [1:0] int_cnt;

//Pour rester deux temps dans le break
reg hold_break;

//Pour se situer dans l'image
reg [19:0] pixel_count;

/////////////Calcul séquentiel de l'état suivant///////////////////////
////////////////////Et gestion des compteurs///////////////////////////
always_ff @(posedge clk or negedge nRST)
if (~nRST)
	begin
		state <= WAIT_ADDR;
		pixel_count <= 0;
		int_cnt <= 0;
		hold_break <= 1;
	end
else
	case (state)
		WAIT_ADDR:
			begin
				if (new_addr) state <= WAIT_PACK_AVB;
				int_cnt <= 0;
			end
		WAIT_PACK_AVB:
			if (nb_pack_available) state <= WAIT_ACK;
		BREAK:
			state <= WAIT_ACK;
		WAIT_ACK:
			begin
			if (p_wb_ACK_I)
				begin
					//Incrémentation des compteurs
					pixel_count <= pixel_count + 4;
					state <= BREAK;
					//Cas de la fin d'un paquet
					if (pixel_count%NB_PACK == NB_PACK - 4)
						begin
							state <= WAIT_PACK_AVB;
							//Cas de fin d'une image
							if (pixel_count == (WIDTH*HEIGHT) - 4)
								begin
									pixel_count <= 0;
									state <= IMAGE_PROCESSED;
								end
						end
					end
				end
		IMAGE_PROCESSED:
			begin 
				int_cnt <= int_cnt + 1;
				if (int_cnt == 3)
					state <= WAIT_ADDR;
			end
	endcase

////////////////Calcul combinatoire des sorties/////////////
always_comb
begin
	//Toujours vrai
	p_wb_SEL_O <= 4'hf;
	p_wb_LOCK_O <= 0;
	p_wb_WE_O <= 1;

	//Vrai sauf si le contraire est indiqué ci-dessous
	p_wb_STB_O <= 0;
	p_wb_CYC_O <= 0;
	r_ack <= 0;
	interrupt = 0;
	case (state)
		WAIT_ADDR:
			deb_im <= wb_reg_data;
		WAIT_PACK_AVB:
			begin
				p_wb_ADR_O <= deb_im + pixel_count;
				p_wb_DAT_O <= data_fifo;
				if (nb_pack_available) r_ack <= 1;
			end
		BREAK:
			begin
				p_wb_DAT_O <= data_fifo;
				p_wb_ADR_O <= deb_im + pixel_count;
				r_ack <= 1;
				p_wb_CYC_O <= 1;
			end
		WAIT_ACK:
			begin
				p_wb_STB_O <= 1;
				p_wb_CYC_O <= 1;
			end
		IMAGE_PROCESSED:
			interrupt <= 1;
	endcase
end

endmodule

/*
*	Caroline Kéramsi
*	Projet Étrange INF342
*	Télécom ParisTech
*/



/*
*	Ce module lit les pixels une image en RAM et
*	place les pixels dans la fifo
*/

module video_out_read
	#(parameter NBPACK = 16,
	parameter WIDTH = 640,
	HEIGHT = 480
	)
	(
	input wire clk,
	input wire nRST,
	input wire [31:0] wb_reg_data,
	input wire [31:0] wb_reg_ctr,
	
	output reg interrupt,
	
	input wire [31:0] p_wb_DAT_I,
	input wire p_wb_ACK_I,
	output reg p_wb_STB_O,
	output reg p_wb_CYC_O,
	output reg p_wb_LOCK_O,
	output reg [3:0] p_wb_SEL_O,
	output reg p_wb_WE_O,
	output reg [31:0] p_wb_ADR_O,

	//Pour écrire dans la fifo
	input wire full,
	output reg w_e,
	output reg [7:0] pixel_out
	);

//On utilise le wishbone en lecture seule
assign p_wb_WE_O = 0;
assign p_wb_SEL_O = 4'hf;
assign p_wb_LOCK_O = 0;

/*************************************************
*	Détection de l'arrivée d'une nouvelle adresse
***************************************************/

wire new_addr;
reg old_reg_ctr_0;

always_ff @(posedge clk)
	old_reg_ctr_0 <= wb_reg_ctr[0];

assign new_addr = (~old_reg_ctr_0 & wb_reg_ctr[0]);


/***************************************************
*			Machine à états
***************************************************/

/*
*	WAIT_ADDR :			On attend que le processeur fournisse une
*						adresse RAM
*	WRITE_FIFO :		On écrit les pixels lus en RAM dans la fifo
*	WAIT_FIFO :			On attend quand la fifo est pleine
*	READ_RAM :			Lecture en RAM
*	WAIT_ACK :			On attend le ack de la RAM
*	BREAK :				Temps d'arrêt entre chaque lecture
*	IMAGE_PROCESSED :	L'image est finie, on l'indique par une interruption
*/

enum logic [2:0] {WAIT_ADDR, WAIT_ACK, WRITE_FIFO, READ_RAM, IMAGE_PROCESSED} state, next_state;

//Adresse du début de l'image en RAM
reg [31:0] deb_im;

//Buffer pour les données à placer en fifo
reg [7:0] pack [NBPACK];

///////////////////////////////////////////
//			Compteurs
//////////////////////////////////////////

//Compteur pour se situer dans l'image
reg [19:0] next_pixel_count;
reg [19:0] pixel_count;
//Compteur pour se situer dans le paquet de pixels
reg [3:0] next_pack_count; 
reg [3:0] pack_count; 

assign pixel_out = pack[pack_count];

//Compteur pour maintenir l'interruption
reg [1:0] next_int_cnt;
reg [1:0] int_cnt;

//Mise à jour des compteurs:
always_ff @(posedge clk or negedge nRST)
if (~nRST)
begin
	int_cnt <= 0;
	pixel_count <= 0;
	pack_count <= 0;
end
else
begin
	int_cnt <= next_int_cnt;
	pixel_count <= next_pixel_count;
	pack_count <= next_pack_count;
end

//////////////Mise à jour de l'état/////////////////
always_ff @(posedge clk or negedge nRST)
if (~nRST)
	state <= WAIT_ADDR;
else
	state <= next_state;

///////////Calcul combinatoire de l'état suivant////////////////
always_comb
case (state)
	//On attend une nouvelle adresse pour commencer la lecture
	WAIT_ADDR:
	begin
		if (new_addr) next_state <= READ_RAM;
		next_int_cnt <= 0;
		next_pixel_count <= 0;
		next_pack_count <= 0;

	end
	//On passe directement à l'état WAIT_ACK
	READ_RAM:
		next_state <= WAIT_ACK;
	WAIT_ACK:
		if (p_wb_ACK_I) 
			begin
				next_pack_count <= pack_count + 4;
				next_pixel_count <= pixel_count + 4;
				if (pack_count != NBPACK-4) 
					begin
						next_state <= READ_RAM;
					end
				//Cas de la fin d'un paquet
				else
				begin
					next_pack_count <= 0;
					next_state <= WRITE_FIFO;
				end
			end


	WRITE_FIFO:
	//Si la fifo était pleine le cycle n'a servi a rien.
	//Il n'y a donc des changements que si la fifo n'était pas pleine
	if (~full)
		begin
		next_pack_count <= pack_count + 1;
			//Fin d'un paquet
			if (pack_count == NBPACK-1)
				begin
				next_state <= READ_RAM;
				//Fin d'une image
				if (pixel_count == WIDTH * HEIGHT)
					next_state <= IMAGE_PROCESSED;
				end
		end
	IMAGE_PROCESSED:
		if (int_cnt == 3)
			begin
				next_state <= WAIT_ADDR;
				next_int_cnt <= 0;
				next_pack_count <= 0;
				next_pixel_count <= 0;
			end
		else 
			begin
				next_state <= IMAGE_PROCESSED;
				next_int_cnt <= int_cnt + 1;
			end
endcase

///////////////Calcul combinatoire des sorties/////////////////
always_comb 
	begin
		//W_e vaut 0 presque partout
		w_e <= 0;
		case (state)
			WAIT_ADDR:
				begin
					p_wb_STB_O <= 0;
					p_wb_CYC_O <= 0;
					interrupt <= 0;
					deb_im <= wb_reg_data;
				end

			READ_RAM:
				begin
					p_wb_STB_O <= 1;
					p_wb_CYC_O <= 1;
					p_wb_ADR_O <= deb_im + pixel_count;
				end

			WAIT_ACK:
				begin
					pack[pack_count] <= p_wb_DAT_I[31:24];
					pack[pack_count+1] <= p_wb_DAT_I[23:16];
					pack[pack_count+2] <= p_wb_DAT_I[15:8];
					pack[pack_count+3] <= p_wb_DAT_I[7:0];
				end

			WRITE_FIFO:
				begin
					p_wb_STB_O <= 0;
					p_wb_CYC_O <= 0;
					w_e <= 1;
				end
			IMAGE_PROCESSED:
				begin
					interrupt <= 1;
				end
		endcase
	end
	
endmodule



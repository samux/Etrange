/**********************
*	Caroline Kéramsi
***********************/


module fill_cache
	#(
	parameter IM_WIDTH = 480,
	parameter IM_HEIGHT = 640,
	parameter ADDR_SIZE_W = 5, //Cache de largeur maximum 2**ADDR_SIZE_W
	parameter ADDR_SIZE_H = 5, //Cache de hauteur maximum 2**ADDR_SIZE_H
	parameter DATA_SIZE = 8,
	parameter )
	(
	//Signaux système
	input wire clk,
	input wire nRST,

	//Position de la zone à 
	//charger dans la RAM
	input wire [9:0] pixel_c_I, 
	input wire [9:0] pixel_l_I,
	input wire [ADDR_SIZE_W:0] cache_w_I,
	input wire [ADDR_SIZE_H:0] cache_h_I,
	input wire [31:0] im_addr_I,

	//Signaux de contrôle
	input wire go,
	output reg cache_ready,

	//Signaux wishbone pour le
	//dialogue avec la RAM
	input wire (31:0] p_wb_DAT_I,
	input wire p_wb_ACK_I,
	output reg p_wb_STB_O,
	output reg p_wb_CYC_O,
	output reg p_wb_LOCK_O,
	output reg [3:0] p_wb_SEL_O,
	output reg p_wb_WE_O,
	output reg [31:0] p_wb_ADR_O,

	//Vers la RAM interne
	//RAM de 
	output reg [DATA_SIZE-1:0] pixel_out,
	output reg [ADDR_SIZE_W +ADDR_SIZE_H -1 : 0] addr, 
	output reg w_e
	);

parameter C_W = 2**ADDR_SIZE_W;
parameter C_H = 2**ADDR_SIZE_H;

/********************************************************
*			Machine à états
*********************************************************/

/////////////////////////États////////////////////////////
/*
*	IDLE :	On est entre deux tuiles. On attend le signal
*			go pour échantillonner les entrées et commencer
*			le traitement.
*	
*	READ_RAM :	Positionnement des signaux wishbone pour
*				la lecture en RAM
*
*	WAIT_ACK :  On attend la ack du wishbone et on stocke
*				dans le pixel dans la ram
*	
*/

enum logic [1:0] {IDLE, READ_RAM, WAIT_ACK} state;

/////////////////Positions en RAM///////////////////////////
reg [9:0] pixel_c;
reg [9:0] pixel_l;
reg [ADDR_SIZE_W:0] cache_w;
reg [ADDR_SIZE_H:0] cache_h;
reg [31:0] im_addr;

//////////////Compteurs///////////////////////////////////
reg [ADDR_SIZE_W-1:0] cnt_line;
reg [ADDR_SIZE_H-1:0] cnt_column;


////////////Cacul séquentiel de l'état suivant/////////////
////////////et gestion des compteurs		 //////////////
always_ff @(posedge clk or negedge nRST)
if (~nRST)
	begin
		state <= IDLE;
		cnt_line <= 0;
		cnt_column <= 0;
	end
else
	case (state)
		IDLE:
			begin
				pixel_c <= pixel_c_I;
				pîxel_l <= pixel_l_I;
				cache_w <= cache_w_I;
				cache_h <= cache_h_I;
				im_addr <= im_addr_I;
				cnt_line <= 0;
				cnt_column <= 0;
				if (go)
					state <= READ_RAM;
			end
		READ_RAM:
				state <= WAIT_ACK;
		WAIT_ACK:
			if (p_wb_ACK_I) 
				begin
					state <= READ_RAM
					//Cas de la fin d'une ligne
					if (cnt_column == cache_w - 1)
						begin
							cnt_column <= 0;
							cnt_line <= cnt_line + 1;
						//Cas de la fin du cache
						if (cnt_line == cache_h -1) 
							state <= IDLE;
						end
				end
endcase

/////////Calcul combinatoire des sorties///////////////////
always_comb
begin
	p_wb_LOCK_O <= 0;
	p_wb_SEL_O <= 4'hf;
	p_wb_WE_O <= 0;
	w_e <= 0;
	case (state)
		IDLE:
		begin
			p_wb_STB_O <= 0;
			p_wb_CYC_O <= 0;
			w_e <= 0;
		end
		READ_RAM:
		begin
			p_wb_STB_O <= 1;
			p_wb_CYC_O <= 1;
			p_wb_ADR_O <= im_addr + (pixel_l + line_cnt)* WIDTH + pixel_c + column_cnt;
			w_e <= 0;
		end
		WAIT_ACK:
			begin
				p_wb_STB <= 1;
				p_wb_CYC_O <= 1;
				pixels_out <= p_wb_DAT_I;
				w_e <= 1;
			end
	endcase
end

endmodule;

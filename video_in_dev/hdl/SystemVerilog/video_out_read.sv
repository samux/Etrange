parameter NBPACK = 16; //Par paquet de 32 bits
parameter p_WIDTH = 640;
parameter p_HEIGHT = 480;


//Ce module lit les pixels une image en RAM
//et le place dans la fifo

module video_out_read (
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
	output reg pixel_out
	);

assign p_wb_LOCK_O = 0;
//On utilise le wishbone en lecture seule
assign p_wb_WE_O = 0;
assign p_wb_SEL_O = 4'hf;

//Détection de l'arrivée d'une nouvelle adresse
wire new_addr;
reg old_reg_ctr_0;

always_ff @(posedge clk)
	old_reg_ctr_0 <= wb_reg_ctr[0];

assign new_addr = (~old_reg_ctr_0 & ~wb_reg_ctr[0]);


//WAIT_ADDR : attend que le processeur fournisse une
//adresse RAM
//WRITE_FIFO : écrit les pixels lus en RAM dans la fifo
//WAIT_FIFO : attend quand la fifo est pleine
//READ_RAM : lecture en RAM
//WAIT_ACK : attend le ack de la RAM
//BREAK : temps d'arrêt entre chaque lecture
//IMAGE_PROCESSED : l'image est finie, on l'indique par une interruption


enum logic [2:0] {WAIT_ADDR, WAIT_ACK, BREAK, WRITE_FIFO, WAIT_FIFO, READ_RAM, IMAGE_PROCESSED} state, next_state;

//Adresse du début de l'image en RAM
reg [31:0] deb_im;

//Buffer pour les données à placer en fifo :
reg [7:0] pack [4*NBPACK];

//Compteur pour se situer dans l'image
reg [19:0] pixel_count;
//Compteur pour se situer dans le paquet de pixels
reg [NBPACK-1:0] pack_count; 

//Compteur pour maintenir l'interruption
reg [1:0] int_cnt;

always_ff @(posedge clk or negedge nRST)
if (~nRST)
	state <= WAIT_ADDR;
else
	state <= next_state;

//Calcul combinatoire de l'état suivant
always_comb
case (state)
	//On attend une nouvelle adresse pour commencer 
	//la lecture
	WAIT_ADDR:
		if (new_addr) next_state <= READ_RAM;
	//On passe directement à l'état WAIT_ACK
	READ_RAM:
		next_state <= WAIT_ACK;
	WAIT_ACK:
		if (p_wb_ACK_I) next_state <= BREAK;
	BREAK:
		if (pack_count != NBPACK) 
			next_state <= READ_RAM;
		//Cas de la fin d'un paquet
		else
		begin
			if (~full)
				next_state <= WRITE_FIFO;
			else
				next_state <= WAIT_FIFO;
		end


	//Si fin de l'écriture d'un paquet, on passe au paquet
	//suivant
	//Si fin de l'écriture d'un paquet et de l'image
	//On passe dans l'état IMAGE_PROCESSED
	WRITE_FIFO:
		if (pack_count == 1)
			begin
			next_state <= READ_RAM;
			if (pixel_count == p_WIDTH * p_HEIGHT)
				next_state <= IMAGE_PROCESSED;
			end
		else
			if (~full)
				next_state <= WRITE_FIFO;
			else
				next_state <= WAIT_FIFO;
	WAIT_FIFO:
		if (full)
			next_state <= WAIT_FIFO;
		else
			next_state <= WRITE_FIFO;
	IMAGE_PROCESSED:
		if (int_cnt == 3)
			next_state <= WAIT_ADDR;
		else 
			next_state <= IMAGE_PROCESSED;
endcase

//Calcul combinatoire des sorties
always_comb 
if (~nRST)
	begin
		p_wb_STB_O <= 0;
		p_wb_CYC_O <= 0;
		interrupt <= 0;
		w_e <= 0;
		p_wb_ADR_O <= 0;
		int_cnt <= 0;
		pixel_count <= 0;
		pack_count <= 0;

	end
else
	begin
		case (state)
			WAIT_ADDR:
				begin
					p_wb_STB_O <= 0;
					p_wb_CYC_O <= 0;
					interrupt <= 0;
					w_e <= 0;
					deb_im <= wb_reg_data;
					int_cnt <= 0;
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

			BREAK:
				begin
					p_wb_STB_O <= 0;
					p_wb_CYC_O <= 0;
					pack_count <= pack_count + 4;
				end

			WRITE_FIFO:
				begin
					w_e <= 1;
					pixel_out <= pack[pack_count];
					pack_count <= pack_count - 1;
				end

			WAIT_FIFO:
					w_e <= 0;

			IMAGE_PROCESSED:
				begin
					pixel_count <= 0;
					pack_count <= 0;
					int_cnt <= int_cnt + 1;
					interrupt <= 1;
				end
		endcase
	end
	
	



endmodule;


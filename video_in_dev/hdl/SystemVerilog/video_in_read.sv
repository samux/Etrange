//Ce module lit les signaux envoyes par display et place
//les pixels dans la fifo
parameter p_WIDTH = 640;
parameter p_HEIGHT = 480;
parameter p_LSYNC = 160;
parameter p_FSYNC = 40;



module video_in_read (
	input wire clk,
	input wire clk_in,
	input wire nRST,
	input wire line_valid,
	input wire frame_valid,
	input [7:0] pixel_in,
	output reg w_e,
	output wire [31:0] pixels_out
	);

//Pour situer la position courante au sein de l'image
reg [9:0] pixel_c;
reg [9:0] pixel_l;

reg problem; //TODO enlever !


//On va grouper les pixels par paquets de 4 avant
//de les stocker dans la fifo
union packed { logic [31:0] pack;
				struct packed {
					logic [7:0] pixel_0;
					logic [7:0] pixel_1;
					logic [7:0] pixel_2;
					logic [7:0] pixel_3;
				} pixels;
} data;

assign pixels_out = data.pack;

reg write_fifo_slow;

//Processus de stockage dans la fifo à 100 MHz
reg old_write_fifo_slow;
wire write_fifo_posedge;

assign write_fifo_posedge = (~old_write_fifo_slow & write_fifo_slow);

always @(posedge clk or negedge nRST)
if (~nRST)
	old_write_fifo_slow<= 0;
else
	old_write_fifo_slow <= write_fifo_slow;
	


always @(posedge clk or negedge nRST)
if (~nRST)
	w_e <= 0;
else
begin
	if (write_fifo_posedge)
		w_e <= 1;
	else 
		w_e <= 0;
end

//Processus de lecture à 25 Mhz
always @(posedge clk_in or negedge nRST)
begin
	//w_e vaut 0 sauf si le contraire est précisé
	write_fifo_slow <= 0;
	problem <= 0;
	if (nRST == 0)
	begin
		pixel_c <= 0;
		pixel_l <= 0;
	end
	else
	begin
		//Stockage d'un pixel dans la fifo
		//Attention, contrairement au module systemC, les
		//différents types d'erreurs ne génèrent qu'un
		//seul type de message d'erreur.
		if (line_valid && frame_valid)
		begin
			if (pixel_c < p_WIDTH && pixel_l < p_HEIGHT)
			begin
				case (pixel_c%4)
					0:
						data.pixels.pixel_0 <= pixel_in;
					1:
						data.pixels.pixel_1 <= pixel_in;
					2:
						data.pixels.pixel_2 <= pixel_in;
					3:
					begin
						data.pixels.pixel_3 <= pixel_in;
						write_fifo_slow <= 1;
					end
				endcase
				pixel_c <= pixel_c + 1;
				if (pixel_c == p_WIDTH-1)
				begin
					pixel_c <= 0;
					pixel_l <= pixel_l + 1;
					if (pixel_l == p_HEIGHT-1)
						pixel_l <= 0;
				end
			end
			else
			begin
				//pragma translate_off
				$display("Ligne trop grande 41! \n");
				$stop();
				problem <= 1;
				//pragma translate_on
			end
		end

		else if (frame_valid && !line_valid)
			if (pixel_c != 0 )
				begin
					//pragma translate_off
					$display("Ligne trop grande 2! \n");
					$stop();
					problem <= 1;
					//pragma translate_on
				end
				
		else if (!frame_valid && !line_valid)
			if (pixel_l != 0 || pixel_c !=0)
				begin
					//pragma translate_off
					$display("Image trop grande !\n");
					$stop();
					problem <= 1;
					//pragma translate_on
				end
	end
end

endmodule
	

//Ce module lit les signaux envoyes par display et place
//les pixels dans la fifo
parameter p_WIDTH = 640;
parameter p_HEIGHT = 480;
parameter p_LSYNC = 160;
parameter p_FSYNC = 40;



module video_in_read (
	input wire clk,
	input wire nRST,
	input wire line_valid,
	input wire frame_valid,
	input wire pixel_in,
	output reg w_e,
	output reg pixel_out
	);

//Pour situer la position courante au sein de l'image
reg [9:0] pixel_c;
reg [9:0] pixel_l;


assign pixel_out = pixel_in;

//Processus de lecture
always @(posedge clk or negedge nRST)
begin
	//w_e vaut 0 sauf si le contraire est précisé
	w_e <= 0;
	if (nRST == false)
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
		if (line_valid && frame_valid && pixel_c <p_WIDTH && pixel_l < p_HEIGHT)
		begin
			w_e <= 1;
			pixel_c <= pixel_c + 1;
		end
		else if (frame_valid && !line_valid && pixel_c == p_WIDTH)
		begin
			pixel_c =  0;
			pixel_l = pixel_l + 1;
		end
		else if (!frame_valid && pixel_c == p_WIDTH && pixel_l == (p_HEIGHT -1))
		begin
			pixel_c = 0;
			pixel_l = 0;
		end
		//pragma translate_off
		else
			$display("Probleme lors de l'acquisition des pixels video_in\n");
		//pragma translate_on
	end
end
			
			

		




					


	
				
end
	






endmodule
	

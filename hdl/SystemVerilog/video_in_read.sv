/*
*	Caroline Kéramsi
*	Projet Etrange INF342
*	Telecom ParisTech
*/

/*
*	Ce module lit le flux vidéo et
*	place les pixels correspondants 
*	dans la fifo.
*/

module video_in_read 
	#(
	parameter WIDTH = 640,
    parameter HEIGHT = 480
	)
	(
	//horloge système, 100 MHz
	input wire clk,

	//horloge du flux vidéo, 25 MHz
	input wire clk_in,

	input wire nRST,

	//Signaux du flux_vidéo
	input wire line_valid,
	input wire frame_valid,
	input [7:0] pixel_in,

	//Signal en provenance de video_in_store
	input wire new_addr,

	//Signaux en direction de la fifo
	output reg w_e,
	output reg [31:0] pixels_out
	);


/*
*	Il faut attendre la première adresse du processeur pour démarrer video_in. 
*	First_addr passe à 1 quand la première adresse 	est envoyée.
*	Ready passe à 1 pendant la première période "inter-image" qui suit le passage à 1 de first_interrupt. 
*	On peut alors commencer le stockage.
*/

reg ready;
reg first_addr;

always_ff @(posedge clk or negedge nRST)
if (~nRST)
	begin
		ready <= 0;
		first_addr <= 0;
	end
else 
	begin
		if (new_addr)
			first_addr <= 1;
		if (first_addr & ~frame_valid) 
			ready <= 1;
	end
	


/********************************************************************
*	Processus de stockage dans la fifo à 100 MHz.
*	On génère w_e à partir de write_fifo_slow
*******************************************************************/

reg write_fifo_slow;
reg old_write_fifo_slow;
wire write_fifo_posedge;

assign write_fifo_posedge = (~old_write_fifo_slow & write_fifo_slow);

always @(posedge clk or negedge nRST)
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



/**************************************************************
*		Processus de lecture des pixels à 25 MHz
*		On générère write_fifo_slow
**************************************************************/

//Compteurs pour situer la position courante au sein de l'image
reg [9:0] pixel_c; 
reg [9:0] pixel_l; 

always @(posedge clk_in)
begin
	//write_fifo_slow vaut 0 sauf si le contraire est précisé
	write_fifo_slow <= 0;
	if (~nRST)
		begin
			pixel_c <= 0;
			pixel_l <= 0;
			ready <= 0;
		end
	else
	begin
	//Stockage d'un pixel dans la fifo
		if (line_valid && frame_valid)
		begin
			if (pixel_c < WIDTH && pixel_l < HEIGHT)
			begin
				//On regroupe les pixels par 4 pour les stocker dans la fifo 32 bits
				case (pixel_c%4)
					0:
						pixels_out[31:24] <= pixel_in;
					1:
						pixels_out[23:16] <= pixel_in;
					2:
						pixels_out[15:8] <= pixel_in;
					3:
					begin
						pixels_out[7:0]  <= pixel_in;
						if (ready) write_fifo_slow <= 1;
					end
				endcase
				pixel_c <= pixel_c + 1;

				//Fin d'une ligne
				if (pixel_c == WIDTH-1)
				begin
					pixel_c <= 0;
					pixel_l <= pixel_l + 1;
					//Fin d'une image
					if (pixel_l == HEIGHT-1)
						pixel_l <= 0;
				end
			end
			else if (ready)
			begin
				//pragma translate_off
				$display("Ligne trop grande! \n");
				$stop();
				//pragma translate_on
			end
		end

		if (frame_valid && !line_valid)
			if (pixel_c != 0 && ready )
				begin
					//pragma translate_off
					$display("Ligne trop grande! \n");
					$stop();
					//pragma translate_on
				end
				
		if (!frame_valid && !line_valid)
			begin
			ready <= 1;
			pixel_c <= 0;
			pixel_l <= 0;
			end
	end
end

endmodule
	

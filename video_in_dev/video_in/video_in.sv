module video_in (
	input wire clk,
	input wire nRST,
	input wire line_valid,
	input wire frame_valid,
	input wire pixel_in
	);

//Signaux pour faire le lien video_in_read <---> fifo
wire w_e;
wire [7:0] pixel_fifo_in;
wire empty;
//Rmque : Si la fifo est pleine, on ne peut rien y faire
//Le module video_in_read ne peut pas attendre et ne vérifie
//Donc pas le signal full


video_in_read video_in_read (	
	//Signaux directement reliés aux signaux d'entrée
	.clk(clk),
	.nRST(nRST),
	.line_valid(line_valid),
	.frame_valid(frame_valid),
	.pixel_in(pixel_in),

	//Signaux reliés à la fifo
	.w_e(w_e),
	.full(full), //TODO à rajouter dans video_in_read
	.pixel_out(pixel_fifo_in)
);

fifo fifo(
	.clk(clk),
	.nRST(nRST),
	.data_in(pixel_fifo_in),
	.w_e(w_e),
	//TO DO data_out n'est relié à rien pour l'instant. Il faut 
	//changer ça
	.empty(empty)
	);
		



endmodule

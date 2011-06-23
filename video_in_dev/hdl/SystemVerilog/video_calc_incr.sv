module video_calc_incr (
	input wire clk, //horloge interne
	input wire reset_n,
	
	input wire go,
	input logic signed [25:0] [31:0] coeff;

	output logic [7:0] pixel_out
	);

logic signed [25:0] [31:0] coeff_image;
logic [7:0] count_c;
logic [7:0] count_line;
logic [7:0] count_pixel;
enum {Wait_Go, Process} state;



always_ff @(posedge clk or negedge reset_n)
begin
	if(~reset_n)
	begin
		state <= Wait_Go;
		count_c <= 0;
		count_pixel <= 0;
		count_line <= 0;

	end

	else
	begin

		case (state)

			Wait_Go:
			begin
				mage[2] <= coeff_image[10] + coeff_image[9];
					coeff_image[15] <= coeff_image[23] + coeff_image[22];if ( go )
				begin
					for ( count_c = 0; count_c < 8d26; count_c = count_c + 1 )
						coeff_image[count_c][31:0] <= coeff[count_c][31:0];
					state <= Process;
				end
			end

			Process:
			begin
				if ( count_pixel < 8d16 )
				begin
					// P x coeff
					coeff_image[3] <= coeff_image[3] + coeff_image[2];
					coeff_image[2] <= coeff_image[2] + coeff_image[1];
					coeff_image[1] <= coeff_image[1] + coeff_image[0];

					// P y coeff
					coeff_image[16] <= coeff_image[16] + coeff_image[15];
					coeff_image[15] <= coeff_image[15] + coeff_image[14];
					coeff_image[14] <= coeff_image[13] + coeff_image[12];

					count_pixel = count_pixel + 1;
				end
				else
				begin
					count_pixel <= 0;
					count_line <= count_line + 1;
					// Q x coeff
					coeff_image[7] <= coeff_image[7] + coeff_image[6];
					coeff_image[6] <= coeff_image[6] + coeff_image[5];
					coeff_image[5] <= coeff_image[5] + coeff_image[4];

					// Q y coeff
					coeff_image[20] <= coeff_image[20] + coeff_image[19];
					coeff_image[19] <= coeff_image[19] + coeff_image[18];
					coeff_image[18] <= coeff_image[18] + coeff_image[17];


					// R x coeff
					coeff_image[10] <= coeff_image[10] + coeff_image[9];
					coeff_image[9] <= coeff_image[9] + coeff_image[8];

					// R y coeff
					coeff_image[23] <= coeff_image[23] + coeff_image[22];
					coeff_image[22] <= coeff_image[22] + coeff_image[21];

					//S x coeff
					coeff_image[12] <= coeff_image[12] + coeff_image[11];
					//S y coeff
					coeff_image[25] <= coeff_image[25] + coeff_image[24];


					//Px3 = Qx3 + Qx2
					coeff_image[3] <= coeff_image[7] + coeff_image[6];
					coeff_image[16] <= coeff_image[20] + coeff_image[19];

					//Px2 = Rx2 + Rx1
					coeff_image[2] <= coeff_image[10] + coeff_image[9];
					coeff_image[15] <= coeff_image[23] + coeff_image[22];

					//Px1 = Rx1 + Rx0
					coeff_image[1] <= coeff_image[12] + coeff_image[11];
					coeff_image[14] <= coeff_image[25] + coeff_image[24];

				end
				if ( count_line == 8d16)
					state = Wait_Go;

				pixel_out <= (coeff_image[3] >> 16);

			end
		endcase
	end
end

endmodule

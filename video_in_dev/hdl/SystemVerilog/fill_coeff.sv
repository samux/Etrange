/*********************
*	Caroline Kéramsi
************************/

/*
*	Ce module est un wrapper qui permet
*	d'utiliser fill_cache pour aller chercher
*	26 coefficients en RAM
*/

module fill_coeff
	#(	//26 coefficients
		parameter COEFF_ADDR_SIZE = 5,
		parameter COEFF_NB = 26
	)

	(
	//Signaux système
	input wire clk,
	input wire nRST,

	//Zone de la RAM où se trouvent
	//les coefficients
	input wire coeff_addr,

	//Signaux de contrôle
	input wire go,
	output reg coeff_ready,

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

	//Vers la RAM interne qui contient le cache
	output reg [DATA_SIZE-1:0] coeffs_out,
	output reg [ADDR_SIZE_W +ADDR_SIZE_H -1 : 0] ram_addr, 
	output reg w_e
	);

	fill_cache #(
				ADDR_SIZE_W = COEFF_ADDR_SIZE,
				ADDR_SIZE_H = 1,
				DATA_SIZE = 32
				)
				(.clk(clk)
				.nRST(nRST,
				.im_addr(coeff_addr),
				.go(go),
				.cache_ready(coeff_ready),
				.p_wb_DAT_I(p_wb_DAT_I),
				.p_wb_STB_O(p_wb_STB_O),
				.p_wb_CYC_O(p_wb_CYC_O),
				.p_wb_LOCK_O(p_wb_LOCK_O),
				.p_wb_SEL_O(p_wb_SEL_O),
				.p_wb_WE_O(p_wb_WE_O),
				.p_wb_ADR_O(p_wb_ADR_O),
				.coeffs_out(pixels_out),
				.ram_addr(ram_addr)
				);


endmodule;

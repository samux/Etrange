/********************************
* Source sen.enst.fr
* Modifié par Caroline Kéramsi
*********************************/

parameter SLAVE_ADDR = 32'hb0000000;
parameter SIZE = 9;

module wb_simple_slave 
                        (
						input  p_clk,
                        input  p_resetn,
                        //  WB signals
                        input  [31:0] p_wb_DAT_I,
                        output [31:0] p_wb_DAT_O,
                        input  [31:0] p_wb_ADR_I,
                        output p_wb_ACK_O,
                        input  p_wb_CYC_I,
                        output p_wb_ERR_O,
                        input  p_wb_LOCK_I,
                        output p_wb_RTY_O,
                        input  [3:0]p_wb_SEL_I,
                        input  p_wb_STB_I,
                        input  p_wb_WE_I,
						output [31:0] wb_data_0,
						output [31:0] wb_data_1,
						output [31:0] wb_data_2,
						output [31:0] wb_data_3,
						output [31:0] wb_data_4,
						output [31:0] wb_data_5,
						output [31:0] wb_data_6,
						output [31:0] wb_data_7,
						output [31:0] wb_data_8

                        );

					
integer cycle;

//Tableau interne : 
reg [31:0] wb_data[SIZE];
logic [5:0] i;

assign wb_data_0 = wb_data[0];
assign wb_data_1 = wb_data[1];
assign wb_data_2 = wb_data[2];
assign wb_data_3 = wb_data[3];
assign wb_data_4 = wb_data[4];
assign wb_data_5 = wb_data[5];
assign wb_data_6 = wb_data[6];
assign wb_data_7 = wb_data[7];
assign wb_data_8 = wb_data[8];


// always acknowledge requests
assign p_wb_ACK_O = p_wb_STB_I && p_wb_STB_I;

assign p_wb_DAT_O = 0;

always_ff @(posedge p_clk)
begin
    if (!p_resetn)
    cycle = 0;
    else
	begin
		cycle++;
		//Remise à 0 des registres de contrôle
		wb_data[1] <= 0;
		wb_data[3] <= 0;
		wb_data[5] <= 0;
		wb_data[7] <= 0;
		if (p_wb_STB_I ) 
		begin
		$display("strobe");
			if (p_wb_CYC_I ) 
			begin
				$display("and cycle %x", p_wb_ADR_I);
				if (p_wb_WE_I && p_wb_ADR_I >= SLAVE_ADDR && p_wb_ADR_I < SLAVE_ADDR + 4*SIZE)
					begin
					$display("JKJLJ");
					i = (p_wb_ADR_I - SLAVE_ADDR)/4;
					$display("i : %x",(p_wb_ADR_I -SLAVE_ADDR)/4);
					wb_data[i] <= p_wb_DAT_I; 
//pragma translate_off
			$display ("SLAVE recieved Write REQ 0x%08X @cycle %d",p_wb_DAT_I,cycle);
					end
//pragma translate_on
            end
        end
    end
end

endmodule

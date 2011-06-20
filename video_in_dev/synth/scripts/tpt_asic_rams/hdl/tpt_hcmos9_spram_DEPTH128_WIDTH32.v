// TPT/COMELEC: Modèle générique Généré par gen_hcmos9_ram pour l'ue ELEC342 SOC  V1.0 Fri Jun 17 17:42:05 2011 
module tpt_hcmos9_spram_DEPTH128_WIDTH32 (CK, WRITE, AD, DI, DO);
  localparam       add_bit  = 4,
                   data_bit = 32,
                   nb_word  = 16;

  input                     CK,        /* operation on rising edge */
                            WRITE;     /*  write operation on CK rising edge,*/
  input  [add_bit-1:0]      AD;        /* address bus */
  input  [data_bit-1:0]     DI;        /* input data bus */
  output [data_bit-1:0]     DO;        /* output data bus */

  reg [data_bit-1:0] mem [nb_word-1:0] ;
  reg [add_bit-1:0] address_reg ;
  assign DO = mem[address_reg] ;
  always @(posedge CK)
    if (WRITE)
      mem[AD] <= DI ;
   else
     address_reg <= AD ;
endmodule 

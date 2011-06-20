// TPT/COMELEC: Modèle générique Généré par gen_hcmos9_ram pour l'ue ELEC342 SOC  V1.0 Mon Jun 28 18:00:01 2010 
module tpt_hcmos9_dpram_DEPTH256_WIDTH32 (CK, READ, WRITE, RAD, WAD, DI, DO);
  localparam       add_bit  = 8,
                   data_bit = 32,
                   nb_word  = 256;

  input                     CK,        /* operation on rising edge */
                            WRITE,     /*  write operation on CK rising edge,*/
                            READ;      /*  reaf operation on CK rising edge,*/
  input  [add_bit-1:0]      RAD;        /* address bus */
  input  [add_bit-1:0]      WAD;        /* address bus */
  input  [data_bit-1:0]     DI;        /* input data bus */
  output [data_bit-1:0]     DO;        /* output data bus */

  reg [data_bit-1:0] mem [nb_word-1:0] ;
  reg [add_bit-1:0] address_reg ;
  assign DO = mem[address_reg] ;
  always @(posedge CK)
  begin
    if (WRITE)
      mem[WAD] <= DI ;
    if (READ)
      address_reg <= RAD ;
  end
endmodule 

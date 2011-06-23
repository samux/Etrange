module controler (
                  // Signaux
                  input logic p_clk,
                  input logic p_resetn,
                  output logic p_interrupt,
                  // Connexion avec le bus
                  // wishbone slave
                  input logic [31:0] wb_reg_data,
                  input logic [31:0] wb_reg_ctr,
                  // Signaux wishbone master
                  output logic p_wb_STB_O,
	          output logic p_wb_CYC_O,
	          output logic p_wb_LOCK_O,
	          output logic [3:0] p_wb_SEL_O,
	          output logic p_wb_WE_O,
	          output logic [31:0] p_wb_ADR_O,
	          input logic p_wb_ACK_I,
	          output logic [31:0] p_wb_DAT_O
                  };

   // Machine à états du contrôleur
   enum                               {WAIT_ADR, GET_COEFF, INIT_CALC, FILL_CACHE, CALC_INCR} c_state, n_state;

   // registres internes
   logic                              old_wb_reg_ctr;

   // Signaux de synchro entre états
   logic                              new_adr;
   logic                              coeff_ok;

   // Variables internes
   logic [10:0]                         tile_nb;

   // Détection d'un front montant de wb_reg_ctr
   // Une nouvelle adresse est disponible
   always_ff @(posedge clk or negedge preset_n)
     if (~p_resetn)
       begin
          old_wb_reg_ctr <= 0;
          new_adr <= 0;
       end
     else
       old_wb_reg_ctr <= wb_reg_ctr[0];
   assign new_adr = (~old_wb_reg_ctr && wb_reg_ctr[0]);

   // Choix du nouvel état
   always_comb
     begin
        n_state <= c_state;
        case (c_state)
          WAIT_ADR:
            if (new_adr)
              n_state <= GET_COEFF;
          GET_COEFF:
            if (coeff_ok)
              n_state <= INIT_CALC;
          INIT_CALC:
            if (init_ok)
              n_state <= FILL_CACHE;
          FILL_CACHE:
            if (cache_rdy)
              n_state <= CALC_INCR;
          CALC_INCR:
            if (calc_incr_ok)
              begin
                 if (nb_tile == 1200)
                   n_state <= WAIT_ADR;
                 else
                   n_state <= GET_COEFF;
                 nb_tile <= nb_tile + 1;
              end
          default:
            n_state <= WAIT_ADR;
        endcase // case (c_state)
     end // always_comb

   // Vérification du nouvel état à chaque coup d'horloge
   always_ff @(posedge clk or negedge p_resetn)
     if (~p_resetn)
       c_state <= WAIT_ADR;
     else
       c_state <= n_state;

endmodule // controler



















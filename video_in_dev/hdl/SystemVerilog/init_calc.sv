module init_calc
  #(
    parameter IM_WIDTH = 480,
    parameter IM_HEIGHT = 640,
    parameter DATA_SIZE = 32,
    parameter ADDR_SIZE_W = 5, //Cache de largeur maximum 2**ADDR_SIZE_W
    parameter ADDR_SIZE_H = 5, //Cache de hauteur maximum 2**ADDR_SIZE_H
    )
   (
    // Signaux système
    input logic p_clk,
    input logic p_resetn,

    // Signaux de contrôle
    input logic [31:0] im_addr_I,
    input logic init_ok,
    output logic cache_rdy

    // signaux de validité de ce qui se trouve
    // en cache
    input logic [31:0] cache_x_in;
    input logic [31:0] cache_y_in;

    output logic [9:0] decalage_w;
    output logic [9:0] decalage_h;
    output logic [9:0] cache_w;
    output logic [9:0] cache_h;

    // Signaux Wishbone pour le dialog
    // avec la RAM
    input logic (31:0] p_wb_DAT_I,
    input logic p_wb_ACK_I,
    output logic p_wb_STB_O,
    output logic p_wb_CYC_O,
    output logic p_wb_LOCK_O,
    output logic [3:0] p_wb_SEL_O,
    output logic p_wb_WE_O,
    output logic [31:0] p_wb_ADR_O,

    //Vers la RAM interne qui contient le cache
    output logic [DATA_SIZE-1:0] pixels_out,
    output logic [ADDR_SIZE_W +ADDR_SIZE_H -1 : 0] ram_addr,
    output logic w_e
    );

   // Signaux internes
   logic [9:0]   cache_x;
   logic [9:0]   cache_y;
   logic [ADDR_SIZE_W:0] cache_w;
   logic [ADDR_SIZE_H:0] cache_h;
   logic                 cache_in;

    fill_cache fill_cache(
                         // Signaux système
                         .clk(p_clk),
                         .nRST(p_resetn),

                         // Position de la zone à
                         // charger en RAM
                         .pixel_c_I(cache_x),
                         .pixel_l_I(cache_y),
                         .cache_w_I(cache_w),
                         .cache_h_I(cache_h),
                         .im_addr_I(img_adr_I),

                         // Signaux de contrôle
                         .go(cache_in),
                         .cache_ready(cache_rdy),

                         // Signaux Wishbone pour le
                         // dialoge avec la RAM
                         .p_wb_DAT_I(p_wb_DAT_I),
                         .p_wb_ACK_I(p_wb_ACK_I),
                         .p_wb_STB_O(p_wb_STB_O),
                         .p_wb_CYC_O(p_wb_CYC_O),
                         .p_wb_LOCK_O(p_wb_LOCK_O),
                         .p_wb_SEL_O(p_wb_SEL_O),
                         .p_wb_WE_O(p_wb_WE_O),
                         .p_wb_ADR_O(p_wb_ADR_O),

                         // Vers la RAM interne qui contient
                         // le cache
                         .pixels_out(pixels_out),
                         .ram_addr(ram_addr),
                         .w_e(w_e)
                          )

   always @(posedge p_clk or negedge preset_n)
     if (~preset_n)
       cache_in <= 0;
     else
       if (init_ok)
         begin
            // Hors de l'image à droite et en bas
            if (cache_x_in > (IM_WIDTH - 1) || cache_y_in > (IM_HEIGHT - 1))
              cache_in <= 0;

            // Dépassement à droite
            else if ((cache_x_in + DATA_SIZE) > IM_WIDTH)
              begin
                 cache_w <= -cache_x_in + IM_WIDTH;
                 cache_in <= 1;
              end

            // Dépassement en bas
            else if ((cache_y_in + DATA_SIZE) > IM_HEIGHT)
              begin
                 cache_h = -cache_y_in + IM_HEIGHT;
                 cache_in <= 1;
              end

            else if (cache_x_in < 0)
              // Hors de l'image à gauche
              if (cache_x_in + DATA_SIZE -1 < 0)
                cache_in <= 0;
              else
                // Dépassement à gauche
                begin
                   cache_w <= DATA_SIZE + cache_x_in;
                   decalage_w <= - cache_x_in;
                   cache_x <= 0;
                   cache_in <= 1;
                end

            else if (cache_y_in < 0)
              // Hors de l'image en haut
              if (cache_y_in + DATA_SIZE -1 < 0)
                cache_in <= 0;
              else
                // Dépassement en haut
                begin
                   cache_h <= DATA_SIZE + cache_y_in;
                   decalage_h <= - cache_y_in;
                   cache_y <= 0;
                   cache_in <= 1;
                end

            // Dans l'image et pas de dépassement
            else
              begin
                 cache_x <= cache_x_in;
                 cache_y <= cache_y_in;
                 cache_w <= DATA_SIZE;
                 cache_h <= DATA_SIZE;
                 decalage_h <= 0;
                 decalage_w <= 0;
                 cache_in <= 1;
              end // else: !if(cache_y_in + DATA_SIZE -1 < 0)
         end // if (init_ok)
       else
         cache_in <= 0;

endmodule // init_calc




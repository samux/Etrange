module init_calc (input logic signed [31:0] cache_x,
                  input logic signed [31:0] cache_y,
                  output logic signed [31:0] cache_x_img,
                  output logic signed [31:0] cache_y_img,
                  output logic signed [31:0] cache_w_img,
                  output logic signed [31:0] cache_h_img,
                  output logic signed [31:0] decalage_w_cache,
                  output logic signed [31:0] decalage_h_cache,
                  output logic signedcache_in_img
                  );

   always_comb
     begin
        // Hors de l'image à droite et en bas
        if (cache_x > (640 - 1) || cache_y > (480 - 1))
          cache_in_img <= 0;

        // Dépassement à gauche
        else if (cache_x < 0)
          begin
             if (cache_x + 32 - 1 < 0)
               // Hors de l'image à gauche
               cache_in_img <= 0;
             else
               begin
                  cache_w_img <= 32 + cache_x;
                  decalage_w_cache <= - cache_x;
                  cache_x_img <= 0;
               end
          end // if (cache_x < 0)

        // Dépassement en haut
        else if (cache_y < 0)
          begin
             if (cache_y + 32 - 1 < 0)
               // Hors de l'image en haut
               cache_in_img <= 0;
             else
               begin
                  cache_h_img <= 32 + cache_y;
                  decalage_h_cache <= - cache_y;
                  cache_y_img <= 0;
               end
          end // if (cache_y < 0)

        // Dépassement à droite
        else if ((cache_x + 32) > 640)
          cache_w_img <= -cache_x + 640;

        // Dépassement à gauche
        else if
        if ((cache_y + 32) > 480)
          cache_h_img <= -cache_y + 480;

     end // always_comb


   endfunction // init_calc



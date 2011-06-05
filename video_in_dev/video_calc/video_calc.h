#ifndef VIDEO_CALC_H
# define VIDEO_CALC_H

#include <systemc>
#include "wb_master_module.h"
#include "../segmentation.h"

// Largeur tuile traitée
# define T_W 16
// Hauteur tuile traitée
# define T_H 16
// Largeur zone buffer
# define B_W 32
// Hauteur zone buffer
# define B_H 32
// Taille FIFO en nb de tuiles
# define F_SIZE 4
// Nb tuiles d'entrée dans le buffer
# define T_IN_NB (p_WIDTH * p_HEIGHT) / (B_W * B_H)
// Nb tuiles de sortie
# define T_OUT_NB (p_WIDTH * p_HEIGHT) / (T_W * T_H)
// Nb tuiles d'entrée par ligne de buffer
# define T_IN_L_NB p_WIDTH / B_W
// Nb tuiles de sortie par ligne
# define T_OUT_L_NB p_WIDTH / T_W

#define tmpl(x) template<typename wb_param> x VideoCalc<wb_param>

using namespace sc_core;
using namespace std;

//Pour l'instant on se contente de lire puis de stocker les
//tules en ram sans effectuer aucune operation.

namespace soclib { namespace caba {
    template<typename wb_param>
      class VideoCalc:sc_module
    {
      public:
      // Ports E/S

      sc_in_clk	clk;
      sc_in<bool> reset_n;

      //Wishbone
      sc_core::sc_in<bool> p_clk;
      sc_core::sc_in<bool> p_resetn;
      soclib::caba::WbMaster<wb_param> p_wb;

      //Interruptions de fin d'écriture d'une image
      sc_out<bool> img_rdy;

      /////////////////////////////////////
      // Constructeur
      /////////////////////////////////////

      VideoCalc (sc_core::sc_module_name insname,
                 uint32_t * tab,
                 const int w = 640,		//largeur d'image par défaut
                 const int h = 480		//hauteur d'image par défaut
        );

      //////////////////////////////////////
      // Methodes et parametres internes
      /////////////////////////////////////
      void get_buffer();
      void process_tile();
      void store_tile();
      void process_center(int tile_nb);
      void process_invimg(int tile_nb, int invimg_c[T_H][T_W], int invimg_l[T_H][T_W]);
      void buffer_fill(uint32_t img_addr,int tile_nb);

      private:

      // Pour contenir la zone en buffer
      uint8_t buffer[B_H][B_W];

      // Coordonnées du pixel au centre de la zone de buffer
      int buffer_center_c;
      int buffer_center_l;

      // Numéro de la tuile en train d'être traitée.
      //   - Si vaut 0, c'est que l'on est entre deux images.
      //   - Si vaut n, on traite la nième tuile de l'image
      uint32_t nb_tile = 0;

      // paramètres de l'image
      const uint32_t p_WIDTH ;
      const uint32_t p_HEIGHT ;

      // Pour indiquer qu'il est temps de remplir
      // le buffer
      sc_signal<bool> ask_buffer;

      // Pour indiquer qu'il est temps de store
      // une tuile en ram
      sc_signal<bool> process_rdy;;

      // Pour indique que le buffer a été rempli
      sc_signal<bool> buffer_rdy;

      // Fifo de tuile à stocker en RAM
      sc_fifo<unsigned char> fifo;

      // Adresse de lecture et ecriture en RAM
      uint32_t * wb_tab;

      // Maître wishbone pour l'écriture et la lecture en RAM
      WbMasterModule<wb_param> master0;

      protected:
      SC_HAS_PROCESS(VideoCalc);

    };
  }}
#endif //VIDEO_CALC_H





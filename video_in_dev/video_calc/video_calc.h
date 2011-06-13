#ifndef VIDEO_CALC_H
# define VIDEO_CALC_H

#include <systemc>
#include "wb_master_module.h"
#include "../segmentation.h"

// Largeur tuile traitée
# define T_W 16
// Hauteur tuile traitée
# define T_H 16
// Taille FIFO en nb de tuiles
# define F_SIZE 2
// Nb tuiles de sortie
# define T_NB (p_WIDTH * p_HEIGHT) / (T_W * T_H)
// Nb tuiles de sortie par ligne
# define T_L_NB p_WIDTH / T_W

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

      sc_core::sc_in<bool> p_clk;
      sc_core::sc_in<bool> p_resetn;
      sc_out<bool> p_interrupt;

      soclib::caba::WbMaster<wb_param> p_wb_read;
      soclib::caba::WbMaster<wb_param> p_wb_write;

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
      void get_tile();
      void store_tile();

      private:

      // paramètres de l'image
      const uint32_t p_WIDTH ;
      const uint32_t p_HEIGHT ;

      // Fifo de tuile à stocker en RAM
      sc_fifo<unsigned char> fifo;

      // variable contenant l'adresse des images
      uint32_t deb_im_in;
      uint32_t deb_im_out;

      // Adresse de lecture et ecriture en RAM
      uint32_t * wb_tab;

      bool get_tile_end;

      // Maître wishbone pour la lecture en RAM
      WbMasterModule<wb_param> master0;

      // Maître wishbone pour l'écriture en RAM
      WbMasterModule<wb_param> master1;

      protected:
      SC_HAS_PROCESS(VideoCalc);

    };
  }}
#endif //VIDEO_CALC_H





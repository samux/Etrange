#ifndef VIDEO_CALC_H
# define VIDEO_CALC_H

#include <systemc>
#include "wb_master_module.h"
#include "../segmentation.h"

// Largeur tuile traitée
# define T_W 16
// Hauteur tuile traitée
# define T_H 16
// Largeur du cache
# define C_W 48
// Hauteur du cache
# define C_H 48
// Taille FIFO en nb de tuiles
# define F_SIZE 2
// Nb tuiles de sortie
# define T_NB (640 * 480) / (16 * 16)
// Nb tuiles de sortie par ligne
# define T_L_NB p_WIDTH / T_W
// Pixel Blanc
# define PIXEL_BLANC 255
// Pixel Noir
# define PIXEL_NOIR 0
// Nb Coeffs
# define NB_COEFF 26

#define tmpl(x) template<typename wb_param> x VideoCalc<wb_param>

typedef union
{
  int32_t raw[NB_COEFF + 2];
  struct
  {
    int32_t Px[4], Qx[4], Rx[3], Sx[2], cache_x;
    int32_t Py[4], Qy[4], Ry[3], Sy[2], cache_y;
  } reg;
} cic_u;

using namespace sc_core;
using namespace std;


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
      void process_tile();
      void store_tile();

      void fill_cache(uint32_t deb_im_in, int tile_nb);
      void init_coeff();
		int32_t fx_mul(int32_t A, int32_t B);

      private:

      // paramètres de l'image
      const uint32_t p_WIDTH ;
      const uint32_t p_HEIGHT ;

      // Fifo de tuile à stocker en RAM
      sc_fifo<uint8_t> fifo;

      // variable contenant l'adresse des images
      uint32_t deb_im_in;
      uint32_t deb_im_out;

      // Adresse de lecture et ecriture en RAM
      uint32_t * wb_tab;

      // Cache
      uint8_t cache[C_H][C_W];

      // Coordonnées du coin supérieur gauche
      // de la tuile en traitement
      int16_t cache_x;
      int16_t cache_y;

      // Process_tile demande à Get_tile de
      // remplir le cache
      bool ask_cache;

      // Get_tile indique à Process_tile que
      // le cache est remplit
      bool cache_rdy;

      // Tableau contenant les coeffs de chaque
      // tuile
      cic_u coeff[T_NB];
      cic_u coeff_image[T_NB];

      // Maître wishbone pour la lecture en RAM
      WbMasterModule<wb_param> master0;

      // Maître wishbone pour l'écriture en RAM
      WbMasterModule<wb_param> master1;

      protected:
      SC_HAS_PROCESS(VideoCalc);

    };
  }}
#endif //VIDEO_CALC_H





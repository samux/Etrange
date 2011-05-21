/**********************************************************************
 *
 *    Futur module de calcul de transformation géométrique
 *    Pour l'instant : 
 *    -Va chercher une zone a mettre en cache dans la ram,
 *    -Met une partie de cette zone en cache dans la tuile à stocker
 *    -Stocke la tuile en RAM
 *
 *********************************************************************/    

#include "video_calc.h"
#define DEBUG_CALC 0

#define tmpl(x) template<typename wb_param> x Video_in<wb_param>

namespace soclib { namespace caba {
/////////////////////////////////////////////////////////////////////
//	Constructeur
////////////////////////////////////////////////////////////////////
	
	tmpl(/**/)::VideoCalc(sc_core::sc_module_name insname,
			uint32_t * tab):
		sc_core::sc_module(insname),
		p_clk("p_clk"),p_resetn("p_resetn"),
		master0(p_clk,presetn,p_wb),

}

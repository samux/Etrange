/************************************************************
 *
 *      File : display.h
 *      Author: T. Graba
 *      Credits: A. Polti
 *      Telecom ParisTECH
 *
 ************************************************************/


#ifndef DISPLAY_H
#define DISPLAY_H

#include <systemc>
#include <SDL.h>

using namespace sc_core;
using namespace std;

namespace soclib { namespace caba {

    class Display
        :sc_module {

            public:
                // IO PORTS

                sc_in_clk           clk;
                sc_in<bool>         reset_n;

                sc_in <bool>        line_valid;
                sc_in <bool>        frame_valid;

                sc_in<unsigned char> pixel_in;


                ////////////////////////////////////////////////////
                //	constructor
                ////////////////////////////////////////////////////

                Display (sc_module_name insname,
                        const int w = 640,          // largeur d'image par defaut
                        const int h = 480,          // hauteur par defaut
                        const int lsync = 160,      // synchro ligne par defaut
                        const int fsync = 40        // synchro trame par defaut
                        );
                ////////////////////////////////////////////////////
                //	destructor
                ////////////////////////////////////////////////////

                ~Display ();

                ////////////////////////////////////////////////////
                //	methods and structural parameters
                ////////////////////////////////////////////////////
                void read_pixels();

                bool must_update_display;           // refresh display

            private:

                // paramètres de l'image
                const uint32_t  p_WIDTH      ;
                const uint32_t  p_HEIGHT     ;
                const uint32_t  p_LINE_SYNC  ;
                const uint32_t  p_FRAME_SYNC ;

                unsigned int pixel_c; 
                unsigned int pixel_l; 

                // SDL frame
                SDL_Surface *fb_memory; 
                SDL_Thread *thread;

                void update_display();
                void update_display_line(int);

            protected:
                SC_HAS_PROCESS(Display);

        };

}}
#endif //DISPLAY_H


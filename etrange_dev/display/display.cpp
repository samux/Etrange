/************************************************************
 *
 *      File : display.cpp
 *      Author: T. Graba
 *      Credits: A. Polti
 *      Telecom ParisTECH
 *
 ************************************************************/


#include "display.h"

namespace soclib { namespace caba {

    // le constructeur 
    Display::Display (sc_core::sc_module_name insname, 
            const int w , const int h ,
            const int lsync , const int fsync ):
        sc_core::sc_module(insname), 
        p_WIDTH(w), p_HEIGHT(h), p_LINE_SYNC(lsync), p_FRAME_SYNC(fsync)
    {

        SC_THREAD(read_pixels);
        sensitive << clk.pos();
        dont_initialize();

        // SDL INIT
        if ( SDL_Init(SDL_INIT_VIDEO) < 0 )
        {
            cerr << "Unable to initilize SDL: " 
                << SDL_GetError() << endl;
            exit(1);
        }

        // allocating frame pointer
        fb_memory = SDL_SetVideoMode(p_WIDTH, p_HEIGHT, 32, SDL_SWSURFACE); // 32 bits per pixel
        if ( fb_memory == NULL ) {
            cerr <<  "Unable to create  " 
                << p_WIDTH <<"x"<< p_HEIGHT << "display with 1 bpp:" 
                << SDL_GetError() << endl;
            exit(1);
        }

        // debut de l'image
        pixel_c = 0;
        pixel_l = 0;

        std::cout << "Display "  << name()
            << " was created succesfully " << std::endl;
    }

    // le destructeur
    Display::~Display ()
    {
        SDL_FreeSurface(fb_memory);
        SDL_Quit();
    }

    // mise à jour de l'image SDL
    void Display::update_display()
    {
        SDL_UpdateRect(fb_memory, 0, 0, 0, 0);
    }

    // mise à jour d'une ligne de l'image SDL
    void Display::update_display_line(int line)
    {
        SDL_UpdateRect(fb_memory, 0, line, p_WIDTH, 1);
    }

    // Thread de lecture des pixels
    void Display::read_pixels()
    {
        Uint32 *buf;
        Uint32 color;

        while(true)
        {
            if(reset_n == false)
            {
reset:
                pixel_c = 0;
                pixel_l = 0;
                wait();
            }
            else
            {
                //                    for (int i =0 ; i< (p_HEIGHT + p_FRAME_SYNC); i++)
                //                    for (int j = 0; j< (p_WIDTH +  p_LINE_SYNC) ; j++)

                // image valide
                if (line_valid && frame_valid)
                {
                    //cout << name() << " valid ..." <<  " lines : " << pixel_l << "col :" << pixel_c << endl;
                    if (pixel_c < p_WIDTH && pixel_l < p_HEIGHT)
                    {
                        // image en niveau de gris => R, G, et B sont egaux
                        color = SDL_MapRGB(fb_memory->format, pixel_in.read(),pixel_in.read(),pixel_in.read());
                        buf  = (Uint32*) fb_memory->pixels + pixel_c + p_WIDTH*pixel_l;
                        *buf = (Uint32) color;
                        pixel_c++;
                    }
                    else
                    {
                        cout << name() << " WARNING: Too much pixels..!!!!!" 
                            << " lines : " << pixel_l << " col : " << pixel_c << endl;
                        exit(-1);
                    }
                }
                else {
                    // synchro horizontale
                    if (frame_valid && !line_valid)
                    {
                        if (pixel_c == p_WIDTH)
                        {
                            pixel_c = 0;
                            update_display_line(pixel_l);
                            pixel_l++;
                        }
                        else if (pixel_c != 0)
                        {
                            cout << name() << " Warning.........!!" 
                                << " lines : " << pixel_l << "col :" << pixel_c << endl;
                        }
                    }
                    // synchro verticale
                    else if (!frame_valid)
                    {
                        if((pixel_c == p_WIDTH) && (pixel_l == p_HEIGHT -1))
                        {
                            //cout << name() << " Updating frame " << endl;
                            //update_display();
                            pixel_c = 0;
                            pixel_l = 0;
                        }
                    }
                }

                wait();
                if(reset_n == false)
                    goto reset;
            }
        }
    }

}}

// Local Variables:
// tab-width: 4
// c-basic-offset: 4
// c-file-offsets:((innamespace . 0)(inline-open . 0))
// indent-tabs-mode: nil
// End:


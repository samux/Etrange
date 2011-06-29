/************************************************************
 *
 *      File : video_gen.cpp
 *      Author: T. Graba
 *      Credits: A. Polti
 *      Telecom ParisTECH
 *
 ************************************************************/

extern "C" {
// bibliothèques non c++
#include<png.h>

}

#include <cstdio>
#include "video_gen.h"

namespace soclib { namespace caba {

    // le constructeur 
    VideoGen::VideoGen (sc_core::sc_module_name insname, const char * bn,
            const int w , const int h ,
            const int lsync , const int fsync ):
        sc_core::sc_module(insname), base_name(bn),
        p_WIDTH(w), p_HEIGHT(h), p_LINE_SYNC(lsync), p_FRAME_SYNC(fsync)
    {
        image.pixel = NULL;
        current_image_number = 0;
        read_image();


        SC_THREAD(gen_sorties);
        sensitive << clk.pos();
        dont_initialize();

        std::cout << "Image generator "  << name()
            << " was created succesfully " << std::endl;
    }

    // la génération des sorties
    void VideoGen::gen_sorties()
    {
        while(1)
        {
            if(reset_n == false)
            {
reset:

#ifdef SOCLIB_MODULE_DEBUG
                cout << name() << " Reset ..." << endl;
#endif
                // Reset : on remet tous les paramètres à zéro
                current_image_number = 0;
                // on recarge l'image
                read_image();
                // on met les sorties à zero
                pixel_out = 0;
                line_valid = false;
                frame_valid = false;

                // Puis on attend le prochain coup d'horloge, ou un reset
                wait();
            }
            else
            {
                unsigned int i,j;
                for(i=0; i<( p_HEIGHT + p_FRAME_SYNC ); i++)
                    for(j=0; j<( p_WIDTH + p_LINE_SYNC ); j++)
                    {
                        // Si on est dans la fenêtre active, on sort le pixel courant
                        // Rappel : une trame video fait ( p_WIDTH + p_LINE_SYNC )*( p_HEIGHT + p_FRAME_SYNC ),
                        // l'image active est de p_WIDTH*p_HEIGHT
                        if((i<p_HEIGHT) && (j>p_LINE_SYNC-1))
                            pixel_out = image.pixel[i*image.width+(j-p_LINE_SYNC)];
                        else
                            pixel_out = 0xBB;

                        // Generation de line valid
                        line_valid = (i<p_HEIGHT) && (j>p_LINE_SYNC-1);

                        // Generation de frame valid
                        frame_valid = (i<p_HEIGHT);

                        // Puis on attend le prochain coup d'horloge, ou un reset
                        wait();

                        // Si on est reveillé par un reset, on revient à la case départ
                        // (les goto sont dans ce genre de cas bien pratique...)
                        if(reset_n == false)
                            goto reset;
                    }
#ifdef SOCLIB_MODULE_DEBUG
                cout << name()
                    << " Fin image " << base_name << " n° "
                    << current_image_number
                    << endl;
#endif

                // On a fini une image, on passe à la suivante
                current_image_number ++;
                read_image();         
            }
        }
    }



    // lecture de l'image...
    void VideoGen::read_image()
    {
        char filename[1024]; //grmpf, il vaudrait mieux faire un malloc... 
        FILE           *file;
        png_structp     png_ptr;
        png_infop       info_ptr;
        png_uint_32     width, height;
        int             bit_depth, color_type; 
        png_bytep      *row_pointers;
        unsigned int             i;  

        // Calcul du nom de la prochaine image.
        // Si ce nom ne correspond a aucun fichier, on remet l'index a zero
        sprintf(filename, "%s%02d.png", base_name, current_image_number);


        // on désalloue l'espace mémoire occupé par l'image précédente
        if (image.pixel)
            free(image.pixel);
        image.pixel = NULL;

        // on ouvre l'image
        file = fopen(filename, "rb");
        if(file == NULL) {
            // On reessaye la première image
            current_image_number = 0;
            sprintf(filename, "%s%02d.png", base_name, current_image_number);
            file = fopen(filename, "rb");
            // Si la première image n'existe pas on quite
            if(file == NULL) {
                cerr << name()
                    << " read_image : impossible d'ouvrir l'image source " 
                    << filename << endl;
                exit(-1);
            }
        }

        // Les structures de l'image png
        png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
        assert((png_ptr != NULL) && "png_create_read_struct...");

        info_ptr = png_create_info_struct(png_ptr);
        assert((info_ptr != NULL) && "png_create_info_struct...");

        if (setjmp(png_jmpbuf(png_ptr)))
        {
            png_destroy_read_struct(&png_ptr, &info_ptr, png_infopp_NULL);
            fclose(file);
            exit(-1);
        }

        png_init_io(png_ptr, file);

        png_read_info(png_ptr, info_ptr);
        png_get_IHDR(png_ptr, info_ptr, &width, &height, &bit_depth, &color_type,
                NULL, NULL, NULL);
        assert((bit_depth == 8) && "bit_depth!=8");
        if(color_type != PNG_COLOR_TYPE_GRAY) {
            cerr << name()
                << " Erreur sur le format de l'image d'entrée" << endl
                << "Le seul format PNG supporté est : "
                << "niveaux de gris, 8bpp, sans canal alpha" << endl;
            png_destroy_read_struct(&png_ptr, &info_ptr, png_infopp_NULL);
            fclose(file);
            exit(-1);
        }

        // on recupère la taille de l'image
        if(( width != p_WIDTH ) || ( height != p_HEIGHT ))
        {
            cerr <<  name() 
                << " l'image " << filename << " n'a pas les bonnes dimensions ("
                << p_WIDTH << "x" << p_HEIGHT << ")"<< endl;
            exit(-1);
        }

        image.width = width;
        image.height = height;
        // on alloue l'espace necessaire à l'image
        image.pixel = (unsigned char *)malloc(width * height * sizeof(unsigned char));
        assert( (image.pixel != NULL) && "alloc image failed");

        row_pointers = (png_bytep*) png_malloc(png_ptr, height * sizeof(png_bytep));
        assert((row_pointers != NULL) && "row_pointer");
        for (i = 0; i < height; i++)
            row_pointers[i] = (png_bytep)(&image.pixel[i*width]);
        png_set_rows(png_ptr, info_ptr, row_pointers);

        png_read_image(png_ptr, row_pointers);

        png_free(png_ptr, row_pointers);

        png_destroy_read_struct(&png_ptr, &info_ptr, NULL);

#ifdef SOCLIB_MODULE_DEBUG
        cout << name()
            << " Lecture de l'image " << filename << endl;
#endif
        fclose(file);
    }

}}

// Local Variables:
// tab-width: 4
// c-basic-offset: 4
// c-file-offsets:((innamespace . 0)(inline-open . 0))
// indent-tabs-mode: nil
// End:


/*
 *  gpppkill. X11/GTK+ program that kill pppd if it not recive a minimal amount 
 *  of bytes during certain time. It also plot the amount bytes/seg recived.
 *  Copyright (C) 1998  Oliver Schulze L.
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *  
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *  
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *  
 *  You can reach the author at: 
 *    oliver@pla.net.py
 *
 *  gpppkill Home Page:
 *    http://www.pla.net.py/home/oliver/gpppkill/
 *  
 *  A copy of the GNU General Public License is included with this program.
 */
#ifndef _GRAF_OSL_H_
#define _GRAF_OSL_H_

#include <iostream.h>
#include <stdio.h>
#include <gtk/gtk.h>
#include <math.h>

#include "gpppkill_config.h"
#include "gpppkill.h"

class graf {
	private:
		int x,							//posicion a dibujar la sigte barra
		    linea_ancho,	//se fue a gpppkill::linea_ancho //antes era DRAW_LINEA_ANCHO
		    cant_barras,
		    cambio,		//bandera para cambiar de escala. Ver comentario de la funcion.
		    cambiar_escala,	//bandera para ver si redibujo el grafico con una nueva escala.
		    factor_escala_minima,	//(DRAW_ALTO*10) == 1000 bytes
		    factor_escala_maxima,	//escala maxima (DRAW_ALTO*100) == 10000 bytes
		    incremento_escala,	//es la diferencia entre escalas
		    escala,	//escala minima. 
		    altura_in,
		    altura_out,
		    i,
		    max_in,
		    max_out;

		listac *barras_in,
		       *barras_out;

		GdkPixmap *pixmap;	// pixmap para el drawing_area
		GtkWidget *draw;	//Widget drawing_area
		GtkWidget *frame_graf;	//una copia local para ganar velocidad
		GdkPixmap *pix;	//pixmap auxiliar
		
		GdkRectangle update_rect;
		nodoc *ini_in,
					*ini_out,
					*fin;

		class gpppkill *gpppk;

		//Colores
		GdkGC *gc;
		GdkColor *rojo, 
						 *verde, 
						 *amarillo,
						 *color_linea;

		void color_init(void);

		// callbacks
		//X Window events
		friend gint graf_expose_event_callback (GtkWidget *widget, GdkEventExpose *event, graf *grafico);
		friend gint graf_configure_event_callback(GtkWidget *widget, GdkEventConfigure *event, graf *grafico);

	public:
		graf(class gpppkill *g);
		~graf();

		void crear_draw(void);
		void graf_init(void);
		void dibujar(void);
		void dibujar_reset(void);
		//void update_pixmap(void);
		void borrar(void);
		void copiar_pixmap(void);
		void mover(void);
		void linea_in(int pos_y, int alto);
		void linea_out(int pos_y, int alto);
		void linea_inout(int pos_y, int alto);

		//GdkPixmap *getpixmap(void);
		GtkWidget *getdraw(void);

		//void setpixmap(GdkPixmap *pix);
		void setlinea_ancho(int la);
		void setframe_graf(GtkWidget *f);
} ;


#endif


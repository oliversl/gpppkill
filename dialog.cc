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
 *    oliversl@gmail.com
 *
 *  gpppkill Home Page:
 *    http://www.pla.net.py/home/oliver/gpppkill/
 *  
 *  A copy of the GNU General Public License is included with this program.
 */
#include "dialog.h"

// ############################### class dialog ################################

/*------------------------------------------------------------------------------
 *	Pregunta si se quiere terminar el enlace por causa de alguna opcion.
 *	return:
 *		0 -> no se quiere terminar el enlace ppp
 *		1 -> si se quiere terminar el enlace ppp
 */
dialog::dialog(void)
{
	window = gtk_window_new(GTK_WINDOW_TOPLEVEL);

	//gtk_container_border_width (GTK_CONTAINER (window), 2*CONTAINER_ANCHO);
	gtk_container_set_border_width (GTK_CONTAINER (window), 2*CONTAINER_ANCHO);

	g_signal_connect (G_OBJECT(window), "destroy",
											G_CALLBACK(gtk_widget_destroyed),
											&window);

	/* gtk_window_set_policy(window, allow_shrink, allow_grow, auto_shrink);
	 * para que no se pueda cambiar el tamanho 
	 */
	gtk_window_set_resizable(GTK_WINDOW(window), FALSE);

	construir_box();

	resultado = 0;
}

/*------------------------------------------------------------------------------
 *	Construir los vbox y hbox.
 */
void dialog::construir_box(void)
{
	// el dialog es un vbox dentro de una ventana
	vbox_main = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
	gtk_container_add(GTK_CONTAINER(window), vbox_main);

	// un vbox encima del separator
	vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
	gtk_box_pack_start( GTK_BOX(vbox_main), vbox, FALSE, FALSE, 2);

	// el separator
	separator = gtk_separator_new(GTK_ORIENTATION_HORIZONTAL);
	gtk_box_pack_start(GTK_BOX(vbox_main), separator, FALSE, TRUE, 5);

	// un hbox debajo del saparator, para los botones.
	hbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
	gtk_box_pack_start( GTK_BOX(vbox_main), hbox, FALSE, FALSE, 5);
}

/*------------------------------------------------------------------------------
*/
GtkWidget *dialog::getwindow(void)
{
	return window;
}

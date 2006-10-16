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
#include "messagebox.h"

messagebox::messagebox(char *str, class gpppkill	*g)
{
	cad = str;
	gpppk = g;
}

/*------------------------------------------------------------------------------
 */
int messagebox::mostrar()
{
	cargar_widget();
	conectar_signal();
	
	//No permitir focus en la ventana pppkill
	gtk_window_set_modal(GTK_WINDOW(window), TRUE);
	//esto es modal para mi. Por fin! gtk 1.2 hiso esta funcion!
	gtk_window_set_transient_for(GTK_WINDOW(window), GTK_WINDOW(gpppk->getwindow()) );
	gtk_widget_show_all(window);

	gtk_main();

	return resultado;
}

/*------------------------------------------------------------------------------
 */
void messagebox::cargar_widget()
{
	// propiedades de la ventana
	gtk_window_set_wmclass(GTK_WINDOW (window), "gpppkill_messagebox", "Gpppkill");
	gtk_window_set_title (GTK_WINDOW (window), PROGRAMA " Message:");
	gtk_window_set_position(GTK_WINDOW(window), GTK_WIN_POS_CENTER);

	// label a mostrar
	label = gtk_label_new((gchar *)cad);
	gtk_label_set_justify(GTK_LABEL(label), GTK_JUSTIFY_CENTER);
	gtk_box_pack_start (GTK_BOX(vbox), label, FALSE, FALSE, 5);

	// unico boton.
	button = gtk_button_new_with_label ("  OK  ");
	GTK_WIDGET_SET_FLAGS (button, GTK_CAN_DEFAULT);	
	gtk_box_pack_start ( GTK_BOX(hbox), button, TRUE, FALSE, 10);
	gtk_widget_grab_default (button);
}

/*------------------------------------------------------------------------------
 */
void messagebox::conectar_signal()
{
	// signal de la ventana
	gtk_signal_connect (GTK_OBJECT(window), "delete_event",
											GTK_SIGNAL_FUNC(messagebox_delete_event_callback),
											this);

	// signal del boton OK
	gtk_signal_connect (GTK_OBJECT(button), "clicked",
											GTK_SIGNAL_FUNC(messagebox_clicked_callback),
											this);
}

/*------------------------------------------------------------------------------
 */
void messagebox::quit()
{
	resultado = 0;
	gtk_main_quit();
	gtk_widget_destroy(window);
}

/*------------------------------------------------------------------------------
 */
gint messagebox_clicked_callback(GtkButton *button, class messagebox *mbox)
{
	mbox->quit();
	return TRUE;
}

/*------------------------------------------------------------------------------
 *	Para messagebox()
 *	Si retorno FALSE, gtk envia la signal destroy al widget
 */
gint messagebox_delete_event_callback(GtkWidget *widget, GdkEventAny *event, class messagebox *mbox)
{
	mbox->quit();
	return TRUE;
}

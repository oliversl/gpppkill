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
#ifndef _DIALOG_OSL_H_
#define _DIALOG_OSL_H_

#include <iostream.h>
#include <stdio.h>
#include <gtk/gtk.h>
#include <sys/types.h>

#include "gpppkill_config.h"

class dialog {
	protected:
		GtkWidget	*window,
		          *vbox_main,
		          *vbox,
		          *separator,
		          *hbox;
		int resultado;

		void construir_box(void);
		virtual void cargar_widget(void) = 0;
		virtual void conectar_signal(void) = 0;
		virtual void quit(void) = 0;
		
	public:
		dialog();
		//~dialog();	//no usar si no es un destructor virtual

		GtkWidget *getwindow(void);

		virtual int mostrar(void) = 0;
} ; 

#endif


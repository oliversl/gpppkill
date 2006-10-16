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
#ifndef _WARNING_OSL_H_
#define _WARNING_OSL_H_

#include <iostream.h>
#include <stdio.h>
#include <gtk/gtk.h>
#include <gdk/gdkkeysyms.h> // GDK_Escape
#include <sys/types.h>

#include "gpppkill_config.h"
#include "gpppkill.h"

class warning : public dialog {
	private:
		GtkWidget *label,
		          //*label_time,
		          *pbar,
		          *ok_button,
		          *cancel_button;
		GtkTooltips *tooltips;
		class gpppkill *gpppk;	//solo se usa para hacer modal esta ventana
		gint timeout_id,
		     intervalo;
		gfloat incremento;
		pid_t pppd_pid;
		int tiempo_total,	//tiempo en mseg a esperar. Warntime en mseg.
		    causa;
		char interface[10],
		     warn_beep;

		void conectar_signal(void);
		void cargar_widget(void);
		void quit(void);

		void cargar_tabla(void);

		// callbacks
		friend gint warning_delete_event_callback(GtkWidget *widget, GdkEventAny *event, warning *w);
		friend gint warning_ok_callback(GtkButton *button, warning *w);
		friend gint warning_cancel_callback(GtkButton *button, warning *w);
		friend gboolean warning_key_callback(GtkWidget *widget, GdkEventKey *event, warning *w);
		friend gint warning_timeout_callback(warning *w);

	public:
		warning(class gpppkill *g, int opcion);
		
		int mostrar(void);

		// set
		void settiempo_total(int total);
		void setpppd_pid(pid_t pid);
		void setinterface(char *str);
		void setwarn_beep(char beep);
} ;

#endif


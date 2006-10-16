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
#ifndef	_PREFERENCES_HPP_OSL_
#define _PREFERENCES_HPP_OSL_

#include <iostream.h>
#include <gtk/gtk.h>

#include "gpppkill_config.h"
#include "dialog.h"

#include "gpppkill.h"

//----------- class preferences --------------------
class preferences : public dialog {
	private:
		GtkWidget *spinner_idle_h,
							*spinner_idle_m,
							*spinner_idle_s,
							*spinner_byte_in,
							*spinner_byte_out,
							*spinner_online_h,
							*spinner_online_m,
							*spinner_online_s,
							*combo,
							*scrolled_win,
							*clist_interface,
							*button_option_warn,
							*label_warntime,
							*label_warntime_spinner,
							*label_warntime_beep,
							*spinner_warntime,
							*button_warn_beep,
							*button_kill_warn,
							*idletime_on_button,
							*onlinetime_on_button,
							*button_ok,
							*button_cancel,
							*label_prefered_interface,
							*spinner_prefered,
							*button_refresh;

		class gpppkill *gpppk;
		GList *glist_interface;

		// banderas
		char flag_idletime,
		     flag_onlinetime,
		     flag_interface,
		     flag_byte_in,
		     flag_byte_out,
		     flag_option_warntime,
		     flag_warntime,
		     flag_warn_beep,
		     flag_idletime_button,
		     flag_onlinetime_button,
		     flag_prefered_interface;

		int row_ppp,		//fila del pppd monitoreado
		    row_select,	//fila del pppd a monitorear
		    num_ppp;    //numero de pppd que se encontraron.

		void conectar_signal(void);
		void cargar_widget(void);

		void cargar_notebook(void);
		void cargar_spinner_idle(GtkWidget *vbox);
		void cargar_spinner_online(GtkWidget *vbox);
		void cargar_byte_in(GtkWidget *vbox);
		void cargar_byte_out(GtkWidget *vbox);
		void cargar_interface(GtkWidget *vbox);
		void cargar_clist_interface(void);
		void cargar_botones(void);
		void cargar_option_warning(GtkWidget *vbox);
		void cargar_idletime_on(GtkWidget *vbox);
		void cargar_prefered_interface(GtkWidget *vbox);
		void cargar_onlinetime_on(GtkWidget *vbox);
		void option_warn_sensitive(void);
		void button_ok_sensitive(void);
		void ok(void);
		void cancel(void);
		void quit(void);

		// callbacks
		friend gint preferences_ok_callback(GtkButton *button, preferences *pref);
		friend gint preferences_cancel_callback(GtkButton *button, preferences *pref);
		friend gint preferences_idletime_callback(GtkEditable *editable, preferences *pref);		
		friend gint preferences_onlinetime_callback(GtkEditable *editable, preferences *pref);
		friend gint preferences_byte_in_callback(GtkEditable *editable, preferences *pref);
		friend gint preferences_byte_out_callback(GtkEditable *editable, preferences *pref);
		friend gint preferences_idletime_button_callback(GtkToggleButton *toggle_button, preferences *pref);
		friend gint preferences_onlinetime_button_callback(GtkToggleButton *toggle_button, preferences *pref);
		//friend gint preferences_combo_callback(GtkEditable *editable, preferences *pref);
		friend gint preferences_spinner_prefered_callback(GtkEditable *editable, preferences *pref);
		friend gint preferences_refresh_callback(GtkButton *button, preferences *pref);
		friend gint preferences_select_row_callback(GtkWidget *widget, gint row, gint column, GdkEventButton *event, preferences *pref);
		friend gint preferences_delete_event_callback(GtkWidget *widget, GdkEventAny *event, preferences *pref);
		friend gint preferences_button_option_warn_callback(GtkToggleButton *toggle_button, preferences *pref);
		friend gint preferences_spinner_warntime_callback(GtkEditable *editable, preferences *pref);
		friend gint preferences_button_warn_beep_callback(GtkToggleButton *toggle_button, preferences *pref);

	public:
		preferences(class gpppkill *g);
		int mostrar(void);
};

#endif


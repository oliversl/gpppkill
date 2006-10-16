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
#ifndef	_GPPPKILL_HPP_OSL_
#define _GPPPKILL_HPP_OSL_

#include <iostream.h>
#include <gtk/gtk.h>
#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <getopt.h>
#include <math.h>

#include "gpppkill_config.h"
//#include "message.h"
//#include "callbacks.h"
#include "preferences.h"
#include "warning.h"
#include "messagebox.h"
#include "rcgpppkill.h"

#include "listac.h"
#include "graf.h"
#include "dialog.h"

//forward reference
//class preferences;

class gpppkill : public pppkill {
	private:
	  GtkWidget *window,
	            *dialog_window;
  	GtkWidget *vbox;
	
		gint      id_timeout,
						  sbar_index;

		GtkWidget *popup_menu;	//el pop-up menu

		//frames
		GtkWidget *frame_graf,
							*frame_stats,
							*frame_time,
							*frame_info,
							*frame_status;

		//statistic labels
		GtkWidget *label_in,
							*label_out,
							*label_total_in,
							*label_total_out,
							*label_promedio_in,
							*label_promedio_out,
							*label_promedio_total_in,
							*label_promedio_total_out;
		//pppkill labels
		GtkWidget *label_idletime,
							*e_box_idle,
							*label_onlinetime,
							*e_box_online;
		//info labels
		GtkWidget	*label_interface,
							*label_pid,
							*label_uptime;
							
		//opciones de gpppkill
		GtkWidget *idletime_menuitem,
							*onlinetime_menuitem;

		int pos_dx, 
			  pos_dy,
			  x, y, w, h;

		//warntime
		int warntime,
		    option_warntime;	//bandera para ver si uso esta opcion
		
		// banderas 
		char corriendo,	//para saber si pedir o no estadisticas.	
		     idletime_cl,	//cl == command line
			   onlinetime_cl, //cl == command line
			   labels_sensitive,	//para el estado de los labels.
			   salir,
			   warn_beep,
			   human_in,
			   human_out;
		
		class graf *grafico;

		void start(void);
		void continuar(void);
		void stop(void);
		void quit(void);
		void xkill_pppd(void);
		//void save_diff_wm(void);
		void save_window_position(void);

		void toggle_idletime_menuitem(void);
		void toggle_onlinetime_menuitem(void);

		void cargar_frame_graf(GtkWidget *box);
		void cargar_frame_stats(GtkWidget *vbox);
		void cargar_frame_time(GtkWidget *vbox);
		void cargar_frame_info(GtkWidget *vbox);
		void cargar_frame_status(GtkWidget *vbox);
		void crear_popup_menu(void);
		void color_init(void);

		void dibujar_frame_stats(void);
		void dibujar_frame_time(void);
		void dibujar_frame_info(void);
		void dibujar_update_frame_info(void);
		void dibujar_labels_sensitive(void);
		void dibujar_labels_insensitive(void);
		void dibujar_labels_reset(void);

		void show_graf(void);
		void show_stats(void);
		void show_time(void);
		void show_info(void);
		void show_status(void);
		int  hide_graf(void);
		int  hide_stats(void);
		int  hide_time(void);
		int  hide_info(void);
		int  hide_status(void);

		void quit_request(void);
		void mostrar_cursores(void);

		// callbacks		
		friend gint gpppkill_timeout_callback(gpppkill *gpppk);
		friend gint gpppkill_kill_timeout_callback(gpppkill *gpppk);
		friend gint gpppkill_status_timeout_callback(gpppkill *gpppk);
		friend gint gpppkill_button_press_callback (GtkWidget *widget, GdkEvent *event);
		friend gint gpppkill_about_pressed_callback(GtkMenuItem *item, class gpppkill *gpppk);
		friend gint gpppkill_pref_pressed_callback(GtkMenuItem *item, class gpppkill *gpppk);
		friend gint gpppkill_exit_pressed_callback(GtkMenuItem *item, class gpppkill *gpppk);
		friend gint gpppkill_idletime_menuitem_pressed_callback(GtkCheckMenuItem *item, class gpppkill *gpppk);
		friend gint gpppkill_onlinetime_menuitem_pressed_callback(GtkCheckMenuItem *item, class gpppkill *gpppk);
		friend gint gpppkill_ppp_kill_pressed_callback(GtkCheckMenuItem *item, class gpppkill *gpppk);
		friend gint gpppkill_view_graf_pressed_callback(GtkCheckMenuItem *item, class gpppkill *gpppk);
		friend gint gpppkill_view_stats_pressed_callback(GtkCheckMenuItem *item, class gpppkill *gpppk);
		friend gint gpppkill_view_idletime_pressed_callback(GtkCheckMenuItem *item, class gpppkill *gpppk);
		friend gint gpppkill_view_info_pressed_callback(GtkCheckMenuItem *item, class gpppkill *gpppk);
		friend gint gpppkill_view_status_pressed_callback(GtkCheckMenuItem *item, class gpppkill *gpppk);
		friend gint gpppkill_total_in_enter_callback(GtkWidget *widget, GdkEventCrossing *event, gpppkill *gpppk);
		friend gint gpppkill_total_in_leave_callback(GtkWidget *widget, GdkEventCrossing *event, gpppkill *gpppk);
		friend gint gpppkill_total_out_enter_callback(GtkWidget *widget, GdkEventCrossing *event, gpppkill *gpppk);
		friend gint gpppkill_total_out_leave_callback(GtkWidget *widget, GdkEventCrossing *event, gpppkill *gpppk);
		friend gint gpppkill_total_in_click_callback(GtkWidget *widget, GdkEventButton *event, gpppkill *gpppk);
		friend gint gpppkill_total_out_click_callback(GtkWidget *widget, GdkEventButton *event, gpppkill *gpppk);
		friend gint gpppkill_idletime_2click_callback(GtkWidget *widget, GdkEventButton *event, gpppkill *gpppk);
		friend gint gpppkill_onlinetime_2click_callback(GtkWidget *widget, GdkEventButton *event, gpppkill *gpppk);
/*
		friend gint gpppkill_idletime_enter_callback(GtkWidget *widget, GdkEventCrossing *event, gpppkill *gpppk);
		friend gint gpppkill_idletime_leave_callback(GtkWidget *widget, GdkEventCrossing *event, gpppkill *gpppk);
		friend gint gpppkill_onlinetime_enter_callback(GtkWidget *widget, GdkEventCrossing *event, gpppkill *gpppk);
		friend gint gpppkill_onlinetime_leave_callback(GtkWidget *widget, GdkEventCrossing *event, gpppkill *gpppk);
*/
		//X Window events
		friend gint gpppkill_delete_event_callback(GtkWidget *widget, GdkEventAny *event, class gpppkill *gpppk);
		friend gint gpppkill_configure_event_callback(GtkWidget *widget, GdkEventConfigure *event, class gpppkill *gpppk);
	public:
		gpppkill();
		~gpppkill();

		// replace viene despues
		int DRAW_ANCHO, 
		    DRAW_ALTO;

		class rcgpppkill *rc;

		int gpppkill_init(int argc, char *argv[]);
		void leer_rc(void);
		void crear_ventana(void);
		void cargar_ventana(void);
		void mostrar_ventana(void);

		// para los dialogs
		int warning_window(int causa);
		int preference_window(void);
		int message_box_window(char *str);
		int status(char *str, int mode);
		
		// get
		int       is_pppkill_running(void);
		gint			getid_timeout(void);
		GtkWidget *getwindow(void);
		GtkWidget *getframe_graf(void);
		GtkWidget *getidletime_menuitem(void);
		GtkWidget *getonlinetime_menuitem(void);
		//GtkWidget *getlabel_in(void);
		//GtkWidget *getlabel_out(void);
		//GtkWidget *getlabel_total_in(void);
		//GtkWidget *getlabel_total_out(void);
		//GtkWidget *getlabel_uptime(void);
		//GtkWidget *getlabel_idletime(void);
		//GtkWidget *getlabel_interface(void);
		//GtkWidget *getlabel_pid(void);
		int       getwarntime(void);

		// set
		//void setid_timeout(gint tout);
		void setwarntime(int warn);
		void setwarn_beep(int beep);
		void setx(int xpos);
		void sety(int ypos);
		void set_width(int new_w);
		void configure_size(int new_w, int new_h);
};

#endif


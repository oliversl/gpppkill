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
#include "preferences.h"

//########################## class preferences ###########################

//------------------------------------------------------------------------------
preferences::preferences(gpppkill *g)
{
	gpppk = g;
	glist_interface = NULL;

	flag_idletime = 0;
	flag_onlinetime = 0;
	flag_interface = 0;
	flag_byte_in = 0;
	flag_byte_out = 0;
	flag_option_warntime = 0;
	flag_warntime = 0;
	flag_warn_beep = 0;
	flag_idletime_button = 0;
	flag_onlinetime_button = 0;
	flag_prefered_interface = 0;
	
	row_ppp = -1;
	row_select = -1;
	num_ppp = 0;
}

/*------------------------------------------------------------------------------
 */
int preferences::mostrar(void)
{
	gtk_window_set_title(GTK_WINDOW(window), PROGRAMA " " VERSION " Preferences");
	//gtk_window_set_position( GTK_WINDOW(window), GTK_WIN_POS_MOUSE);
	gtk_window_set_position( GTK_WINDOW(window), GTK_WIN_POS_CENTER);

	cargar_widget();
	conectar_signal();

	//No permitir focus en la ventana pppkill
	gtk_window_set_modal(GTK_WINDOW(window), TRUE);
	//esto es modal para mi. Por fin! gtk 1.2 hiso esta funcion!
	gtk_window_set_transient_for(GTK_WINDOW(window), GTK_WINDOW(gpppk->getwindow()));
	gtk_widget_show_all(window);
	gtk_main();

	// loop

	g_list_free(glist_interface);
	glist_interface = NULL;
	
	//devolver la bandera de cambio de flag_interface
	resultado = flag_interface;

	return resultado;
}

/*------------------------------------------------------------------------------
 */
void preferences::cargar_widget(void)
{
	//cargar el notebook
	cargar_notebook();
	
	//cargo los botones
	cargar_botones();
}

/*------------------------------------------------------------------------------
 */
void preferences::conectar_signal(void)
{
	gtk_signal_connect (GTK_OBJECT(window), "delete_event",
												GTK_SIGNAL_FUNC(preferences_delete_event_callback),
												this);

	// para ver si se cambio de estado spinner_idle_*
	gtk_signal_connect (GTK_OBJECT(spinner_idle_h), "changed", 
                      GTK_SIGNAL_FUNC (preferences_idletime_callback), this);
	gtk_signal_connect (GTK_OBJECT(spinner_idle_m), "changed", 
                      GTK_SIGNAL_FUNC (preferences_idletime_callback), this);
	gtk_signal_connect (GTK_OBJECT(spinner_idle_s), "changed", 
                      GTK_SIGNAL_FUNC (preferences_idletime_callback), this);
/*
	gtk_signal_connect (GTK_OBJECT(&GTK_SPIN_BUTTON(spinner_idle_h)->entry), "key_release_event", 
                      GTK_SIGNAL_FUNC (preferences_idletime_callback), this);
	gtk_signal_connect (GTK_OBJECT(&GTK_SPIN_BUTTON(spinner_idle_m)->entry), "key_release_event", 
                      GTK_SIGNAL_FUNC (preferences_idletime_callback), this);
	gtk_signal_connect (GTK_OBJECT(&GTK_SPIN_BUTTON(spinner_idle_s)->entry), "key_release_event", 
                      GTK_SIGNAL_FUNC (preferences_idletime_callback), this);
*/
	// para ver si se cambio de estado spinner_online_*
	gtk_signal_connect (GTK_OBJECT(spinner_online_h), "changed", 
                      GTK_SIGNAL_FUNC (preferences_onlinetime_callback), this);
	gtk_signal_connect (GTK_OBJECT(spinner_online_m), "changed", 
                      GTK_SIGNAL_FUNC (preferences_onlinetime_callback), this);
	gtk_signal_connect (GTK_OBJECT(spinner_online_s), "changed", 
                      GTK_SIGNAL_FUNC (preferences_onlinetime_callback), this);
/*
	gtk_signal_connect (GTK_OBJECT(&GTK_SPIN_BUTTON(spinner_online_h)->entry), "key_release_event", 
                      GTK_SIGNAL_FUNC (preferences_onlinetime_callback), this);
	gtk_signal_connect (GTK_OBJECT(&GTK_SPIN_BUTTON(spinner_online_m)->entry), "key_release_event", 
                      GTK_SIGNAL_FUNC (preferences_onlinetime_callback), this);
	gtk_signal_connect (GTK_OBJECT(&GTK_SPIN_BUTTON(spinner_online_s)->entry), "key_release_event", 
                      GTK_SIGNAL_FUNC (preferences_onlinetime_callback), this);
*/
	// lo que cambia en realidad es el entry del widget combo. (combo->entry)
//	gtk_signal_connect (GTK_OBJECT(GTK_COMBO(combo)->entry), "changed", 
//                      GTK_SIGNAL_FUNC (preferences_combo_callback), this);

	// Interface
	gtk_signal_connect (GTK_OBJECT(spinner_prefered), "changed", 
                      GTK_SIGNAL_FUNC (preferences_spinner_prefered_callback), this);

	gtk_signal_connect (GTK_OBJECT(button_refresh), "clicked",
											GTK_SIGNAL_FUNC(preferences_refresh_callback), this);

	gtk_signal_connect(GTK_OBJECT(clist_interface), "select_row",
                  	 GTK_SIGNAL_FUNC(preferences_select_row_callback), this);

	// botones OK | CANCEL
	gtk_signal_connect (GTK_OBJECT(button_ok), "clicked",
											GTK_SIGNAL_FUNC(preferences_ok_callback),	this);

	gtk_signal_connect (GTK_OBJECT(button_cancel), "clicked",
											GTK_SIGNAL_FUNC(preferences_cancel_callback),	this);

	// max byte in, max byte out
	gtk_signal_connect (GTK_OBJECT(spinner_byte_in), "changed", 
                      GTK_SIGNAL_FUNC (preferences_byte_in_callback), this);

	gtk_signal_connect (GTK_OBJECT(spinner_byte_out), "changed", 
                      GTK_SIGNAL_FUNC (preferences_byte_out_callback), this);

	// warnnings
	gtk_signal_connect(GTK_OBJECT(button_option_warn), "toggled",
	                   GTK_SIGNAL_FUNC(preferences_button_option_warn_callback), this);

	gtk_signal_connect(GTK_OBJECT(spinner_warntime), "changed",
	                   GTK_SIGNAL_FUNC(preferences_spinner_warntime_callback), this);

	gtk_signal_connect(GTK_OBJECT(button_warn_beep), "toggled",
	                   GTK_SIGNAL_FUNC(preferences_button_warn_beep_callback), this);

	// start up options 
	gtk_signal_connect (GTK_OBJECT(idletime_on_button), "toggled",
                      GTK_SIGNAL_FUNC (preferences_idletime_button_callback), this);

	gtk_signal_connect (GTK_OBJECT(onlinetime_on_button), "toggled",
                      GTK_SIGNAL_FUNC (preferences_onlinetime_button_callback), this);

}

/*------------------------------------------------------------------------------
 */
void preferences::quit(void)
{
	gtk_widget_destroy(window);
	gtk_main_quit();
}

/*------------------------------------------------------------------------------
 *	Carga el notebook a un vbox.
 */
void preferences::cargar_notebook(void)
{
	GtkWidget	*frame,
						*vbox_frame,
						*notebook,
						*label;

	//creo un notebook y lo coloco en el vbox_main
	notebook = gtk_notebook_new ();
	gtk_notebook_set_tab_pos (GTK_NOTEBOOK (notebook), GTK_POS_TOP);
	gtk_box_pack_start(GTK_BOX(vbox), notebook, FALSE, FALSE, 5);

	// --- pagina UNO ---
	//creo un frame y lo coloco en la primera hoja del notebook
	frame = gtk_frame_new("pppkill options");
	//gtk_container_border_width(GTK_CONTAINER(frame), CONTAINER_ANCHO);
	gtk_container_set_border_width(GTK_CONTAINER(frame), CONTAINER_ANCHO);
	label = gtk_label_new(" Options times ");
	gtk_notebook_append_page(GTK_NOTEBOOK (notebook), frame, label);

	//creo un vbox y lo coloco dentro de frame
	vbox_frame = gtk_vbox_new(FALSE, 5);
	gtk_container_add(GTK_CONTAINER(frame), vbox_frame);

	//cargo el vbox_frame
	cargar_spinner_idle(vbox_frame);
	cargar_spinner_online(vbox_frame);
	//cargar_interface(vbox_frame);
	//cargar_clist_interface();

	// --- pagina DOS ---
	//creo un frame y lo coloco en la primera hoja del notebook
	frame = gtk_frame_new(" Detected pppds  ");
	//gtk_container_border_width(GTK_CONTAINER(frame), CONTAINER_ANCHO);
	gtk_container_set_border_width(GTK_CONTAINER(frame), CONTAINER_ANCHO);
	label = gtk_label_new(" Interfaces ");
	gtk_notebook_append_page(GTK_NOTEBOOK (notebook), frame, label);

	//creo un vbox y lo coloco dentro de frame
	vbox_frame = gtk_vbox_new(FALSE, 5);
	gtk_container_add(GTK_CONTAINER(frame), vbox_frame);

	//cargo el vbox_frame
	cargar_interface(vbox_frame);
	cargar_clist_interface();

	// --- pagina TRES ---
	frame = gtk_frame_new("Bytes in/out");
	//gtk_container_border_width(GTK_CONTAINER(frame), CONTAINER_ANCHO);
	gtk_container_set_border_width(GTK_CONTAINER(frame), CONTAINER_ANCHO);
	label = gtk_label_new(" Bytes ");
	gtk_notebook_append_page(GTK_NOTEBOOK (notebook), frame, label);

	//creo un vbox y lo coloco dentro de frame
	vbox_frame = gtk_vbox_new(FALSE, 5);
	gtk_container_add(GTK_CONTAINER(frame), vbox_frame);

	//cargar en contenido de la pagina 2
	cargar_byte_in(vbox_frame);
	cargar_byte_out(vbox_frame);

	// --- pagina CUATRO ---
	frame = gtk_frame_new("Warning messages");
	//gtk_container_border_width(GTK_CONTAINER(frame), CONTAINER_ANCHO);
	gtk_container_set_border_width(GTK_CONTAINER(frame), CONTAINER_ANCHO);
	label = gtk_label_new(" Warnings ");
	gtk_notebook_append_page(GTK_NOTEBOOK (notebook), frame, label);

	//creo un vbox y lo coloco dentro de frame
	vbox_frame = gtk_vbox_new(FALSE, 5);
	gtk_container_add(GTK_CONTAINER(frame), vbox_frame);

	//cargar en contenido de la pagina 3
	cargar_option_warning(vbox_frame);

	// --- pagina CINCO ---
	frame = gtk_frame_new("Startup options");
	//gtk_container_set_border_width(GTK_CONTAINER(frame), CONTAINER_ANCHO);
	//gtk_container_border_width(GTK_CONTAINER(frame), CONTAINER_ANCHO);
	gtk_container_set_border_width(GTK_CONTAINER(frame), CONTAINER_ANCHO);
	label = gtk_label_new(" Startup ");
	gtk_notebook_append_page(GTK_NOTEBOOK (notebook), frame, label);

	//creo un vbox y lo coloco dentro de frame
	vbox_frame = gtk_vbox_new(FALSE, 5);
	gtk_container_add(GTK_CONTAINER(frame), vbox_frame);

	//cargar en contenido de la pagina 4
	cargar_prefered_interface(vbox_frame);
	cargar_idletime_on(vbox_frame);
	cargar_onlinetime_on(vbox_frame);
}

/*------------------------------------------------------------------------------
 *	Carga un hbox con un label y spinner a un vbox.
 */
void preferences::cargar_spinner_idle(GtkWidget *vbox)
{
	GtkWidget	*hbox_aux,
						*label;
	GtkTooltips   *tooltips;
	GtkAdjustment *adj;

	//-------------- idletime ----------------
	hbox_aux  = gtk_hbox_new(FALSE, 0);

	label = gtk_label_new("Idle time limit:\n"
	                      "This es the maximum amount\n"
	                      "of idle (inactivity) time\n"
	                      "to wait before stoping pppd.");
	gtk_label_set_pattern(GTK_LABEL(label), 	"________________");
	gtk_label_set_justify(GTK_LABEL(label), GTK_JUSTIFY_LEFT);


	//colcar el label en el hbox
	gtk_box_pack_start(GTK_BOX(hbox_aux), label, FALSE, FALSE, BOX_PADDING);

	// --- spinner seconds
	adj = (GtkAdjustment *) gtk_adjustment_new (gpppk->getidletime()%60, 0.0,
																							59.0, 1.0, 
																							5.0    , 0.0);
	spinner_idle_s = gtk_spin_button_new (adj, 1.0, 0);
	gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (spinner_idle_s), TRUE);
	gtk_widget_set_usize(spinner_idle_s, 40, 0);
	gtk_spin_button_set_update_policy(GTK_SPIN_BUTTON(spinner_idle_s), GTK_UPDATE_IF_VALID);
	gtk_spin_button_set_numeric(GTK_SPIN_BUTTON(spinner_idle_s), TRUE);
	gtk_box_pack_end (GTK_BOX(hbox_aux), spinner_idle_s, FALSE, FALSE, BOX_PADDING);

	label = gtk_label_new("S:");
	gtk_box_pack_end(GTK_BOX(hbox_aux), label, FALSE, FALSE, 1);

	// --- spinner minute
	adj = (GtkAdjustment *) gtk_adjustment_new ( (gpppk->getidletime()/60)%60, 0.0, 
																							59.0, 1.0, 
																							5.0    , 0.0);
	spinner_idle_m = gtk_spin_button_new (adj, 1.0, 0);
	gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (spinner_idle_m), TRUE);
	gtk_widget_set_usize(spinner_idle_m, 40, 0);
	gtk_spin_button_set_numeric(GTK_SPIN_BUTTON(spinner_idle_m), TRUE);
	gtk_spin_button_set_update_policy(GTK_SPIN_BUTTON(spinner_idle_m), GTK_UPDATE_IF_VALID);
	gtk_box_pack_end (GTK_BOX(hbox_aux), spinner_idle_m, FALSE, FALSE, BOX_PADDING);

	label = gtk_label_new("M:");
	gtk_box_pack_end(GTK_BOX(hbox_aux), label, FALSE, FALSE, 1);

	// --- spinner hour
	adj = (GtkAdjustment *) gtk_adjustment_new (gpppk->getidletime()/3600, 0.0,
																							8.0, 1.0, 
																							5.0    , 0.0);
	spinner_idle_h = gtk_spin_button_new (adj, 1.0, 0);
	gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (spinner_idle_h), TRUE);
	gtk_widget_set_usize(spinner_idle_h, 65, 0);
	gtk_spin_button_set_numeric(GTK_SPIN_BUTTON(spinner_idle_h), TRUE);
	gtk_spin_button_set_update_policy(GTK_SPIN_BUTTON(spinner_idle_h), GTK_UPDATE_IF_VALID);
	gtk_box_pack_end (GTK_BOX(hbox_aux), spinner_idle_h, FALSE, FALSE, BOX_PADDING);

	label = gtk_label_new("H:");
	gtk_box_pack_end(GTK_BOX(hbox_aux), label, FALSE, FALSE, 1);

	// propiedades
	//gtk_spin_button_set_wrap(GTK_SPIN_BUTTON (spinner_idle_h), TRUE);
	//gtk_spin_button_set_update_policy(GTK_SPIN_BUTTON(spinner_idle_h), (GTK_UPDATE_ALWAYS | GTK_UPDATE_IF_VALID | GTK_UPDATE_SNAP_TO_TICKS));
	//GTK_SPIN_BUTTON(spinner_idle_h)->entry.editable.editable = 0;
	//GTK_SPIN_BUTTON(spinner_idle_m)->entry.editable.editable = 0;
	//GTK_SPIN_BUTTON(spinner_idle_s)->entry.editable.editable = 0;

	tooltips = gtk_tooltips_new();
	gtk_tooltips_set_tip(tooltips, spinner_idle_h, "hours", "");
	tooltips = gtk_tooltips_new();
	gtk_tooltips_set_tip(tooltips, spinner_idle_m, "minutes", "");
	tooltips = gtk_tooltips_new();
	gtk_tooltips_set_tip(tooltips, spinner_idle_s, "seconds", "");

	gtk_box_pack_start( GTK_BOX(vbox), hbox_aux, TRUE, TRUE, 3);
}

/*------------------------------------------------------------------------------
 *	Carga un hbox con un label y spinner a un vbox.
 */
void preferences::cargar_spinner_online(GtkWidget *vbox)
{
	GtkWidget	*hbox_aux,
						*label;
	GtkTooltips   *tooltips;
	GtkAdjustment *adj;

	//-------------- idletime ----------------
	hbox_aux  = gtk_hbox_new(FALSE, 0);

	label = gtk_label_new("Online time limit:\n"
												"This the maximum amount\n"
												"of time that the ppp \n"
												"will stay running.");
	gtk_label_set_pattern(GTK_LABEL(label), 	"__________________");
	gtk_label_set_justify(GTK_LABEL(label), GTK_JUSTIFY_LEFT);

	//colcar el label en el hbox
	gtk_box_pack_start(GTK_BOX(hbox_aux), label, FALSE, FALSE, BOX_PADDING);

	// --- second
	adj = (GtkAdjustment *) gtk_adjustment_new (gpppk->getonlinetime()%60, 0.0, 
																							59.0, 1.0, 
																							5.0    , 0.0);
	spinner_online_s = gtk_spin_button_new (adj, 1.0, 0);
	gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (spinner_online_s), TRUE);
	gtk_spin_button_set_numeric(GTK_SPIN_BUTTON(spinner_online_s), TRUE);
	gtk_widget_set_usize(spinner_online_s, 40, 0);
	gtk_box_pack_end (GTK_BOX(hbox_aux), spinner_online_s, FALSE, FALSE, BOX_PADDING);

	label = gtk_label_new("S:");
	gtk_box_pack_end(GTK_BOX(hbox_aux), label, FALSE, FALSE, 1);
	
	// --- minute
	adj = (GtkAdjustment *) gtk_adjustment_new ( (gpppk->getonlinetime()/60)%60, 0.0, 
																							59.0, 1.0, 
																							5.0    , 0.0);
	spinner_online_m = gtk_spin_button_new (adj, 1.0, 0);
	gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (spinner_online_m), TRUE);
	gtk_widget_set_usize(spinner_online_m, 40, 0);
	gtk_spin_button_set_numeric(GTK_SPIN_BUTTON(spinner_online_m), TRUE);
	gtk_box_pack_end (GTK_BOX(hbox_aux), spinner_online_m, FALSE, FALSE, BOX_PADDING);

	label = gtk_label_new("M:");
	gtk_box_pack_end(GTK_BOX(hbox_aux), label, FALSE, FALSE, 1);

	// --- hour
	adj = (GtkAdjustment *) gtk_adjustment_new (gpppk->getonlinetime()/3600, 0.0, 
																							59000.0, 1.0, 
																							5.0    , 0.0);
	spinner_online_h = gtk_spin_button_new (adj, 1.0, 0);
	gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (spinner_online_h), TRUE);
	gtk_widget_set_usize(spinner_online_h, 65, 0);
	gtk_spin_button_set_numeric(GTK_SPIN_BUTTON(spinner_online_h), TRUE);
	gtk_box_pack_end (GTK_BOX(hbox_aux), spinner_online_h, FALSE, FALSE, BOX_PADDING);

	label = gtk_label_new("H:");
	gtk_box_pack_end(GTK_BOX(hbox_aux), label, FALSE, FALSE, 1);

	tooltips = gtk_tooltips_new();
	gtk_tooltips_set_tip(tooltips, spinner_online_h, "hours", "");
	tooltips = gtk_tooltips_new();
	gtk_tooltips_set_tip(tooltips, spinner_online_m, "minutes", "");
	tooltips = gtk_tooltips_new();
	gtk_tooltips_set_tip(tooltips, spinner_online_s, "seconds", "");

	// propiedades
	//GTK_SPIN_BUTTON(spinner_online_h)->entry.editable.editable = 0;
	//GTK_SPIN_BUTTON(spinner_online_m)->entry.editable.editable = 0;
	//GTK_SPIN_BUTTON(spinner_online_s)->entry.editable.editable = 0;

	gtk_box_pack_start( GTK_BOX(vbox), hbox_aux, TRUE, TRUE, 3);
}

/*------------------------------------------------------------------------------
 *	Carga un hbox(con un label y spinner adentro) a un vbox.
 */
void preferences::cargar_byte_in(GtkWidget *vbox) 
{
	GtkWidget	*hbox_aux,
						*label;
	GtkTooltips   *tooltips;
	GtkAdjustment *adj;

	//-------------- transmicion minima -------------
	hbox_aux  = gtk_hbox_new(FALSE, 0);

	label = gtk_label_new("Minimum bytes in:\n"
												"If you receive fewer bytes/sec than this amount,\n"
												"the ppp link is in inactivity.");
	gtk_label_set_pattern(GTK_LABEL(label), 	"_________________");
	gtk_label_set_justify(GTK_LABEL(label), GTK_JUSTIFY_LEFT);

	//colocar el label en el box
	gtk_box_pack_start (GTK_BOX(hbox_aux), label, FALSE, FALSE, BOX_PADDING);

	adj = (GtkAdjustment *) gtk_adjustment_new (gpppk->getbyte_min_in(), 1.0, 
																							32767.0, 1.0, 
																							5.0, 0.0);
	spinner_byte_in = gtk_spin_button_new (adj, 1.0, 0);	//acelerado
	gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (spinner_byte_in), TRUE);
	gtk_widget_set_usize(spinner_byte_in, 65, 22);
	gtk_spin_button_set_numeric(GTK_SPIN_BUTTON(spinner_byte_in), TRUE);
	gtk_box_pack_end (GTK_BOX(hbox_aux), spinner_byte_in, FALSE, FALSE, BOX_PADDING);

	gtk_box_pack_start(GTK_BOX(vbox), hbox_aux, TRUE, TRUE, 3);
}

/*------------------------------------------------------------------------------
 *	Carga un hbox(con un label y spinner adentro) a un vbox.
 */
void preferences::cargar_byte_out(GtkWidget *vbox) 
{
	GtkWidget	*hbox_aux,
						*label;
	GtkTooltips   *tooltips;
	GtkAdjustment *adj;

	//-------------- transmicion minima -------------
	hbox_aux  = gtk_hbox_new(FALSE, 0);

	label = gtk_label_new("Minimum bytes out:\n" 
												"If you send fewer bytes/sec than this amount,\n" 
												"the ppp link is in inactivity.");
	gtk_label_set_pattern(GTK_LABEL(label), 	"__________________");
	gtk_label_set_justify(GTK_LABEL(label), GTK_JUSTIFY_LEFT);

	//colocar el label en el box
	gtk_box_pack_start (GTK_BOX(hbox_aux), label, FALSE, FALSE, BOX_PADDING);

	adj = (GtkAdjustment *) gtk_adjustment_new (gpppk->getbyte_min_out(), 1.0, 
																							32767.0, 1.0, 
																							5.0, 0.0);
	spinner_byte_out = gtk_spin_button_new (adj, 1.0, 0);	//acelerado
	gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (spinner_byte_out), TRUE);
	gtk_widget_set_usize(spinner_byte_out, 65, 22);
	gtk_spin_button_set_numeric(GTK_SPIN_BUTTON(spinner_byte_out), TRUE);
	gtk_spin_button_set_numeric(GTK_SPIN_BUTTON(spinner_byte_out), TRUE);
	gtk_box_pack_end (GTK_BOX(hbox_aux), spinner_byte_out, FALSE, FALSE, BOX_PADDING);

	gtk_box_pack_start(GTK_BOX(vbox), hbox_aux, TRUE, TRUE, 3);
}

/*------------------------------------------------------------------------------
 *	Cargar un hbox(con un label y un combo adentro) a un vbox
 */
void preferences::cargar_interface(GtkWidget *vbox)
{
	GtkWidget	*hbox_aux,
						*vbox_aux,
						*label;
	GtkTooltips   *tooltips;
	GtkAdjustment *adj;
	char *titles[] =
				{
					"pid",
					"chat",
					"Status",
					"Interface",
					"Up"
				};
	int i;
	
	//------------------- label -----------------
	hbox_aux  = gtk_hbox_new(FALSE, 0);

	label = gtk_label_new("List of pppds:\n"
												"This is the list of all running pppd's.\n"
												"Here you select the name of the ppp interface\n"
												"from which to obtain the statistics.");
	gtk_label_set_pattern(GTK_LABEL(label), "______________");
	gtk_label_set_justify(GTK_LABEL(label), GTK_JUSTIFY_LEFT);

	//colocar el label en el box
	gtk_box_pack_start(GTK_BOX(hbox_aux), label, FALSE, FALSE, BOX_PADDING);
	gtk_box_pack_start(GTK_BOX(vbox), hbox_aux, FALSE, FALSE, 1);	//padding ==1 para que este cerca de la lista de interfaces.

	// ------------ clist -----------------------
	hbox_aux  = gtk_hbox_new(FALSE, 0);

	scrolled_win = gtk_scrolled_window_new (NULL, NULL);
	gtk_container_set_border_width (GTK_CONTAINER (scrolled_win), 0);
	//scrollbars
	gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW (scrolled_win),
  															 GTK_POLICY_AUTOMATIC,
  															 GTK_POLICY_AUTOMATIC);
	gtk_widget_set_usize(scrolled_win, SCROLL_ANCHO, SCROLL_ALTO);
	clist_interface = gtk_clist_new_with_titles (CLIST_INTERFACE_COLUMNS, titles);
	//gtk_box_pack_end (GTK_BOX(hbox_aux), clist_interface, FALSE, FALSE, 10);

	// clist propiedades
	//para la justificacion de las columnas
	for (i = 0; i < CLIST_INTERFACE_COLUMNS; i++) {
		gtk_clist_set_column_justification(GTK_CLIST(clist_interface), i, GTK_JUSTIFY_CENTER);
		gtk_clist_set_column_auto_resize(GTK_CLIST(clist_interface), i, gtk_true());
	}
	//solo una fila se puede seleccionar
	gtk_clist_set_selection_mode(GTK_CLIST(clist_interface), GTK_SELECTION_BROWSE);
  //gtk_clist_set_border(GTK_CLIST(clist), GTK_SHADOW_ETCHED_IN);
  gtk_clist_set_shadow_type(GTK_CLIST(clist_interface), GTK_SHADOW_IN);
  gtk_clist_column_titles_passive(GTK_CLIST(clist_interface));   

	gtk_container_add (GTK_CONTAINER (scrolled_win), clist_interface);

	gtk_box_pack_start(GTK_BOX(hbox_aux), scrolled_win, FALSE, FALSE, BOX_PADDING);
	gtk_box_pack_start(GTK_BOX(vbox), hbox_aux, TRUE, TRUE, BOX_PADDING);

	// ------------ refresh button  -----------------------
	vbox_aux  = gtk_vbox_new(FALSE, 0);
	button_refresh = gtk_button_new_with_label(" Refresh List ");

	GTK_WIDGET_SET_FLAGS(button_refresh, GTK_CAN_DEFAULT);	
	//gtk_widget_set_sensitive(button_ok, gtk_false());
	gtk_box_pack_start(GTK_BOX(vbox_aux), button_refresh, FALSE, FALSE, 0);
	gtk_box_pack_end(GTK_BOX(hbox_aux), vbox_aux, FALSE, FALSE, BOX_PADDING);
}

/*------------------------------------------------------------------------------
 *	Llena el clist_interface con los pppd.
 *	Aqui se inicializa:
 *		row_ppp
 *		row_select
 */
void preferences::cargar_clist_interface(void)
{
	nododg<pppd> *p;
	pid_t ppp_pid;
	int i;
	//int num_e;
	char *append[CLIST_INTERFACE_COLUMNS],
			 str_pid[6],
			 str_chat[6],
			 str_state[2],
			 str_interface[STR_INTERFACE_LEN],
			 str_up[10],
			 *str_text;
	
	// init
	row_ppp = -1;
	row_select = -1;
	num_ppp = 0;
	
	//stop clist
	gtk_clist_freeze(GTK_CLIST(clist_interface));

	//limpiar la lista actual
	gtk_clist_clear(GTK_CLIST(clist_interface));

	//actualizar la lista de pppd que estan corriendo
	gpppk->actualizar_lista_pppd();
		
	num_ppp = (gpppk->getlista_pppd())->get_numero_de_elementos();
	if(num_ppp) {
		p = (gpppk->getlista_pppd())->get_inicio();
		while(p) {
			//pid
			sprintf(str_pid, "%d", p->data.pid);
			append[0]=str_pid;

			//chat
			sprintf(str_chat, "%d", p->data.chat);
			append[1]=str_chat;

			//state
			str_state[0] = p->data.state;
			str_state[1] = '\0';
			append[2]=str_state;

			//interface
			if(p->data.up)
				strcpy(str_interface, p->data.interface);
			else
				strcpy(str_interface, "ppp-");
			append[3] = str_interface;

			//up
			if(p->data.up)
				strcpy(str_up, STR_PPPD_UP);
			else
				strcpy(str_up, STR_PPPD_DOWN);
			append[4] = str_up;
			
			gtk_clist_append(GTK_CLIST(clist_interface), append);
			p = p->sigte;
		};

		if(num_ppp >= 1) {
			//seleccionar la fila del pppd actual
			ppp_pid	= gpppk->getpppd_pid();
			for(i=0; i < num_ppp; i++) {
				gtk_clist_get_text(GTK_CLIST(clist_interface), i, 0, &str_text);
				if(ppp_pid == atoi(str_text)) {
					row_ppp = i;	//guardar la fila del ppp.
					row_select = row_ppp;	//al comienzo row_ppp y row_select son iguales.
					gtk_clist_select_row(GTK_CLIST(clist_interface), i, 0);
					i = num_ppp;	//encontre lo que biscaba, salir.
				}	//end if(ppp_pid == atoi(str_text))
			}	//end for
		}	//end if(num_ppp > 1)
	}	//end if(num_ppp)
	else {
		strcpy(str_interface, STR_NO_PPPD);
		append[0] = str_interface;
		append[1] = str_interface;
		append[2] = str_interface;
		append[3] = str_interface;
		append[4] = str_interface;
		gtk_clist_append(GTK_CLIST(clist_interface), append);
	}

	//continue clist
	gtk_clist_thaw(GTK_CLIST(clist_interface));
}

/*------------------------------------------------------------------------------
 *	Cargar un hbox(con dos botones OK y CANCEL adentro) en un vbox.
 */
void preferences::cargar_botones(void)
{
	//------------------- botones ----------------
	button_ok = gtk_button_new_with_label("   OK   ");

	button_cancel = gtk_button_new_with_label(" CANCEL ");

	GTK_WIDGET_SET_FLAGS(button_ok, GTK_CAN_DEFAULT);	
	//GTK_WIDGET_SET_FLAGS(button_ok, GTK_HAS_FOCUS);	
	//gtk_box_pack_start (GTK_BOX (GTK_DIALOG (window)->action_area), button_ok, TRUE, FALSE, 10);
	gtk_box_pack_start (GTK_BOX(hbox), button_ok, TRUE, FALSE, BOX_PADDING);
	//gtk_widget_set_sensitive(button_ok, gtk_false());

	GTK_WIDGET_SET_FLAGS(button_cancel, GTK_CAN_DEFAULT);	
	//GTK_WIDGET_SET_FLAGS(button_cancel, GTK_HAS_FOCUS);	
	//GTK_WIDGET_SET_FLAGS(button_cancel, GTK_HAS_DEFAULT);	
	//gtk_box_pack_start(GTK_BOX (GTK_DIALOG (window)->action_area), button_cancel, TRUE, FALSE, 10);
	gtk_box_pack_start(GTK_BOX(hbox), button_cancel, TRUE, FALSE, BOX_PADDING);

	button_ok_sensitive();	//estados iniciales de los botones
}

/*------------------------------------------------------------------------------
 */
void preferences::cargar_option_warning(GtkWidget *vbox)
{
	GtkWidget *frame_warn,
	          *vbox_warn,
						*hbox_aux,
						*label;
	GtkTooltips   *tooltips;
	GtkAdjustment *adj;

	frame_warn = gtk_frame_new("Warning options");
	gtk_container_set_border_width(GTK_CONTAINER(frame_warn), CONTAINER_ANCHO);
	gtk_box_pack_start(GTK_BOX(vbox), frame_warn, TRUE, TRUE, BOX_PADDING);

	vbox_warn = gtk_vbox_new(FALSE, BOX_PADDING);
	gtk_container_add(GTK_CONTAINER(frame_warn), vbox_warn);

	// --- boton_option_warn
	hbox_aux  = gtk_hbox_new(FALSE, 0);
	label = gtk_label_new("Option warning\n"
												"Show a warning message when we\n"
												"are going to kill the ppp link?");
	gtk_label_set_pattern(GTK_LABEL(label), 	"______________");
	gtk_label_set_justify(GTK_LABEL(label), GTK_JUSTIFY_LEFT);

	button_option_warn = gtk_check_button_new_with_label("Enable");

	if(gpppk->rc->load_option_warn())
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(button_option_warn), TRUE);

	gtk_box_pack_start(GTK_BOX(hbox_aux), label, FALSE, FALSE, BOX_PADDING);
	gtk_box_pack_end  (GTK_BOX(hbox_aux), button_option_warn, FALSE, FALSE, BOX_PADDING);
	gtk_box_pack_start(GTK_BOX(vbox_warn), hbox_aux, TRUE, TRUE, 0);

	// --- spinner_warntime
	hbox_aux  = gtk_hbox_new(FALSE, 0);

	label_warntime = gtk_label_new("Warntime\n"
																 "How many seconds do you want " PROGRAMA "\n"
																 "warn you that the pppd is about to be stoped?");
	gtk_label_set_pattern(GTK_LABEL(label_warntime), "________");
	gtk_label_set_justify(GTK_LABEL(label_warntime), GTK_JUSTIFY_LEFT);

	label_warntime_spinner = gtk_label_new("S:");

	adj = (GtkAdjustment *) gtk_adjustment_new (gpppk->getwarntime(), 0.0, 
																							59.0, 1.0, 
																							5.0 , 0.0);
	spinner_warntime = gtk_spin_button_new (adj, 1.0, 0);	//acelerado
	gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (spinner_warntime), TRUE);
	gtk_spin_button_set_numeric(GTK_SPIN_BUTTON(spinner_warntime), TRUE);
	gtk_widget_set_usize(spinner_warntime, 40, 0);

	tooltips = gtk_tooltips_new();
	gtk_tooltips_set_tip(tooltips, spinner_warntime, "seconds", "");

	gtk_box_pack_start (GTK_BOX(hbox_aux), label_warntime, FALSE, FALSE, BOX_PADDING);
	gtk_box_pack_end   (GTK_BOX(hbox_aux), spinner_warntime, FALSE, FALSE, BOX_PADDING);
	gtk_box_pack_end   (GTK_BOX(hbox_aux), label_warntime_spinner, FALSE, FALSE, 1);
	gtk_box_pack_start(GTK_BOX(vbox_warn), hbox_aux, TRUE, TRUE, 0);

	// --- button_warn_beep
	hbox_aux  = gtk_hbox_new(FALSE, 0);
	label_warntime_beep = gtk_label_new("Audible warning (beep)\n"
												"Use an audible bell, alias beep?");
	gtk_label_set_pattern(GTK_LABEL(label_warntime_beep), "______________________");
	gtk_label_set_justify(GTK_LABEL(label_warntime_beep), GTK_JUSTIFY_LEFT);
	button_warn_beep = gtk_check_button_new_with_label("Enable");

	if(gpppk->rc->load_warn_beep())
		//gtk_toggle_button_set_state(GTK_TOGGLE_BUTTON(button_warn_beep), TRUE);
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(button_warn_beep), TRUE);

	gtk_box_pack_start(GTK_BOX(hbox_aux), label_warntime_beep, FALSE, FALSE, BOX_PADDING);
	gtk_box_pack_end  (GTK_BOX(hbox_aux), button_warn_beep, FALSE, FALSE, BOX_PADDING);
	gtk_box_pack_start(GTK_BOX(vbox_warn), hbox_aux, TRUE, TRUE, 0);

	// --- warnings habilitados?
	option_warn_sensitive();
}

/*------------------------------------------------------------------------------
 *	Carga un toggle button en un vbox.
 */
void preferences::cargar_idletime_on(GtkWidget *vbox) 
{
	GtkWidget *hbox_aux,
						*label;

	hbox_aux  = gtk_hbox_new(FALSE, 0);
	label = gtk_label_new("idle_time option:\n"
												"Start the idle_time option when " PROGRAMA " start up?");
	gtk_label_set_pattern(GTK_LABEL(label), "_________________");
	gtk_label_set_justify(GTK_LABEL(label), GTK_JUSTIFY_LEFT);

	idletime_on_button = gtk_check_button_new_with_label("Enable");

	if(gpppk->rc->load_idletime_on())
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(idletime_on_button), TRUE);

	gtk_box_pack_start(GTK_BOX(hbox_aux), label, FALSE, FALSE, BOX_PADDING);
	gtk_box_pack_end  (GTK_BOX(hbox_aux), idletime_on_button, FALSE, FALSE, BOX_PADDING);
	gtk_box_pack_start(GTK_BOX(vbox), hbox_aux, TRUE, TRUE, BOX_PADDING);
}

/*------------------------------------------------------------------------------
 *	Carga un toggle button en un vbox.
 */
void preferences::cargar_onlinetime_on(GtkWidget *vbox) 
{
	GtkWidget *hbox_aux,
						*label;

	hbox_aux  = gtk_hbox_new(FALSE, 0);
	label = gtk_label_new("online_time option:\n"
												"Start the online_time option when " PROGRAMA " start up?");
	gtk_label_set_pattern(GTK_LABEL(label), "___________________");
	gtk_label_set_justify(GTK_LABEL(label), GTK_JUSTIFY_LEFT);

	onlinetime_on_button = gtk_check_button_new_with_label("Enable");

	if(gpppk->rc->load_onlinetime_on())
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(onlinetime_on_button), TRUE);

	gtk_box_pack_start(GTK_BOX(hbox_aux), label, FALSE, FALSE, BOX_PADDING);
	gtk_box_pack_end  (GTK_BOX(hbox_aux), onlinetime_on_button, FALSE, FALSE, BOX_PADDING);
	gtk_box_pack_start(GTK_BOX(vbox), hbox_aux, TRUE, TRUE, BOX_PADDING);
}

/*------------------------------------------------------------------------------
 *	Carga prefered Interface.
 */
void preferences::cargar_prefered_interface(GtkWidget *vbox) 
{
	//------------------- prefered interface -----------------
	GtkWidget *hbox_aux,
						*label;
	GtkAdjustment *adj;
	char iface[10];
	int  iface_num;

	hbox_aux  = gtk_hbox_new(FALSE, 0);
	label = gtk_label_new("Preferred Interface:\n"
												"This is the ppp interface you'd\n"
												"like to use when gpppkill starts.");
	gtk_label_set_pattern(GTK_LABEL(label), "____________________");
	gtk_label_set_justify(GTK_LABEL(label), GTK_JUSTIFY_LEFT);

	//spinner
	strcpy(iface, gpppk->rc->load_pppInterface());
	sscanf(iface, "ppp%d", &iface_num);

	adj = (GtkAdjustment *) gtk_adjustment_new (iface_num, 0.0, 
																							99.0, 1.0, 
																							5.0    , 0.0);
	spinner_prefered = gtk_spin_button_new (adj, 1.0, 0);	//acelerado
	gtk_widget_set_usize(spinner_prefered, 40, 0);
	gtk_spin_button_set_wrap(GTK_SPIN_BUTTON (spinner_prefered), TRUE);
	gtk_spin_button_set_numeric(GTK_SPIN_BUTTON(spinner_prefered), TRUE);

	//label prefered interface
	label_prefered_interface = gtk_label_new(iface);

	gtk_box_pack_start(GTK_BOX(hbox_aux), label, FALSE, FALSE, BOX_PADDING);
	gtk_box_pack_end  (GTK_BOX(hbox_aux), spinner_prefered, FALSE, FALSE, BOX_PADDING);
	gtk_box_pack_end  (GTK_BOX(hbox_aux), label_prefered_interface, FALSE, FALSE, BOX_PADDING);

	gtk_box_pack_start( GTK_BOX(vbox), hbox_aux, TRUE, TRUE, BOX_PADDING);
}

/*------------------------------------------------------------------------------
 *	Setea el frame_warn un/sensitive de acuerdo a:
 *		preferences::button_warn_option.
 */
void preferences::option_warn_sensitive(void)
{
	if(GTK_TOGGLE_BUTTON(button_option_warn)->active) {
		gtk_widget_set_sensitive(label_warntime, gtk_true());
		gtk_widget_set_sensitive(spinner_warntime, gtk_true());
		gtk_widget_set_sensitive(button_warn_beep, gtk_true());
		gtk_widget_set_sensitive(label_warntime_spinner, gtk_true());
		gtk_widget_set_sensitive(label_warntime_beep, gtk_true());
	}
	else {
		gtk_widget_set_sensitive(label_warntime, gtk_false());
		gtk_widget_set_sensitive(spinner_warntime, gtk_false());
		gtk_widget_set_sensitive(button_warn_beep, gtk_false());
		gtk_widget_set_sensitive(label_warntime_spinner, gtk_false());
		gtk_widget_set_sensitive(label_warntime_beep, gtk_false());
	}
}

/*------------------------------------------------------------------------------
 *	Setea el preferences::button_ok un/sensitive de acuerdo a un OR de todas 
 *		las banderas de preferences.
 */
void preferences::button_ok_sensitive(void)
{
	int warn_aux;
	//sacado de preferences :: ok()
	if(gpppk->rc->load_option_warn()) {
		warn_aux = (flag_option_warntime || flag_warntime || flag_warn_beep);	//flag_option_warntime representa a warntime
		//flag_option_warntime = flag_warn_beep;	//flag_option_warntime representa a flag_warn_beep
	}

	// GTK_HAS_FOCUS da problema con el spinn de preferences.
	if( flag_idletime || flag_onlinetime || flag_byte_in || flag_byte_out || flag_idletime_button ||
	    flag_onlinetime_button || flag_interface || warn_aux || flag_prefered_interface) {
		//button_ok tiene el focus
		gtk_widget_set_sensitive(button_ok, gtk_true());
		//GTK_WIDGET_UNSET_FLAGS(button_cancel, GTK_HAS_FOCUS);	
		GTK_WIDGET_UNSET_FLAGS(button_cancel, GTK_HAS_DEFAULT);	
		//GTK_WIDGET_SET_FLAGS(button_ok, GTK_HAS_FOCUS);
		GTK_WIDGET_SET_FLAGS(button_ok, GTK_HAS_DEFAULT);
		gtk_widget_queue_draw(button_cancel);	//actualizar el boton sin HAS_DEFAULT
	}
	else {	//button_cancel tiene el focus
		gtk_widget_set_sensitive(button_ok, gtk_false());
		//GTK_WIDGET_UNSET_FLAGS(button_ok, GTK_HAS_FOCUS);
		GTK_WIDGET_UNSET_FLAGS(button_ok, GTK_HAS_DEFAULT);	
		//GTK_WIDGET_SET_FLAGS(button_cancel, GTK_HAS_FOCUS);
		GTK_WIDGET_SET_FLAGS(button_cancel, GTK_HAS_DEFAULT);	
		gtk_widget_queue_draw(button_cancel);	//actualizar el boton sin HAS_DEFAULT
	}
}

/*------------------------------------------------------------------------------
 *	Se llama desde el callback cuando se oprimio OK
 */
void preferences::ok(void)
{
	char *str_text, str_tmp[10];
	pid_t ppp_pid;
	int byte_in_tmp, byte_out_tmp, i, num_e, row_anterior;
	long idle, online, aux;
	
	if(flag_idletime) {
		aux = gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(spinner_idle_h));
		aux = aux * 3600;
		idle = aux;
		aux = gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(spinner_idle_m));
		aux = aux * 60;
		idle = idle + aux;
		aux = gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(spinner_idle_s));
		idle = idle + aux;

		gpppk->setidletime(idle);

		gpppk->rc->save_idletime(idle);
	}

	if(flag_onlinetime) {
		aux = (long)gtk_spin_button_get_value_as_float(GTK_SPIN_BUTTON(spinner_online_h));
		aux = aux * 3600;
		online = aux;
		aux = gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(spinner_online_m));
		aux = aux * 60;
		online = online + aux;
		aux = gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(spinner_online_s));
		online = online + aux;

		gpppk->setonlinetime(online);

		gpppk->rc->save_onlinetime(online);
	}	

	if(flag_byte_in) {
		byte_in_tmp  = gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(spinner_byte_in));

		gpppk->setbyte_min_in(byte_in_tmp);

		gpppk->rc->save_byte_min_in(byte_in_tmp);
	}

	if(flag_byte_out) {
		byte_out_tmp = gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(spinner_byte_out));

		gpppk->setbyte_min_out(byte_out_tmp);

		gpppk->rc->save_byte_min_out(byte_out_tmp);
	}

	if(flag_idletime_button) 
		gpppk->rc->save_idletime_on(GTK_TOGGLE_BUTTON(idletime_on_button)->active);

	if(flag_onlinetime_button)
		gpppk->rc->save_onlinetime_on(GTK_TOGGLE_BUTTON(onlinetime_on_button)->active);


	//si hay algun pppd corriendo. Si row_ppp>=0, entonces tambien lo es row_select
	if(row_ppp >= 0) {
		//revisar si elegi una interface que esta up.
		gtk_clist_get_text(GTK_CLIST(clist_interface), row_select, 4, &str_text);
		if( !strcmp(STR_PPPD_DOWN, (char *)str_text) )
			flag_interface = 0;

		if(flag_interface) {	//si se cambio de interface
			// verificar que se haya seleccionado una nueva interface
			if(row_select != row_ppp) {
				gtk_clist_get_text(GTK_CLIST(clist_interface), row_select, 3, &str_text);

				gpppk->setinterface((char *)str_text);
			}
			else  //seleccione la interface actual
				flag_interface = 0;
		}
	}

	if(flag_prefered_interface) {
		i = gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(spinner_prefered));
		sprintf(str_tmp, "ppp%d", i);
		gpppk->rc->save_pppInterface(str_tmp);
	}

	if(flag_option_warntime)
		gpppk->rc->save_option_warn(GTK_TOGGLE_BUTTON(button_option_warn)->active);

	if(gpppk->rc->load_option_warn()) {
		if(flag_warntime) {
			flag_option_warntime = flag_warntime;	//flag_option_warntime representa a warntime
			aux = gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(spinner_warntime));
			gpppk->setwarntime(aux);
			gpppk->rc->save_warntime(aux);
		}
		if(flag_warn_beep) {
			flag_option_warntime = flag_warn_beep;	//flag_option_warntime representa a warn_beep
			gpppk->rc->save_warn_beep(GTK_TOGGLE_BUTTON(button_warn_beep)->active);
			gpppk->setwarn_beep(GTK_TOGGLE_BUTTON(button_warn_beep)->active);
		}
	}

	if( flag_idletime || flag_onlinetime || flag_byte_in || flag_byte_out || flag_idletime_button
	    || flag_onlinetime_button || flag_interface || flag_option_warntime)
		gpppk->status("Preferences saved!", SBAR_TIMEOUT);
		//quickmessage("preferences_ok_callback():", "Preferences saved.", (char *)NULL);

	resultado = 1;
	quit();
}

/*------------------------------------------------------------------------------
 *	Se llama desde el callback cuando se oprimio CANCEL
 */
void preferences::cancel(void)
{
	resultado = 0;
	flag_interface = 0;
	quit();
}

// ### callbacks ###############################################################

/*------------------------------------------------------------------------------
 *	Para el boton OK de preferences.
 */
gint preferences_ok_callback(GtkButton *button, preferences *pref)
{
	button = button;	//unused parameter
	pref->ok();

	return TRUE;
}

/*------------------------------------------------------------------------------
 *	Para el boton CANCEL de preferences.
 */
gint preferences_cancel_callback(GtkButton *button, preferences *pref)
{
	button = button;	//unused parameter
	pref->cancel();

	return TRUE;
}

/*------------------------------------------------------------------------------
 *	Para el tiempo idletime de preferences.
 */
gint preferences_idletime_callback(GtkEditable *editable, preferences *pref)
{
	//GtkSpinButton *sp;
	pref->flag_idletime = 1;
	pref->button_ok_sensitive();

	// aqui esta el problema con el spinn que no se actualiza.
	//sp = GTK_SPIN_BUTTON(pref->spinner_idle_h);
	//cout << gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(pref->spinner_idle_h)) << endl;

	return FALSE;
}

/*------------------------------------------------------------------------------
 *	Para el tiempo onlinetime de preferences.
 */
gint preferences_onlinetime_callback(GtkEditable *editable, preferences *pref)
{
	pref->flag_onlinetime = 1;
	pref->button_ok_sensitive();

	return FALSE;
}

/*------------------------------------------------------------------------------
 *	Para el tiempo flag_byte_in de preferences.
 */
gint preferences_byte_in_callback(GtkEditable *editable, preferences *pref)
{
	pref->flag_byte_in = 1;
	pref->button_ok_sensitive();

	return TRUE;
}

/*------------------------------------------------------------------------------
 *	Para el tiempo flag_byte_out de preferences.
 */
gint preferences_byte_out_callback(GtkEditable *editable, preferences *pref)
{
	pref->flag_byte_out = 1;
	pref->button_ok_sensitive();

	return TRUE;
}

/*------------------------------------------------------------------------------
 *	Para el cambio de idletime_button de preferences.
 */
gint preferences_idletime_button_callback(GtkToggleButton *toggle_button, preferences *pref)
{
	//es un toggle button, solo tiene dos opciones
	pref->flag_idletime_button = !(pref->flag_idletime_button);
	pref->button_ok_sensitive();

	return TRUE;
}

/*------------------------------------------------------------------------------
 *	Para el cambio de onlinetime_button de preferences.
 */
gint preferences_onlinetime_button_callback(GtkToggleButton *toggle_button, preferences *pref)
{
	//es un toggle button, solo tiene dos opciones
	pref->flag_onlinetime_button = !(pref->flag_onlinetime_button);
	pref->button_ok_sensitive();

	return TRUE;
}

/*------------------------------------------------------------------------------
 *	Para el cambio de interface de preferences.
 */
/*
gint preferences_combo_callback(GtkEditable *editable, preferences *pref)
{
	pref->interface = 1;
	pref->button_ok_sensitive();

	return TRUE;
}
*/

/*------------------------------------------------------------------------------
 * Para spinner_prefered del tab Interfaces
 */
gint preferences_spinner_prefered_callback(GtkEditable *editable, preferences *pref)
{
	char str_tmp[7];
	int num;
	
	pref->flag_prefered_interface = 1;
	pref->button_ok_sensitive();

	num = gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(pref->spinner_prefered));
	sprintf(str_tmp, "ppp%d", num);

	gtk_label_set_text(GTK_LABEL(pref->label_prefered_interface), str_tmp);

	return TRUE;
}

/*------------------------------------------------------------------------------
 *	Para el boton refresh del tab Interfaces
 */
gint preferences_refresh_callback(GtkButton *button, preferences *pref)
{
	pref->cargar_clist_interface();

	return TRUE;
}

/*------------------------------------------------------------------------------
 *	Para cuando se selecciona una fila
 */
gint preferences_select_row_callback(GtkWidget *widget, gint row, gint column, GdkEventButton *event, preferences *pref)
{
	//si pppkill tiene un pppd pid que esta en la lista
	if(pref->row_ppp >= 0) {
		pref->row_select = row;
		//la segunda vez que se llama al callback, es por culpa del usuario.
		if(pref->row_select != pref->row_ppp) {
			pref->flag_interface = 1;
			pref->button_ok_sensitive();
		}
		else { //seleccione la fila del pid que tiene pppkill
			pref->flag_interface = 0;
			pref->button_ok_sensitive();
		}
	}
	else {	// el pid de pppkill no esta en la lista. Puede ser de que hay nuevos pppds
		//aqui row_ppp < 0, indica que el pid del pppd que tiene pppkill no esta en la lista
		if(pref->num_ppp > 0) {	
			//si hay pppd nuevos, entonces todos son nuevos. 
			//Eso lo marco con (row_ppp < 0) y (row_selecy>=0)
			pref->row_select = row;
			pref->flag_interface = 1;
			pref->button_ok_sensitive();
		}
		else {	
			//no se encuentra el pid que tiene pppkill
			//y no hay ningun pppd en la lista.
			pref->flag_interface = 0;
			pref->button_ok_sensitive();
		}
	}

	return TRUE;
}

/*------------------------------------------------------------------------------
 *	Para habilitar la opcion warning.
 */
gint preferences_button_option_warn_callback(GtkToggleButton *toggle_button, preferences *pref)
{
	//es un toggle button, solo tiene dos opciones
	pref->flag_option_warntime = !(pref->flag_option_warntime);

	pref->option_warn_sensitive();
	pref->button_ok_sensitive();

	return TRUE;
}

/*------------------------------------------------------------------------------
 *	Para el tiempo flag_warntime.
 */
gint preferences_spinner_warntime_callback(GtkEditable *editable, preferences *pref)
{
	pref->flag_warntime = 1;
	pref->button_ok_sensitive();

	return TRUE;
}

/*------------------------------------------------------------------------------
 *	Para habilitar la warning beep.
 */
gint preferences_button_warn_beep_callback(GtkToggleButton *toggle_button, preferences *pref)
{
	//es un toggle button, solo tiene dos opciones
	pref->flag_warn_beep = !(pref->flag_warn_beep);
	pref->button_ok_sensitive();

	return TRUE;
}

// ### X11 events ##############################################################
/*------------------------------------------------------------------------------
 *	Para la clase preferences.
 *	Si retorno FALSE, gtk envia la signal destroy al widget
 *	Cuando se oprime el boton close de la ventana del windwow manager.
 */
gint preferences_delete_event_callback(GtkWidget *widget, GdkEventAny *event, preferences *pref)
{
	pref->cancel();

	return TRUE;
}

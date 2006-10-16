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
#include "warning.h"
// ############################# class warning #############################

/*------------------------------------------------------------------------------
 *	parameters:
 *		opcion:
 *			1 -> causa idletime_left == 0
 *			2 -> causa onlinetime_left == 0
 */
warning::warning(class gpppkill *g, int opcion)
{
	causa = opcion;
	gpppk = g;
	
	tiempo_total = 0;

	incremento = WARNING_INCREMENTO;

	warn_beep = 1;
}

/*------------------------------------------------------------------------------
 *	return:
 *		0: -> no terminar el enlace pppd
 *		1: -> terminar el enlace pppd
 */
int warning::mostrar()
{
	if(!tiempo_total)
		return -1;
		
	cargar_widget();
	conectar_signal();
	
	//No permitir focus en la ventana pppkill
	gtk_window_set_modal(GTK_WINDOW(window), TRUE);
	//esto es modal para mi. Por fin! gtk 1.2 hiso esta funcion!
	gtk_window_set_transient_for(GTK_WINDOW(window), GTK_WINDOW(gpppk->getwindow()) );
	gtk_widget_show_all(window);

	gtk_main();

	//en gtk_main() loop

	return resultado;
}

/*------------------------------------------------------------------------------
 */
void warning::cargar_widget()
{
	GtkWidget *table,
						*hbox_aux;
  GtkAdjustment *adj;
	
	// propiedades de la ventana
	gtk_window_set_wmclass(GTK_WINDOW (window), "gpppkill_warning", "Gpppkill");
	gtk_window_set_title (GTK_WINDOW (window), PROGRAMA " Option message" );
	gtk_window_set_position(GTK_WINDOW(window), GTK_WIN_POS_CENTER);

	// antes de la tabla de informacion
	hbox_aux  = gtk_hbox_new(FALSE, 0);

	label = gtk_label_new("The pppd is about to be killed");
	gtk_label_set_pattern(GTK_LABEL(label), "______________________________");
	gtk_label_set_justify(GTK_LABEL(label), GTK_JUSTIFY_LEFT);
	gtk_box_pack_start (GTK_BOX(hbox_aux), label, FALSE, FALSE, WARN_H_PADDING);
	gtk_box_pack_start (GTK_BOX(vbox), hbox_aux, TRUE, TRUE, WARN_V_PADDING);

	hbox_aux  = gtk_hbox_new(FALSE, 0);

	label = gtk_label_new("Information:");
	gtk_label_set_justify(GTK_LABEL(label), GTK_JUSTIFY_LEFT);
	gtk_box_pack_start (GTK_BOX(hbox_aux), label, FALSE, FALSE, WARN_H_PADDING);
	gtk_box_pack_start (GTK_BOX(vbox), hbox_aux, TRUE, TRUE, WARN_V_PADDING);

	//cargo la tabla con la informacion
	cargar_tabla();

	// despues de la tabla de informacion
	hbox_aux  = gtk_hbox_new(FALSE, 0);

	label = gtk_label_new("The default action is to kill the pppd");
	gtk_label_set_justify(GTK_LABEL(label), GTK_JUSTIFY_LEFT);
	gtk_box_pack_start (GTK_BOX(hbox_aux), label, FALSE, FALSE, WARN_H_PADDING);
	gtk_box_pack_start (GTK_BOX(vbox), hbox_aux, TRUE, TRUE, WARN_V_PADDING);

	hbox_aux  = gtk_hbox_new(FALSE, 0);

	label = gtk_label_new("What would you like to do?");
	gtk_label_set_justify(GTK_LABEL(label), GTK_JUSTIFY_LEFT);
	gtk_box_pack_start (GTK_BOX(hbox_aux), label, FALSE, FALSE, WARN_H_PADDING);
	gtk_box_pack_start (GTK_BOX(vbox), hbox_aux, TRUE, TRUE, WARN_V_PADDING);

	// progres bar
  adj = (GtkAdjustment *) gtk_adjustment_new (0, 1, WARNING_PBAR_PASOS, 0, 0, 0);
	//pbar = gtk_progress_bar_new ();
	pbar = gtk_progress_bar_new_with_adjustment(adj);
	gtk_progress_set_format_string (GTK_PROGRESS (pbar), "%p %%");
	gtk_progress_set_show_text(GTK_PROGRESS (pbar), TRUE);
	gtk_widget_set_events (pbar, GDK_ENTER_NOTIFY_MASK | GDK_LEAVE_NOTIFY_MASK);
	gtk_widget_set_usize (pbar, WARNING_PBAR_LARGO, WARNING_PBAR_ANCHO);
	gtk_box_pack_start (GTK_BOX (vbox), pbar, TRUE, TRUE, WARN_V_PADDING);

	tooltips = gtk_tooltips_new();
	gtk_tooltips_set_tip (tooltips, pbar, " Countdown is in progress... ", "");
	gtk_tooltips_set_delay (tooltips, 0);

	// botones
	ok_button = gtk_button_new_with_label (" kill ppp ");
	GTK_WIDGET_SET_FLAGS (ok_button, GTK_CAN_DEFAULT);	
	gtk_box_pack_start ( GTK_BOX(hbox), ok_button, 
											TRUE, FALSE, 10);
	//gtk_widget_grab_default (ok_button);

	if(causa == 1)
		cancel_button = gtk_button_new_with_label ("  Restart idletime  ");
	else
		cancel_button = gtk_button_new_with_label (" Restart onlinetime ");
	GTK_WIDGET_SET_FLAGS (cancel_button, GTK_CAN_DEFAULT);	
	gtk_box_pack_start ( GTK_BOX(hbox), cancel_button, 
											TRUE, FALSE, 10);
	// requested by quenhan@pla.net.py
	gtk_widget_grab_default (cancel_button);
}

/*------------------------------------------------------------------------------
 */
void warning::cargar_tabla(void)
{
	GtkWidget *table;
	char str[120];

	// tabla
	table = gtk_table_new(3, 3, FALSE);
	gtk_box_pack_start (GTK_BOX(vbox), table, FALSE, FALSE, WARN_V_PADDING+10);

	//fila 1
	label = gtk_label_new("interface");
	//gtk_table_attach_defaults(GTK_TABLE(table), label, 0, 1, 0, 1);
	gtk_table_attach(GTK_TABLE(table), label, 0, 1, 0, 1,
                  GTK_EXPAND, GTK_EXPAND, 0, 0);

	label = gtk_label_new(":");
	gtk_table_attach(GTK_TABLE(table), label, 1, 2, 0, 1,
                  GTK_SHRINK, GTK_SHRINK, 0, 0);

	sprintf(str, "%s", interface);
	label = gtk_label_new(str);
	gtk_table_attach(GTK_TABLE(table), label, 2, 3, 0, 1,
                  GTK_EXPAND, GTK_EXPAND, 0, 0);

	// fila 2
	label = gtk_label_new("pid");
	gtk_table_attach(GTK_TABLE(table), label, 0, 1, 1, 2,
                  GTK_EXPAND, GTK_EXPAND, 0, 0);

	label = gtk_label_new(":");
	gtk_table_attach(GTK_TABLE(table), label, 1, 2, 1, 2,
                  GTK_SHRINK, GTK_SHRINK, 0, 0);

	sprintf(str, "%d", pppd_pid);
	label = gtk_label_new(str);
	gtk_table_attach(GTK_TABLE(table), label, 2, 3, 1, 2,
                  GTK_EXPAND, GTK_EXPAND, 0, 0);

	// fila 3
	label = gtk_label_new("cause");
	gtk_table_attach(GTK_TABLE(table), label, 0, 1, 2, 3,
                  GTK_EXPAND, GTK_EXPAND, 0, 0);

	label = gtk_label_new(":");
	gtk_table_attach(GTK_TABLE(table), label, 1, 2, 2, 3,
                  GTK_SHRINK, GTK_SHRINK, 0, 0);

	if(causa == 1)
		label = gtk_label_new("idletime option");
	else
		label = gtk_label_new("onlinetime option");
	gtk_table_attach(GTK_TABLE(table), label, 2, 3, 2, 3,
                  GTK_EXPAND, GTK_EXPAND, 0, 0);
}

/*------------------------------------------------------------------------------
 */
void warning::conectar_signal()
{
	gtk_signal_connect (GTK_OBJECT(window), "delete_event",
											GTK_SIGNAL_FUNC(warning_delete_event_callback),
											this);
	//callbacks
	gtk_signal_connect (GTK_OBJECT(ok_button), "clicked",
											GTK_SIGNAL_FUNC(warning_ok_callback),
											this);
	gtk_signal_connect (GTK_OBJECT(cancel_button), "clicked",
											GTK_SIGNAL_FUNC(warning_cancel_callback),
											this);

	gtk_signal_connect (GTK_OBJECT(window), "key-press-event",
											GTK_SIGNAL_FUNC(warning_key_callback),
											this);

	timeout_id = gtk_timeout_add(intervalo, warning_timeout_callback, this);
}

/*------------------------------------------------------------------------------
 */
void warning::quit()
{
  gtk_timeout_remove(timeout_id);
	gtk_main_quit();
	gtk_widget_destroy(window);

}


// --- set
/*------------------------------------------------------------------------------
 */
void warning::settiempo_total(int total)
{
	total = total * 1000;	//total esta en segundos, pasar a mseg.
	tiempo_total = total;
	intervalo = tiempo_total/WARNING_PBAR_PASOS;	//el progress bar tiene 100 pasos
}

/*------------------------------------------------------------------------------
 */
void warning::setpppd_pid(pid_t pid)
{
	pppd_pid = pid;
}

/*------------------------------------------------------------------------------
 */
void warning::setinterface(char *str)
{
	strcpy(interface, str);
}

/*------------------------------------------------------------------------------
 */
void warning::setwarn_beep(char beep)
{
	warn_beep = beep;
}

//------------------------------- callback -------------------------------------

/*------------------------------------------------------------------------------
 */
gint warning_ok_callback(GtkButton *button, warning *w)
{
	button = button;	//unused variable
	w->resultado = 1;
	w->quit();

	return TRUE;
}

/*------------------------------------------------------------------------------
 */
gint warning_cancel_callback(GtkButton *button, warning *w)
{
	button = button;	//unused variable
	w->resultado = 0;
	w->quit();

	return TRUE;
}

/*------------------------------------------------------------------------------
 *	The Esc key es the same as pressing the "Cancel" button 
 *		in the warning window
 */
gboolean warning_key_callback(GtkWidget *widget, GdkEventKey *event, warning *w)
{
	if(event->keyval == GDK_Escape) {
		gtk_signal_emit_by_name(GTK_OBJECT(w->cancel_button), "clicked");
		return TRUE;
	}
	return FALSE;
}

/*------------------------------------------------------------------------------
 */
gint warning_timeout_callback(warning *w)
{
	gfloat val;
	int beep;
  GtkAdjustment *adj;
/*
	static long tiempo = 0;
	static char str[30];
	
	tiempo += w->intervalo;	//aumentar un intervalo mas hasta que sea multiplo de 1000, osea 1 seg.


	g_print("timepo: %ld\n", tiempo);
	if( !(tiempo%1000) ) {
		sprintf(str, "seconds left: %d", ((w->tiempo_total)-(int)tiempo)/1000);
		gtk_label_set(GTK_LABEL(w->label_time), str);
		gdk_beep();
		//g_print("quedan: %d segundos\n", (w->tiempo_total)-(int)tiempo);
	}
*/

  adj = GTK_PROGRESS(w->pbar)->adjustment;
  val = adj->value;
	//val = GTK_PROGRESS_BAR(w->pbar)->percentage;
	
	//g_print("val: %f\n", val);

	if(w->warn_beep) {
		//beep = (int)(val*WARNING_PBAR_PASOS);
		beep = (int)(val);
		beep++;
		if( !(beep%10) || (beep==1))
			gdk_beep();
	}
	
	//if(val < 1.0) {
	if(val < adj->upper) {
		//val += (w->incremento);
		val++;
		//gtk_progress_bar_update(GTK_PROGRESS_BAR(w->pbar), val);
		gtk_progress_set_value (GTK_PROGRESS(w->pbar), val);
		return TRUE;
	}
	else {
		w->resultado = 1;
		w->quit();
		//return FALSE;	//remuevo el timeout
		return TRUE;	//no remuevo el timeout
	}
}

// X11 callback
/*------------------------------------------------------------------------------
 *	Si retorno FALSE, gtk envia la signal destroy al widget
 */
gint warning_delete_event_callback(GtkWidget *widget, GdkEventAny *event, warning *w)
{
	widget = widget;	//unused variable
	event = event;	//unused variable
	w->resultado = 0;
	w->quit();
	
	//return FALSE;
	return TRUE;
}

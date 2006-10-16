/*
 *  gpppkill. X11/GTK+ program that kills pppd after not receiving a certain 
 *  amount of bytes/sec during certain time. It can also plot the amount 
 *  bytes/sec received. Copyright (C) 1998-1999  Oliver Schulze L.
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
/*
 *	gpppkill.cc
 *	ventana.cc -> gpppkill.cc -> gpppkill.cc
 */
#include "gpppkill.h"

//############################# class gpppkill #################################
/*------------------------------------------------------------------------------
 *	Constructor
 */
gpppkill::gpppkill(void)
{
	dialog_window = NULL;

	//banderas
	corriendo = 0;	//pedir statdisticas a pppkill? Dibujar?
	idletime_cl = 0;
	onlinetime_cl = 0;
	labels_sensitive = 1;	//al comienzo se ven los labels.
	salir = 0;
	sbar_index = 0;
	w = 0;
	h = 0;
	
	option_warntime = 1;
	warntime = DEF_WARNTIME;
	warn_beep = 1;

	human_in = 0;
	human_out = 0;

	// dimension. Debe estar antes de new graf.
	DRAW_ANCHO = DRAW_ANCHO_OLD;
	DRAW_ALTO  = DRAW_ALTO_OLD;

	grafico = new graf(this);
}

/*------------------------------------------------------------------------------
 *	Destructor
 */
gpppkill::~gpppkill()
{
	//cout << "Destructor gpppkill" << endl;
}

/*--------------------------------------------------------------------
 *	Llamar cuando se cambia de interface.
 *	Pone pppd_ok == 0 de modo que sea necesario llamar 
 *		a pppkill::setup()
 *	Se usa para forzar la llamada a pppkill::setup(). Usado en 
 *		preferences por ejemplo.
 */
void gpppkill::start(void)
{
	pppd_ok = 0;	//se necesita hacer setup!
	corriendo = 1;	
}

/*--------------------------------------------------------------------
 *	NO hace que sea necesario llamar a pppkill::setup()
 *	Usado mas que nada en los callback de los menuitem view.
 */
void gpppkill::continuar(void)
{
	// para el problema del pico de escala al salir de preferences.
	reset_pppstats();
	
	corriendo = 1;
}

//--------------------------------------------------------------------
void gpppkill::stop(void)
{
	corriendo = 0;	
}

/*------------------------------------------------------------------------------
 *	Lee los parametros pasados a gpppkill.
 *	return:
 *		0:  --> Levantar GUI. no se imprimio nada
 *		1:  --> No levantar GUI. se pidio un opcion u opcion no valida.
 */
int gpppkill::gpppkill_init(int argc, char *argv[])
{
	char *str_l_ancho,
			 *str_idle_t,
			 *str_online_t;
	int salir,
			show_version,
			show_help,
			show_copy,
			l_ancho,
			online_t,
			idle_t,
			met,
			err,
			time_err;
	int tmp_linea_ancho;

	struct option options[] =
	{
	  {"copyright", no_argument, &show_copy, 1},
	  {"help", no_argument, &show_help, 1},
	  {"idle", required_argument, NULL, 'i'},	//gpppkill --idle_time
	  {"online", required_argument, NULL, 'o'},	//gpppkill --idle_time
	  {"version", no_argument, &show_version, 1},
	  {"width", required_argument, NULL, 'w'},	//devolver 'w' si gpppkill --width
	  {"metallica", no_argument, &met, 1},
	  {NULL, 0, NULL, 0}
	};

	l_ancho = 0;
	show_version = 0;
	show_help = 0;
	show_copy = 0;
	online_t = 0;
	idle_t = 0;
	met = 0;
	err = 0;
	while((salir = getopt_long (argc, argv, "hi:o:Vw:c", options, NULL)) != EOF) {
		switch (salir) {
			case 0:			//si lee una long option, revuelve 0
			break;
			case 'c':
				show_copy = 1;
			break;
			case 'h':
				show_help = 1;
			break;
			case 'i':
				idle_t = 1;
				str_idle_t = optarg;
			break;
			case 'o':
				online_t = 1;
				str_online_t = optarg;
			break;
			case 'V':
				show_version = 1;
			break;
			case 'w':
				l_ancho = 1;
				str_l_ancho = optarg;
			break;
			default:
				err = 1;
			break;
		}
	}

	salir	=	0;	//no salir. Levantar GUI.
	time_err = 0;

	if(err)
		salir = 1;
	
	// --- idle_t ------------------------------------
	if(idle_t && !salir) {
		int i, dos_puntos, h, m, s;

		dos_puntos = 0;
		for(i=0; str_idle_t[i]; i++) {	//ver cuanto dos puntos hay
			if( str_idle_t[i] == ':' )
				dos_puntos++;
		}

		switch(dos_puntos) {
			case 0:	// segundos
				idletime_cl = 1;
				idletime = atoi(str_idle_t);
			break;
			case 1:	// mm:ss
				idletime_cl = 1;
				sscanf(str_idle_t, "%d:%d", &m, &s);
				if( ((m < 60) && (m>=0)) && (s>=0) ) {
					m = m * 60;
					idletime = m + s;
				}
				else {
					time_err = 1;
					salir = 1;
				}
			break;
			case 2:
				idletime_cl = 1;
				sscanf(str_idle_t, "%d:%d:%d:", &h, &m, &s);
				if( (h>=0) && ((m < 60) && (m>=0)) && (s>=0) ) {
					h = h * 3600;
					m = m * 60;
					idletime = h + m + s;
				}
				else {
					time_err = 1;
					salir = 1;
				}
			break;
			default:
				salir = 1;
			break;
		}
	}

	// --- online_t ------------------------------------
	if(online_t && !salir) {
		int i, dos_puntos, h, m, s;

		dos_puntos = 0;
		for(i=0; str_online_t[i]; i++) {	//ver cuanto dos puntos hay
			if( str_online_t[i] == ':' )
				dos_puntos++;
		}
		switch(dos_puntos) {
			case 0:	// segundos
				onlinetime_cl = 1;
				onlinetime = atoi(str_online_t);
			break;
			case 1:	// mm:ss
				onlinetime_cl = 1;
				sscanf(str_online_t, "%d:%d", &m, &s);
				if( ((m < 60) && (m>=0)) && (s>=0) ) {
					m = m * 60;
					onlinetime = m + s;
				}
				else {
					time_err = 1;
					salir = 1;
				}
			break;
			case 2:
				onlinetime_cl = 1;
				sscanf(str_online_t, "%d:%d:%d:", &h, &m, &s);
				if( (h>=0) && ((m < 60) && (m>=0)) && (s>=0) ) {
					h = h * 3600;
					m = m * 60;
					onlinetime = h + m + s;
				}
				else {
					time_err = 1;
					salir = 1;
				}
			break;
			default:
				salir = 1;
			break;
		}
	}

	if(time_err)
		printf("\n"
					 "Bad time argument argument.\n"
					 "  Syntax hh:mm:ss\n"
					 "         hh -> are hours   within [0-2Giga]\n"
					 "         mm -> are minutes within [0-59]\n"
					 "         ss -> are seconds within [0-32767]\n"
					 "\n"
					 );

	// --- l_ancho ------------------------------------
	if(l_ancho && !salir) {
		tmp_linea_ancho = atoi(str_l_ancho);
		if( !(tmp_linea_ancho > 0) && (tmp_linea_ancho < DRAW_ANCHO-1) ) {
			salir = 1;	//salir
			printf( "\n"
							PROGRAMA ": the 'width' option must be between '1' and '%d'. Default[%d]\n"
							"\n",
						  (DRAW_ANCHO-2), DEF_LINEA_ANCHO);
		}
		else
			grafico->setlinea_ancho(tmp_linea_ancho);
	}

	// --- show_version ------------------------------------
	if(show_version && !salir) {
		salir = 1;	//salir
		printf( "\n"
						PROGRAMA " version " VERSION " (" FECHA ")\n" \
						COPYLEFT "\n" \
						"Distributed under the GPL License." \
						" For details type '" PROGRAMA " -c' \n" \
						"\n");
	}

	// --- show_copy ------------------------------------
	if(show_copy && !salir) {
		salir = 1;	//salir
		printf( PROGRAMA
		" is a X11/GTK+ program that kill pppd if it not recive a minimal amount\n"
		"of bytes during certain time. It also plot the amount bytes/seg recived.\n"
		COPYLEFT "\n"
		"\n"
		"This program is free software; you can redistribute it and/or modify\n"
		"it under the terms of the GNU General Public License as published by\n"
		"the Free Software Foundation; either version 2 of the License, or\n"
		"(at your option) any later version.\n"
		"\n"  
		"This program is distributed in the hope that it will be useful,\n"
		"but WITHOUT ANY WARRANTY; without even the implied warranty of\n"
		"MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the\n"
		"GNU General Public License for more details.\n"
		"\n"  
		"You should have received a copy of the GNU General Public License\n"
		"along with this program; if not, write to the Free Software\n"
		"Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.\n"
		"\n"  
		"You can reach the author at: \n"
		"oliver@pla.net.py\n"
		"\n"
		"gpppkill Home Page:\n"
		"http://www.pla.net.py/home/oliver/gpppkill/\n");
	}

	// --- show_help ------------------------------------
	if(show_help && !salir) {
		salir = 1;	//salir
		printf( 
       //------------------------------------------------------------------------------
			"\n"
			PROGRAMA " version " VERSION " (" FECHA ")\n" 
			COPYLEFT "\n" 
			"usage: " PROGRAMA " [options ...]\n" 
			"\n" 
			" -c    , --copyright   Shows copyright, description, contact info and exit.\n" 
			" -h    , --help        Shows this help and exit.\n" 
			" -i <T>, --idle=T      Where 'T' is the time to use for the idle_time option.\n"
			" -o <T>, --online=T    Where 'T' is the time to use for the online_time option.\n"
			" -V    , --version     Shows the version and exit.\n" 
			" -w <X>, --width=X     Where 'X' is the width of the bar in the bar plot.\n"
			"\n"
			" The 'T' argument for '-i' and '-o' could be: 'hh:mm:ss' or 'mm:ss' or 'ss'.\n"
			"\n" 
		);
	}

	if(err) {
		printf("Try '" PROGRAMA " --help' for more information.\n\n");
	}

	/*
	 * well, a inofensive easter egg. 
	 * If you are reading this: THANKS FOR READ MY F*CKING CODE ;-)
	 * This is the song "To live is to die" from Metallica
	 * album "Metallica... and justice for all" Copyright(c)1988 METALLICA.
	 *
	 * 1.0.0  > One year ago I post the first public version of gpppkill!
	 *          It has been a long way, but... The garage remains the same! Yea!
	 * 0.8.15 > today(24-jun-98) start the Metallica Tour. The concert of my life!
	 * 0.8.15 > ya know, Metallica Rulz! C-ya
	 */
	if(met && !show_version && !show_help && !show_copy && 
		 !err && !idle_t && !online_t && !l_ancho) {
		salir = 1;
		printf( "\n"
						"TO LIVE IS TO DIE (Hetfield, Ulrich, Burton)\n"
						"--------------------------------------------\n"
						"\n"
						"When a man lies he murders\n"
						" some part of the world\n"
						"These are the pale deaths which\n"
						" men miscall their lives\n"
						"All this I cannot bear\n"
						" to witness any longer\n"
						"Cannot the Kingdom of Salvation\n"
						" take me home.\n"
						"\n"
						"Copyright(c)1988 METALLICA\n"
						"\n"
						"\n");
	}
	
	return salir;
}

/*------------------------------------------------------------------------------
 *	Funcion que se llama para salir
 *	Aqui se finaliza el gpppkill GUI.
 *		- gpppkill->quit()
 *		- se remueve el timeout
 *		- se guarda la posicion final de la ventana
 */
void gpppkill::quit(void)
{
	stop();

	//if("guardar posicion al salir?")
	save_window_position();

  gtk_main_quit();
  gtk_widget_destroy(window);
}

/*------------------------------------------------------------------------------
 *	Llamo a esta funcion para no leer el rc si es que hay una opcion en linea
 *		de commando como: --version.
 *	Llamar a esta funcion despues de haber llamado a gpppkill::gpppkill_init(),
 *		puesto que para ese fue el motimo de haber sacado este codigo de
 *		gpppkill::gpppkill()
 */
void gpppkill::leer_rc(void)
{
	rc = new rcgpppkill(this);

	//leer el archivo rc
	if( !idletime_cl )
		idletime = rc->load_idletime();
	if( !onlinetime_cl )
		onlinetime = rc->load_onlinetime();
	byte_min_in = rc->load_byte_min_in();
	byte_min_out = rc->load_byte_min_out();
	pos_dx = rc->load_dxWM();
	pos_dy = rc->load_dyWM();
	strcpy(interface, rc->load_pppInterface());
	option_warntime = rc->load_option_warn();
	warntime = rc->load_warntime();
	warn_beep = rc->load_warn_beep();
}

/*------------------------------------------------------------------------------
 *	Setea la opcion idletime, de acuerdo al estado del boton:
 *		'idletime_menuitem' del menu.
 *	Llamar a esta funcion si se desea de/activar la opcion idletime.
 *	Aqui se actualiza el 'idletime_menuitem' para reflejar el estado de la 
 *		opcion idletime.
 */
void gpppkill::toggle_idletime_menuitem(void)
{
	//despues de esto, se envia la senhal "toggled"
	//gtk_check_menu_item_set_state(GTK_CHECK_MENU_ITEM(idletime_menuitem), 
	gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(idletime_menuitem), 
	                  !(GTK_CHECK_MENU_ITEM(idletime_menuitem)->active));
}							

/*------------------------------------------------------------------------------
 *	Setea la opcion idletime, de acuerdo al estado del boton:
 *		'onlinetime_menuitem' del menu.
 *	Llamar a esta funcion si se desea de/activar la opcion onlinetime.
 *	Aqui se actualiza el 'onlinetime_menuitem' para reflejar el estado de la 
 *		opcion onlinetime.
 */
void gpppkill::toggle_onlinetime_menuitem(void)
{
	//despues de esto, se envia la senhal "toggled"
	//gtk_check_menu_item_set_state(GTK_CHECK_MENU_ITEM(onlinetime_menuitem), 
	gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(onlinetime_menuitem), 
	                  !(GTK_CHECK_MENU_ITEM(onlinetime_menuitem)->active));
}							

/*------------------------------------------------------------------------------
 *	Inicializa la ventana principal
 */
void gpppkill::crear_ventana(void)
{
	x = rc->load_xPos();
	y = rc->load_yPos();

  window = gtk_widget_new(GTK_TYPE_WINDOW,
                          "type", GTK_WINDOW_TOPLEVEL,
                          "x", x,
                          "y", y,
                          NULL);

	gtk_window_set_wmclass(GTK_WINDOW(window), "gpppkill", "Gpppkill");

  gtk_signal_connect (GTK_OBJECT(window), "destroy", 
  										GTK_SIGNAL_FUNC (gtk_widget_destroyed), &window);

	//connectar window a sus callback
  gtk_signal_connect (GTK_OBJECT(window), "delete_event", 
  										GTK_SIGNAL_FUNC (gpppkill_delete_event_callback), this);

	//Para obtener la posicion de la ventana cada vez que se mueve.
  gtk_signal_connect (GTK_OBJECT(window), "configure_event", 
  										GTK_SIGNAL_FUNC (gpppkill_configure_event_callback), this);

	//Propiedades de window
	gtk_window_set_title (GTK_WINDOW (window), PROGRAMA " " VERSION);
	//gtk_container_border_width(GTK_CONTAINER(window), CONTAINER_ANCHO);
	gtk_container_set_border_width(GTK_CONTAINER(window), CONTAINER_ANCHO);
	/* gtk_window_set_policy(window, allow_shrink, allow_grow, auto_shrink);
	 * para que no se pueda cambiar el tamanho 
	 */
	gtk_window_set_policy(GTK_WINDOW(window), 0, 0, 1);
	//gtk_window_set_policy(GTK_WINDOW(window), 0, 1, 0);
}

/*------------------------------------------------------------------------------
 *	Carga el vbox principal. Osea, carga la ventana.
 */
void gpppkill::cargar_ventana(void)
{
	GtkWidget *event_box_main;

	//crear el eventbox principal y colocarlo debajo de window
	event_box_main = gtk_event_box_new();
	gtk_container_add (GTK_CONTAINER(window), event_box_main);

	//	Este es el vbox principal. Aqui se carga todo.
  vbox = gtk_vbox_new (FALSE, 0);
	//agregar al contenedor event_box_main
	gtk_container_add(GTK_CONTAINER(event_box_main), vbox);

	crear_popup_menu();

	cargar_frame_graf(vbox);	//tambien se inicializar el timeout
	
	cargar_frame_stats(vbox);

	cargar_frame_time(vbox);

	cargar_frame_info(vbox);

	cargar_frame_status(vbox);

	dibujar_labels_insensitive();
	
	gtk_widget_set_events (event_box_main, GDK_BUTTON_PRESS_MASK);
	gtk_signal_connect_object(GTK_OBJECT(event_box_main), "event",
														GTK_SIGNAL_FUNC (gpppkill_button_press_callback),
														GTK_OBJECT(popup_menu));

	// --- Timeout para la funcion grafico ---
	id_timeout = gtk_timeout_add(INTERVALO,
	                             (GtkFunction) gpppkill_timeout_callback,
	                             this);
}

/*------------------------------------------------------------------------------
 *
 */
void gpppkill::crear_popup_menu(void)
{
	GtkWidget *popup_file,
						*popup_ppp,
						*popup_view,
						*popup_help;
	GtkWidget *file_menu,
						*file_exit;
	GtkWidget *ppp_menu,
						*ppp_pref,
						*ppp_kill;
	GtkWidget *sep1,
						*sep2;
	GtkWidget *view_menu,
						*view_graf,
						*view_stats,
						*view_idletime,
						*view_info,
						*view_status;
	GtkWidget *help_menu,
						*help_about;
	static gpppkill *gpppktmp;

	gpppktmp = this;

	//------ pop-up root menu ---------
	popup_menu = gtk_menu_new();

	popup_file  = gtk_menu_item_new_with_label("File");
	popup_ppp   = gtk_menu_item_new_with_label("PPP");
	popup_view  = gtk_menu_item_new_with_label("View");
	popup_help  = gtk_menu_item_new_with_label("Help");
	
	gtk_menu_append(GTK_MENU (popup_menu), popup_file);
	gtk_menu_append(GTK_MENU (popup_menu), popup_ppp );
	gtk_menu_append(GTK_MENU (popup_menu), popup_view);
	gtk_menu_append(GTK_MENU (popup_menu), popup_help);

	//------ submenu file -----------
	file_menu = gtk_menu_new();

	file_exit = gtk_menu_item_new_with_label("Exit");

	gtk_menu_append(GTK_MENU(file_menu), file_exit);
  gtk_menu_item_set_submenu(GTK_MENU_ITEM(popup_file), file_menu);

	//------ submenu ppp -----------
	ppp_menu = gtk_menu_new();

	ppp_pref            = gtk_menu_item_new_with_label("Preferences ... ");
	sep1                = gtk_menu_item_new();
	idletime_menuitem   = gtk_check_menu_item_new_with_label("idle time option");
	onlinetime_menuitem = gtk_check_menu_item_new_with_label("online time option");
	sep2                = gtk_menu_item_new();
	ppp_kill            = gtk_menu_item_new_with_label("Kill pppd now!");

	//solo para menuitem que son toggle
	//idletime
	gtk_check_menu_item_toggled(GTK_CHECK_MENU_ITEM(idletime_menuitem));
	gtk_check_menu_item_set_show_toggle(GTK_CHECK_MENU_ITEM(idletime_menuitem), TRUE);
	gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(idletime_menuitem), 0);
	//onlinetime
	gtk_check_menu_item_toggled(GTK_CHECK_MENU_ITEM(onlinetime_menuitem));
	gtk_check_menu_item_set_show_toggle(GTK_CHECK_MENU_ITEM(onlinetime_menuitem), TRUE);
	gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(onlinetime_menuitem), 0);

	gtk_menu_append(GTK_MENU(ppp_menu), ppp_pref);
	gtk_menu_append(GTK_MENU(ppp_menu), sep1);
	gtk_menu_append(GTK_MENU(ppp_menu), idletime_menuitem);
	gtk_menu_append(GTK_MENU(ppp_menu), onlinetime_menuitem);
	gtk_menu_append(GTK_MENU(ppp_menu), sep2);
	gtk_menu_append(GTK_MENU(ppp_menu), ppp_kill);

	GTK_WIDGET_UNSET_FLAGS(sep1, GTK_SENSITIVE);
	GTK_WIDGET_UNSET_FLAGS(sep2, GTK_SENSITIVE);

  gtk_menu_item_set_submenu(GTK_MENU_ITEM (popup_ppp), ppp_menu);

	//------ submenu view -----------
	view_menu = gtk_menu_new();

	view_graf     = gtk_check_menu_item_new_with_label("Graph");
	view_stats    = gtk_check_menu_item_new_with_label("Statistics");
	view_idletime = gtk_check_menu_item_new_with_label("pppkill");
	view_info     = gtk_check_menu_item_new_with_label("Info");
	view_status   = gtk_check_menu_item_new_with_label("Statusbar");

	gtk_check_menu_item_toggled(GTK_CHECK_MENU_ITEM(view_graf));
	gtk_check_menu_item_toggled(GTK_CHECK_MENU_ITEM(view_stats));
	gtk_check_menu_item_toggled(GTK_CHECK_MENU_ITEM(view_idletime));
	gtk_check_menu_item_toggled(GTK_CHECK_MENU_ITEM(view_info));
	gtk_check_menu_item_toggled(GTK_CHECK_MENU_ITEM(view_status));

	gtk_check_menu_item_set_show_toggle(GTK_CHECK_MENU_ITEM(view_graf)    , TRUE);
	gtk_check_menu_item_set_show_toggle(GTK_CHECK_MENU_ITEM(view_stats)   , TRUE);
	gtk_check_menu_item_set_show_toggle(GTK_CHECK_MENU_ITEM(view_idletime), TRUE);
	gtk_check_menu_item_set_show_toggle(GTK_CHECK_MENU_ITEM(view_info)    , TRUE);
	gtk_check_menu_item_set_show_toggle(GTK_CHECK_MENU_ITEM(view_status)  , TRUE);

	if(rc->load_ViewGraph())
		gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(view_graf)    , gtk_true());
	if(rc->load_ViewStats())
		gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(view_stats)   , gtk_true());
	if(rc->load_ViewTime())
		gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(view_idletime), gtk_true());
	if(rc->load_ViewInfo())
		gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(view_info) , gtk_true());
	if(rc->load_ViewStatus())
		gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(view_status) , gtk_true());
	
	gtk_menu_append(GTK_MENU(view_menu), view_graf);
	gtk_menu_append(GTK_MENU(view_menu), view_stats);
	gtk_menu_append(GTK_MENU(view_menu), view_idletime);
	gtk_menu_append(GTK_MENU(view_menu), view_info);
	gtk_menu_append(GTK_MENU(view_menu), view_status);

  gtk_menu_item_set_submenu(GTK_MENU_ITEM (popup_view), view_menu);

	//------ submenu help -----------
	help_menu = gtk_menu_new();

	help_about = gtk_menu_item_new_with_label("About");

	gtk_menu_append(GTK_MENU(help_menu), help_about);
  gtk_menu_item_set_submenu(GTK_MENU_ITEM(popup_help), help_menu);

	//con esto se muestra todo el menu
	gtk_widget_show_all(popup_menu);

	// ------ menu signals ---------
	//file
	gtk_signal_connect(	GTK_OBJECT(file_exit), "activate",
											GTK_SIGNAL_FUNC(gpppkill_exit_pressed_callback), this);

	//ppp
	gtk_signal_connect(	GTK_OBJECT(ppp_pref), "activate",
											GTK_SIGNAL_FUNC(gpppkill_pref_pressed_callback), 
											this);
	gtk_signal_connect(	GTK_OBJECT(idletime_menuitem), "toggled",
											GTK_SIGNAL_FUNC(gpppkill_idletime_menuitem_pressed_callback), 
											this);
	gtk_signal_connect(	GTK_OBJECT(onlinetime_menuitem), "toggled",
											GTK_SIGNAL_FUNC(gpppkill_onlinetime_menuitem_pressed_callback), 
											this);
	gtk_signal_connect(	GTK_OBJECT(ppp_kill), "activate",
											GTK_SIGNAL_FUNC(gpppkill_ppp_kill_pressed_callback), this);

	//view
	gtk_signal_connect(	GTK_OBJECT(view_graf), "activate",
											GTK_SIGNAL_FUNC(gpppkill_view_graf_pressed_callback), this);
	gtk_signal_connect(	GTK_OBJECT(view_stats), "activate",
											GTK_SIGNAL_FUNC(gpppkill_view_stats_pressed_callback), this);
	gtk_signal_connect(	GTK_OBJECT(view_idletime), "activate",
											GTK_SIGNAL_FUNC(gpppkill_view_idletime_pressed_callback), this);
	gtk_signal_connect(	GTK_OBJECT(view_info), "activate",
											GTK_SIGNAL_FUNC(gpppkill_view_info_pressed_callback), this);
	gtk_signal_connect(	GTK_OBJECT(view_status), "activate",
											GTK_SIGNAL_FUNC(gpppkill_view_status_pressed_callback), this);

	//help
	gtk_signal_connect(	GTK_OBJECT(help_about), "activate",
											GTK_SIGNAL_FUNC(gpppkill_about_pressed_callback), this);

}

/*------------------------------------------------------------------------------
 *	Crea e inicializa el drawing_area principal.
 *	Inicializa el timeout de pppkill().
 */
void gpppkill::cargar_frame_graf(GtkWidget *vbox)
{
	GtkWidget       *event_box,
	                *tmp_draw;
	GtkTooltips     *tooltips;

	grafico->crear_draw();

	tmp_draw = grafico->getdraw();
	
	// -- Crear event_box ---
	event_box = gtk_event_box_new ();

	//colocar dentro del event_box el drawing_area
	gtk_container_add (GTK_CONTAINER(event_box), tmp_draw);

	//tooltips
	tooltips = gtk_tooltips_new();
	gtk_tooltips_set_tip(tooltips, event_box, "Graph: plots the number bytes/sec received", "");

	// --- frame ---
	frame_graf = gtk_frame_new(NULL);

	gtk_frame_set_shadow_type(GTK_FRAME(frame_graf), GTK_SHADOW_IN);

	//colocar en el frame_graf el event_box
	gtk_container_add(GTK_CONTAINER(frame_graf), event_box);

	//colocar el frame en el vbox
	gtk_box_pack_start(GTK_BOX(vbox), frame_graf, FALSE, FALSE, 0);

	grafico->setframe_graf(frame_graf);
}

/*------------------------------------------------------------------------------
 *	Crea, inicializa y carga los labels de statistic.
 */
void gpppkill::cargar_frame_stats(GtkWidget *vbox)
{
	GtkWidget *aux_vbox;
	GtkWidget *aux_hbox;	
	//GtkWidget *frame;
	GtkWidget   *e_box_in,
							*e_box_out;
	GtkTooltips *tooltips;

	aux_hbox = gtk_hbox_new(FALSE, 0);

	// -- labels 'in' y 'out' ----------------------------------------------------
	aux_vbox = gtk_vbox_new(TRUE, 0);	//(TRUE, 0) -> todos con tamanhos iguales

	//event boxs para los labels
	e_box_in  = gtk_event_box_new ();
	e_box_out = gtk_event_box_new ();

	//crear
	label_in  = gtk_label_new(DEF_LABEL_IN_OUT);
	label_out = gtk_label_new(DEF_LABEL_IN_OUT);

	//colocar los labels en su event box correspondiente
	gtk_container_add(GTK_CONTAINER(e_box_in) , label_in );
	gtk_container_add(GTK_CONTAINER(e_box_out), label_out);

	//ocupar el maximo espacio posible
	gtk_box_pack_start(GTK_BOX(aux_vbox), e_box_in , TRUE, TRUE, 0);
	gtk_box_pack_start(GTK_BOX(aux_vbox), e_box_out, TRUE, TRUE, 0);

	//tooltips 
	tooltips = gtk_tooltips_new();
	gtk_tooltips_set_tip(tooltips, e_box_in, "Bytes/sec in", "");
	tooltips = gtk_tooltips_new();
	gtk_tooltips_set_tip(tooltips, e_box_out, "Bytes/sec out", "");

	//empaquetar en el box auxiliar horizontal
	gtk_box_pack_end(GTK_BOX(aux_hbox), aux_vbox, TRUE, TRUE, 0);	

	// --- labels 'promedio_in' y 'promedio_out' ---------------------------------
	aux_vbox = gtk_vbox_new(TRUE, 0);	//(TRUE, 0) -> todos con tamanhos iguales

	//event boxs para los labels
	e_box_in  = gtk_event_box_new ();
	e_box_out = gtk_event_box_new ();

	//crear
	label_promedio_in  = gtk_label_new(DEF_LABEL_PROMEDIO_IN_OUT);
	label_promedio_out = gtk_label_new(DEF_LABEL_PROMEDIO_IN_OUT);

	//colocar los labels en su event box correspondiente
	gtk_container_add(GTK_CONTAINER(e_box_in) , label_promedio_in );
	gtk_container_add(GTK_CONTAINER(e_box_out), label_promedio_out);

	//ocupar el maximo espacio posible
	gtk_box_pack_start(GTK_BOX(aux_vbox), e_box_in , TRUE, TRUE, 0);
	gtk_box_pack_start(GTK_BOX(aux_vbox), e_box_out, TRUE, TRUE, 0);

	//tooltips 
	tooltips = gtk_tooltips_new();
	gtk_tooltips_set_tip(tooltips, e_box_in, "Average of 'bytes in' in the last minute (Kbytes/sec)", "");
	tooltips = gtk_tooltips_new();
	gtk_tooltips_set_tip(tooltips, e_box_out, "Average of 'bytes out' in the last minute (Kbytes/sec)", "");

	//empaquetar en el box auxiliar horizontal
	gtk_box_pack_end(GTK_BOX(aux_hbox), aux_vbox, TRUE, TRUE, 0);	

	// --- labels 'total_in' y 'total_out' ---------------------------------------
	aux_vbox = gtk_vbox_new(TRUE, 0);	//(TRUE, 0) -> todos con tamanhos iguales

	//event boxs para los labels
	e_box_in  = gtk_event_box_new ();
	e_box_out = gtk_event_box_new ();

	//crear
	label_total_in  = gtk_label_new(DEF_LABEL_TOTAL_IN_OUT);
	label_total_out = gtk_label_new(DEF_LABEL_TOTAL_IN_OUT);		

	//colocar los labels en su event box correspondiente
	gtk_container_add(GTK_CONTAINER(e_box_in) , label_total_in );
	gtk_container_add(GTK_CONTAINER(e_box_out), label_total_out);

	//ocupar el maximo espacio posible
	gtk_box_pack_start(GTK_BOX(aux_vbox), e_box_in , TRUE, TRUE, 0);
	gtk_box_pack_start(GTK_BOX(aux_vbox), e_box_out, TRUE, TRUE, 0);

	//empaquetar en el box auxiliar horizontal
	gtk_box_pack_end(GTK_BOX(aux_hbox), aux_vbox, TRUE, TRUE, 0);	

	//tooltips 
	tooltips = gtk_tooltips_new();
	gtk_tooltips_set_tip(tooltips, e_box_in , "Total bytes in ", "");
	tooltips = gtk_tooltips_new();
	gtk_tooltips_set_tip(tooltips, e_box_out, "Total bytes out", "");

	//callbacks
	// total_in
	gtk_signal_connect(GTK_OBJECT(e_box_in), "enter_notify_event",
	                  GTK_SIGNAL_FUNC(gpppkill_total_in_enter_callback), this);
	gtk_signal_connect(GTK_OBJECT(e_box_in), "leave_notify_event",
	                   GTK_SIGNAL_FUNC(gpppkill_total_in_leave_callback), this);
	gtk_signal_connect(GTK_OBJECT(e_box_in), "button_press_event",
	                   GTK_SIGNAL_FUNC(gpppkill_total_in_click_callback), this);
	// total_out
	gtk_signal_connect(GTK_OBJECT(e_box_out), "enter_notify_event",
	                  GTK_SIGNAL_FUNC(gpppkill_total_out_enter_callback), this);
	gtk_signal_connect(GTK_OBJECT(e_box_out), "leave_notify_event",
	                   GTK_SIGNAL_FUNC(gpppkill_total_out_leave_callback), this);
	gtk_signal_connect(GTK_OBJECT(e_box_out), "button_press_event",
	                   GTK_SIGNAL_FUNC(gpppkill_total_out_click_callback), this);

	// --- labels 'promedio_in' y 'promedio_out' ---------------------------------
	aux_vbox = gtk_vbox_new(TRUE, 0);	//(TRUE, 0) -> todos con tamanhos iguales

	//event boxs para los labels
	e_box_in  = gtk_event_box_new ();
	e_box_out = gtk_event_box_new ();

	//crear
	label_promedio_total_in  = gtk_label_new(DEF_LABEL_PROMEDIO_IN_OUT);
	label_promedio_total_out = gtk_label_new(DEF_LABEL_PROMEDIO_IN_OUT);

	//colocar los labels en su event box correspondiente
	gtk_container_add(GTK_CONTAINER(e_box_in) , label_promedio_total_in );
	gtk_container_add(GTK_CONTAINER(e_box_out), label_promedio_total_out);

	//ocupar el maximo espacio posible
	gtk_box_pack_start(GTK_BOX(aux_vbox), e_box_in , TRUE, TRUE, 0);
	gtk_box_pack_start(GTK_BOX(aux_vbox), e_box_out, TRUE, TRUE, 0);

	//tooltips 
	tooltips = gtk_tooltips_new();
	gtk_tooltips_set_tip(tooltips, e_box_in, "Average of the total bytes/sec received (Total_Kbytes_in/ppp_uptime)", "");
	tooltips = gtk_tooltips_new();
	gtk_tooltips_set_tip(tooltips, e_box_out, "Average of the total bytes/sec sent (Total_Kbytes_out/ppp_uptime)", "");

	//empaquetar en el box auxiliar horizontal
	gtk_box_pack_end(GTK_BOX(aux_hbox), aux_vbox, TRUE, TRUE, 0);	

	// --- frame ---------------------
	frame_stats = gtk_frame_new(" Statistics ");

	gtk_frame_set_shadow_type(GTK_FRAME(frame_stats), GTK_SHADOW_ETCHED_OUT);

	//colocar el aux_hbox(que tiene los dos aux_vbox) en el frame
	gtk_container_add(GTK_CONTAINER(frame_stats), aux_hbox);

	//colocar el frame en el vbox
	gtk_box_pack_start(GTK_BOX(vbox), frame_stats, TRUE, TRUE, 0);
}

/*------------------------------------------------------------------------------
 *	Cargar el frame_time. 
 *	Estan el toggle button y el tiempo que queda antes de cortar.
 */
void gpppkill::cargar_frame_time(GtkWidget *vbox)
{
	GtkWidget   *aux_hbox;
							//*e_box_idle,
							//*e_box_online;
	GtkTooltips *tooltips;

	aux_hbox = gtk_hbox_new(TRUE, 0);//(TRUE, 0) -> todos con tamanhos iguales

	//crear
	label_idletime  = gtk_label_new(KILLTIME_OFF_STR);
	label_onlinetime = gtk_label_new(KILLTIME_OFF_STR);
	e_box_idle   = gtk_event_box_new ();
	e_box_online = gtk_event_box_new ();
 
	//propiedades
	gtk_widget_set_events(e_box_idle, 
	                      GDK_BUTTON_PRESS_MASK | GDK_ENTER_NOTIFY_MASK | 
	                      GDK_LEAVE_NOTIFY_MASK);
	gtk_signal_connect(GTK_OBJECT(e_box_idle), "button_press_event",
	                   GTK_SIGNAL_FUNC(gpppkill_idletime_2click_callback), this);
/*
	gtk_signal_connect(GTK_OBJECT(e_box_idle), "enter_notify_event",
	                   GTK_SIGNAL_FUNC(gpppkill_idletime_enter_callback), this);
	gtk_signal_connect(GTK_OBJECT(e_box_idle), "leave_notify_event",
	                   GTK_SIGNAL_FUNC(gpppkill_idletime_leave_callback), this);
*/

	gtk_widget_set_events(e_box_online, 
	                      GDK_BUTTON_PRESS_MASK | GDK_ENTER_NOTIFY_MASK | 
	                      GDK_LEAVE_NOTIFY_MASK);
	gtk_signal_connect(GTK_OBJECT(e_box_online), "button_press_event",
	                   GTK_SIGNAL_FUNC(gpppkill_onlinetime_2click_callback), this);
/*
	gtk_signal_connect(GTK_OBJECT(e_box_online), "enter_notify_event",
	                   GTK_SIGNAL_FUNC(gpppkill_onlinetime_enter_callback), this);
	gtk_signal_connect(GTK_OBJECT(e_box_online), "leave_notify_event",
	                   GTK_SIGNAL_FUNC(gpppkill_onlinetime_leave_callback), this);
*/

	//colocar el label en el event box
	gtk_container_add(GTK_CONTAINER(e_box_idle)  , label_idletime );
	gtk_container_add(GTK_CONTAINER(e_box_online), label_onlinetime );

	//ocupar el maximo espacio posible
	gtk_box_pack_start(GTK_BOX(aux_hbox), e_box_idle  , TRUE, FALSE, 0);
	gtk_box_pack_start(GTK_BOX(aux_hbox), e_box_online, TRUE, FALSE, 0);

	//tooltips 
	tooltips = gtk_tooltips_new();
	gtk_tooltips_set_tip(tooltips, e_box_idle, "idle_time option: Time left before killing ppp due to inactivity. Double click to [de]activate.", "");
	tooltips = gtk_tooltips_new();
	gtk_tooltips_set_tip(tooltips, e_box_online, "online_time option: Time left before killing ppp due to the online time limit. Double click to [de]activate.", "");

	//frame ------------------------
	frame_time = gtk_frame_new(" pppkill ");

	gtk_frame_set_shadow_type(GTK_FRAME(frame_time), GTK_SHADOW_ETCHED_OUT);

	//colocar el aux_hbox en el frame
	gtk_container_add(GTK_CONTAINER(frame_time), aux_hbox);

	//colocar el frame en el vbox
	gtk_box_pack_start(GTK_BOX(vbox), frame_time, TRUE, TRUE, 0);
}

/*------------------------------------------------------------------------------
 *	Crea, inicializa y carga los time labels.
 */
void gpppkill::cargar_frame_info(GtkWidget *vbox)
{
	GtkWidget   *aux_hbox;	
	//GtkWidget   *frame;
	GtkWidget   *e_box_interface,
						  *e_box_pid,
						  *e_box_uptime;
	GtkTooltips *tooltips;

	aux_hbox = gtk_hbox_new(TRUE, 0);//(TRUE, 0) -> todos con tamanhos iguales

	//crear
	e_box_interface = gtk_event_box_new ();
	e_box_pid       = gtk_event_box_new ();
	e_box_uptime    = gtk_event_box_new ();
	label_interface = gtk_label_new(DEF_LABEL_INTERFACE);
	label_pid       = gtk_label_new(DEF_LABEL_PID);	//pid va hasta 32565
	label_uptime    = gtk_label_new(KILLTIME_OFF_STR);

	//colocar los labels en los event box
	gtk_container_add(GTK_CONTAINER(e_box_interface), label_interface );
	gtk_container_add(GTK_CONTAINER(e_box_pid)      , label_pid );
	gtk_container_add(GTK_CONTAINER(e_box_uptime)   , label_uptime );

	//tooltips 
	tooltips = gtk_tooltips_new();
	gtk_tooltips_set_tip(tooltips, e_box_interface, "ppp Interface name", "");
	gtk_tooltips_set_tip(tooltips, e_box_pid      , "ppp Process ID", "");
	gtk_tooltips_set_tip(tooltips, e_box_uptime   , "ppp Uptime", "");

	//ocupar el maximo espacio posible
	gtk_box_pack_start(GTK_BOX(aux_hbox), e_box_interface, TRUE, TRUE, 2);
	gtk_box_pack_start(GTK_BOX(aux_hbox), e_box_pid      , TRUE, TRUE, 2);
	gtk_box_pack_start(GTK_BOX(aux_hbox), e_box_uptime   , TRUE, TRUE, 2);

	//frame ------------------------
	frame_info = gtk_frame_new(" Info ");

	gtk_frame_set_shadow_type(GTK_FRAME(frame_info), GTK_SHADOW_ETCHED_OUT);

	//colocar el aux_hbox en el frame
	gtk_container_add(GTK_CONTAINER(frame_info), aux_hbox);

	//colocar el frame en el vbox
	gtk_box_pack_start(GTK_BOX(vbox), frame_info, TRUE, TRUE, 0);
}

/*------------------------------------------------------------------------------
 */
void gpppkill::cargar_frame_status(GtkWidget *vbox)
{
	frame_status = gtk_statusbar_new();

	gtk_frame_set_shadow_type(GTK_FRAME(GTK_STATUSBAR(frame_status)->frame), GTK_SHADOW_ETCHED_OUT);
	//gtk_frame_set_shadow_type(GTK_FRAME(GTK_STATUSBAR(frame_status)->frame), GTK_SHADOW_NONE);

	gtk_box_pack_start(GTK_BOX(vbox), frame_status, TRUE, TRUE, 0);
}

//------------------------------------------------------------------------------
void gpppkill::mostrar_ventana()
{
	GtkRequisition rq;
	int x, y;

	stop();
	gtk_widget_show_all(window);

	// --- leer rc ---
	if(!rc->load_ViewGraph())
		hide_graf();

	if(!rc->load_ViewStats())
		hide_stats();

	if(!rc->load_ViewTime())
		hide_time();

	if(!rc->load_ViewInfo())
		hide_info();

	if(!rc->load_ViewStatus())
		hide_status();

	if(rc->load_idletime_on())
		toggle_idletime_menuitem();

	if(rc->load_onlinetime_on())
		toggle_onlinetime_menuitem();

	// --- propiedades de gpppkill::window ---
//	gtk_widget_size_request(frame_graf, &rq);
	/*	El ancho de la ventana es igual a:
	 *		(ancho de frame_graf) + (2 * el ancho del container), osea
	 *		(rq.width)            + (2 * CONTAINER_ANCHO)
	 *		
	 *		Actual:
	 *		(ancho_del_darea) + (Constante magica) + (2*ancho_del_container), osea
	 *		(DRAW_ANCHO)      + (4)                + (2*CONTAINER_ANCHO)
	 *		                    
	 *	Ancho de la ventana fijo. Para que no se achique mucho cuando solo se 
	 *	ve el modulo stats. Para que se vea bien el titulo de la ventana.
	 */
	//gtk_widget_set_usize(window, (rq.width) + (2*CONTAINER_ANCHO), -1);

	// aqui se generea en segundo configure_event
//	gtk_widget_set_usize(window, (DRAW_ANCHO+4) + (2*CONTAINER_ANCHO), -1);

	//g_print("DRAW_ANCHO:%d TOTAL:%d\n", DRAW_ANCHO, ( DRAW_ANCHO+4 + (2*CONTAINER_ANCHO) )); 
	//g_print("x:%d y:%d \n", rq.width, rq.height);

/*
	x = rc->load_xPos();
	y = rc->load_yPos();
	if((pos_dx == -1) || (pos_dy == -1)) {
		x = 0; 
		y = 0;
	}
	else {	//ajustar las coordenadas incluyendo la decoracion del WM
		if(x > pos_dx)
			x = x - pos_dx;
		else
			x = pos_dx - x;
		if(y > pos_dy)
			y = y - pos_dy;
		else
			y = pos_dy - y;
	}
	gtk_widget_set_uposition(window, x, y);
*/

	mostrar_cursores();

	//hacer que grafico guarde estas variables
	grafico->graf_init();
	//grafico->color_init();

	status(BUSCANDO_PPPD, SBAR_STATIC);

	start();
}

/*------------------------------------------------------------------------------
 *	Para llamar a pppkill::kill_pppd() desde gpppkill.
 */
void gpppkill::xkill_pppd(void)
{
	gint kill_timeout;
	
	stop();

	kill_timeout = gtk_timeout_add(KILL_INTERVALO,
	                              (GtkFunction) gpppkill_kill_timeout_callback,
	                              this);

	gtk_main();

	continuar();
}

/*------------------------------------------------------------------------------
 *	Guarda la posicion de la ventana en el archivo rc
 */
void gpppkill::save_window_position(void)
{
	rc->save_xPos(x);
	rc->save_yPos(y);
}

/*------------------------------------------------------------------------------
 *	Actualiza el gpppkill::frame_stats
 */
void gpppkill::dibujar_frame_stats(void)
{
	static char str[10];
	//static int debug_in=0;

	//debug_in = debug_in + 123;

	//stats.total_in = debug_in;

	// save CPU cycles
	if(!GTK_WIDGET_VISIBLE(frame_stats))
		return;

	//bytes in (recividos)
	sprintf(str, "%u", stats.in);
	gtk_label_set_text(GTK_LABEL(label_in), str); 

	//bytes out (enviados)
	sprintf(str, "%u", stats.out);
	gtk_label_set_text(GTK_LABEL(label_out), str); 

	//total_out
	/*
		if(stats.total_in > ONE_MB)
			sprintf(str, "%.2fM", float(stats.total_in)/ONE_MB);
		else if(stats.total_in > ONE_KB)
			sprintf(str, "%.2fk", float(stats.total_in)/ONE_KB);
		else
			sprintf(str, "%ld", stats.total_in);
	*/
	if(human_in == 1)
		sprintf(str, "%.2fk", float(stats.total_in)/ONE_KB);
	else if(human_in == 2)
		sprintf(str, "%.2fM", float(stats.total_in)/ONE_MB);
	else // == 0
		sprintf(str, "%ld", stats.total_in);
	gtk_label_set_text(GTK_LABEL(label_total_in), str); 

	//total_out
	if(human_out == 1)
			sprintf(str, "%.2fk", float(stats.total_out)/ONE_KB);
	else if(human_out == 2)
			sprintf(str, "%.2fM", float(stats.total_out)/ONE_MB);
	else
			sprintf(str, "%ld", stats.total_out);
	gtk_label_set_text(GTK_LABEL(label_total_out), str);

	//promedio_in
	sprintf(str, "%2.3f", stats.promedio_in);
	gtk_label_set_text(GTK_LABEL(label_promedio_in), str); 

	//promedio_out
	sprintf(str, "%2.3f", stats.promedio_out);
	gtk_label_set_text(GTK_LABEL(label_promedio_out), str); 
	
	//promedio_total_in
	sprintf(str, "%2.3f", stats.promedio_total_in);
	gtk_label_set_text(GTK_LABEL(label_promedio_total_in), str); 

	//promedio_total_out
	sprintf(str, "%2.3f", stats.promedio_total_out);
	gtk_label_set_text(GTK_LABEL(label_promedio_total_out), str); 
}

/*------------------------------------------------------------------------------
 *	Actualiza el gpppkill::frame_time
 */
void gpppkill::dibujar_frame_time(void)
{
	static char str[10];

	if(!GTK_WIDGET_VISIBLE(frame_time))
		return;

	//idletime
	if(idletime_corriendo) {
		sprintf(str, "%02u:%02u:%02u",	(stats.idletime)  / 3600,
																		((stats.idletime) / 60) % 60, 
																		(stats.idletime)  % 60);
		gtk_label_set_text(GTK_LABEL(label_idletime), str); 
	}
	else
		gtk_label_set_text(GTK_LABEL(label_idletime), KILLTIME_OFF_STR);

	//max online time
	if(onlinetime_corriendo) {
		sprintf(str, "%02d:%02d:%02d",	(stats.onlinetime)  / 3600,
																		((stats.onlinetime) / 60) % 60, 
																		(stats.onlinetime)  % 60);
		gtk_label_set_text(GTK_LABEL(label_onlinetime), str);
	}
	else
		gtk_label_set_text(GTK_LABEL(label_onlinetime), KILLTIME_OFF_STR);

}

/*------------------------------------------------------------------------------
 *	Actualiza en el gpppkill::frame_info el uptime.
 */
void gpppkill::dibujar_frame_info(void)
{
	static char str[10];

	if(!GTK_WIDGET_VISIBLE(frame_info))
		return;

	//uptime
	if(stats.uptime < 86400) {	//si estamos online menos que un dia.
		sprintf(str, "%02ld:%02ld:%02ld",	(stats.uptime) / 3600,
																		((stats.uptime) / 60) %60,
																		(stats.uptime) %  60 );
		gtk_label_set_text(GTK_LABEL(label_uptime), str);
	}
	else {
		sprintf(str, "%02ld:d%02ld:%02ld:%02ld", (stats.uptime) /86400,
																				((stats.uptime) / 3600) % 24,
																				((stats.uptime) / 60) % 60,
																				(stats.uptime) %  60 );
		gtk_label_set_text(GTK_LABEL(label_uptime), str); 
	}
}

/*------------------------------------------------------------------------------
 *	Actualiza en el gpppkill::frame_info la interface y el pid.
 */
void gpppkill::dibujar_update_frame_info(void)
{
	static char str[12];

	if(pppd_ok) {
		gtk_widget_set_sensitive(label_interface, gtk_true());
		gtk_label_set_text(GTK_LABEL(label_interface), interface);

		gtk_widget_set_sensitive(label_pid, gtk_true());
		sprintf(str, "%d", pppd_pid);
		gtk_label_set_text(GTK_LABEL(label_pid), str);
	}
}

/*------------------------------------------------------------------------------
 *	Coloca todas las labels en forma sensitiva.
 */
void gpppkill::dibujar_labels_sensitive(void)
{
	if(labels_sensitive)
		return;
	else
		labels_sensitive = 1;

	//statistics labels
	gtk_widget_set_sensitive(label_in, gtk_true());
	gtk_widget_set_sensitive(label_out, gtk_true());
	gtk_widget_set_sensitive(label_total_in, gtk_true());
	gtk_widget_set_sensitive(label_total_out, gtk_true());
	gtk_widget_set_sensitive(label_promedio_in, gtk_true());
	gtk_widget_set_sensitive(label_promedio_out, gtk_true());
	gtk_widget_set_sensitive(label_promedio_total_in, gtk_true());
	gtk_widget_set_sensitive(label_promedio_total_out, gtk_true());

	//pppkill labels
	if(es_mio) {
		gtk_widget_set_sensitive(label_idletime, gtk_true());
		gtk_widget_set_sensitive(label_onlinetime, gtk_true());
	}

	//info labels
	gtk_widget_set_sensitive(label_interface, gtk_true());
	gtk_widget_set_sensitive(label_pid, gtk_true());
	gtk_widget_set_sensitive(label_uptime, gtk_true());
}

/*------------------------------------------------------------------------------
 *	Coloca todas las labels en forma insensitiva.
 */
void gpppkill::dibujar_labels_insensitive(void)
{
	if(!labels_sensitive)
		return;
	else
		labels_sensitive = 0;
		
	//statistics labels
	gtk_widget_set_sensitive(label_in, gtk_false());
	gtk_widget_set_sensitive(label_out, gtk_false());
	gtk_widget_set_sensitive(label_total_in, gtk_false());
	gtk_widget_set_sensitive(label_total_out, gtk_false());
	gtk_widget_set_sensitive(label_promedio_in, gtk_false());
	gtk_widget_set_sensitive(label_promedio_out, gtk_false());
	gtk_widget_set_sensitive(label_promedio_total_in, gtk_false());
	gtk_widget_set_sensitive(label_promedio_total_out, gtk_false());

	//pppkill labels
	gtk_widget_set_sensitive(label_idletime, gtk_false());
	gtk_widget_set_sensitive(label_onlinetime, gtk_false());

	//info labels
	gtk_widget_set_sensitive(label_interface, gtk_false());
	gtk_widget_set_sensitive(label_pid, gtk_false());
	gtk_widget_set_sensitive(label_uptime, gtk_false());
}

/*------------------------------------------------------------------------------
 *	Coloca todas las labels en su valor inicial.
 */
void gpppkill::dibujar_labels_reset(void)
{
	//statistics labels
	gtk_label_set_text(GTK_LABEL(label_in), DEF_LABEL_IN_OUT);
	gtk_label_set_text(GTK_LABEL(label_out), DEF_LABEL_IN_OUT);
	gtk_label_set_text(GTK_LABEL(label_total_in), DEF_LABEL_TOTAL_IN_OUT);
	gtk_label_set_text(GTK_LABEL(label_total_out), DEF_LABEL_TOTAL_IN_OUT);
	gtk_label_set_text(GTK_LABEL(label_promedio_in), DEF_LABEL_PROMEDIO_IN_OUT);
	gtk_label_set_text(GTK_LABEL(label_promedio_out), DEF_LABEL_PROMEDIO_IN_OUT);
	gtk_label_set_text(GTK_LABEL(label_promedio_total_in), DEF_LABEL_PROMEDIO_IN_OUT);
	gtk_label_set_text(GTK_LABEL(label_promedio_total_out), DEF_LABEL_PROMEDIO_IN_OUT);

	//pppkill labels
	gtk_label_set_text(GTK_LABEL(label_idletime), KILLTIME_OFF_STR);
	gtk_label_set_text(GTK_LABEL(label_onlinetime), KILLTIME_OFF_STR);

	//info labels
	gtk_label_set_text(GTK_LABEL(label_interface), DEF_LABEL_INTERFACE);
	gtk_label_set_text(GTK_LABEL(label_pid), DEF_LABEL_PID);
	gtk_label_set_text(GTK_LABEL(label_uptime), KILLTIME_OFF_STR);
}

//------------------------------------------------------------------------------
void gpppkill::show_graf(void)
{
	if(!GTK_WIDGET_VISIBLE(frame_graf)) 
		gtk_widget_show(frame_graf);
}

//------------------------------------------------------------------------------
void gpppkill::show_stats(void)
{
	if(!GTK_WIDGET_VISIBLE(frame_stats)) 
		gtk_widget_show(frame_stats);
}

//------------------------------------------------------------------------------
void gpppkill::show_time(void)
{
	if(!GTK_WIDGET_VISIBLE(frame_time)) 
		gtk_widget_show(frame_time);
}

//------------------------------------------------------------------------------
void gpppkill::show_info(void)
{
	if(!GTK_WIDGET_VISIBLE(frame_info)) 
		gtk_widget_show(frame_info);
}

//------------------------------------------------------------------------------
void gpppkill::show_status(void)
{
	if(!GTK_WIDGET_VISIBLE(frame_status)) 
		gtk_widget_show(frame_status);
}

/*------------------------------------------------------------------------------
 *	return
 *		0: -> ok
 *		1: -> error
 */
int gpppkill::hide_graf(void)
{
	if(	GTK_WIDGET_VISIBLE(frame_stats) || GTK_WIDGET_VISIBLE(frame_time) ||
			GTK_WIDGET_VISIBLE(frame_info)  || GTK_WIDGET_VISIBLE(frame_status) ) {
		if(GTK_WIDGET_VISIBLE(frame_graf)) {
			gtk_widget_hide(frame_graf);
			return 0;
		}
	}
	else 
		message_box_window("You can't hide the last component!");

	return 1;
}

/*------------------------------------------------------------------------------
 *	return
 *		0: -> ok
 *		1: -> error
 */
int gpppkill::hide_stats(void)
{
	if(	GTK_WIDGET_VISIBLE(frame_graf) || GTK_WIDGET_VISIBLE(frame_time) ||
			GTK_WIDGET_VISIBLE(frame_info) || GTK_WIDGET_VISIBLE(frame_status) ) {
		if(GTK_WIDGET_VISIBLE(frame_stats)) {
			gtk_widget_hide(frame_stats);
			return 0;
		}
	}
	else 
		message_box_window("You can't hide the last component!");

	return 1;
}

/*------------------------------------------------------------------------------
 *	return
 *		0: -> ok
 *		1: -> error
 */
int gpppkill::hide_time(void)
{
	if(	GTK_WIDGET_VISIBLE(frame_graf) || GTK_WIDGET_VISIBLE(frame_stats) ||
			GTK_WIDGET_VISIBLE(frame_info)  || GTK_WIDGET_VISIBLE(frame_status) ) {
		if(GTK_WIDGET_VISIBLE(frame_time)) {
			gtk_widget_hide(frame_time);
			return 0;
		}
	}
	else 
		message_box_window("You can't hide the last component!");

	return 1;
}

/*------------------------------------------------------------------------------
 *	return
 *		0: -> ok
 *		1: -> error
 */
int gpppkill::hide_info(void)
{
	if(	GTK_WIDGET_VISIBLE(frame_graf) || GTK_WIDGET_VISIBLE(frame_stats) ||
			GTK_WIDGET_VISIBLE(frame_time) || GTK_WIDGET_VISIBLE(frame_status) ) {
		if(GTK_WIDGET_VISIBLE(frame_info)) {
			gtk_widget_hide(frame_info);
			return 0;
		}
	}
	else 
		message_box_window("You can't hide the last component!");

	return 1;
}

/*------------------------------------------------------------------------------
 *	return
 *		0: -> ok
 *		1: -> error
 */
int gpppkill::hide_status(void)
{
	if(	GTK_WIDGET_VISIBLE(frame_stats) || GTK_WIDGET_VISIBLE(frame_time) ||
			GTK_WIDGET_VISIBLE(frame_graf)  || GTK_WIDGET_VISIBLE(frame_info)) {
		if(GTK_WIDGET_VISIBLE(frame_status)) {
			gtk_widget_hide(frame_status);
			return 0;
		}
	}
	else 
		message_box_window("You can't hide the last component!");

	return 1;
}

//------------------------------------------------------------------------------
void gpppkill::quit_request(void)
{
	salir = 1;

	stop();
	status("quiting ...", SBAR_STATIC);
	dibujar_labels_insensitive();
	grafico->dibujar_reset();
	dibujar_labels_reset();

	if(dialog_window != NULL)
		gtk_signal_emit_by_name(GTK_OBJECT(dialog_window), "delete_event");
}

//------------------------------------------------------------------------------
void gpppkill::mostrar_cursores(void)
{
	GdkCursor *cursor;

	cursor = gdk_cursor_new(GDK_HAND2);
	gdk_window_set_cursor(e_box_idle->window, cursor);
	gdk_cursor_destroy(cursor);

	cursor = gdk_cursor_new(GDK_HAND2);
	gdk_window_set_cursor(e_box_online->window, cursor);
	gdk_cursor_destroy(cursor);

}

/*------------------------------------------------------------------------------
 *	return:
 *		0: -> no terminar el enlace pppd
 *		1: -> terminar el enlace pppd
 */
int gpppkill::warning_window(int causa)
{
	warning *q;
	int res;

	stop();
	option_warntime = rc->load_option_warn();
	if(option_warntime) {
		res = 0;
		q = new warning(this, causa);
		if(q == NULL)
			return res;
		q->settiempo_total(warntime);
		q->setpppd_pid(pppd_pid);
		q->setinterface(interface);
		q->setwarn_beep(warn_beep);
		dialog_window = q->getwindow();
		res = q->mostrar();

		delete q;
		dialog_window = NULL;
		continuar();
	}
	else
		res = 1;
		
	return res;
}

/*------------------------------------------------------------------------------
 *	Crea y muestra una clase preferences.
 *	return:
 *	 -1: error -> no se pudo crear la clase
 *		0: ok    -> no se cambio la interface -> se llamo a gpppkill::continuar()
 *		1: ok    -> se cambio la interface -> se llamo a gpppkill::start()
 */
int gpppkill::preference_window(void)
{
	preferences *pref;
	int res;

	stop();
	dibujar_labels_insensitive();
	status("In preferences window ...", SBAR_STATIC);

	pref = new preferences(this);
	if(!pref) 
		return -1;

	dialog_window = pref->getwindow();
	res = pref->mostrar();

	delete pref;
	dialog_window = NULL;

	dibujar_labels_sensitive();
	if( (res == -1) || (res == 0) )
		continuar();
	else {
		status("interface changed", SBAR_TIMEOUT);
		start();
	}

	status("", SBAR_POP);
	return res;
}

/*------------------------------------------------------------------------------
 *	Muestra un message box con la cadena str.
 *	Para mostrar varias lineas usar \n
 *	return:
 *		0: -> ok
 *		1: -> error
 */
int gpppkill::message_box_window(char *str)
{
	messagebox *mbox;
	int res;
	
	stop();
	dibujar_labels_insensitive();
	status("In message box window ...", SBAR_STATIC);

	mbox = new messagebox(str, this);
	if(!mbox)
		return 1;
		
	dialog_window = mbox->getwindow();
	res = mbox->mostrar();

	delete mbox;
	dialog_window = NULL;

	dibujar_labels_sensitive();
	status("", SBAR_POP);

	continuar();

	return res;
}

/*------------------------------------------------------------------------------
 */
int gpppkill::status(char *str, int mode)
{
/*
	if(status_index<0) {	//primer mensaje
		status_index=0;
		gtk_statusbar_push(GTK_STATUSBAR(frame_status), 1, "Ready...");
	}
	if(status_index) {	//si hay un mensaje
		status_index--;
		gtk_statusbar_pop(GTK_STATUSBAR(frame_status), 1);
	}
	else {
		status_index++;
		gtk_statusbar_push(GTK_STATUSBAR(frame_status), 1, str);
	}
*/

	gint status_timeout_id;
	gint tmp;
	
	switch(mode) {
		case SBAR_STATIC:
			tmp = gtk_statusbar_push(GTK_STATUSBAR(frame_status), SBAR_CONTEXT_STATIC, str);
		break;
		case SBAR_POP_STATIC:
			gtk_statusbar_pop(GTK_STATUSBAR(frame_status), SBAR_CONTEXT_STATIC);
			tmp = gtk_statusbar_push(GTK_STATUSBAR(frame_status), SBAR_CONTEXT_STATIC, str);
		break;
		case SBAR_POP:
			gtk_statusbar_pop(GTK_STATUSBAR(frame_status), SBAR_CONTEXT_STATIC);
		break;
		case SBAR_TIMEOUT:
			sbar_index = gtk_statusbar_push(GTK_STATUSBAR(frame_status), SBAR_CONTEXT_TIMEOUT, str);
			status_timeout_id = gtk_timeout_add(SBAR_INTERVALO,
	  	    	           				    (GtkFunction) gpppkill_status_timeout_callback,
	    		  	                		this);
			tmp = sbar_index;
		break;
	}

	//cerr << "stack: " << tmp << endl;

	return 0;
}

// --- Get Funtions ------------------------------------------------------------
//------------------------------------------------------------------------------
int gpppkill::is_pppkill_running(void)
{
	return corriendo;
}

//------------------------------------------------------------------------------
gint gpppkill::getid_timeout(void)
{
	return id_timeout;
}

//------------------------------------------------------------------------------
GtkWidget *gpppkill::getwindow(void)
{
	return window;
}

//------------------------------------------------------------------------------
GtkWidget *gpppkill::getframe_graf(void)
{
	return frame_graf;
}

//------------------------------------------------------------------------------
GtkWidget *gpppkill::getidletime_menuitem(void)
{
	return idletime_menuitem;
}

//------------------------------------------------------------------------------
GtkWidget *gpppkill::getonlinetime_menuitem(void)
{
	return onlinetime_menuitem;
}

//------------------------------------------------------------------------------
int gpppkill::getwarntime(void)
{
	return warntime;
}

// ---- set funtions ----

//------------------------------------------------------------------------------
void gpppkill::setwarntime(int warn)
{
	warntime = warn;
}

//------------------------------------------------------------------------------
void gpppkill::setwarn_beep(int beep)
{
	warn_beep = beep;
}

//------------------------------------------------------------------------------
void gpppkill::setx(int xpos)
{
	x = xpos;
}

//------------------------------------------------------------------------------
void gpppkill::sety(int ypos)
{
	y = ypos;
}

//------------------------------------------------------------------------------
void gpppkill::set_width(int new_w)
{
	//g_print("gpppkill::set_width(): %d\n", new_w);
	return;
	if(new_w != w) {
		w = new_w;
		gtk_widget_set_usize(window, -1, w+4);
	}
}

/*------------------------------------------------------------------------------
 *	Configure the size of all widget that their default size must be changed.
 */
void gpppkill::configure_size(int new_w, int new_h)
{
	GtkRequisition rq;

	if(!w) {
		w = new_w;
		h = new_h;
		//g_print("window size: actual(%dx%d) req(%d)\n", w, h, (DRAW_ANCHO+4) + (2*CONTAINER_ANCHO));
		//gtk_widget_set_usize(window, (DRAW_ANCHO+4) + (2*CONTAINER_ANCHO), -1);
		gtk_widget_set_usize(window, w, -1); // allow vertical shrink
		gtk_widget_size_request(frame_stats, &rq);
		//g_print("frame_stats: %dx%d\n", rq.width, rq.height);
		gtk_widget_set_usize(frame_stats, -1, rq.height+4);	// allow horizontal shrink
	}
}

//############################## callbacks #####################################

/*------------------------------------------------------------------------------
 *	Este ciclo infinito de gpppkill, alias La funcion.
 *	
 *	Esta funcion es llamada por el timeout cada INTERVALO segundos.
 *	return:
 *		si se retorna TRUE: entonces gtk sigue manteniendo el timeout.
 *		si se retorna FALSE: entonces gtk borra, cancela el timeout.
 */
gint gpppkill_timeout_callback(gpppkill *gpppk)
{
	static int pppkill_error;	//static porque se le llama cada seg a esta funcion

	if(gpppk->salir) {
		gpppk->quit();
		return FALSE;
	}
	
	if( !((gpppk)->is_pppkill_running()) ) { 	//si no estamos funcionando
		//conclusion luego de 2 horas: 
		//si no se hace algo en la funcion callback, no funca el asunto.
		gpppk = gpppk;
		return TRUE;
	}
	
	if(gpppk->getpppd_ok()) {
		pppkill_error = gpppk->get_stats();
		switch(pppkill_error) {	//solo detenerme en errores.
			case 1:	//pppd is not running
				return TRUE;
			break;
			case 2:
				gpppk->message_box_window("Kernel support for ppp missing.");
				return TRUE;
			break;
			case 3:
				gpppk->message_box_window("Can not get statistics from ppp link.");
				return TRUE;
			break;
		}	//end switch(pppkill_error)

		pppkill_error = gpppk->update_time_options();
		switch(pppkill_error) {
			case 0:
			break;
			case 1:	//salir por causa de la opcion idletime
				if(gpppk->warning_window(pppkill_error)) {
					//gtk_check_menu_item_set_state(GTK_CHECK_MENU_ITEM(gpppk->getidletime_menuitem()), gtk_false());
					gpppk->xkill_pppd();
					return TRUE;
				}
			break;
			case 2:	//salir por causa de la opcion onlinetime
				if(gpppk->warning_window(pppkill_error)) {
					//gtk_check_menu_item_set_state(GTK_CHECK_MENU_ITEM(gpppk->getonlinetime_menuitem()), gtk_false());
					gpppk->xkill_pppd();
					return TRUE;
				}
			break;
			case 3:	//no es_mio
			break;
		}	//end switch(pppkill_error)

		gpppk->dibujar_labels_sensitive();

		gpppk->grafico->dibujar();
		gpppk->dibujar_frame_stats();
		gpppk->dibujar_frame_time();
	 	gpppk->dibujar_frame_info();
	}
	else {	//else if(gpppk->getpppd_ok())
		gpppk->status(BUSCANDO_PPPD, SBAR_POP_STATIC);
		gpppk->dibujar_labels_insensitive();
		
		//gpppk->save_diff_wm();

		pppkill_error = gpppk->setup();
		//cerr << "setup: " << pppkill_error << endl;
		switch(pppkill_error) {
			case 0:	//sin error
				gpppk->status("running", SBAR_POP_STATIC);
				gpppk->status("pppd found", SBAR_TIMEOUT);
				//resetear el grafico a cero.
				gpppk->grafico->dibujar_reset();
				gpppk->dibujar_labels_reset();
				//dibujar el nuevo nombre de interface y su nuevo pid.
				gpppk->dibujar_update_frame_info();
			break;
			case 1:	//pppd is not running
				return TRUE;
			break;
			case 2:	//no se encuentra la interface especificada.
				if(gpppk->hay_algun_pppd()) {
					gpppk->sigte_interface();
					gpppk->status("Using next available interface", SBAR_TIMEOUT);
				}
				return TRUE;
			break;
			case 3:
				gpppk->message_box_window("Socket error:\nCannot create IP Socket.");
				return TRUE;
			break;
			case 4:
				char str[120];
				sprintf(str, "Interface error:\nInterface name '%s' doesn't exist.", gpppk->getinterface());
				gpppk->message_box_window(str);
				return TRUE;
			break;
			case 5:	//pppd is dialing
				gpppk->status("pppd is dialing ...", SBAR_TIMEOUT);
				return TRUE;
			break;
		}	//end switch()

		pppkill_error = gpppk->process_option_request();
		switch(pppkill_error) {
			case 0:
			break;
			case 1:
				gpppk->setidletime_corriendo(0);
				gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(gpppk->getidletime_menuitem()), gtk_false());
				gpppk->message_box_window("idle_time:\nYou are not the owner/group(member of the group) of this pppd.\nYou wouldn't be able to kill this ppp link.");
			break;
			case 2:
				gpppk->setonlinetime_corriendo(0);
				gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(gpppk->getonlinetime_menuitem()), gtk_false());
				gpppk->message_box_window("online_time:\nYou are not the owner/group(member of the group) of this pppd.\nYou wouldn't be able to kill this ppp link.");
			break;
			case 3:
				gpppk->setidletime_corriendo(0);
				gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(gpppk->getidletime_menuitem()), gtk_false());
				gpppk->message_box_window("idle_time:\nYou are not the owner/group(member of the group) of this pppd.\nYou wouldn't be able to kill this ppp link.");
				gpppk->setonlinetime_corriendo(0);
				gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(gpppk->getonlinetime_menuitem()), gtk_false());
				gpppk->message_box_window("online_time:\nYou are not the owner/group(member of the group) of this pppd.\nYou wouldn't be able to kill this ppp link.");
			break;
		};
	}

	return TRUE;
}

/*------------------------------------------------------------------------------
 *	Esta funcion es llamada por el kill_timeout desde gpppkill::xkill_pppd()
 *		 cada 1 segundo.
 *	return:
 *		si se retorna TRUE: entonces gtk sigue manteniendo el timeout.
 *		si se retorna FALSE: entonces gtk borra, cancela el timeout.
 */
gint gpppkill_kill_timeout_callback(gpppkill *gpppk)
{
	char buffer[80];
	int kill_error, i;
	static int senhal = 0,	//==0 -> TERM; ==1 -> KILL
	           count = 0;	//para ver cuantos seg esperar por el pppd que termine
	static char salir = 0;	//==0 -> no salir; ==1 -> esperar; ==2 -> salir
	//salir hasta que termine el pppd

	switch(salir) {
		case 0:	//me quedo y llamo a pppkill::kill_pppd();
		break;
		case 1:
			sprintf(buffer, "Wait for pppd to quit");
			for(i=0; i<=count; i++)
				strcat(buffer, ".");
			gpppk->status(buffer, SBAR_TIMEOUT);
			//quickmessage("Wait for pppd to quit...", (char *)NULL);
			if(!gpppk->exist_pid())	//si existe el pid, seguir esperando
				salir = 2;
			else {
				count++;
				if(count > 10) {
					gpppk->status("aborting ...", SBAR_TIMEOUT);
					//quickmessage("pppkill::kill_pppd():",
					//					   "can NOT wait more than 10 seconds.", "aborting...", (char *)NULL);
					salir = 2;
				}
			}
			return TRUE;
		break;
		case 2:
			salir = 0;	//reset
			senhal = 0;	//reset
			count = 0;	//reset
			gtk_main_quit();
			return FALSE;
		break;
	}
	
	//gpppk->status("Sending TERM signal to pppd", SBAR_TIMEOUT);
	//quickmessage("Sending the TERM signal to pppd ...", (char *)NULL);
	gpppk->status("stopping pppd ...", SBAR_TIMEOUT);
	kill_error = gpppk->kill_pppd(senhal);
	switch(kill_error) {
		case 0:
			//gpppk->status("ppp link finished", SBAR_TIMEOUT);
			//quickmessage("pppkill::kill_pppd():",
			//					   "finished ppp link:", gpppk->interface, (char *)NULL);
			salir = 1;
		break;
		case 1:	//SIGTERM dio error, probar con SIGKILL
			if(senhal == 0) {	//si SIGTERM no funca, probar con SIGKILL
				salir = 0;
				senhal = 1;
				gpppk->status("Sending KILL signal to pppd", SBAR_TIMEOUT);
				gpppk->message_box_window("TERM signal doesn't work.\nKILL signal will be send...");
			}
			else {	//si ya probe SIGTERM y SIGKILL, salir de aqui viejo!
				salir = 2;
				gpppk->message_box_window("TERM signal doesn't work.\nKILL signal doesn't work.\n>>> Can NOT kill the pppd! <<<");
			}
		break;
		case 2:
			sprintf(buffer, "ppp interface '%s' is not running", gpppk->interface);
			gpppk->message_box_window(buffer);
			salir = 2;
		break;
		case 3:
			gpppk->message_box_window("You are not the owner/group(member of the group) of this pppd.\nYou wouldn't be able to kill this ppp link.");
			salir = 2;
		break;
	}

	return TRUE;
}

/*------------------------------------------------------------------------------
 *	timeout callback para un mensaje del context SBAR_CONTEXT_TIMEOUT
 *	del statusbar gpppkill::frame_status
 *	
 *	return:
 *		si se retorna TRUE: entonces gtk sigue manteniendo el timeout.
 *		si se retorna FALSE: entonces gtk borra, cancela el timeout.
 */
gint gpppkill_status_timeout_callback(gpppkill *gpppk)
{
	GSList *list;
	GtkStatusbarMsg *msg, *tmp;

	//el primer elemento de la lista es el mas viejo. FIFO
	list = GTK_STATUSBAR(gpppk->frame_status)->messages;
	//buscar el primer elemento del context SBAR_CONTEXT_TIMEOUT
	msg = NULL;
	//for(; list && !msg; list = list->next) {
	for(; list; list = list->next) {
		tmp = (GtkStatusbarMsg *)(list->data);
		if(tmp->context_id == SBAR_CONTEXT_TIMEOUT)
			msg = (GtkStatusbarMsg *)(list->data);	//con esto se sale del for
	}
	//gtk_statusbar_pop(GTK_STATUSBAR(gpppk->frame_status), gpppk->sbar_index);
	//gtk_statusbar_pop(GTK_STATUSBAR(gpppk->frame_status), 2);
	gtk_statusbar_remove(GTK_STATUSBAR(gpppk->frame_status),
											 msg->context_id, msg->message_id);

	return FALSE;
}

/*------------------------------------------------------------------------------
 *	Callback para cuando se oprime un boton del mouse en:
 *		gpppkill::event_box_main
 */
gint gpppkill_button_press_callback (GtkWidget *widget, GdkEvent *event)
{
	GdkEventButton *bevent;

	bevent = (GdkEventButton *) event; 

	if( (event->type == GDK_BUTTON_PRESS) && (bevent->button==3) ) {
		
		gtk_menu_popup(GTK_MENU(widget), NULL, NULL, NULL, NULL,
                              bevent->button, bevent->time);
		// Tell calling code that we have handled this event; the buck stops here.
		return TRUE;
	}

	// Tell calling code that we have not handled this event; pass it on. 
	return FALSE;
}

/*------------------------------------------------------------------------------
 *	Para el menuitem title
 */
gint gpppkill_about_pressed_callback(GtkMenuItem *item, class gpppkill *gpppk)
{
	item = item; //warning unused variable

	gpppk->message_box_window(PROGRAMA " " VERSION "\n" COPYLEFT "\nDistributed under the GPL, run '"PROGRAMA" -c' for more info.\nhttp://www.pla.net.py/home/oliver/gpppkill/");

	return TRUE;
}

/*------------------------------------------------------------------------------
 *	menuitem preference
 */
gint gpppkill_pref_pressed_callback(GtkMenuItem *item, gpppkill *gpppk)
{
	item = item; //warning unused variable

	gpppk->preference_window();

	return TRUE;
}

/*------------------------------------------------------------------------------
 *	menuitem quit
 */
gint gpppkill_exit_pressed_callback(GtkMenuItem *item, gpppkill *gpppk)
{
	item = item; //warning unused variable

	if(!gpppk->salir)	//si nadie me pidio salir
		gpppk->quit_request();	//solicitar la terminacion de gpppkill

	return TRUE;
}

/*------------------------------------------------------------------------------
 *	toggle menuitem idletime
 */
gint gpppkill_idletime_menuitem_pressed_callback(GtkCheckMenuItem *item, gpppkill *gpppk)
{
	char res;
	
	item = item;	//warning: unused variable

	//g_print("es item:%d  es button:%d\n", GTK_IS_CHECK_MENU_ITEM(item), GTK_IS_TOGGLE_BUTTON(item));
	//g_print("estado: %d\n", GTK_CHECK_MENU_ITEM(item)->active);

	if(gpppk->getpppd_ok()) {
		res =	gpppk->setidletime_corriendo(GTK_CHECK_MENU_ITEM(item)->active);
		if(res) {
			gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(item), gtk_false());
			gpppk->message_box_window("idle_time:\nYou are not the owner/group(member of the group) of this pppd.\nYou wouldn't be able to kill this ppp link.");
		}
	}
	else
		gpppk->idletime_request(GTK_CHECK_MENU_ITEM(item)->active);

	return TRUE;
}

/*------------------------------------------------------------------------------
 *	toggle menuitem onlinetime
 */
gint gpppkill_onlinetime_menuitem_pressed_callback(GtkCheckMenuItem *item, gpppkill *gpppk)
{
	char res;
	
	item = item;	//warning: unused variable

	if(gpppk->getpppd_ok()) {
		res =	gpppk->setonlinetime_corriendo(GTK_CHECK_MENU_ITEM(item)->active);
		if(res) {
			gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(item), gtk_false());
			gpppk->message_box_window("online_time:\nYou are not the owner/group(member of the group) of this pppd.\nYou wouldn't be able to kill this ppp link.");
		}
	}
	else
		gpppk->onlinetime_request(GTK_CHECK_MENU_ITEM(item)->active);

	return TRUE;
}

/*------------------------------------------------------------------------------
 *	toggle button idletime
 */
gint gpppkill_ppp_kill_pressed_callback(GtkCheckMenuItem *item, gpppkill *gpppk)
{
	item = item;	//warning: unused variable

	gpppk->xkill_pppd();

	return TRUE;
}

/*------------------------------------------------------------------------------
 *	toggle menuitem view graf
 */
gint gpppkill_view_graf_pressed_callback(GtkCheckMenuItem *item, gpppkill *gpppk)
{
	gpppk->stop();

	if(item->active) {
		gpppk->show_graf();
		gpppk->rc->save_ViewGraph(1);
	}
	else {
		if(gpppk->hide_graf()) 
			gtk_check_menu_item_set_active(item, 1);
		else  //no hubo error al ocultar el componente
			gpppk->rc->save_ViewGraph(0);
	}

	gpppk->continuar();

	return TRUE;
}

/*------------------------------------------------------------------------------
 *	toggle menuitem view stats
 */
gint gpppkill_view_stats_pressed_callback(GtkCheckMenuItem *item, gpppkill *gpppk)
{
	gpppk->stop();

	if(item->active) {
		gpppk->show_stats();
		gpppk->rc->save_ViewStats(1);
	}
	else {
		if(gpppk->hide_stats())
			gtk_check_menu_item_set_active(item, 1);
		else  //no hubo error al ocultar el componente
			gpppk->rc->save_ViewStats(0);
	}

	gpppk->continuar();

	return TRUE;
}

/*------------------------------------------------------------------------------
 *	toggle menuitem view Killtime
 */
gint gpppkill_view_idletime_pressed_callback(GtkCheckMenuItem *item, gpppkill *gpppk)
{
	gpppk->stop();

	if(item->active) {
		gpppk->show_time();
		gpppk->rc->save_ViewTime(1);
	}		
	else {
		if(gpppk->hide_time())
			gtk_check_menu_item_set_active(item, 1);
		else  //no hubo error al ocultar el componente
			gpppk->rc->save_ViewTime(0);
	}		

	gpppk->continuar();

	return TRUE;
}

/*------------------------------------------------------------------------------
 *	toggle menuitem view ppp info
 */
gint gpppkill_view_info_pressed_callback(GtkCheckMenuItem *item, gpppkill *gpppk)
{
	gpppk->stop();

	if(item->active) {
		gpppk->show_info();
		gpppk->rc->save_ViewInfo(1);
	}		
	else {
		if(gpppk->hide_info())	//tratar de ocultar
			gtk_check_menu_item_set_active(item, 1);	//no se puede ocultar
		else  //no hubo error al ocultar el componente
			gpppk->rc->save_ViewInfo(0);
	}

	gpppk->continuar();

	return TRUE;
}

/*------------------------------------------------------------------------------
 *	toggle menuitem view ppp status
 */
gint gpppkill_view_status_pressed_callback(GtkCheckMenuItem *item, gpppkill *gpppk)
{
	gpppk->stop();

	if(item->active) {
		gpppk->show_status();
		gpppk->rc->save_ViewStatus(1);
	}		
	else {
		if(gpppk->hide_status())	//tratar de ocultar
			gtk_check_menu_item_set_active(item, 1);	//no se puede ocultar
		else  //no hubo error al ocultar el componente
			gpppk->rc->save_ViewStatus(0);
	}

	gpppk->continuar();

	return TRUE;
}

/*------------------------------------------------------------------------------
 *	enter label label_total_in
 */
gint gpppkill_total_in_enter_callback(GtkWidget *widget, GdkEventCrossing *event, gpppkill *gpppk)
{
	// if this label i
	if(GTK_WIDGET_IS_SENSITIVE(gpppk->label_total_in) == FALSE)
		return FALSE;

	// solo si se oprimio doble click con el boton izquierdo.
	//if((event->type == GDK_ENTER_NOTIFY) && (event->mode == GDK_CROSSING_NORMAL)) {
	if(event->mode == GDK_CROSSING_NORMAL) {
		gtk_label_set_pattern(GTK_LABEL(gpppk->label_total_in), DEF_PATTERN_TOTAL_IN_OUT);

		//g_print("enter\n");

		// Tell calling code that we have handled this event; the buck stops here.
		return TRUE;
	}

	// Tell calling code that we have not handled this event; pass it on. 
	return FALSE;
}

/*------------------------------------------------------------------------------
 *	leave label label_total_in
 */
gint gpppkill_total_in_leave_callback(GtkWidget *widget, GdkEventCrossing *event, gpppkill *gpppk)
{
	// solo si se oprimio doble click con el boton izquierdo.
	//if( (event->type == GDK_LEAVE_NOTIFY) && (event->mode == GDK_CROSSING_NORMAL)) {
	if(event->mode == GDK_CROSSING_NORMAL) {
		gtk_label_set_pattern(GTK_LABEL(gpppk->label_total_in), "");

		//g_print("leave\n");

		// Tell calling code that we have handled this event; the buck stops here.
		return TRUE;
	}

	// Tell calling code that we have not handled this event; pass it on. 
	return FALSE;
}

/*------------------------------------------------------------------------------
 *	enter label label_total_out
 */
gint gpppkill_total_out_enter_callback(GtkWidget *widget, GdkEventCrossing *event, gpppkill *gpppk)
{
	if(GTK_WIDGET_IS_SENSITIVE(gpppk->label_total_out) == FALSE)
		return FALSE;

	if(event->mode == GDK_CROSSING_NORMAL) {
		gtk_label_set_pattern(GTK_LABEL(gpppk->label_total_out), DEF_PATTERN_TOTAL_IN_OUT);

		//g_print("enter\n");
		// Tell calling code that we have handled this event; the buck stops here.
		return TRUE;
	}
	// Tell calling code that we have not handled this event; pass it on. 
	return FALSE;
}

/*------------------------------------------------------------------------------
 *	leave label label_total_out
 */
gint gpppkill_total_out_leave_callback(GtkWidget *widget, GdkEventCrossing *event, gpppkill *gpppk)
{
	if(event->mode == GDK_CROSSING_NORMAL) {
		gtk_label_set_pattern(GTK_LABEL(gpppk->label_total_out), "");
		// Tell calling code that we have handled this event; the buck stops here.
		return TRUE;
	}
	// Tell calling code that we have not handled this event; pass it on. 
	return FALSE;
}

/*------------------------------------------------------------------------------
 */
gint gpppkill_total_in_click_callback(GtkWidget *widget, GdkEventButton *event, gpppkill *gpppk)
{
	if(GTK_WIDGET_IS_SENSITIVE(gpppk->label_total_in) == FALSE)
		return FALSE;

	// solo si se hizo click con el boton izquierdo.
	if((event->type == GDK_BUTTON_PRESS) && (event->button == 1) ) {
		if(gpppk->human_in == 3)
			gpppk->human_in = 0;
		else
			gpppk->human_in = gpppk->human_in ++;

		//g_print("gpppkill_total_in_click_callback(): in:%d out:%d\n", gpppk->human_in, gpppk->human_out);
		// Tell calling code that we have handled this event; the buck stops here.
		return TRUE;
	}
	// Tell calling code that we have not handled this event; pass it on. 
	return FALSE;
}

/*------------------------------------------------------------------------------
 */
gint gpppkill_total_out_click_callback(GtkWidget *widget, GdkEventButton *event, gpppkill *gpppk)
{
	if(GTK_WIDGET_IS_SENSITIVE(gpppk->label_total_out) == FALSE)
		return FALSE;

	// solo si se hizo click con el boton izquierdo.
	if((event->type == GDK_BUTTON_PRESS) && (event->button == 1) ) {
		if(gpppk->human_out == 3)
			gpppk->human_out = 0;
		else
			gpppk->human_out = gpppk->human_out ++;
		//g_print("gpppkill_total_out_click_callback(): in:%d out:%d\n", gpppk->human_in, gpppk->human_out);
		// Tell calling code that we have handled this event; the buck stops here.
		return TRUE;
	}
	// Tell calling code that we have not handled this event; pass it on. 
	return FALSE;
}

/*------------------------------------------------------------------------------
 */
gint gpppkill_idletime_2click_callback(GtkWidget *widget, GdkEventButton *event, gpppkill *gpppk)
{
	// solo si se oprimio doble click con el boton izquierdo.
	if((event->type == GDK_2BUTTON_PRESS) && (event->button == 1) ) {
		gpppk->toggle_idletime_menuitem();

		// Tell calling code that we have handled this event; the buck stops here.
		return TRUE;
	}

	// Tell calling code that we have not handled this event; pass it on. 
	return FALSE;
}

/*------------------------------------------------------------------------------
 */
gint gpppkill_onlinetime_2click_callback(GtkWidget *widget, GdkEventButton *event, gpppkill *gpppk)
{
	// solo si se oprimio doble click con el boton izquierdo.
	if((event->type == GDK_2BUTTON_PRESS) && (event->button == 1) ) {
		gpppk->toggle_onlinetime_menuitem();

		// Tell calling code that we have handled this event; the buck stops here.
		return TRUE;
	}

	// Tell calling code that we have not handled this event; pass it on. 
	return FALSE;

}

/*------------------------------------------------------------------------------
 */
/*
gint gpppkill_idletime_enter_callback(GtkWidget *widget, GdkEventCrossing *event, gpppkill *gpppk)
{
	gtk_widget_set_state(gpppk->label_idletime, GTK_STATE_SELECTED);
	//gtk_style_set_background(gpppk->e_box_idle->style, 
	//                         gpppk->e_box_idle->window, GTK_STATE_PRELIGHT);

	return TRUE;
}
*/
/*------------------------------------------------------------------------------
 */
/*
gint gpppkill_idletime_leave_callback(GtkWidget *widget, GdkEventCrossing *event, gpppkill *gpppk)
{
	gtk_widget_set_state(gpppk->label_idletime, GTK_STATE_NORMAL);
	//gtk_style_set_background(gpppk->e_box_idle->style, 
	//                         gpppk->e_box_idle->window, GTK_STATE_NORMAL);

	return TRUE;
}
*/

/*------------------------------------------------------------------------------
 */
/*
gint gpppkill_onlinetime_enter_callback(GtkWidget *widget, GdkEventCrossing *event, gpppkill *gpppk)
{
	gtk_widget_set_state(gpppk->label_onlinetime, GTK_STATE_SELECTED);

	return TRUE;
}
*/

/*------------------------------------------------------------------------------
 */
/*
gint gpppkill_onlinetime_leave_callback(GtkWidget *widget, GdkEventCrossing *event, gpppkill *gpppk)
{
	gtk_widget_set_state(gpppk->label_onlinetime, GTK_STATE_NORMAL);

	return TRUE;
}
*/

// ### X Windows events ########################################################

/*------------------------------------------------------------------------------
 *	Para la clase gpppkill.
 *	Si retorno FALSE, gtk envia la signal destroy al widget
 *	Cuando se oprime close en la ventana del window manager.
 */
gint gpppkill_delete_event_callback(GtkWidget *widget, GdkEventAny *event, gpppkill *gpppk)
{
	if(!gpppk->salir)	//si nadie me pidio salir
		gpppk->quit_request();	//solicitar la terminacion de gpppkill

	return TRUE;
}

gint gpppkill_configure_event_callback(GtkWidget *widget, GdkEventConfigure *event, gpppkill *gpppk)
{
	gint xpos, ypos;

	gdk_window_get_root_origin(widget->window, &xpos, &ypos);
	if(GPPPKILL_DEBUG)
		printf("(%d, %d)\n", xpos, ypos);

	gpppk->setx(xpos);
	gpppk->sety(ypos);	

	gpppk->configure_size(event->width, event->height);

	gpppk->rc->about_version();

	//g_print("[%d,%d] (%dx%d)\n", event->x, event->y, event->width, event->height);
	//if(event->x != 0) {
	//	gpppk->set_width(event->height);
	//}
	
	return TRUE;
}

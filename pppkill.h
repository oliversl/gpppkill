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
/*
 *	Header file para ppp.cc
 *	+ enviar a xpppkill:
 *			update_frame_pppinfo()
 *			pppkill_dibujar()
 */
#ifndef _PPPKILL_HPP_OSL_
#define _PPPKILL_HPP_OSL_

//ppp includes
#include <iostream.h>
#include <gtk/gtk.h>
#include <stdio.h>
#include <sys/ioctl.h>
#include <string.h>
#include <net/if.h>
#include <net/if_ppp.h>
#include <net/ppp_defs.h>
#include <errno.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>		/* BSD, Linux, NeXT, Ultrix etc. */
#include <signal.h>
//ppp_id includes
#include <stdlib.h>
#include <sys/stat.h>
#include <dirent.h>
//find_uptime
#include <sys/stat.h>
#include <unistd.h> 
#include <sys/types.h>
#include <time.h>
#include <string.h>
#include <stdlib.h>
#include <asm/param.h>	//#define HZ 100
//interface
//#include <arpa/inet.h>

//#include "dlista.h"
#include "listadg.tmpl"
#include "listac.h"

//#define BUFSIZ en stdio.h @ line 42
#define DEF_IDLETIME       120
#define DEF_BYTE_MIN      1024
#define DEF_ONLINETIME    3600
#define DEF_PPPINTERFACE "ppp0"
#define DEF_TIEMPO_PROMEDIO 60	//1 minuto

#define PPPD_RUN_DIR "/var/run"
//longitud de los comandos de /proc/xxx/stat
#define CMD_BUF 64	//usado para definir arrays
#define CMD_BUF_STR "64"	//usado para scanf
#define STR_INTERFACE_LEN 7 //"pppxxx\0"

// how many bytes is 1kb: 1024 bytes
#define ONE_KB 1024
// how many bytes is 1mb: 1024*1024 bytes
#define ONE_MB 1048576

/*
 *	Estructura para pasar los datos a pppkill::dibujar().
 *	Es para no calcular nada en pppkill::dibujar().
 *	Asi, la funcion pppkill::dibujar() es mas portable.
 */
struct pppkill_pppstats {
	int           onlinetime;
	unsigned int  in,
								out,
								idletime;
	long					total_in,
								total_out,
								uptime;
	float					promedio_in,
								promedio_out,
								promedio_total_in,
								promedio_total_out;
} ;
//typedef struct pppkill_pppstats pppkill_stats;

//############################## class pppd ####################################
/*------------------------------------------------------------------------------
 *	representa la informacion que quiero saber del pppd
 */
class pppd {
	public:
		pppd();

		pid_t pid,
					chat;
		char state,
				 interface[STR_INTERFACE_LEN];
		int  up;

		int operator==(pppd p1);
};

//############################## class chat ####################################
/*------------------------------------------------------------------------------
 *	representa la informacion que quiero saber del chat
 */
class chat {
	public:
		chat();

		pid_t pid;
		pid_t ppid;
} ;

//################################# Class pppkill ##############################
class pppkill {
	protected:
		char interface[7];
		int	 s;	// socket o /dev/ppp file descriptor

		pid_t pppd_pid;
		//class dlista *dlista_pid;
		class listadg<pppd> *lista_pppd;
		class listadg<chat> *lista_chat;
		class listac *promedio_in,
								 *promedio_out;
		//banderas
		int pppd_ok,
				idletime_corriendo,	//para ver si la opcion idletime esta corriendo.
				idletime_corriendo_request,
				onlinetime_corriendo,	//para ver si la opcion onlinetime esta corriendo.
				onlinetime_corriendo_request,
				es_mio;	//este proceso me pertenece?
		
		long uptime; //uptime time del pppd. Ex: tiempo_online;
		int  idletime;	//tiempo maximo que el pppd puede estar idle. Ex: tiempo_pppkill;
		int  idletime_left;	//tiempo que falta para cortar. Va de idletime a cero. Ex: tiempo_pppkill_a_cortar;
		long onlinetime;	//tiempo maximo para estar online. Ex: tiempo_max_online;
		int  onlinetime_left;	//tiempo que falta para cortar. Va de onlinetime a cero.
		int  byte_min_in;
		int  byte_min_out;

		//int  buscar_pppd(char *cad);
		int  buscar_stat_cmd(char *buscar_cad, char *cad);
		long find_pppd_uptime(pid_t pid);
		int  exist_pid(void);
		int  es_mi_proceso(void);

		int  setup(void);
		int cargar_lista_pppd(void);
		int cargar_lista_chat(void);
		void borrar_lista_pppd(void);
		void borrar_lista_chat(void);
		pid_t chat_ppid(pid_t ppid);
		void reset_pppstats(void);
		void update_pppstats(void);
		void update_pppkill_pppstats(void);
		int  update_time_options(void);
		int  kill_pppd(int signal);
		int  get_stats(void);
		int  hay_algun_pppd(void);
		void sigte_interface(void);
		int  process_option_request(void);
		int  find_interface_pid(pid_t pid, char *iface);
		int  is_ifup(char *iface);

		unsigned int get_ibytes_diff();
		unsigned int get_obytes_diff();
		unsigned int get_ibytes_total();
		unsigned int get_obytes_total();

	public:
		pppkill();
		~pppkill();

		struct  ppp_stats ppp_stats_actual, 
											ppp_stats_viejo;
		pppkill_pppstats	stats;

		void actualizar_lista_pppd(void);
		pid_t get_interface_pid(char *match);
		pppd *get_pppd(char *match);
		char *get_dnodo_interface(int index);

		//get
		char *getinterface();
		listadg<pppd> *getlista_pppd();
		int  getpppd_ok(void);
		pid_t getpppd_pid();
		int  getes_mio(void);
		int  getidletime_corriendo();
		int  getonlinetime_corriendo();
		int  getidletime(void);
		int  getonlinetime(void);
		int  getbyte_min_in(void);
		int  getbyte_min_out(void);
			
		//set
		void setinterface(char *set);
		void setpppd_ok(int set);
		int  setidletime_corriendo(int set);
		int  setonlinetime_corriendo(int set);
		void setidletime(int set);
		void setonlinetime(int set);
		void reset_idletime_left(void);
		void reset_onlinetime_left(void);
		void setbyte_min_in(int set);
		void setbyte_min_out(int set);
		void idletime_request(int pedido);
		void onlinetime_request(int pedido);
};

#endif


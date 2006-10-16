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
#include "pppkill.h"

//############################## class pppd ####################################
/*------------------------------------------------------------------------------
 */
pppd::pppd()
{
	pid  = 0;
	chat = 0;
	state = 0;
	strcpy(interface, "");
	up   = 0;
}

/*------------------------------------------------------------------------------
 *	la igualda se da al comparar las interfaces
 */
int pppd::operator==(pppd p1)
{
	if(!strlen(interface))
		return 0;
	if(!strlen(p1.interface))
		return 0;
		
	if( !strcmp(interface, p1.interface) )
		return 1;
	else
		return 0;
}

//############################## class chat ####################################
/*------------------------------------------------------------------------------
 */
chat::chat()
{
	pid  = 0;	
	ppid = 0;
}

//############################## class pppkill #################################
     
/*------------------------------------------------------------------------------
 *	Constructor
 */
pppkill::pppkill(void)
{
	//banderas
	pppd_ok = 0;							
	idletime_corriendo = 0;
	idletime_corriendo_request = -1;
	onlinetime_corriendo = 0;
	onlinetime_corriendo_request = -1;
	es_mio = 0;

	pppd_pid  = 0;

	strcpy(interface, DEF_PPPINTERFACE);

	lista_pppd = new listadg<pppd>;
	lista_chat = new listadg<chat>;

	uptime = 0;
	idletime      = DEF_IDLETIME;
	idletime_left = idletime;
	onlinetime      = DEF_ONLINETIME;
	onlinetime_left = onlinetime;
	byte_min_in  = DEF_BYTE_MIN;
	byte_min_out = DEF_BYTE_MIN;

	promedio_in  = new listac(DEF_TIEMPO_PROMEDIO);
	promedio_out = new listac(DEF_TIEMPO_PROMEDIO);
	
	memset(&ppp_stats_actual, 0, sizeof(ppp_stats_actual));
	memset(&ppp_stats_viejo , 0, sizeof(ppp_stats_viejo) );
	memset(&stats , 0, sizeof(stats) );
}

/*------------------------------------------------------------------------------
 *	Destructor
 */
pppkill::~pppkill()
{
	if(lista_pppd) {
		delete lista_pppd;
		lista_pppd = NULL;
	}

	if(lista_chat) {
		delete lista_chat;
		lista_chat = NULL;
	}

	delete promedio_in;
	delete promedio_out;
}

/*------------------------------------------------------------------------------
 *	Inicializa todo lo necesario para usar los pppstats
 *	parametros:
 *		pppkill::interface debe estar seteado para que funcione.
 *	return:
 *		0 -> ok
 *		1 -> error: no se encuentra ningun pid de pppd.
 *		2 -> error: no se encuentra la interface buscada en pppkill::lista_pppd
 *		3 -> error: no se puede crear un socket IP.
 *		4 -> error: error de ioctl. La interface pppkill::interface no existe.
 *		5 -> error: el pppd esta discando
 */
int pppkill::setup(void)
{
	struct ifreq ifr;
	pppd *ppp;

  pppd_ok = 1;	// soy inocente hasta que se demuestre la contrario

	//ver si existe al menos un pppd
	actualizar_lista_pppd();
	ppp = get_pppd(interface);
	if(ppp) {	//si hay uno o mas pppd
		if(!ppp->up) {//si no encontro la interface pedida. este pppd esta discando.
			pppd_ok = 0;
			return 5;
		}
		if( ! strcmp(interface, ppp->interface) )	//si es el que estoy buscando
			pppd_pid = ppp->pid;
		else {
			pppd_ok = 0;
			return 2;
		}
	}
	else {	//no hay ningun pppd
		pppd_ok = 0;
		return 1;
	}
	
/*
  pppd_pid = get_interface_pid(interface);
  if( pppd_pid <= 0 ) {
  	pppd_ok = 0;
		if(pppd_pid == -1)
  		return 1;
		if(pppd_pid == 0)
  		return 2;
  }
*/

	//abrir socket
	s = socket(AF_INET, SOCK_DGRAM, 0);
	if (s < 0) {
		pppd_ok = 0;
    return 3;
	}

	//funca ioctl ?
//#undef  ifr_name
//#define ifr_name ifr_ifrn.ifrn_name
	strncpy(ifr.ifr_name, interface, sizeof(ifr.ifr_name));
	if (ioctl(s, SIOCGIFFLAGS, (caddr_t)&ifr) < 0) {
		pppd_ok = 0;
	  return 4;
  }

	//por si actualiza mientras pppkill esta coriendo
	uptime   = find_pppd_uptime(pppd_pid);
	idletime_left = idletime;
	onlinetime_left = onlinetime;
	reset_pppstats();
	es_mio = es_mi_proceso();
	
	return 0;
}

/*----------------------------------------------------------------------
 *	Ve si el pid/gid del pppd es igual al mio.
 *		O si el gid_pppd es igual a uno del los gid a los cuales yo 
 *			pertenesco.
 *
 *	return:
 *		0 -> no es de mio o no es de mi gid.
 *		1 -> si es mio o pertenece a mi gid.
 */
int pppkill::es_mi_proceso(void)
{
	char str[24];	//12 digitos para el pid
	int grps, i, res;
	gid_t *list, gid, uid;
	struct stat fs;

	res = 0;

	grps = 0;
	grps = getgroups(0, (gid_t *)0);
	if(grps) {
		list = (gid_t *)calloc(grps, sizeof(gid_t));
		if(list == NULL) {
			fprintf(stderr, "\npppkill::es_mi_proceso(): No memory for calloc!\n");
			return 0;
		}
		grps = getgroups(grps, list);
		//for(i=0; i<grps; i++) 
			//cout << list[i] << endl;
	}
	//cout << grps << endl;

	/* 
	 *	Patch para cuando root ejecuta el pppd
	 *  email de: Jean-Louis Leroy <jll@skynet.be>	
	 */
	//uid = getuid();
	//gid = getgid();
	uid = geteuid();
	gid = getegid();

	sprintf(str, "/proc/%d", pppd_pid);
	stat(str, &fs);
	
	//si yo o mi grupo levanto el pppd
	if( (uid == fs.st_uid) || (gid == fs.st_gid))
		res = 1;
	else {
		//si yo estoy en el grupo que levanto es pppd
		if(grps) {
			for(i=0; i<grps; i++) {
				if( list[i] == fs.st_gid) {
					res = 1;
					i = grps;
				}
			}
		}
	}

	return res;
}

/*----------------------------------------------------------------------
 *	Hace la llamada ioctl para optener las estadisticas del pppd.
 *	Si hay error setea la variable pppkill::pppd_ok a 0(cero), haciendo
 *		que se tenga que llamar a setup si se quiere obtener estadisticas.
 *	return:
 *		0 -> ok
 *		1 -> error: pppd no esta corriendo.
 *		2 -> error: falta soporte para ppp en el kernel.
 *		3 -> error: no se puede obtener las statdisticas de ppp.
 */
int pppkill::get_stats()
{
  struct ifpppstatsreq req;

  memset (&req, 0, sizeof (req));

	if(exist_pid() != 1) {
		pppd_ok = 0;
		return 1;
	}

	update_pppstats();	//ppp_stats_actual -> ppp_stats_viejo
	
  req.stats_ptr = (caddr_t) &req.stats;

// esto eta recontra mal!!!
//#undef ifr_name
//#define ifr_name ifr__name

  strncpy(req.ifr__name, interface, sizeof(req.ifr__name));
	if (ioctl(s, SIOCGPPPSTATS, &req) < 0) {
		pppd_ok = 0;
		if (errno == ENOTTY) 
			return 2;
		else
			return 3;
  }
  ppp_stats_actual = req.stats;	//guardar las nuevas stats

	update_pppkill_pppstats();	//actualizar pppkill::stats
	  
  return 0;
}

/*------------------------------------------------------------------------------
 *	Carga todos los pid de los pppd pppkill::lista_pppd.
 *	Lo hace recoriendo todos los procesos del directorio /proc
 *	return:
 *		>0: numero de elementos en la lista_pppd.
 *		 0: lista vacia, no hay ningun pppd
 *		
 */
int pppkill::cargar_lista_pppd(void)
{
	char    dir_actual[BUFSIZ+1],
			    str_pid[12],
			    str_cmd[CMD_BUF+1],
			    str_state[CMD_BUF+1];	
	pid_t pid_pppd;
  FILE 	  *fp_cmdline;
  DIR     *dp;
  umode_t mode;
  struct dirent *dir_entry;
  struct stat   file_stat;
  pppd nuevo_ppp;
	
	pid_pppd = 0;
	lista_pppd->borrar_listadg();	//borrar la lista para cargarla de nuevo
	
	//guardo el directorio actual
	getcwd(dir_actual, BUFSIZ);

  //trato de abrir el dir "/proc"
  if((dp = opendir("/proc")) == NULL) {
    //perror("no puedo abrir el dir /proc:");
    exit(1);
  }

	//voy a /proc
	chdir("/proc");

  while((dir_entry = readdir(dp)) != NULL) {
		//obtengo el modo
    stat(dir_entry->d_name, &file_stat);
    mode = file_stat.st_mode;

		if(S_ISDIR(mode)) {		//si es dir
			
			if(!strcmp(dir_entry->d_name, ".") || !strcmp(dir_entry->d_name, ".."))
				continue;
			chdir(dir_entry->d_name);	//entrar al dir del proceso

			fp_cmdline = fopen("stat", "r");
			if(fp_cmdline == NULL) {
				//perror("No puedo abrir archivo");
				chdir("..");	// vuelvo a /proc
				continue;
			}

			fscanf(fp_cmdline, "%"CMD_BUF_STR"s", str_pid);
			fscanf(fp_cmdline, "%"CMD_BUF_STR"s", str_cmd);
			fscanf(fp_cmdline, "%"CMD_BUF_STR"s", str_state);

			fclose(fp_cmdline);

			if(buscar_stat_cmd("pppd", str_cmd)) {
				pid_pppd = atol(str_pid);
				nuevo_ppp.pid = pid_pppd;
				nuevo_ppp.state = str_state[0];
				lista_pppd->agregar(nuevo_ppp);
				//cerr << "pppd->state: " << nuevo_ppp.state << endl;
				//break;
			}

			chdir("..");	// vuelvo a /proc
		}
	}
  closedir(dp);

	chdir(dir_actual);

	return lista_pppd->get_numero_de_elementos();
}

/*------------------------------------------------------------------------------
 *	Carga todos los pid de los pppd pppkill::lista_pppd.
 *	Lo hace recoriendo todos los procesos del directorio /proc
 *	return:
 *		>0: numero de elementos en la lista_chat.
 *		 0: lista vacia, no hay ningun chat
 */
int pppkill::cargar_lista_chat(void)
{
	char    dir_actual[BUFSIZ+1],
			    str_pid[12],
			    str_cmd[CMD_BUF+1],
			    str_state[CMD_BUF+1],
			    str_ppid[CMD_BUF+1];
	pid_t pid_chat, ppid_chat;
  FILE 	  *fp_cmdline;
  DIR     *dp;
  umode_t mode;
  struct dirent *dir_entry;
  struct stat   file_stat;
  chat nuevo_chat;
	
	pid_chat = 0;
	if(lista_chat) lista_chat->borrar_listadg();	//borrar la lista para cargarla de nuevo
	
	//guardo el directorio actual
	getcwd(dir_actual, BUFSIZ);

  //trato de abrir el dir "/proc"
  if((dp = opendir("/proc")) == NULL) {
    //perror("no puedo abrir el dir /proc:");
    exit(1);
  }

	//voy a /proc
	chdir("/proc");

  while((dir_entry = readdir(dp)) != NULL) {
		//obtengo el modo
    stat(dir_entry->d_name, &file_stat);
    mode = file_stat.st_mode;

		if(S_ISDIR(mode)) {		//si es dir
			
			if(!strcmp(dir_entry->d_name, ".") || !strcmp(dir_entry->d_name, ".."))
				continue;
			chdir(dir_entry->d_name);	//entrar al dir del proceso

			fp_cmdline = fopen("stat", "r");
			if(fp_cmdline == NULL) {
				//perror("No puedo abrir archivo");
				chdir("..");	// vuelvo a /proc
				continue;
			}

			fscanf(fp_cmdline, "%"CMD_BUF_STR"s", str_pid);
			fscanf(fp_cmdline, "%"CMD_BUF_STR"s", str_cmd);
			fscanf(fp_cmdline, "%"CMD_BUF_STR"s", str_state);
			fscanf(fp_cmdline, "%"CMD_BUF_STR"s", str_ppid);

			fclose(fp_cmdline);

			if(buscar_stat_cmd("chat", str_cmd)) {
				pid_chat = atol(str_pid);
				ppid_chat = atol(str_ppid);
				//cout << pid_chat << endl;
				nuevo_chat.pid = pid_chat;
				nuevo_chat.ppid = ppid_chat;
				lista_chat->agregar(nuevo_chat);
				//break;
			}

			chdir("..");	// vuelvo a /proc
		}
	}
  closedir(dp);

	chdir(dir_actual);

	return lista_chat->get_numero_de_elementos();
}

/*------------------------------------------------------------------------------
 */
void pppkill::borrar_lista_pppd(void)
{
	lista_pppd->borrar_listadg();
}

/*------------------------------------------------------------------------------
 */
void pppkill::borrar_lista_chat(void)
{
	lista_chat->borrar_listadg();
}

/*--------------------------------------------------------------------
 *	Modificado de buscar.c
 *
 *	usado para buscar en /proc/xxx/stat
 *	- En el archivo stat se encuentra el nombre del programa 
 *			entre parentesis.
 *	- Asi el programa '/usr/sbin/pppd' 
 *			estara asi: (pppd) en cad.
 *
 *	izq -> posicion de cad en donde empieza match.
 *	der -> posicion de cad en donde termina match. Es la posicion del '\0'.
 *	longitud -> longitud de macth
 */
/*
int pppkill::buscar_pppd(char *cad)
{
	char match[]="pppd";
	int izq, longitud;
	int encontrado, der;

	//aqui se busca a 'match' en 'cad'
	encontrado=0;
	for(izq=0; cad[izq] && !encontrado; izq++) {
		if( cad[izq]==match[0] )
		{
			for(der=izq+1, longitud=1; 
					cad[der] && match[longitud] && (cad[der]==match[longitud]);
					der++, longitud++);

			if((unsigned)longitud==(strlen(match))) //strlen("abc")==3, abc[3]==\0
			{
				//hasta aqui encontre una subcadena 'match' en 'cad'
				//si match empieza en la posicion 1 de cad, despues del '('
				if(izq == 1) {	
					//si despues de la 'd' esta el ')'
					if( cad[der] == ')' )	 
							encontrado=1;	
				}//end if(izq==1)
			}//end if(strlen)
		}//end if(cad[izq]==match[0])
	}//end for izq

	if(!encontrado) 
		return 0;
	else 
		return 1;
}
*/

/*--------------------------------------------------------------------
 *	Busca la cadena "(buscar_cad)" en cad. cad es de la forma "(str)"
 *	
 *	Algoritmo:
 *		Se recorre cad hasta que se encuentre un caracter igual a 
 *		match[0]. Luego se recorrer mientras sigan siguan siendo 
 *		iguales los caracteres de match y cad. Si en la segunda
 *		recorrida se encuentra un caracter diferente, se sigue desde
 *		donde se quedo el primer for(recorrida).
 *		
 *	Maxima longitud de biscar_cad = 61 bytes
 *	
 *	usado para buscar en /proc/xxx/stat
 *	- En el archivo stat se encuentra el nombre del programa 
 *			entre parentesis.
 *	- Asi el programa '/usr/sbin/pppd' 
 *			estara asi: (pppd) en cad.
 *
 */
int pppkill::buscar_stat_cmd(char *buscar_cad, char *cad)
{
	char match[64];
	int i, j, encontrado;

	sprintf(match, "(%s)", buscar_cad);

	//aqui se busca a 'match' en 'cad'
	encontrado=0;
	for(i=0; cad[i] && !encontrado; i++) {
		if( cad[i]==match[0] )
		{
			encontrado = 1;	//doy por encontrado hasta que se demuestre lo contrario.
			for(j=1; cad[i+j] && match[j] && encontrado; j++) {
				if( cad[i+j] != match[j] )
					encontrado = 0;
			}

		}//end if(cad[i]==match[0])
	}//end for i

	if(!encontrado) 
		return 0;
	else 
		return 1;
}

/*--------------------------------------------------------------------
 *	Devuelve:
 *	- el uptime de pid en segundos si pid!=0.
 *	- 0 si pid == 0.
 *
 *	Devolviendo como long la cantidad de segundos tenemos 
 *		como maximos valores:
 *	  (((2^32)/2)-1) = 2147483647 segundos
 *									 o   35791394 minutos
 *									 o     596523 horas
 *									 o      24855 dias
 *  								 o	       68 anhos :-)
 *
 */
long pppkill::find_pppd_uptime(pid_t pid)
{
  char   str[CMD_BUF], ch;
  int    i;
	long   process_uptime, 
				 process_start_time;
	double system_uptime;
	FILE   *fp;

	if(!pid)
		return pid;

	system_uptime = 0;

	//------------- Uptime del systema ----------
	fp = fopen("/proc/uptime", "r");
	if(fp == NULL)
		exit(1);
    
	if( fscanf(fp, "%lf", &system_uptime) < 1 ) {
		fprintf(stderr, "bad data in " "/proc/uptime" "\n");
		return 0;
	}

	fclose(fp);
	
	//------------- leer stat --------------------
	sprintf(str, "/proc/%d/stat", pid);

	fp = fopen(str, "r");
	if(fp == NULL)
		exit(1);

	//posicionarme despues del ')' del nombre del proceso
	do {
		ch = fgetc(fp);
	} while( ch != ')' );
	ch = fgetc(fp);	//obtener el espacio en blanco

	//obtener el string 20 que viene despues del nombre del proceso.
	for(i=0; i<20; i++)
		fscanf(fp, "%"CMD_BUF_STR"s", str);
		
	fclose(fp);

	process_start_time = atol(str);

	//------------- calculo de process_uptime ---
	process_uptime = (long)((system_uptime * HZ) - process_start_time) / HZ;

  return process_uptime;
}

/*--------------------------------------------------------------------
 *	Coloca en el estado inicial del pppstats.
 *	Se usa en pppkill::setup() y en gpppkill::continuar().
 */
void pppkill::reset_pppstats()
{
	ppp_stats_actual.p.ppp_ibytes = 0;
	ppp_stats_actual.p.ppp_obytes = 0;
}

//--------------------------------------------------------------------
void pppkill::update_pppstats()
{
	ppp_stats_viejo = ppp_stats_actual;
}

//--------------------------------------------------------------------
void pppkill::update_pppkill_pppstats(void)
{
	//bytes in (recividos)
	stats.in = get_ibytes_diff();

	//bytes out (enviados)
	stats.out =  get_obytes_diff();

	//bytes total_in (total de bytes recividos)
	stats.total_in  = get_ibytes_total();

	//bytes total_out(total de bytes enviados)
	stats.total_out = get_obytes_total();

	//agregar los datos actuales para que entren en promedio actual
	promedio_in->agregar_al_final(stats.in);
	promedio_out->agregar_al_final(stats.out);

	//actualizar promedios:
	stats.promedio_in  = ( ((float)(promedio_in->sumatoria())  / ONE_KB) / (float)DEF_TIEMPO_PROMEDIO);
	stats.promedio_out = ( ((float)(promedio_out->sumatoria()) / ONE_KB) / (float)DEF_TIEMPO_PROMEDIO);

	stats.promedio_total_in  = (((float)stats.total_in  / ONE_KB) / (float)uptime) ;
	stats.promedio_total_out = (((float)stats.total_out / ONE_KB) / (float)uptime);
}

/*--------------------------------------------------------------------
 *	Actuliza los contadores de las opciones de tiempo:
 *		idletime y onlinetime
 *	return:
 *		0 -> ok:    NO se necesita terminar el enlace ppp.
 *		1 -> error: terminar el enlace ppp. idletime_left == 0
 *		2 -> error: terminar el enlace ppp. onlinetime_left == 0
 */
int pppkill::update_time_options(void)
{
	if( !(uptime % 10) )	//leer el uptime del pppd del /proc cada 10s
		uptime = find_pppd_uptime(pppd_pid);
	else 
		uptime++;
	stats.uptime = uptime;

	if(!es_mio)
		return 0;
	
	if(idletime_corriendo) {
		if( (get_ibytes_diff() < (unsigned)byte_min_in) && 
				(get_obytes_diff() < (unsigned)byte_min_out) ) {	//si estoy idle
			if(idletime_left > 0)	//si puedo disminuir idletime_left
				idletime_left--;
			else {	//situacion de idletime_left == 0
				stats.idletime = 0;
				//setpppd_ok(0);
				//idletime_corriendo = 0;
				reset_idletime_left();
				return 1;
			}
		}
		else	//no estoy idle
			idletime_left = idletime;

		stats.idletime = idletime_left;
	}
	else	//no tengo la opcion idle_time habilitada
		idletime_left = idletime;

	if(onlinetime_corriendo) {
		if(onlinetime_left > 0) {
			onlinetime_left--;
		}
		else {
			stats.onlinetime = 0;
			//setpppd_ok(0);
			//onlinetime_corriendo = 0;
			reset_onlinetime_left();
			return 2;
		}
		stats.onlinetime = (int)onlinetime_left;
	}
	else 
		onlinetime_left = onlinetime;
		
	return 0;
}

/*--------------------------------------------------------------------
 *	argumento signal:
 *		0 -> enviar un SIGTERM al pid
 *		1 -> enviar un SIGKILL al pid
 *
 *	return:
 *		0 -> ok   : Senhal seleccionada fue enviada sin problemas.
 *		1 -> error: la senhal enviada al pid dio error.
 *		2 -> error: el pid pppkill::pppd_pid no existe. No se envio la senhal.
 *		3 -> error: el pid pppkill::pppd_pid no es mio. No se envio la senhal.
 *	 -1 -> error: parametro en argumento signal no valido.
 */
int pppkill::kill_pppd(int signal)
{
	if(exist_pid() != 1)
		return 2;

	if(!es_mio)
		return 3;

	if(signal == 0) {
		if(kill(pppd_pid, SIGTERM))
			return 1;
		else
			return 0;
	}
	else {
		if(signal == 1) {
			if(kill(pppd_pid, SIGKILL))
				return 1;
			else 
				return 0;
		}
		else
			return (-1);
	}
}

/*--------------------------------------------------------------------
 *	Revisa si existe el pid que esta en pppkill::pppd_pid
 *	Lo hace revisando si existe el archivo /proc/pppd_pid/stat
 *	return:
 *	 -1 -> error : no puedo leer el archivo stat. No tengo permiso.
 *		0 -> error : el pid pppkill::pppd_pid NO existe
 *		1 -> ok    : el pid pppkill::pppd_pid SI existe
 */
int pppkill::exist_pid(void)
{
	char str[24];	//12 digitos para el pid

	sprintf(str, "/proc/%d/stat", pppd_pid);
	if( !access(str, F_OK) )	//si existe el archivo /proc/pid/stat
		return 1;
	else
		return 0;
}

/*--------------------------------------------------------------------
 *	Ve si hay algun pppd en pppkill::lista_pppd
 *	return:
 *		0 -> no hay ningun pppd en la dlista
 *		1 -> hay uno o mas pppds en la dlista
 */
int pppkill::hay_algun_pppd(void)
{
	int num;
	
	num = lista_pppd->get_numero_de_elementos();

	if(num)
		return 1;
	else
		return 0;
}

/*--------------------------------------------------------------------
 *	Busca en la lista_pppd si existe algun pppd con (pppd->up == 1).
 *	Si lo encuentra, configura pppkill::interface con pppd->interface
 *		del pppd que tiene (pppd->up == 1)
 *	
 *	//Setea la string pppkill::interface con el nombre de la primera
 *	//	interface que aparece en pppkill::lista_pppd.
 */
void pppkill::sigte_interface(void)
{
	int encontrado;
	nododg<pppd> *p;

	p = lista_pppd->get_inicio();

	encontrado = 0;
	while(p && !encontrado) {
		if(p->data.up)
			encontrado = 1;
		else
		 p = p->sigte;
	};
	
	if(encontrado)
		strcpy(interface, p->data.interface);
/*
	//la primera interface, ppp0
	strcpy(interface, get_dnodo_interface(1));
	//strcpy(interface, "ppp0");
*/
}

/*--------------------------------------------------------------------
 *	Satiface los pedidos de activacion de las opciones.
 *	Esta funcion se llama cada vez que se detecta un nuevo pppd.
 *	return:
 *		0: ok -> no hubo problema
 *		1: error -> error al activar idletime
 *		2: error -> error al activar onlinetime
 *		3: error -> error al activar idletime y onlinetime.
 */
int pppkill::process_option_request(void)
{
	int idle_res, online_res, res;

	if(idletime_corriendo_request >= 0) {
		idle_res = setidletime_corriendo(idletime_corriendo_request);
		idletime_corriendo_request = -1;
	}
	else
		idle_res = 0;

	if(onlinetime_corriendo_request >= 0) {
		online_res = setonlinetime_corriendo(onlinetime_corriendo_request);
		onlinetime_corriendo_request = -1;
	}
	else
	online_res = 0;

	res = 0;
	if(idle_res)
		res=1;
	if(online_res) {
		if(idle_res)
			res=3;
		else
			res=2;
	}

	return res;
}

/*--------------------------------------------------------------------
 *	Dado un pid, se busca dentro de los archivos /var/run/ppp*.pid 
 *		ese pid.
 *	Es decir, busca si alguna interface tiene el pid dado.
 *	Si se encuentra se retorna 0 y el nombre de la interface en iface.
 *	
 *	return:
 *		0: ok    -> se ha encontrado el pid en un archivo
 *		            se retorna el nombre de la interface en iface
 *		1: error -> no se encontro el pid en ningun archivo
 */
int pppkill::find_interface_pid(pid_t pid, char *iface)
{
	char    dir_actual[BUFSIZ+1];
	int     encontrado;
	pid_t   run_pid;
  FILE 	  *fp;
  DIR     *dp;
  struct dirent *dir_entry;
	
	//guardo el directorio actual
	getcwd(dir_actual, BUFSIZ);

  //trato de abrir el dir "/proc"
  if((dp = opendir(PPPD_RUN_DIR)) == NULL) {
    //perror("no puedo abrir el dir /proc:");
    exit(1);
  }

	//voy a /proc
	chdir(PPPD_RUN_DIR);

	encontrado = 0;
  while( !encontrado && ((dir_entry = readdir(dp)) != NULL) ) {
		//revisar si un archivo pppx.pid
		if( !strncmp(dir_entry->d_name, "ppp", 3) && (dir_entry->d_name[3] != '-') ) {

			fp = fopen(dir_entry->d_name, "r");
			if(fp == NULL) {
				cerr << "pppkill::find_interface_pid(): no se puede leer el archivo '" << dir_entry->d_name << "'" << endl;
				cerr << "Esto se deberia poder hacer!" << endl;
				exit(1);
			}
			fscanf(fp, "%d", &run_pid);
			fclose(fp);

			if(pid == run_pid) {
				encontrado = 1;

				sscanf(dir_entry->d_name, "ppp%d.pid", &run_pid);
				sprintf(iface, "ppp%d", run_pid);
			}

		}
	} ; // while

  closedir(dp);
	chdir(dir_actual);

	return !encontrado;
}

/*--------------------------------------------------------------------
 *	Busca entre las interfaces del sistema una interface igual a la
 *		pasada en la cadena iface.
 *	
 *	return:
 *		0: ok    -> se encontro la interface.
 *		1: error -> no se econtro la interface.
 */
int pppkill::is_ifup(char *iface)
{
	char buffer[BUFSIZ];
	int s_ifc, res, i;
	struct ifconf ifc;
	// uso estos punteros para que sea mas legible el codigo al imprimir
	//las interfaces.
	struct ifreq *ifr;	

	memset(buffer, '0', BUFSIZ);

	//abrir socket
	s_ifc = socket(AF_INET, SOCK_DGRAM, 0);
	if (s_ifc < 0) {
		perror("pppkill::is_ifup()");
    return 1;
	}

	//lenar datos antes de llamar a ioctl
	ifc.ifc_buf = buffer;
	ifc.ifc_len = BUFSIZ;
	//llamar a ioctl
	res = ioctl(s_ifc, SIOCGIFCONF, (struct ifconf *)&ifc);
	if(res < 0) {
		perror("pppkill::is_ifup()");
		return 1;
	}

	// ciclar todas las interfaces
	res = 0;
	for(i=0; (char)ifc.ifc_req[i].ifr_name[0] != '0'; i++) {
		ifr = &(ifc.ifc_req[i]);
		if( !strcmp(iface, ifr->ifr_name) )
			res = 1;
	}

	return res;
}

/*------------------------------------------------------------------------------
 *	return:
 *	 >0:ok    --> se retorna el pid.
 *		0:error --> no se encuentra la interface especificada(match).
 *	 -1:error --> no hay ningun pppd corriendo.
 */
pid_t pppkill::get_interface_pid(char *match)
{
	int pid_tmp;
	nododg<pppd> *p;
	
	p = lista_pppd->get_inicio();
	
	if(!p)
		return -1;	//no hay ningun pppd corriendo.

	//buscar la interface pedida
	pid_tmp = 0;
	while(p && !pid_tmp) {
		if(!strcmp(match, p->data.interface))
			pid_tmp = p->data.pid;
		
		p = p->sigte;	//paso al sigte
	};

	//si no se encontro la interface pedida, ver si hay algun pppd discando.
	p = lista_pppd->get_inicio();
	while(p && !pid_tmp) {
		if(p->data.chat)
			pid_tmp = p->data.pid;
		
		p = p->sigte;	//paso al sigte
	};

	return pid_tmp;

/*
	if(n) { //si hay algun elemento
		if(!strcmp(match, n->data.interface))	//busco en el primer nodo
			return n->data.pid;
		while(n->sigte) {	//sigo buscando
			n = n->sigte;	//paso al sigte
			if(!strcmp(match, n->data.interface))
				return n->data.pid;
		};
		return (long)0;	//no se encontro la interface
	}
	else
		return (long)-1;	//no hay ningun pppd corriendo.
*/
}

/*------------------------------------------------------------------------------
 *	Busca si algun class pppd con el pppd.interface == match y retorna ese pppd.
 *	Si no se encuentra el pppd pedido, se retorna el sigte pppd up.
 *	Si no se encuentra un pppd up, se retorna el sigte pppd que esta discando.
 *	return:
 *		NULL: error -> no se encuentra ningun pppd.
 *	 !NULL: ok    ->
 *	 	if(pppd.interface == match) 
 *	 		return pppd
 *	 	else
 *	 		if(pppd.up != 0)
 *	 			return pppd
 *	 		else
 *	 			if(pppd.chat != 0)
 *	 				return pppd
 */
pppd *pppkill::get_pppd(char *match)
{
	int encontrado;
	nododg<pppd> *p;
	
	p = lista_pppd->get_inicio();
	
	//buscar la interface pedida
	encontrado = 0;
	while(p && !encontrado) {
		if(!strcmp(match, p->data.interface))
			encontrado = 1;
		else
			p = p->sigte;	//paso al sigte
	};

	//si no se encontro la interface pedida, ver si hay algun pppd up.
	if(!encontrado) {
		p = lista_pppd->get_inicio();
		while(p && !encontrado) {
			if(p->data.up)	//si hay algun pppd discando o up.
				encontrado = 1;
			else
				p = p->sigte;	//paso al sigte
		};
	}

	//si no se encontro la interface pedida, ver si hay algun pppd discando.
	if(!encontrado) {
		p = lista_pppd->get_inicio();
		while(p && !encontrado) {
			if(p->data.chat)	//si hay algun pppd discando o up.
				encontrado = 1;
			else
				p = p->sigte;	//paso al sigte
		};
	}

	if(p)
		return &(p->data);
	else
		return (pppd *)NULL;
}

/*------------------------------------------------------------------------------
 *	argument:
 *		index : numero de interface. La primera interface es la numero 1.
 *						que equivale a la interface ppp0
 *	return:
 *		(char *): ok -> se retorna dnodo::interface
 *		NULL    : error -> el indice del nodo es mayor que el numero de elementos
 *		                -> dnodo encontrado == NULL
 */
char *pppkill::get_dnodo_interface(int index)
{
	char p[10];
	int encontrado;
	nododg<pppd> *n;

	sprintf(p, "ppp%d", (index-1) );

	encontrado = 0;			
	n = lista_pppd->get_inicio();
	while( !encontrado && n ) {
		if( !strcmp(p, n->data.interface) )
			encontrado = 1;
		else
			n = n->sigte;
	};

	if(encontrado)
		return n->data.interface;
	else
		return (char *)NULL;
}

/*------------------------------------------------------------------------------
 *	Actualiza el listado de interface pppds con todas sus informaciones.
 *		- borrar lista_pppd
 *		- cargar lista_pppd
 *		- borrar lista_chat
 *		- cargar lista_chat
 *		- ver si cada pppd tiene algun chat
 *		- ver si cada pppd tiene una interface
 */
void pppkill::actualizar_lista_pppd(void)
{
	pid_t pid;
	nododg<pppd> *p;
	nododg<chat> *c;

	borrar_lista_pppd();
  borrar_lista_chat();
  if(cargar_lista_pppd()) {
	  cargar_lista_chat();
  }

	/*
	 *	Setear las propiedades de las class pppd de la lista_pppd. 
	 *	Usa la lista_chat para ver si los pppd tiene hijos chat.
	 */
	p = lista_pppd->get_inicio();
	while(p) {
		//ver si el pppd tiene un chat
		pid = chat_ppid(p->data.pid);
		if(pid)
			p->data.chat = pid;

		//ver si el pppd tiene una interface
		if( !find_interface_pid(p->data.pid, p->data.interface) )
			p->data.up = 1;
		
		//revisar sigte pppd
		p = p->sigte;
	};
}

/*------------------------------------------------------------------------------
 *	busca en la lista_chat por un chat que tenga como chat.ppid == ppid.
 *	devuelve chat.pid si lo encuentra.
 *	
 *	return:
 *		0: error -> no se encuentra el ppid.
 *	 >0: ok    -> el pid del chat que tiene chat.ppid == ppid
 */
pid_t pppkill::chat_ppid(pid_t ppid)
{
	pid_t res_pid;
	nododg<chat> *c;

	c = lista_chat->get_inicio();

	res_pid = 0;
	while(c) {
		if( c->data.ppid == ppid ) {
			res_pid = c->data.pid;
			c = NULL;
		}
		else
			c = c->sigte;
	};

	return res_pid;
}

// ---- get functions ---
//--------------------------------------------------------------------
int pppkill::getes_mio(void)
{
	return es_mio;
}

//--------------------------------------------------------------------
char *pppkill::getinterface()
{
	return (char *)interface;
}

//--------------------------------------------------------------------
listadg<pppd> *pppkill::getlista_pppd()
{
	return lista_pppd;
}

//--------------------------------------------------------------------
int pppkill::getpppd_ok(void)
{
	return pppd_ok;
}

//--------------------------------------------------------------------
pid_t pppkill::getpppd_pid()
{
	return pppd_pid;
}

//--------------------------------------------------------------------
int pppkill::getidletime_corriendo()
{
	return idletime_corriendo;
}

//--------------------------------------------------------------------
int pppkill::getonlinetime_corriendo()
{
	return onlinetime_corriendo;
}

//--------------------------------------------------------------------
int pppkill::getidletime(void)
{
	return idletime;
}

//--------------------------------------------------------------------
int pppkill::getonlinetime(void)
{
	return onlinetime;
}

//--------------------------------------------------------------------
int pppkill::getbyte_min_in(void)
{
	return byte_min_in;
}

//--------------------------------------------------------------------
int pppkill::getbyte_min_out(void)
{
	return byte_min_out;
}

//--------------------------------------------------------------------
unsigned int pppkill::get_ibytes_diff()
{
	if(ppp_stats_viejo.p.ppp_ibytes) {
		if(ppp_stats_actual.p.ppp_ibytes)
			return (unsigned int)(ppp_stats_actual.p.ppp_ibytes - ppp_stats_viejo.p.ppp_ibytes);
		else
			return (unsigned)0;
	}
	else
		return (unsigned)0;
}

//--------------------------------------------------------------------
unsigned int pppkill::get_obytes_diff()
{
	if(ppp_stats_viejo.p.ppp_obytes)
		if(ppp_stats_actual.p.ppp_ibytes)
			return (unsigned int)(ppp_stats_actual.p.ppp_obytes - ppp_stats_viejo.p.ppp_obytes);
		else
			return (unsigned)0;
	else
		return (unsigned)0;
}

//--------------------------------------------------------------------
unsigned int pppkill::get_ibytes_total()
{
	return (unsigned int) ppp_stats_actual.p.ppp_ibytes;
}

//--------------------------------------------------------------------
unsigned int pppkill::get_obytes_total()
{
	return (unsigned int) ppp_stats_actual.p.ppp_obytes;
}

// ---- set functions ---
//--------------------------------------------------------------------
void pppkill::setinterface(char *set)
{
	if(set)
		strcpy(interface, set);
}

//--------------------------------------------------------------------
void pppkill::setpppd_ok(int set)
{
	pppd_ok = set;
}

/*--------------------------------------------------------------------
 *	Setea la opcion idletime segun 'set'
 *	return:
 *		0: ok -> se seteo la opcion idletime segun lo pedido.
 *		1: error -> no soy duenho del pppd.
 */
int pppkill::setidletime_corriendo(int set)
{
	if(!set) {	//cualquiera puede dessetear
		idletime_corriendo = set;
		return 0;
	}

	if(es_mio) {	//si quiero setear, solo el duenho lo puede hacer
		idletime_corriendo = set;
		return 0;
	}
	else {
		idletime_corriendo = 0;
		return 1;
	}
}

/*--------------------------------------------------------------------
 *	Setea la opcion onlinetime segun 'set'
 *	return:
 *		0: ok -> se seteo la opcion onlinetime segun lo pedido.
 *		1: error -> no soy el duenho del pppd.
 */
int pppkill::setonlinetime_corriendo(int set)
{
	if(!set) {	//cualquiera puede dessetear
		onlinetime_corriendo = set;
		return 0;
	}

	if(es_mio) {
		onlinetime_corriendo = set;
		return 0;
	}
	else {
		onlinetime_corriendo = 0;
		return 1;
	}
}

//--------------------------------------------------------------------
void pppkill::setidletime(int set)
{
	idletime = set;
	idletime_left = idletime;
}

//--------------------------------------------------------------------
void pppkill::setonlinetime(int set)
{
	onlinetime = set;
	onlinetime_left = onlinetime;
}

//--------------------------------------------------------------------
void pppkill::reset_idletime_left(void)
{
	idletime_left = idletime;
}

//--------------------------------------------------------------------
void pppkill::reset_onlinetime_left(void)
{
	onlinetime_left = onlinetime;
}

//--------------------------------------------------------------------
void pppkill::setbyte_min_in(int set)
{
	byte_min_in = set;
}

//--------------------------------------------------------------------
void pppkill::setbyte_min_out(int set)
{
	byte_min_out = set;
}

//--------------------------------------------------------------------
void pppkill::idletime_request(int pedido)
{
	idletime_corriendo_request = pedido;
}

//--------------------------------------------------------------------
void pppkill::onlinetime_request(int pedido)
{
	onlinetime_corriendo_request = pedido;
}

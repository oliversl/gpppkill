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
#include "rcgpppkill.h"

//------------------------------------------------------------------------------
rcgpppkill::rcgpppkill(class gpppkill *g)
{
	int access_fd;
	uid_t u;
	struct passwd *p;

	gpppk = g;
	/* 
	 *	update_info
	 *	 1 -> fresh install
	 *	 2 -> upgrade
	 *	 3 -> downgrade
	 */
	update_info = 0;
	
	//geteuid() are always sugcessfull!
	u = geteuid();	//efective uid of the running process
	p = getpwuid(u);

	strcpy(path, "");	//limpiar la cadena por las dudas nada mas.
	if(p) 
		strcpy(path, p->pw_dir);
	else {	//no encuentro el nombre de usuario. Esto es grave, imprimir un mensaje!
		strcpy(path, "/tmp");
		fprintf(stderr, "No name for uid:%d in /etc/passwd.\n", u);
	}

	strcat(path, "/." RCFILE_GPPPKILL);

	//cout << "uid: " << u <<endl;
	//cout << "rc path: " << path << endl;
	//if(!access(path, F_OK)) {	//si existe
	access_fd = open(path, O_RDONLY);
	if(access_fd > 0) {	//si existe
		close(access_fd);
		//cout << "access to rc file" << endl;
		check_version();	//verificar version
	}
	else  {
		perror("rc file");
		//cout << "NO access to rc file" << endl;
		escribir_template();
		update_info = 1;
	}
}

//------------------------------------------------------------------------------
rcgpppkill::rcgpppkill(char *rc)
{
	FILE *fp;

	strcat(path, strcat("/tmp/.", rc));

	if(!access(path, F_OK)) {	//si existe
		fp = fopen(path, "r+");
		if(!fp) {
			perror("rcgpppkill");
			exit(1);
		}
	}
	else {
		fp = fopen(path, "w+");
		if(!fp) {
			perror("rcgpppkill");
			exit(1);
		}
	}

	fclose(fp);
}

/*------------------------------------------------------------------------------
 *	Sobreescribe el archivo de configuracion.
 *	Cambiado para gpppkill 0.9.14.
 */
void rcgpppkill::escribir_template(void)
{
	int e;
	FILE *fp;
	
	fp = fopen(path, "w");
	if(!fp) {
		perror("rcgpppkill::escribir_template()");
		gtk_exit(1);
	}
	
	e = fprintf(fp, "# gpppkill configuration file\n"
									"#\n"
									"# Syntax: <OPTION><SPACE>=<SPACE><VALUE><NEW LINE>\n"
									"#\n"
									"# Lines that start with a '#' are comments\n"
									"# The OPTION = VALUE *must* be one per line\n");
	if(!e) {
		fprintf(stderr, "rcgpppkill::escribir_template(): write error!\n");
		return;
	}

	e = fprintf(fp, "\n"
									"# For compatibility with further versions. Do not change please.\n"
									"VERSION = " VERSION "\n");
	if(!e) {
		fprintf(stderr, "rcgpppkill::escribir_template(): write error!\n");
		return;
	}

	e = fprintf(fp, "\n"
							"# Window position\n"
							"xPos = 0\n"
							"yPos = 0\n");
//							"# Window Manager decoration dimension.\n"
//							"# If you switch to another WM, set one(or both) of this options to -1 \n"
//							"dxWM = -1\n"
//							"dyWM = -1\n");
	if(!e) {
		fprintf(stderr, "rcgpppkill::escribir_template(): write error!\n");
		return;
	}

	e = fprintf(fp, "\n"
							"# Show which components?\n"
							"ViewGraph = TRUE\n"
							"ViewStats = TRUE\n"
							"ViewTime = TRUE\n"
							"ViewInfo = TRUE\n"
							"ViewStatus = TRUE\n");
	if(!e) {
		fprintf(stderr, "rcgpppkill::escribir_template(): write error!\n");
		return;
	}

	e = fprintf(fp, "\n"
							"# Preferences:\n"
							"\n"
							"# idletime Option.\n"
							"# start idletime option on startup?\n"
							"idletime_on = FALSE\n"
							"# idletime in seconds.\n"
							"idletime = %d\n"
							"\n"
							"# onlinetime Option.\n"
							"# start onlinetime option on startup?\n"
							"onlinetime_on = FALSE\n"
							"# onlinetime in seconds.\n"
							"onlinetime = %d\n"
							"\n"
							"# If the ppp link recive less than 'byte_min_in', then link is idle.\n"
							"byte_min_in = %d\n"
							"\n"
							"# If the ppp link send less than 'byte_min_out', then link is idle.\n"
							"byte_min_out = %d\n"
							"\n"
							"# Prefered startup interface:\n"
							"pppInterface = " DEF_PPPINTERFACE "\n"
							"\n"
							"# Warning time:\n"
							"# warn when an option is trying to kill the pppd?\n"
							"option_warn = TRUE\n"
							"# how many seconds do you want to see the warning window?\n"
							"warntime = %d\n"
							"# should beep when the warning window is active?\n"
							"warn_beep = TRUE\n",
							DEF_IDLETIME, DEF_ONLINETIME, DEF_BYTE_MIN, DEF_BYTE_MIN, DEF_WARNTIME);

	if(!e) {
		fprintf(stderr, "rcgpppkill::escribir_template(): write error!\n");
		return;
	}

	fclose(fp);
}

/*------------------------------------------------------------------------------
 *	Graba las preferencias del rc file anterior, luego sobreescribe el
 *	rc file, y luego guarda las preferencias viejas en el nuevo archivo.
 *	Se debe sobreescribir el archivo, por si aparecen o desaparecen opciones.
 */
void rcgpppkill::update_template(void)
{
	//preferencias que conosco en esta version
	int xpos_tmp,
			ypos_tmp,
			graph_tmp,
			stats_tmp,
			time_tmp,
			info_tmp,
			status_tmp,
			idletime_on_tmp,
			idletime_tmp,
			onlinetime_on_tmp,
			byte_in_tmp,
			byte_out_tmp,
			warntime_on_tmp,
			warntime_tmp,
			warnbeep_tmp;
	char ppp_if_tmp[10];
	long onlinetime_tmp;

	// guardar
	xpos_tmp = load_xPos();
	ypos_tmp = load_yPos();
	graph_tmp = load_ViewGraph();
	stats_tmp = load_ViewStats();
	time_tmp = load_ViewTime();
	info_tmp = load_ViewInfo();
	status_tmp = load_ViewStatus();
	idletime_on_tmp = load_idletime_on();
	idletime_tmp = load_idletime();
	onlinetime_on_tmp = load_onlinetime_on();
	onlinetime_tmp = load_onlinetime();
	byte_in_tmp = load_byte_min_in();
	byte_out_tmp = load_byte_min_out();

	strcpy(ppp_if_tmp, load_pppInterface());

	warntime_on_tmp = load_option_warn();
	warntime_tmp = load_warntime();
	warnbeep_tmp = load_warn_beep();

	// overwrite rc file
	escribir_template();

	//cout << "update_template()" << endl;

	// save saved options
	save_xPos(xpos_tmp);
	save_yPos(ypos_tmp);
	save_ViewGraph(graph_tmp);
	save_ViewStats(stats_tmp);
	save_ViewTime(time_tmp);
	save_ViewInfo(info_tmp);
	save_ViewStatus(status_tmp);
	save_idletime_on(idletime_on_tmp);
	save_idletime(idletime_tmp);
	save_onlinetime_on(onlinetime_on_tmp);
	save_onlinetime(onlinetime_tmp);
	save_byte_min_in(byte_in_tmp);
	save_byte_min_out(byte_out_tmp);

	save_pppInterface(ppp_if_tmp);

	save_option_warn(warntime_on_tmp);
	save_warntime(warntime_tmp);
	save_warn_beep(warnbeep_tmp);
}

/*------------------------------------------------------------------------------
 *	Revisa la version de rcfile y setea la variable rcgpppkill::update_info
 *		de acuerdo a la accion tomada aqui.
 *	Luego en about_version se lee esta variable y se informa al usuario.
 *	rcgpppkill :: about_version() se llama desde 
 *		gpppkill_configure_event_callback()
 *		
 *	update_info
 *	 1 -> fresh install
 *	 2 -> upgrade
 *	 3 -> downgrade
 */
void rcgpppkill::check_version(void)
{
	char buffer[120];
	char ver_new[]={ VERSION };
	int  aold=0, anew=0,
       bold=0, bnew=0,
       cold=0, cnew=0;

	if(load_version(ver_old))	{	//si no encuentro la opcion VERSION
		escribir_template();	//sobreescribir el rcfile actual
		//fresh install
		update_info = 1;
	}
	else {
		if(strcmp(VERSION, ver_old)) {	//si son distintas
			sscanf(ver_old, "%d.%d.%d", &aold, &bold, &cold);
			sscanf(ver_new, "%d.%d.%d", &anew, &bnew, &cnew);

			//cout << "old:" << aold << bold << cold << endl;
			//cout << "new:" << anew << bnew << cnew << endl;

			//	revisar si hay un upgrade o downgrade
			//	Alfred's code
			if      ( anew > aold ) update_info = 2;	//upgrade
			else if ( anew < aold ) update_info = 3;	//downgrade
			else if ( bnew > bold ) update_info = 2;
			else if ( bnew < bold ) update_info = 3;
			else if ( cnew > cold ) update_info = 2;
			else if ( cnew < cold ) update_info = 3;
			else update_info = 0;	//looks like same version

			// always save previous preferences
			update_template();	//update file

			/*
			if(update_info == 2) 
				update_template();	//update file
			else	//downgrade or fresh install
				escribir_template();	//overwrite file
			*/
		}	//if(strcmp(VERSION, ver_old))
	}	//if(load_version(ver_old))
	//cout << "Version : " << ver_old << " -> " << ver_new << endl;
}

/*------------------------------------------------------------------------------
 *	Imprime un messagebox si es que hubo cambio de version.
 */
void rcgpppkill::about_version(void)
{
	char buffer[120];

	switch(update_info) {
		case 1:
			update_info = 0;
			sprintf(buffer, "You just installed:\n"
											 PROGRAMA " " VERSION " !\n"
											 "Thanks for using " PROGRAMA );
			gpppk->message_box_window(buffer);
		break;
		case 2:
			update_info = 0;
			sprintf(buffer, "You just upgraded from version:\n"
											"%s\n"
											"Thanks for using " PROGRAMA " !\n\n"
											"Old preferences saved", ver_old);
			gpppk->message_box_window(buffer);
		break;
		case 3:
			update_info = 0;
			sprintf(buffer, "You have installed an older \n" 
											"version of " PROGRAMA "\n\n"
											"previous: %s\n"
											"current : " VERSION "\n\n"
											"Old preferences saved", ver_old );
			gpppk->message_box_window(buffer);
		break;
	}
}

/*------------------------------------------------------------------------------
 *	version == a.b.c  Ej. : 0.9.3
 *	parameter:
 *		char *ver: aqui se guarda la cadena de la version
 *	return:
 *		0 -> ok
 *		1 -> error: no se puede leer la opcion VERSION
 */
int rcgpppkill::load_version(char *ver)
{
	strcpy(ver, load_string_option("VERSION"));

	if(ver[0] == '#') {
		cout << "rcgpppkill::load_version(): no existe la entrada VERSION en ~/.gpppkillrc" << endl;
		return 1;
	}

	return 0;
}

/*------------------------------------------------------------------------------
 *	Devuelve el value de la option xPos.
 *	Si hay un error, setea xPos a su valor por defecto, 
 *		y develve este valor.
 */
int rcgpppkill::load_xPos(void)
{
	char str[20];
	int res;
	
	strcpy(str, load_string_option("xPos"));

	if(str[0] == '#') {
		save_xPos(DEF_X_POS);
		res = DEF_X_POS;
	}
	else 
		res = atoi(str);

	return res;
}

/*------------------------------------------------------------------------------
 *	Devuelve el value de la option yPos.
 *	Si hay un error, setea yPos a su valor por defecto, 
 *		y develve este valor.
 */
int rcgpppkill::load_yPos(void)
{
	char str[20];
	int res;
	
	strcpy(str, load_string_option("yPos"));

	if(str[0] == '#') {
		save_xPos(DEF_Y_POS);
		res = DEF_Y_POS;
	}
	else 
		res = atoi(str);

	return res;
}

/*------------------------------------------------------------------------------
 *	Devuelve el value de la option dxWM.
 *	Si hay un error, setea dxWM a su valor por defecto, 
 *		y develve este valor.
 */
int rcgpppkill::load_dxWM(void)
{
	char str[20];
	int res;
	
	strcpy(str, load_string_option("dxWM"));

	if(str[0] == '#') {
		save_xPos(-1);
		res = -1;
	}
	else 
		res = atoi(str);

	return res;
}

/*------------------------------------------------------------------------------
 *	Devuelve el value de la option dyWM.
 *	Si hay un error, setea dyWM a su valor por defecto, 
 *		y develve este valor.
 */
int rcgpppkill::load_dyWM(void)
{
	char str[20];
	int res;
	
	strcpy(str, load_string_option("dyWM"));

	if(str[0] == '#') {
		save_xPos(-1);
		res = -1;
	}
	else 
		res = atoi(str);

	return res;
}

//------------------------------------------------------------------------------
int rcgpppkill::load_ViewGraph(void)
{
	int res;

	res = load_boolean_option("ViewGraph");
	
	if(res < 0) {
		save_ViewGraph(1); //guardar valor por defecto
		return 1;
	}
	return res;
}

//------------------------------------------------------------------------------
int rcgpppkill::load_ViewStats(void)
{
	int res;
	
	res = load_boolean_option("ViewStats");

	if(res < 0) {
		save_ViewStats(1); //guardar valor por defecto
		return 1;
	}
	return res;
}

//------------------------------------------------------------------------------
int rcgpppkill::load_ViewTime(void)
{
	int res;
	
	res = load_boolean_option("ViewTime");

	if(res < 0) {
		save_ViewTime(1); //guardar valor por defecto
		return 1;
	}
	return res;
}

//------------------------------------------------------------------------------
int rcgpppkill::load_ViewInfo(void)
{
	int res;
	
	res = load_boolean_option("ViewInfo");

	if(res < 0) {
		save_ViewInfo(1); //guardar valor por defecto
		return 1;
	}
	return res;
}

//------------------------------------------------------------------------------
int rcgpppkill::load_ViewStatus(void)
{
	int res;
	
	res = load_boolean_option("ViewStatus");

	if(res < 0) {
		save_ViewStatus(1); //guardar valor por defecto
		return 1;
	}
	return res;
}

/*------------------------------------------------------------------------------
 *	return:
 *		0: idletime_on = FALSE
 *		1: idletime_on = TRUE
 *	Si la opcion no se encuentra en el rcfile, graba idletime_on = FALSE 
 *		y retorna ese valor(0)
 */
int rcgpppkill::load_idletime_on(void)
{
	int res;

	res = load_boolean_option("idletime_on");

	if(res < 0) {
		save_idletime_on(0); //guardar valor por defecto == FALSE
		return 0;
	}
	return res;
}

/*------------------------------------------------------------------------------
 *	Devuelve el value de la option idletime.
 *	Si hay un error, setea idletime a su valor por defecto, 
 *		y develve este valor.
 */
int rcgpppkill::load_idletime(void)
{
	char str[20];
	int res;
	
	strcpy(str, load_string_option("idletime"));

	if(str[0] == '#') {
		save_idletime(DEF_IDLETIME);
		res = DEF_IDLETIME;
	}
	else 
		res = atoi(str);

	return res;
}

/*------------------------------------------------------------------------------
 *	return:
 *		0: onlinetime_on = FALSE
 *		1: onlinetime_on = TRUE
 *	Si la opcion no se encuentra en el rcfile, graba onlinetime_on = FALSE 
 *		y retorna ese valor(0)
 */
int rcgpppkill::load_onlinetime_on(void)
{
	int res;

	res = load_boolean_option("onlinetime_on");

	if(res < 0) {
		save_onlinetime_on(0); //guardar valor por defecto
		return 0;
	}
	return res;
}

/*------------------------------------------------------------------------------
 *	Devuelve el value de la option onlinetime.
 *	Si hay un error, setea onlinetime a su valor por defecto, 
 *		y develve este valor.
 */
int rcgpppkill::load_onlinetime(void)
{
	char str[20];
	int res;
	
	strcpy(str, load_string_option("onlinetime"));

	if(str[0] == '#') {
		save_onlinetime(DEF_ONLINETIME);
		res = DEF_ONLINETIME;
	}
	else 
		res = atoi(str);

	return res;
}

/*------------------------------------------------------------------------------
 *	Devuelve el value de la option byte_min_in.
 *	Si hay un error, setea byte_min_in a su valor por defecto, 
 *		y develve este valor.
 */
int rcgpppkill::load_byte_min_in(void)
{
	char str[20];
	int res;
	
	strcpy(str, load_string_option("byte_min_in"));

	if(str[0] == '#') {
		save_byte_min_in(DEF_BYTE_MIN);
		res = DEF_BYTE_MIN;
	}
	else 
		res = atoi(str);

	return res;
}

/*------------------------------------------------------------------------------
 *	Devuelve el value de la option byte_min_out.
 *	Si hay un error, setea byte_min_in a su valor por defecto, 
 *		y develve este valor.
 */
int rcgpppkill::load_byte_min_out(void)
{
	char str[20];
	int res;
	
	strcpy(str, load_string_option("byte_min_out"));

	if(str[0] == '#') {
		save_byte_min_out(DEF_BYTE_MIN);
		res = DEF_BYTE_MIN;
	}
	else 
		res = atoi(str);

	return res;
}

/*------------------------------------------------------------------------------
 *	Devuelve el value de la option pppInterface.
 *	Si hay un error, setea pppInterface a su valor por defecto, 
 *		y develve este valor.
 */
char *rcgpppkill::load_pppInterface(void)
{
	static char str[20];
	
	strcpy(str, load_string_option("pppInterface"));

	if(str[0] == '#') {
		save_pppInterface(DEF_PPPINTERFACE);
		strcpy(str, DEF_PPPINTERFACE);
	}

	return str;
}

//------------------------------------------------------------------------------
int rcgpppkill::load_option_warn(void)
{
	int res;
	
	res = load_boolean_option("option_warn");

	if(res < 0) {
		save_option_warn(1); //guardar valor por defecto
		return 1;
	}
	return res;
}

/*------------------------------------------------------------------------------
 *	Devuelve el value de la option warntime.
 *	Si hay un error, setea warntime a su valor por defecto, 
 *		y develve este valor.
 */
int rcgpppkill::load_warntime(void)
{
	char str[20];
	int res;
	
	strcpy(str, load_string_option("warntime"));

	if(str[0] == '#') {
		save_byte_min_out(DEF_WARNTIME);
		res = DEF_WARNTIME;
	}
	else 
		res = atoi(str);

	return res;
}

//------------------------------------------------------------------------------
int rcgpppkill::load_warn_beep(void)
{
	int res;
	
	res = load_boolean_option("warn_beep");

	if(res < 0) {
		save_warn_beep(1); //guardar valor por defecto
		return 1;
	}
	return res;
}

//----------------------- save options -----------------------------------------

/*------------------------------------------------------------------------------
 *	parameter:
 *		char *ver: esta es la cadena a guardar como en la opcion VERSION
 *	return:
 *		0 -> ok
 *		1 -> error: no se puede leer la opcion VERSION
 */
int rcgpppkill::save_version(char *ver)
{
	return save_option("VERSION", ver);
}

/*------------------------------------------------------------------------------
 *	return:
 *		0:ok
 *		1:error --> error de archivo
 *		2:error --> parametro invalido
 */
int rcgpppkill::save_xPos(int x)
{
	char str[20];

	if(x >= 0) {
		sprintf(str, "%d", x);
		return save_option("xPos", str);
	}
	else
		return 2;
}

/*------------------------------------------------------------------------------
 *	return:
 *		0:ok
 *		1:error --> error de archivo
 *		2:error --> parametro invalido
 */
int rcgpppkill::save_yPos(int y)
{
	char str[20];

	if(y >= 0) {
		sprintf(str, "%d", y);
		return save_option("yPos", str);
	}
	else
		return 2;
}

/*------------------------------------------------------------------------------
 *	return:
 *		0:ok
 *		1:error --> error de archivo
 *		2:error --> parametro invalido
 */
int rcgpppkill::save_dxWM(int dx)
{
	char str[20];

	if(dx >= 0) {
		sprintf(str, "%d", dx);
		return save_option("dxWM", str);
	}
	else
		return 2;
}

/*------------------------------------------------------------------------------
 *	return:
 *		0:ok
 *		1:error --> error de archivo
 *		2:error --> parametro invalido
 */
int rcgpppkill::save_dyWM(int dy)
{
	char str[20];

	if(dy >= 0) {
		sprintf(str, "%d", dy);
		return save_option("dyWM", str);
	}
	else
		return 2;
}

/*------------------------------------------------------------------------------
 *	return:
 *		0:ok
 *		1:error --> error de archivo
 *		2:error --> parametro invalido
 */
int rcgpppkill::save_ViewGraph(int b)
{
	switch(b) {
		case 0:
			return save_option("ViewGraph", "FALSE");
		break;
		case 1:
			return save_option("ViewGraph", "TRUE");
		break;
	}
	return 2;
}

/*------------------------------------------------------------------------------
 *	return:
 *		0:ok
 *		1:error --> error de archivo
 *		2:error --> parametro invalido
 */
int rcgpppkill::save_ViewStats(int b)
{
	switch(b) {
		case 0:
			return save_option("ViewStats", "FALSE");
		break;
		case 1:
			return save_option("ViewStats", "TRUE");
		break;
	}
	return 2;
}

/*------------------------------------------------------------------------------
 *	return:
 *		0:ok
 *		1:error --> error de archivo
 *		2:error --> parametro invalido
 */
int rcgpppkill::save_ViewTime(int b)
{
	switch(b) {
		case 0:
			return save_option("ViewTime", "FALSE");
		break;
		case 1:
			return save_option("ViewTime", "TRUE");
		break;
	}
	return 2;
}

/*------------------------------------------------------------------------------
 *	return:
 *		0:ok
 *		1:error --> error de archivo
 *		2:error --> parametro invalido
 */
int rcgpppkill::save_ViewInfo(int b)
{
	switch(b) {
		case 0:
			return save_option("ViewInfo", "FALSE");
		break;
		case 1:
			return save_option("ViewInfo", "TRUE");
		break;
	}
	return 2;
}

/*------------------------------------------------------------------------------
 *	return:
 *		0:ok
 *		1:error --> error de archivo
 *		2:error --> parametro invalido
 */
int rcgpppkill::save_ViewStatus(int b)
{
	switch(b) {
		case 0:
			return save_option("ViewStatus", "FALSE");
		break;
		case 1:
			return save_option("ViewStatus", "TRUE");
		break;
	}
	return 2;
}

/*------------------------------------------------------------------------------
 *	return:
 *		0:ok
 *		1:error --> error de archivo
 *		2:error --> parametro invalido
 */
int rcgpppkill::save_idletime_on(int b)
{
	switch(b) {
		case 0:
			return save_option("idletime_on", "FALSE");
		break;
		case 1:
			return save_option("idletime_on", "TRUE");
		break;
	}
	return 2;
}

/*------------------------------------------------------------------------------
 *	return:
 *		0:ok
 *		1:error --> error de archivo
 *		2:error --> parametro invalido
 */
int rcgpppkill::save_idletime(int time)
{
	char str[20];

	if(time >= 0) {
		sprintf(str, "%d", time);
		return save_option("idletime", str);
	}
	else
		return 2;
}

/*------------------------------------------------------------------------------
 *	return:
 *		0:ok
 *		1:error --> error de archivo
 *		2:error --> parametro invalido
 */
int rcgpppkill::save_onlinetime_on(int b)
{
	switch(b) {
		case 0:
			return save_option("onlinetime_on", "FALSE");
		break;
		case 1:
			return save_option("onlinetime_on", "TRUE");
		break;
	}
	return 2;
}

/*------------------------------------------------------------------------------
 *	return:
 *		0:ok
 *		1:error --> error de archivo
 *		2:error --> parametro invalido
 */
int rcgpppkill::save_onlinetime(int time)
{
	char str[20];

	if(time >= 0) {
		sprintf(str, "%d", time);
		return save_option("onlinetime", str);
	}
	else
		return 2;
}

/*------------------------------------------------------------------------------
 *	return:
 *		0:ok
 *		1:error --> error de archivo
 *		2:error --> parametro invalido
 */
int rcgpppkill::save_byte_min_in(int min)
{
	char str[20];

	if(min >= 0) {
		sprintf(str, "%d", min);
		return save_option("byte_min_in", str);
	}
	else
		return 2;
}

/*------------------------------------------------------------------------------
 *	return:
 *		0:ok
 *		1:error --> error de archivo
 *		2:error --> parametro invalido
 */
int rcgpppkill::save_byte_min_out(int min)
{
	char str[20];

	if(min >= 0) {
		sprintf(str, "%d", min);
		return save_option("byte_min_out", str);
	}
	else
		return 2;
}

/*------------------------------------------------------------------------------
 *	return:
 *		0:ok
 *		1:error --> error de archivo
 *		2:error --> parametro invalido
 */
int rcgpppkill::save_pppInterface(char *ppp)
{
	int num;

	if(sscanf(ppp, "ppp%d", &num) == 1) 
		num = save_option("pppInterface", ppp);
	else
		num = 2;

	return num;
}

/*------------------------------------------------------------------------------
 *	return:
 *		0:ok
 *		1:error --> error de archivo
 *		2:error --> parametro invalido
 */
int rcgpppkill::save_option_warn(int b)
{
	switch(b) {
		case 0:
			return save_option("option_warn", "FALSE");
		break;
		case 1:
			return save_option("option_warn", "TRUE");
		break;
	}
	return 2;
}

/*------------------------------------------------------------------------------
 *	return:
 *		0:ok
 *		1:error --> error de archivo
 *		2:error --> parametro invalido
 */
int rcgpppkill::save_warntime(int min)
{
	char str[20];

	if(min >= 0) {
		sprintf(str, "%d", min);
		return save_option("warntime", str);
	}
	else
		return 2;
}

/*------------------------------------------------------------------------------
 *	return:
 *		0:ok
 *		1:error --> error de archivo
 *		2:error --> parametro invalido
 */
int rcgpppkill::save_warn_beep(int b)
{
	switch(b) {
		case 0:
			return save_option("warn_beep", "FALSE");
		break;
		case 1:
			return save_option("warn_beep", "TRUE");
		break;
	}
	return 2;
}

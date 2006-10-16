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
 *	Verificar los errores de las funciones save_*() en la funciones load_*().
 */
#include "rcfile.h"


//------------------------------------------------------------------------------
rcfile::rcfile(void)
{
	strcpy(path, "");
}

/*------------------------------------------------------------------------------
 *	Graba un valor para una opcion.
 *	Respeta el orden en el cual estan las opcion. 
 *	Cambia solo el value de la opcion option.
 *	Flujo:
 *		Se copia el archivo fp al archivo tmp. En el archivo tmp se copia la 
 *		opcion con el nuevo valor. Se borra e archivo fp. Se copia el archivo 
 *		tmp al archivo fp. Se borra el archivo tmp.
 *	Si la opcion no se encuentra, la opcion se graba al final del archivo.
 *	return:
 *		0:ok
 *		1:error --> error de archivo, se imprime error en stderr.
 *	variables:
 *		linea: el numero de linea en donde se encuentra la opcion a reeemplazar.
 *					 La primera linea es la linea==0;
 *		fp   : puntero el archivo rcfile::path
 *		tmp  : puntero a un archivo temporal.
 *		er   : int, para chekear error en fwrite().
 *		cer  :(char *) para chekear error en fgets().
 */
int rcfile::save_option(char *option, char *value)
{
	char str[81], opt[40], *cer;
	int linea, i, er;
	FILE *fp, *tmp;
	
	// --- buscar la linea en donde esta la opcion -----------
	fp = fopen(path, "r");
	if(!fp) {
		perror("rcfile::save_option()");
		return 1;
	}

	linea = 0;	//la primera linea es la linea==0
	while(!feof(fp)) {
		cer = fgets(str, 80, fp);
		if((cer == NULL) && !feof(fp)) {
			perror("rcfile::save_option()");
			fclose(fp);
			return 1;
		}
		if(str[0] != '#') {	//si no es un comentario
			if(sscanf(str, "%s", opt) > 0) {	//si no hubo error(str=="\n")
				if(!strcmp(opt, option)) 	//si encuentro la opcion
					break;
			}
		}
		linea++;
	};

	fclose(fp);

	// --- copiar fp a tmp. Copiar a tmp la opcion cambiada -------------
	fp = fopen(path, "r");
	if(!fp) {
		perror("rcfile::save_option()");
		return 1;
	}

	tmp = tmpfile();
	if(!tmp) {
		perror("rcfile::save_option()");
		fclose(fp);
		return 1;
	}

	//ir hasta una linea antes
	for(i=0; i<linea; i++) {	
		cer = fgets(str, 80, fp);
		if((cer == NULL) && !feof(fp)) {
			perror("rcfile::save_option()");
			fclose(fp);
			fclose(tmp);
			return 1;
		}
		if(!feof(fp))	{	//por si no encuentro la opcion
			er = fwrite(str, sizeof(char), strlen(str), tmp);
			if((er == (int)NULL) && !feof(tmp) ) {
				perror("rcfile::save_option()");
				fclose(fp);
				fclose(tmp);
				return 1;
			}
		}
	}

	//construir: "option = value"
	strcpy(str, option);
	strcat(str, " = ");
	strcat(str, value);
	strcat(str, "\n");

	//escribir la nueva opcion cambiada en el archivo tmp.
	//Si no encontre la opcion, La opcion se graba al final.
	er = fwrite(str, sizeof(char), strlen(str), tmp);
	if((er == (int)NULL) && !feof(tmp) ) {
		perror("rcfile::save_option()");
		fclose(fp);
		fclose(tmp);
		return 1;
	}

	//saltar la linea que fue cambiada
	cer = fgets(str, 80, fp);	
	if((cer == NULL) && !feof(fp)) {
		perror("rcfile::save_option()");
		fclose(fp);
		fclose(tmp);
		return 1;
	}

	//seguir copiando el resto
	while(!feof(fp)) {	
		cer = fgets(str, 80, fp);
		if((cer == NULL) && !feof(fp)) {
			perror("rcfile::save_option()");
			fclose(fp);
			fclose(tmp);
			return 1;
		}
		//fgets() para en "\n" o en EOF.
		//Si la cond del if es falsa hubo un "\n" antes del EOF.
		//Osea, en el ciclo anterior se leyo la ultima linea, pero no el EOF.
		if(!feof(fp))	{
			er = fwrite(str, sizeof(char), strlen(str), tmp);
			if((er == (int)NULL) && !feof(tmp) ) {
				perror("rcfile::save_option()");
				fclose(fp);
				fclose(tmp);
				return 1;
			}
		}
	};
	
	fclose(fp);

	// --- copiar el archivo tmp al archivo fp(vacio). Luego borrar tmp -------
	fp = fopen(path, "w");
	if(!fp) {
		perror("rcfile::save_option()");
		exit(1);
	}
	rewind(tmp);

	while(!feof(tmp)) {
		cer = fgets(str, 80, tmp);
		if((cer == NULL) && !feof(tmp)) {
			perror("rcfile::save_option()");
			fclose(fp);
			fclose(tmp);
			return 1;
		}
		if(!feof(tmp)) {
			er = fwrite(str, sizeof(char), strlen(str), fp);
			if((er == (int)NULL) && !feof(fp) ) {
				perror("rcfile::save_option()");
				fclose(fp);
				fclose(tmp);
				return 1;
			}
		}
	};

	fclose(fp);
	fclose(tmp);	//aqui se borra automaticamente el archivo tmp

	return 0;
}

/*------------------------------------------------------------------------------
 *	Devuelve el value de option en forma de (char *).
 *	return:
 *		(char *):ok
 *		"#\0"     :error --> no se encuentra la opcion o la opcion no tiene value.
 */
char *rcfile::load_string_option(char *option)
{
	char str[81], value[40];
	static char opt[40];
	FILE *fp;
	
	fp = fopen(path, "r");
	if(!fp) {
		perror("rcfile::load_string_option()");
		exit(1);
	}

	while(!feof(fp)) {
		fgets(str, 80, fp);
		if(str[0] != '#') {	//si no es un comentario
			if(sscanf(str, "%s", opt) > 0) {	//si no hubo error(str=="\n")
				if(!strcmp(opt, option)) {	//si encuentro la opcion
					if(sscanf(str, "%s = %s", opt, value) == 2) {//si hay value
						strcpy(opt, value);	//encontre la opcion
						break;	//salir para que no se ejecute: strcpy(opt, "#")
					}
				}	
			}
		}
		strcpy(opt, "#");	//no encontre la opcion.
	};

	fclose(fp);
	return opt;
}

/*------------------------------------------------------------------------------
 *	devuelve el valor de la opcion de tipo boolean.
 *	Ej.: ViewGraph = TRUE/FALSE
 *	return:
 *		1:ok    --> TRUE
 *		0:ok    --> FALSE
 *	 -1:error --> no se encuentra la opcion
 *	 -2:error --> value de opcion no valido
 *	 -3:error --> no existe value para la opcion
 */
int rcfile::load_boolean_option(char *option)
{
	char str[81], opt[40], value[40];
	int res;
	FILE *fp;
	
	fp = fopen(path, "r");
	if(!fp) {
		perror("rcfile::load_boolean_option()");
		exit(1);
	}

	res = -1;
	while(!feof(fp)) {
		fgets(str, 80, fp);
		if(str[0] != '#') {	//si no es un comentario
			if(sscanf(str, "%s", opt) > 0) {	//si no hubo error(str=="\n")
				if(!strcmp(opt, option)) {	//si encuentro la opcion
					if(sscanf(str, "%s = %s", opt, value) == 2) {//si hay value
						if(!strcmp(value, "TRUE")) {
							res = 1;
							break;
						}
						else {
							if(!strcmp(value, "FALSE"))
								res = 0;
							else {	//value no reconocido
								res = -2;
								break;
							}
						}
					}
					else {	//no hay value para la opcion
						res = -3;
						break;
					}
				}		
			}
		}
	};

	fclose(fp);
	return res;
}

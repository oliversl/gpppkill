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
#ifndef _LISTAC_OSL_HPP
#define _LISTAC_OSL_HPP

#include <iostream.h>
#include <stdio.h>

/*
 *	Lista continua de longitud fija
 */

class nodoc {
	public:
		nodoc();
		class nodoc *sigte;
		int		valor;
};

class listac {
	private:
		int longitud;
		class nodoc *inicio,
								*final;

		void borrar();
	public:
		listac(int size);
		~listac();

		void vaciar();
		int esta_llena();
		int esta_vacia();
		void borrar_al_inicio(void);
		void agregar_al_final(int val);
		int maximo(void);
		int maximo_saltando(int saltar);
		int usados(void);
		int sumatoria(void);
		int capacidad(void);
		
		nodoc *getinicio(void);
		nodoc *getfinal(void);
		int getlongitud(void);
		
		void imprimir(void);
};

#endif


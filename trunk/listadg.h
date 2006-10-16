/*
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
 *  A copy of the GNU General Public License is included with this program.
 */

/* ----------------------
 *	Lista doble generica.
 *	---------------------
 */
#ifndef _LISTADG_OSL_H_
#define _LISTADG_OSL_H_

#include <iostream.h>
#include <stdio.h>
#include <string.h>

#define DEBUG_LISTADG 0

/*
 *	class nododg
 *	Nodo para Lista Doblemente ligada Generica.
 */
template <class T> class nododg {
	public:
		nododg();
		~nododg();

		T data;
		
		nododg<T> *sigte,
	            *anterior;

	  nododg<T> operator=(nododg<T> n);
} ;

/*
 * class listadg
 * Lista doblemente ligada generica.
 */ 
template <class T> class listadg : public nododg<T> {
	protected:
		nododg<T> *inicio, *final;
		void agregar_nodo(nododg<T> *p);
		void borrar_nodo(nododg<T> *p);
	public:
		listadg();
		~listadg();

		void agregar(T nuevo);
		void borrar_listadg();
		int  get_numero_de_elementos(void);
		//nododg<T> *get_nododg(nododg<T> n1);

		nododg<T> *get_inicio();
		nododg<T> *get_final();
} ;

#endif


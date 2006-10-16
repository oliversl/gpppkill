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
 *	main.cc
 */
#include "main.h"
#include "gpppkill.h"


//##############################################################################
int main (int argc, char *argv[])
{
	gpppkill gpppk;

	if(gpppk.gpppkill_init(argc, argv))
		return 0;

	gtk_init (&argc, &argv);

	gpppk.leer_rc();

	gpppk.crear_ventana();

	gpppk.cargar_ventana();

	gpppk.mostrar_ventana();

	gtk_main ();	//ahora, gtk tiene el control. I got da power, he'i gtk.
  
  return 0;
}

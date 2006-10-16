/*
 *  gpppkill. X11/GTK+ program that kill pppd if it not recive a minimal amount 
 *  of bytes during certain time. It also plot the amount bytes/seg recived.
 *  Copyright (C) 1998-1999  Oliver Schulze L.
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
#ifndef _GPPPKILL_CONFIG_OSL_H_
#define _GPPPKILL_CONFIG_OSL_H_

// Aqui va todo lo modificable en gpppkill
// Los parametros de clases que no son de gpppkill NO van aqui!

#define PROGRAMA	"gpppkill"
#define VERSION		"1.0.3"
#define FECHA     "10-Set-2000"
#define COPYLEFT  "Copyright (C) 1998-1999-2000  Oliver Schulze L. <oliver@pla.net.py>"
// [1.0.3] go ISS go (http://spaceflight.nasa.gov/station)
#define RELEASE "STS-106" 
// [1.0.2] in the memory of MCO. http://mars.jpl.nasa.gov/msp98/orbiter/
//#define RELEASE "MarsClimateOrbiter" 
// [1.0.1] Cassini is passing near the earth, destiny: saturn. http://www.jpl.nasa.gov/cassini/
//#define RELEASE "Cassini"


// gpppkill.h --------------------------------------------------------
//usado para el ancho del borde de un container.(i.e. gpppkill::window)
#define CONTAINER_ANCHO 1

// deprecated
#define	DRAW_ANCHO_OLD				200
#define	DRAW_ALTO_OLD					100

#define DEF_LINEA_ANCHO	    2	//cabiado por gpppkill::dibujar_frame_graf()::linea_ancho
#define INTERVALO				 1000
#define INTERVALO_QUICKMESSAGE 1100
#define KILL_INTERVALO				 1000
#define KILLTIME_OFF_STR       "--:--:--"
#define SBAR_INTERVALO         1500

//labels
#define DEF_LABEL_IN_OUT          "00000"
#define DEF_LABEL_PROMEDIO_IN_OUT "00.000"
#define DEF_LABEL_TOTAL_IN_OUT    "000000000"
#define DEF_PATTERN_TOTAL_IN_OUT  "_________"
#define DEF_LABEL_INTERFACE       "ppp--"
#define DEF_LABEL_PID             "-----"

// warning time
#define DEF_WARNTIME 15

//debug messages
#define GPPPKILL_DEBUG 0

// statusbar
#define SBAR_CONTEXT_STATIC  1
#define SBAR_CONTEXT_TIMEOUT 2
#define SBAR_STATIC     1
#define SBAR_POP_STATIC 2
#define SBAR_POP        3
#define SBAR_TIMEOUT    4
#define BUSCANDO_PPPD "Searching for pppds ..."

// preferences.h --------------------------------------------------------
#define SCROLL_ANCHO 240
#define SCROLL_ALTO  100
#define CLIST_INTERFACE_COLUMNS 5
#define STR_NO_PPPD   "--"
#define STR_PPPD_UP   "up"
#define STR_PPPD_DOWN "down"
#define BOX_PADDING   10

// rcgpppkill.h --------------------------------------------------------
#define RCFILE_GPPPKILL     "gpppkillrc"
#define RCFILE_GPPPKILL_NEW "gpppkillrc.new"
#define DEF_X_POS 10
#define DEF_Y_POS 10
#define DEF_STR_X_POS "10"
#define DEF_STR_Y_POS "10"

// warning.h -----------------------------------------------------------
#define WARNING_INCREMENTO    0.01
#define WARNING_PBAR_LARGO  200
#define WARNING_PBAR_ANCHO   20
#define WARNING_PBAR_PASOS  100
#define WARN_H_PADDING       10
#define WARN_V_PADDING        2

// graf.h --------------------------------------------------------------
#define FACTOR_ESCALA_MINIMA  10
#define FACTOR_ESCALA_MAXIMA 100

#endif


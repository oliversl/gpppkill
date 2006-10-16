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
#ifndef _GPPPKILLRC_HPP_OSL_
#define _GPPPKILLRC_HPP_OSL_

#include <iostream.h>
//para getuid() y getpwuid()
#include <unistd.h>
#include <pwd.h>
#include <sys/types.h>
#include <sys/stat.h> // para open()
#include <fcntl.h>		// para open()
#include "gpppkill_config.h"	//para define VERSION
#include "rcfile.h"
#include "gpppkill.h"


class rcgpppkill : public rcfile {
	protected:
		char ver_old[20];
		int update_info;

		class gpppkill *gpppk;
		
		void escribir_template(void);
		void update_template(void);
	public:
		rcgpppkill(class gpppkill *g);
		rcgpppkill(char *rc);

		void check_version(void);
		void about_version(void);

		int load_version(char *ver);
		int load_xPos();
		int load_yPos();
		int load_dxWM();
		int load_dyWM();
		int load_ViewGraph();
		int load_ViewStats();
		int load_ViewTime();
		int load_ViewInfo();
		int load_ViewStatus();
		int load_idletime_on();
		int load_idletime();
		int load_onlinetime_on();
		int load_onlinetime();
		int load_byte_min_in(void);
		int load_byte_min_out(void);
	char *load_pppInterface(void);
		int load_option_warn(void);
		int load_warntime(void);
		int load_warn_beep(void);

		int save_version(char *ver);
		int save_xPos(int x);
		int save_yPos(int y);
		int save_dxWM(int dx);
		int save_dyWM(int dy);
		int save_ViewGraph(int b);
		int save_ViewStats(int b);
		int save_ViewTime(int b);
		int save_ViewInfo(int b);
		int save_ViewStatus(int b);
		int save_idletime_on(int b);
		int save_idletime(int time);
		int save_onlinetime_on(int b);
		int save_onlinetime(int time);
		int save_byte_min_in(int min);
		int save_byte_min_out(int min);
		int save_pppInterface(char *ppp);
		int save_option_warn(int b);
		int save_warntime(int min);
		int save_warn_beep(int b);
};

#endif


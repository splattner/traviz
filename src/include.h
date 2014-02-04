/*
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software Foundation,
 * Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * This is the Include file for all other files. It contain's all standard librarys and the GTK and PLplot Library
 */


#ifndef INCLUDE_H_
#define INCLUDE_H_

/*
 * Defines
 */

#define DEBUG 0 // Enables debug outputs.

/*
 * Includes
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <math.h>
#include <limits.h>

#include <sys/types.h>
#include <inttypes.h>
#include <float.h>

#include <glib.h>
#include <glib-object.h>
#include <gmodule.h>

#include <gtk/gtk.h>
#include <gdk/gdk.h>
#include <gdk/gdkx.h>

#include <plplot.h>


#include "config.h"


extern GtkWidget	*window; // Main GTK Window
extern GtkBuilder	*builder; // Builder to load and Build GTK Window from Glade File

#endif /* INCLUDE_H_ */

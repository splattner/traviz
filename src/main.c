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
 *
 * This is the "main" File of Tranalyzer-GUI
 * It initialize all GTK related Stuff,
 * loads the Glade File and connects all GTK Signals
 */

#include "main.h"

#define GLADE_FILE "/usr/local/share/traviz/gui.glade"


GtkWidget	*window; // Main GTK Window
GtkBuilder	*builder; // Builder to load and Build GTK Window from Glade File

/*
 * Main Function, the Tranalyzer-GUI start's here
 */
int main (int argc, char *argv[]) {


		// Check if Glade file is available. If not, we canot run traviz!
		FILE * gladefile = fopen (GLADE_FILE,"r");
		if(gladefile == NULL) {
			fprintf(stderr,"Error: Glade File is not available! %s\n", GLADE_FILE);
			return -1;
		} else {
			fclose(gladefile);
		}

		/* Secure glib */
		if( ! g_thread_supported() )
			g_thread_init( NULL );

		/* Secure gtk */
		gdk_threads_init();

		/* Obtain gtk's global lock */
		gdk_threads_enter();


		gtk_init (&argc, &argv);

		// Create new GTK Builder and load glade Ressource File
        builder = gtk_builder_new ();
        gtk_builder_add_from_file (builder, GLADE_FILE, NULL);

        // Create Mainwindows from builder
        window = GTK_WIDGET (gtk_builder_get_object (builder, "mainwindow"));
        gtk_builder_connect_signals (builder, NULL);

        // Load Splash Window and show it
        // Close Splash Window after 1000 ms
        GtkWidget* splash = GTK_WIDGET (gtk_builder_get_object (builder, "splash"));
        gtk_widget_show_all (splash);
        g_timeout_add (1000, close_splash, splash);

        init();

        gtk_main ();

        /* Release gtk's global lock */
        gdk_threads_leave();

        return 0;
}

/*
 * Close the Splash Window and show the main Window
 */
gboolean close_splash(gpointer data){
    GtkWidget * splash = (GtkWidget *) data;
    gtk_widget_hide(splash);
    gtk_widget_show_all (window);
    return(FALSE);
}

/*
 * This function is executed befor the GTK Mainwindows is displayed and befor the GTK Main Loop starts
 */
void init ()
{
	// Init Filter and limits
	feature_filters = linkedlist_new();
	table_filters = linkedlist_new();

	// Init some GUI Stuff
	gui_init();
}

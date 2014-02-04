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
 * You should have received a copy of the GNU General Public Licenseflogging
 * along with this program; if not, write to the Free Software Foundation,
 * Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 *
 * This functions handles all stuff related to plotting the datas.
 */

#include "plot.h"

/*
 * xopts and yopts in plplot_canvas_axes
 * ######################################
 *
 * a: Draws axis, X-axis is horizontal line (y=0), and Y-axis is vertical line (x=0).
 * b: Draws bottom (X) or left (Y) edge of frame.
 * c: Draws top (X) or right (Y) edge of frame.
 * d: Plot labels as date / time. Values are assumed to be seconds since the epoch (as used by gmtime).
 * f: Always use fixed point numeric labels.
 * g: Draws a grid at the major tick interval.
 * h: Draws a grid at the minor tick interval.
 * i: Inverts tick marks, so they are drawn outwards, rather than inwards.
 * l: Labels axis logarithmically. This only affects the labels, not the data, and so it is necessary to compute the logarithms of data points before passing them to any of the drawing routines.
 * m: Writes numeric labels at major tick intervals in the unconventional location (above box for X, right of box for Y)
 * n: Writes numeric labels at major tick intervals in the conventional location (below box for X, left of box for Y).
 * o: Use custom labeling function to generate axis label text. The custom labeling function can be defined with the plslabelfuncplslabelfunc; command.
 * s: Enables subticks between major ticks, only valid if t is also specified.
 * t: Draws major ticks.
 */


/*
 * Some globals to save Data of last Visualization -> used to redraw after a switch has toggled
 */
int saved_col_index = 0;
double saved_data_to_highlight = 0;
int saved_do_highlight = 0;
int saved_is_log_axis = 1; // Use Log Axis or not, default is log axis
int saved_bin_count;
double saved_max_counts;
double saved_per_aggregation;

int saved_flow_plot = FLOW_CHART_TYPE_STATS; // Default is the n First packet stats
int saved_combined_line = 0; // Combine the 2 lines, default is not combined

int saved_reset_bounds = 0; // Set this if in a redraw the bounds needs to be recalculated

// Save bound for 2D and 3D Flow Visualization
double saved_xmin, saved_xmax, saved_ymin, saved_ymax, saved_y2min, saved_y2max;
double saved_xmin_3d, saved_xmax_3d, saved_ymin_3d, saved_ymax_3d, saved_zmin_3d, saved_zmax_3d, saved_iat_with = 0, saved_packet_size = 0;
int view_angle_az = 45;
int view_angle_alt = 25;


double **histo_value_storage;


GtkTreeView * saved_plot_from_treeview; // Save the last used treeview for plotting

/*
 * Private Prototypes
 */

void _plot_multiple_stats(GtkTreeView *treeview);
void _plot_3dbin(double binx, double biny, double height, double width_x, double width_y);
void _plot_3dhisto(GtkTreeView * treeview);
void _plot_histo(int column_index, double dataToHightlight, int doHighlight);
void _plot_pie(int column_index);
char **_separate_feature(int flow_index, int feature_index, int *count);
void _on_sturges_button_clicked(GtkButton *button, gpointer user_data);
void _on_scotts_button_clicked(GtkButton *button, gpointer user_data);
void _init_plplot(Plotarea * plotarea);
void _end_plplot(Plotarea * plotarea);


void _init_plplot(Plotarea *plotarea) {

	// Setup Plplot
	//--------------------------------------------------------------------------------------

	struct {
		Display *display;
		Drawable drawable;
	} xinfo; // Struct to pass the external drawable to cairo

	xinfo.display = GDK_PIXMAP_XDISPLAY(plotarea->plotWindow_pixmap);
	xinfo.drawable = GDK_PIXMAP_XID(plotarea->plotWindow_pixmap);

	// Change the plplot background color to white, and foreground to black
	plscol0(0,255,255,255);
	plscol0(15,0,0,0);

	plsdev("xcairo");
	plsetopt("drvopt","external_drawable"); // Set XCairo to use a external drawable
	plinit();
	pl_cmd(PLESC_DEVINIT, &xinfo); // Set External Drawable


}

void _end_plplot(Plotarea *plotarea) {

	plend();

	// Redraw Pixmap
	//--------------------------------------------------------------------------------------
	gdk_draw_drawable(plotarea->plotWindow->window,
			plotarea->plotWindow->style->fg_gc[GTK_WIDGET_STATE(plotarea->plotWindow)],
			plotarea->plotWindow_pixmap, 0, 0, 0, 0,
			-1, -1);
}


/*
 * Plot the Data according to the chart Type
 */
void plot_col(int active_index, double data_to_highlight, int do_highlight) {

	GtkFixed *optionpane_col = GTK_FIXED(gtk_builder_get_object(builder,"coloptions"));


	// Remove old option panel stuff
	//--------------------------------------------------------------------------------------
	GList *node;
	GList *children = gtk_container_get_children(GTK_CONTAINER(optionpane_col));

	for (node = children; node != NULL; node = node->next) {
		GtkWidget * current_widget = GTK_WIDGET(node->data);
		gtk_container_remove(GTK_CONTAINER(optionpane_col),GTK_WIDGET(current_widget));
	}

	g_list_free(children);
	//--------------------------------------------------------------------------------------


	column_type_t *current_column = (column_type_t*) linkedlist_item_get(header_storage,active_index);

	switch (current_column->chart_type) {
	case COL_CHART_TYPE_HISTO:
		_init_plplot(plotarea_col);
		pladv(0);
		plcol0(0);
		// Plot Histigramm
		_plot_histo(active_index, data_to_highlight,do_highlight);
		// Plot Label
		plcol0(0);
		pllab("Value","Count",current_column->name);
		_end_plplot(plotarea_col);
		break;
	case COL_CHART_TYPE_PIE:
		if (do_highlight == 0) { // We don't need to redraw the Pie Chart
			_init_plplot(plotarea_col);
			pladv(0);
			plcol0(0);
			// Plot Histogramm
			plcol0(15);
			_plot_pie(active_index);
			// Plot Label
			plcol0(15);
			pllab("","",current_column->name);
			_end_plplot(plotarea_col);
		}
		break;
	default:
		// Clear Plot
		_init_plplot(plotarea_col);
		plclear();
		pladv(0);
		_end_plplot(plotarea_col);
		break;
	}

	// Save current values
	//--------------------------------------------------------------------------------------
	saved_col_index = active_index;
	saved_data_to_highlight = data_to_highlight;
	saved_do_highlight = do_highlight;
}



void plot_flow(GtkTreeView * treeview) {
	GtkFixed *optionpane_flow = GTK_FIXED(gtk_builder_get_object(builder,"flowoptions"));

	// Remove old option panel stuff
	//--------------------------------------------------------------------------------------
	GList *node_container;
	GList *children = gtk_container_get_children(GTK_CONTAINER(optionpane_flow));

	for (node_container = children; node_container != NULL; node_container = node_container->next) {
		GtkWidget * current_widget = GTK_WIDGET(node_container->data);
		gtk_container_remove(GTK_CONTAINER(optionpane_flow),GTK_WIDGET(current_widget));
	}

	g_list_free(children);
	//--------------------------------------------------------------------------------------


	// Add a Combobox to select the flow visualizer
	//--------------------------------------------------------------------------------------

	GtkComboBox * flow_plot_combo = GTK_COMBO_BOX(gtk_combo_box_new_text());
	gtk_combo_box_append_text(GTK_COMBO_BOX(flow_plot_combo),"N-first packet statistics");
	gtk_combo_box_append_text(GTK_COMBO_BOX(flow_plot_combo),"Packetsize Inter Arrival Time histogram bins");
	gtk_combo_box_set_active(GTK_COMBO_BOX(flow_plot_combo), saved_flow_plot);

	g_signal_connect (flow_plot_combo, "changed", G_CALLBACK (on_flow_plot_combo_changed), NULL);

	gtk_fixed_put(GTK_FIXED(optionpane_flow),GTK_WIDGET(flow_plot_combo), 320,10);

	gtk_widget_show_all(GTK_WIDGET(optionpane_flow));
	// -------------------------------------------------------------------------------------

	// Return if no treeview is available
	if (treeview == NULL) return;


	_init_plplot(plotarea_flow);


	// Call the corresponding plot function
	//--------------------------------------------------------------------------------------
	switch(saved_flow_plot) {
	case FLOW_CHART_TYPE_STATS:
		pladv(0);
		plcol0(15);
		_plot_multiple_stats(treeview);
		break;
	case FLOW_CHART_TYPE_3DHISTO:
		pladv(0);
		plcol0(15);
		 _plot_3dhisto(treeview);
		break;
	default:
		// You shouldn't be here!!. =)
		// Clear Plot
		plclear();
		pladv(0);
		break;
	}

	_end_plplot(plotarea_flow);

	// Save the last used tree view
	saved_plot_from_treeview = treeview;
}



void _plot_3dhisto(GtkTreeView * treeview) {
	/*
	 * Get the position of the selected Row
	 */
	GtkTreeSelection * tsel = gtk_tree_view_get_selection (treeview);
	GtkTreeIter iter ;
	GtkTreeModel * tm ;
	GtkTreePath * path ;

	GList *node;
	GList * selected_path = gtk_tree_selection_get_selected_rows(tsel, &tm);

	int selected_entrys = g_list_length(selected_path);

	int col_index = get_index_by_columnname("Packetsize Inter Arrival Time histogram bins");
	int flow_index;

	if (col_index == -1 || selected_path == NULL)
		return;


	GtkProgressBar * progress_bar = GTK_PROGRESS_BAR(gtk_builder_get_object(builder,"progressbar"));

	int i = 0;
	int n_size, n_iat;
	double n_height;


	// Num of Bins on each axes
	int bin_width_packetsize;
	int bin_width_iat;
	int bin_nums_packetsize;
	int bin_nums_iat;


	if (saved_reset_bounds == 0) { // This is just a redraw

		bin_width_packetsize = saved_packet_size;
		bin_width_iat = saved_iat_with;

		bin_nums_packetsize = 1500 / bin_width_packetsize;
		bin_nums_iat= 500 / bin_width_iat;
	} else { // This is a new plot
		bin_width_packetsize = 10;
		bin_width_iat = 1;

		bin_nums_packetsize = 1500 / bin_width_packetsize;
		bin_nums_iat= 500 / bin_width_iat;

		// Reset HistoValue Store
		if (saved_packet_size != 0 && saved_iat_with != 0) { // Free old Storeage first
			for (n_size = 0 ; n_size < saved_packet_size ; n_size++) {
				free(histo_value_storage[n_size]);
			}
			free(histo_value_storage);
		}

		// Create New Storage
		histo_value_storage = malloc(sizeof(double*) * bin_nums_packetsize);
		for (n_size = 0 ; n_size < bin_nums_packetsize ; n_size++) {
			histo_value_storage[n_size] = malloc(sizeof(double) * bin_nums_iat);
		}

		for (n_size = 0 ; n_size < bin_nums_packetsize; n_size++) {
			for (n_iat = 0 ; n_iat < bin_nums_iat; n_iat++) {
				histo_value_storage[n_size][n_iat] = 0;
			}
		}

	}



	int count;
	double max_counts = DBL_MIN;

	char **tuples;
	histostat * packets;

	int progress = 1;
	double current_progress;

	if (saved_reset_bounds != 0) { // This is not a redraw
		for (node = selected_path; node != NULL; node = node->next) {

			current_progress = (double) progress++ / (double) selected_entrys;


			gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(progress_bar), current_progress);

			// Get Index for Path
			path = (GtkTreePath *) node->data;
			gtk_tree_model_get_iter (tm,&iter,path);
			gtk_tree_model_get (tm, &iter, 0, &flow_index, -1);

			// Get the separated feature
			count = 0;
			tuples = _separate_feature(flow_index,col_index, &count);

			// seperate all values and store in array of histostat struct
			packets = malloc(sizeof(histostat) * count);
			for (i = 0; i < count ; i++) {
				packets[i].packetsize = atoi(strtok(tuples[i],"_"));
				packets[i].iat = atoi(strtok(NULL,"_"));
				packets[i].count = atoi(strtok(NULL,"_"));
//				printf("i: %d Packetsize: %d, IAT: %d, Count: %d\n", i, packets[i].packetsize, packets[i].iat, packets[i].count);
			}



			// Calculate the coresponding bin and sum the counter
			for (i = 0; i < count ; i++) {
				for (n_size = 0 ; n_size < bin_nums_packetsize ; n_size++) {
					if( (packets[i].packetsize >= n_size * bin_width_packetsize && packets[i].packetsize < (n_size+1)*bin_width_packetsize) ||
							(n_size == bin_nums_packetsize -1 && packets[i].packetsize >= n_size * bin_width_packetsize && packets[i].packetsize <= (n_size+1)*bin_width_packetsize)) {
#if DEBUG != 0
						printf("Histo is in Packet Bin %d (between %d and %d). Value: %d\n", n_size,n_size*bin_width_packetsize, (n_size+1)* bin_width_packetsize, packets[i].packetsize);
#endif
						for (n_iat = 0 ; n_iat < bin_nums_iat; n_iat++) {
							if( (packets[i].iat >= n_iat * bin_width_iat && packets[i].iat < (n_iat+1)*bin_width_iat) ||
									(n_iat == bin_nums_iat -1 && packets[i].iat >= n_iat * bin_width_iat && packets[i].iat <= (n_iat+1)*bin_width_iat)) {
#if DEBUG != 0
								printf("Histo is in IAT Bin %d (between %d and %d). Value: %d\n", n_iat,n_iat*bin_width_iat, (n_iat+1)* bin_width_iat, packets[i].iat);
#endif
								histo_value_storage[n_size][n_iat] += packets[i].count;

								if (histo_value_storage[n_size][n_iat] > max_counts) { max_counts = histo_value_storage[n_size][n_iat]; } // Get Counter Max
							}
						}
					}
				}
			}

			// Clean Up
			for (i = 0 ; i < count ; i++) {
				free(tuples[i]);
			}
			free(tuples);
			free(packets);
		}

		// Log10 all Bins
		for (n_size = 0 ; n_size < bin_nums_packetsize; n_size++) {
			for (n_iat = 0 ; n_iat < bin_nums_iat; n_iat++) {
				histo_value_storage[n_size][n_iat] = log10(histo_value_storage[n_size][n_iat]);
			}
		}
	} else {
		max_counts = saved_max_counts;
	}

	//TODO: maybee this should be done better..
	if (max_counts == 0) {
		max_counts++;
	} else{
		max_counts = log10(max_counts);
		if (max_counts == 0) max_counts++;
	}

	// Store window bounds and create boxes for bounds in option pane
	// -------------------------------------------------------------------------------
	GtkFixed *optionpane_flow = GTK_FIXED(gtk_builder_get_object(builder,"flowoptions"));

	double xmin, xmax, ymin,ymax, zmin, zmax;

	if (saved_reset_bounds == 0) { // This is just a redraw
		xmin = saved_xmin_3d; xmax = saved_xmax_3d;
		ymin = saved_ymin_3d; ymax = saved_ymax_3d;
		zmin = saved_zmin_3d; zmax = saved_zmax_3d;
	} else { // This is a new plot
		saved_reset_bounds = 0;

		xmin = 0; xmax = 1500;
		ymin = 0; ymax = 300;
		zmin = 0; zmax = max_counts;
	}


	char buffer[64];


	// Build Labels
	GtkAccelLabel * xmin_label = GTK_ACCEL_LABEL(gtk_accel_label_new("xmin:"));
	GtkAccelLabel * xmax_label = GTK_ACCEL_LABEL(gtk_accel_label_new("xmax:"));
	GtkAccelLabel * ymin_label = GTK_ACCEL_LABEL(gtk_accel_label_new("ymin:"));
	GtkAccelLabel * ymax_label = GTK_ACCEL_LABEL(gtk_accel_label_new("ymax:"));
	GtkAccelLabel * zmin_label = GTK_ACCEL_LABEL(gtk_accel_label_new("zmin:"));
	GtkAccelLabel * zmax_label = GTK_ACCEL_LABEL(gtk_accel_label_new("zmax:"));

	GtkAccelLabel * packetsize_label = GTK_ACCEL_LABEL(gtk_accel_label_new("Packet-Size Bin Width:"));
	GtkAccelLabel * iat_label = GTK_ACCEL_LABEL(gtk_accel_label_new("IAT Bin Width:"));


	// Build Entrys
	GtkEntry * xmin_entry = GTK_ENTRY(gtk_entry_new());
	GtkEntry * xmax_entry = GTK_ENTRY(gtk_entry_new());
	GtkEntry * ymin_entry = GTK_ENTRY(gtk_entry_new());
	GtkEntry * ymax_entry = GTK_ENTRY(gtk_entry_new());
	GtkEntry * zmin_entry = GTK_ENTRY(gtk_entry_new());
	GtkEntry * zmax_entry = GTK_ENTRY(gtk_entry_new());
	GtkEntry * packetsize_entry = GTK_ENTRY(gtk_entry_new());
	GtkEntry * iat_entry = GTK_ENTRY(gtk_entry_new());


	// Connect Signals
	g_signal_connect(xmin_entry, "activate", G_CALLBACK(on_bound_update_3d), GINT_TO_POINTER(0));
	g_signal_connect(xmax_entry, "activate", G_CALLBACK(on_bound_update_3d), GINT_TO_POINTER(1));
	g_signal_connect(ymin_entry, "activate", G_CALLBACK(on_bound_update_3d), GINT_TO_POINTER(2));
	g_signal_connect(ymax_entry, "activate", G_CALLBACK(on_bound_update_3d), GINT_TO_POINTER(3));
	g_signal_connect(zmin_entry, "activate", G_CALLBACK(on_bound_update_3d), GINT_TO_POINTER(4));
	g_signal_connect(zmax_entry, "activate", G_CALLBACK(on_bound_update_3d), GINT_TO_POINTER(5));
	g_signal_connect(packetsize_entry, "activate", G_CALLBACK(on_bound_update_3d), GINT_TO_POINTER(6));
	g_signal_connect(iat_entry, "activate", G_CALLBACK(on_bound_update_3d), GINT_TO_POINTER(7));


	// Set Entry Text
	sprintf(buffer, "%f", xmin);
	gtk_entry_set_text(GTK_ENTRY(xmin_entry), buffer);
	sprintf(buffer, "%f", xmax);
	gtk_entry_set_text(GTK_ENTRY(xmax_entry), buffer);
	sprintf(buffer, "%f", ymin);
	gtk_entry_set_text(GTK_ENTRY(ymin_entry), buffer);
	sprintf(buffer, "%f", ymax);
	gtk_entry_set_text(GTK_ENTRY(ymax_entry), buffer);
	sprintf(buffer, "%f", zmin);
	gtk_entry_set_text(GTK_ENTRY(zmin_entry), buffer);
	sprintf(buffer, "%f", zmax);
	gtk_entry_set_text(GTK_ENTRY(zmax_entry), buffer);
	sprintf(buffer, "%d", bin_width_packetsize);
	gtk_entry_set_text(GTK_ENTRY(packetsize_entry), buffer);
	sprintf(buffer, "%d", bin_width_iat);
	gtk_entry_set_text(GTK_ENTRY(iat_entry), buffer);


	// Put Labels in Container
	gtk_fixed_put(GTK_FIXED(optionpane_flow), GTK_WIDGET(xmin_label), 10, 50);
	gtk_fixed_put(GTK_FIXED(optionpane_flow), GTK_WIDGET(xmax_label), 10, 75);
	gtk_fixed_put(GTK_FIXED(optionpane_flow), GTK_WIDGET(ymin_label), 220, 50);
	gtk_fixed_put(GTK_FIXED(optionpane_flow), GTK_WIDGET(ymax_label), 220, 75);
	gtk_fixed_put(GTK_FIXED(optionpane_flow), GTK_WIDGET(zmin_label), 430, 50);
	gtk_fixed_put(GTK_FIXED(optionpane_flow), GTK_WIDGET(zmax_label), 430, 75);
	gtk_fixed_put(GTK_FIXED(optionpane_flow), GTK_WIDGET(packetsize_label), 30, 105);
	gtk_fixed_put(GTK_FIXED(optionpane_flow), GTK_WIDGET(iat_label), 350, 105);


	// Put Entrys in Container
	gtk_fixed_put(GTK_FIXED(optionpane_flow), GTK_WIDGET(xmin_entry), 55, 45);
	gtk_fixed_put(GTK_FIXED(optionpane_flow), GTK_WIDGET(xmax_entry), 55, 70);
	gtk_fixed_put(GTK_FIXED(optionpane_flow), GTK_WIDGET(ymin_entry), 265, 45);
	gtk_fixed_put(GTK_FIXED(optionpane_flow), GTK_WIDGET(ymax_entry), 265, 70);
	gtk_fixed_put(GTK_FIXED(optionpane_flow), GTK_WIDGET(zmin_entry), 485, 45);
	gtk_fixed_put(GTK_FIXED(optionpane_flow), GTK_WIDGET(zmax_entry), 485, 70);
	gtk_fixed_put(GTK_FIXED(optionpane_flow), GTK_WIDGET(packetsize_entry), 180, 100);
	gtk_fixed_put(GTK_FIXED(optionpane_flow), GTK_WIDGET(iat_entry), 445, 100);

	gtk_widget_show_all(GTK_WIDGET(optionpane_flow));
	// -------------------------------------------------------------------------------

#if DEBUG != 0
	printf("xmin: %f, xmax: %f\n", xmin, xmax);
	printf("ymin: %f, ymax: %f\n", ymin, ymax);
	printf("zmin: %f, zmax: %f\n", zmin, zmax);
#endif


	// Setup Window
	plvsta();
	plwind(-9,10,-4,15);
	plw3d(12,12,12,xmin,xmax,ymin,ymax,zmin,zmax,view_angle_alt,view_angle_az);
	plbox3("bnstu","Packet Size",0,0,"bnstu","Inter Arrival Time",0,0,"bcmnstuv","Count",0,0);
	pllab("","","Packetsize Inter Arrival Time histogram");

	// Build first a grid
	PLFLT x[3], y[3], z[3];
	// Bottom Parallel Y Axes
	for (n_size = xmin ; n_size <= xmax ; n_size+=((xmax - xmin) / 10.)) {
		x[0] = n_size; 	y[0] = ymin; 	z[0] = zmin;
		x[1] = n_size; 	y[1] = ymax; 	z[1] = zmin;
		plline3(2,x,y,z);
	}

	// Parallel to XY-Pane
	for (n_height = zmin ; n_height <= zmax ; n_height+= ((zmax - zmin) / 10.)) {
		x[0] = xmin; 		y[0] =  ymax;	z[0] = n_height;
		x[1] = xmax; 	y[1] =  ymax;	z[1] = n_height;
		x[2] = xmax; 	y[2] =  ymin;	z[2] = n_height;
		plline3(3,x,y,z);
	}

	// Bottom Parallel X Axes
	for (n_iat = ymin ; n_iat <= ymax ; n_iat+=((ymax - ymin) / 10.)) {
		x[0] = xmin; 		y[0] = n_iat; 	z[0] = zmin;
		x[1] = xmax; 	y[1] = n_iat; 	z[1] = zmin;
		plline3(2,x,y,z);
	}

	// Edge Line
	x[0] = xmax;	y[0] = ymax;	z[0] = zmin;
	x[0] = xmax;	y[0] = ymax;	z[0] = zmax;
	plline3(2,x,y,z);


	if (motion_button_down == 0) { // only if not in move
		// Plot the bin
		for (n_size = bin_nums_packetsize - 1 ; n_size >= 0 ; n_size--) {
			for (n_iat = bin_nums_iat -1 ; n_iat >= 0; n_iat--) {
				if (histo_value_storage[n_size][n_iat] > 0) {
					_plot_3dbin(n_size * bin_width_packetsize + bin_width_packetsize/2,n_iat * bin_width_iat + bin_width_iat/2,histo_value_storage[n_size][n_iat],bin_width_packetsize, bin_width_iat);
				}
			}
		}
	}


	gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(progress_bar), 0);

	// Save the current flow index so it can be redrawed
	//--------------------------------------------------------------------
	saved_xmin_3d = xmin;
	saved_xmax_3d = xmax;
	saved_ymin_3d = ymin;
	saved_ymax_3d = ymax;
	saved_zmin_3d = zmin;
	saved_zmax_3d = zmax;
	saved_packet_size = bin_width_packetsize;
	saved_iat_with = bin_width_iat;
	saved_max_counts = max_counts;

}

/*
 * Generate a 3D Bin.
 * binx and biny is the center point so +- width/2 to get start and end
 */
void _plot_3dbin(double binx, double biny, double height, double width_x, double width_y) {

	PLFLT x[4];
	PLFLT y[4];
	PLFLT z[4];


	//Right
	x[0] = binx + width_x; 		y[0] = biny; 			z[0] = 0;
	x[1] = binx + width_x/2; 	y[1] = biny + width_y; 	z[1] = 0;
	x[2] = binx + width_x/2; 	y[2] = biny + width_y;	z[2] = height;
	x[3] = binx + width_x/2; 	y[3] = biny; 			z[3] = height;
	plcol0(6);
	plfill3(4,x,y,z);


	// Front
	x[0] = binx ; 				y[0] = biny + width_y; 	z[0] = 0;
	x[1] = binx + width_x; 		y[1] = biny + width_y; 	z[1] = 0;
	x[2] = binx + width_x; 		y[2] = biny + width_y; 	z[2] = height;
	x[3] = binx ; 				y[3] = biny + width_y; 	z[3] = height;
	plcol0(1);
	plfill3(4,x,y,z);


	//Back
	x[0] = binx; 				y[0] = biny; 			z[0] = 0;
	x[1] = binx + width_x; 		y[1] = biny; 			z[1] = 0;
	x[2] = binx + width_x; 		y[2] = biny; 			z[2] = height;
	x[3] = binx; 				y[3] = biny; 			z[3] = height;
	plcol0(5);
	plfill3(4,x,y,z);


	//Left
	x[0] = binx; 				y[0] = biny; 			z[0] = 0;
	x[1] = binx; 				y[1] = biny + width_y; 	z[1] = 0;
	x[2] = binx; 				y[2] = biny + width_y; 	z[2] = height;
	x[3] = binx; 				y[3] = biny; 			z[3] = height;
	plcol0(4);
	plfill3(4,x,y,z);


	//TOP
	x[0] = binx; 				y[0] = biny ; 			z[0] = height;
	x[1] = binx; 				y[1] = biny + width_y; 	z[1] = height;
	x[2] = binx + width_x; 		y[2] = biny + width_y; 	z[2] = height;
	x[3] = binx + width_x; 		y[3] = biny; 			z[3] = height;
	plcol0(3);
	plfill3(4,x,y,z);


	//---------------------------------------------------------------------------------------
	plcol0(15);


	//Line Front Left
	x[0] = binx; y[0] = biny + width_y; z[0] = 0;
	x[1] = binx; y[1] = biny + width_y; z[1] = height;
	plline3(2,x,y,z);

	//Line Front Right
	x[0] = binx; y[0] = biny; z[0] = 0;
	x[1] = binx; y[1] = biny; z[1] = height;
	plline3(2,x,y,z);

	//Line Back Right
	x[0] = binx + width_x; y[0] = biny; z[0] = 0;
	x[1] = binx + width_x; y[1] = biny; z[1] = height;
	plline3(2,x,y,z);

	//Line Back TOP
	x[0] = binx + width_x; y[0] = biny + width_y; z[0] = height;
	x[1] = binx + width_x; y[1] = biny; z[1] = height;
	plline3(2,x,y,z);

	//Line Front TOP
	x[0] = binx; y[0] = biny + width_y; z[0] = height;
	x[1] = binx; y[1] = biny ; z[1] = height;
	plline3(2,x,y,z);

	//Line Top Right
	x[0] = binx; y[0] = biny + width_y; z[0] = height;
	x[1] = binx + width_x; y[1] = biny + width_y; z[1] = height;
	plline3(2,x,y,z);

	//Line Top Left
	x[0] = binx; y[0] = biny; z[0] = height;
	x[1] = binx + width_x; y[1] = biny; z[1] = height;
	plline3(2,x,y,z);

//	//Line Back Bottom
//	x[0] = binx + width_x/2; y[0] = biny + width_y/2; z[0] = 0;
//	x[1] = binx + width_x/2; y[1] = biny - width_y/2; z[1] = 0;
//	plline3(2,x,y,z);

	//Line Front Bottom
	x[0] = binx; y[0] = biny + width_y; z[0] = 0;
	x[1] = binx; y[1] = biny; z[1] = 0;
	plline3(2,x,y,z);

//	//Line Bottom Left
//	x[0] = binx - width_x/2; y[0] = biny + width_y/2; z[0] = 0;
//	x[1] = binx + width_x/2; y[1] = biny + width_y/2; z[1] = 0;
//	plline3(2,x,y,z);

	//Line Bottom Right
	x[0] = binx; y[0] = biny ; z[0] = 0;
	x[1] = binx + width_x; y[1] = biny; z[1] = 0;
	plline3(2,x,y,z);
}

void _plot_multiple_stats(GtkTreeView *treeview) {

	GtkFixed *optionpane_flow = GTK_FIXED(gtk_builder_get_object(builder,"flowoptions"));

	// Setup Option Pane
	// ---------------------------------------------------
	GtkCheckButton* combined_line_switch = GTK_CHECK_BUTTON(gtk_check_button_new_with_label("Combined Plot"));
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(combined_line_switch),saved_combined_line);

	g_signal_connect (combined_line_switch, "toggled",G_CALLBACK (on_combined_line_switch_toogle), NULL);

	gtk_fixed_put(GTK_FIXED(optionpane_flow),GTK_WIDGET(combined_line_switch), 20,15);
	gtk_widget_show_all(GTK_WIDGET(optionpane_flow));
	//---------------------------------------------------

	/*
	 * Get the position of the selected Row
	 */
	GtkTreeSelection * tsel = gtk_tree_view_get_selection (treeview);
	GtkTreeIter iter ;
	GtkTreeModel * tm ;
	GtkTreePath * path ;

	GList *node;
	GList * selected_path = gtk_tree_selection_get_selected_rows(tsel, &tm);

	int flow_index;
	int col_index = get_index_by_columnname("Packet lengths and inter-arrival times for the N first packets");

	if (col_index == -1 || selected_path == NULL) { return; } // Return if feature not available or nothing selected

	int maxbytes = INT_MIN;
	double maxtimes = DBL_MIN;
	double maxtimes_combine = DBL_MIN;
	int maxpackets = INT_MIN;

	int numpackets = 0;
	int i = 0;

	char **tuples;

	PLFLT *y1 = NULL;
	PLFLT *y2 = NULL;
	PLFLT *x2 = NULL;
	PLFLT *x1 = NULL;


	// First we need to get the Bounds for the window
	//--------------------------------------------------------------------
	for (node = selected_path; node != NULL; node = node->next) {
		// Get Index for Path
		path = (GtkTreePath *) node->data;
		gtk_tree_model_get_iter (tm,&iter,path);
		gtk_tree_model_get (tm, &iter, 0, &flow_index, -1);

		tuples = _separate_feature(flow_index,col_index, &numpackets);
		if (numpackets > maxpackets) maxpackets = numpackets;

		y1 = malloc(sizeof(PLFLT) * numpackets);
		y2 = malloc(sizeof(PLFLT) * numpackets);
		x2 = malloc(sizeof(PLFLT) * numpackets);

		// Calculate the min and maxs
		// ---------------------------------------------------
		for (i = 0 ; i < numpackets ; i++) {

			y1[i] = (PLFLT) atoi(strtok(tuples[i],"_"));
			y2[i] = (PLFLT) atof(strtok(NULL,"_"));

			// Sum Time's
			if (i == 0 ) {
				x2[i] = y2[i];
			} else {
				x2[i] = x2[i-1] + y2[i];
			}

			if (x2[i] > maxtimes_combine) maxtimes_combine = x2[i];
			if (y1[i] > maxbytes) maxbytes = y1[i];
			if (y2[i] > maxtimes) maxtimes = y2[i];
		}

		// Clean Up
		// ---------------------------------------------------
		for (i = 0 ; i < numpackets ; i++) {
			free(tuples[i]);
		}

		free(tuples);
		free(y1);
		free(y2);
		free(x1);
		free(x2);

	}
	//--------------------------------------------------------------------



	// Store windows bounds and create boxes for bounds in option pane
	//--------------------------------------------------------------------
	double xmin = 0, xmax = 0, ymin = 0, ymax = 0, y2min = 0, y2max = 0;

	if (saved_reset_bounds == 0) { // This is just a redraw
		xmin = saved_xmin; xmax = saved_xmax;
		ymin = saved_ymin; ymax = saved_ymax;
		y2min = saved_y2min; y2max = saved_y2max;
	} else { // This is a new plot
		saved_reset_bounds = 0;
		//Set xmin, xmax, ymin,ymax
		if (saved_combined_line) { // The lines are combined

			if (maxtimes_combine == 0) maxtimes_combine++;
			if (maxbytes == 0)maxbytes++;


			xmin = 0; xmax = maxtimes_combine;
			ymin = 0; ymax = maxbytes;
		} else { //  two seperate lines

			if (maxpackets == 0) maxpackets++;
			if (maxbytes == 0) maxbytes++;
			if (maxtimes == 0) maxtimes++;


			xmin = 0; xmax = maxpackets;
			ymin = 0; ymax = maxbytes;
			y2min = 0; y2max = maxtimes;
		}
	}
#if DEBUG != 0
	printf("xmin: %f, xmax: %f\n", xmin, xmax);
	printf("ymin: %f, ymax: %f\n", ymin, ymax);
	printf("y2min: %f, y2max: %f\n", y2min, y2max);
#endif

	char buffer[64];

	// Build Labels
	GtkAccelLabel * xmin_label = GTK_ACCEL_LABEL(gtk_accel_label_new("xmin:"));
	GtkAccelLabel * xmax_label = GTK_ACCEL_LABEL(gtk_accel_label_new("xmax:"));
	GtkAccelLabel * ymin_label = GTK_ACCEL_LABEL(gtk_accel_label_new("ymin:"));
	GtkAccelLabel * ymax_label = GTK_ACCEL_LABEL(gtk_accel_label_new("ymax:"));
	GtkAccelLabel * y2min_label = GTK_ACCEL_LABEL(gtk_accel_label_new("y2min:"));
	GtkAccelLabel * y2max_label = GTK_ACCEL_LABEL(gtk_accel_label_new("y2max:"));


	// Build Entrys
	GtkEntry * xmin_entry = GTK_ENTRY(gtk_entry_new());
	GtkEntry * xmax_entry = GTK_ENTRY(gtk_entry_new());
	GtkEntry * ymin_entry = GTK_ENTRY(gtk_entry_new());
	GtkEntry * ymax_entry = GTK_ENTRY(gtk_entry_new());
	GtkEntry * y2min_entry = GTK_ENTRY(gtk_entry_new());
	GtkEntry * y2max_entry = GTK_ENTRY(gtk_entry_new());


	// Connect Signals
	g_signal_connect(xmin_entry, "activate", G_CALLBACK(on_bound_update), GINT_TO_POINTER(0));
	g_signal_connect(xmax_entry, "activate", G_CALLBACK(on_bound_update), GINT_TO_POINTER(1));
	g_signal_connect(ymin_entry, "activate", G_CALLBACK(on_bound_update), GINT_TO_POINTER(2));
	g_signal_connect(ymax_entry, "activate", G_CALLBACK(on_bound_update), GINT_TO_POINTER(3));
	g_signal_connect(y2min_entry, "activate", G_CALLBACK(on_bound_update), GINT_TO_POINTER(4));
	g_signal_connect(y2max_entry, "activate", G_CALLBACK(on_bound_update), GINT_TO_POINTER(5));



	// Set Entry Text
	sprintf(buffer, "%f", xmin);
	gtk_entry_set_text(GTK_ENTRY(xmin_entry), buffer);
	sprintf(buffer, "%f", xmax);
	gtk_entry_set_text(GTK_ENTRY(xmax_entry), buffer);
	sprintf(buffer, "%f", ymin);
	gtk_entry_set_text(GTK_ENTRY(ymin_entry), buffer);
	sprintf(buffer, "%f", ymax);
	gtk_entry_set_text(GTK_ENTRY(ymax_entry), buffer);
	sprintf(buffer, "%f", y2min);
	gtk_entry_set_text(GTK_ENTRY(y2min_entry), buffer);
	sprintf(buffer, "%f", y2max);
	gtk_entry_set_text(GTK_ENTRY(y2max_entry), buffer);


	// Put Labels in Container
	gtk_fixed_put(GTK_FIXED(optionpane_flow), GTK_WIDGET(xmin_label), 10, 50);
	gtk_fixed_put(GTK_FIXED(optionpane_flow), GTK_WIDGET(xmax_label), 10, 75);
	gtk_fixed_put(GTK_FIXED(optionpane_flow), GTK_WIDGET(ymin_label), 220, 50);
	gtk_fixed_put(GTK_FIXED(optionpane_flow), GTK_WIDGET(ymax_label), 220, 75);
	if (saved_combined_line == 0) {
		gtk_fixed_put(GTK_FIXED(optionpane_flow), GTK_WIDGET(y2min_label), 430, 50);
		gtk_fixed_put(GTK_FIXED(optionpane_flow), GTK_WIDGET(y2max_label), 430, 75);
	}

	// Put Entrys in Container
	gtk_fixed_put(GTK_FIXED(optionpane_flow), GTK_WIDGET(xmin_entry), 55, 45);
	gtk_fixed_put(GTK_FIXED(optionpane_flow), GTK_WIDGET(xmax_entry), 55, 70);
	gtk_fixed_put(GTK_FIXED(optionpane_flow), GTK_WIDGET(ymin_entry), 265, 45);
	gtk_fixed_put(GTK_FIXED(optionpane_flow), GTK_WIDGET(ymax_entry), 265, 70);
	if (saved_combined_line == 0) {
		gtk_fixed_put(GTK_FIXED(optionpane_flow), GTK_WIDGET(y2min_entry), 485, 45);
		gtk_fixed_put(GTK_FIXED(optionpane_flow), GTK_WIDGET(y2max_entry), 485, 70);
	}

	gtk_widget_show_all(GTK_WIDGET(optionpane_flow));
	//--------------------------------------------------------------------


	// Setup Window
	//--------------------------------------------------------------------
	if (saved_combined_line) {// time/Packet Lenght
		// Setup Axis
		plvsta();
		plcol0(1); // Red
		plwind(xmin,xmax, ymin,ymax);
		plaxes(0,0,"anft",0,0,"fanst",0,0);
		plmtex("l", 3.5, 0.5, 0.5, "Packet Size");

		plcol0(15);
		pllab("time[s]","","N first packet statistics");
	} else {
		// interTime / Packet Lengh, two lines
		// Setup Axis
		plvsta();
		plcol0(1); // Red
		plwind(xmin,xmax, ymin,ymax);
		plaxes(0,1,"anft",0,0,"fanst",0,0);
		plmtex("l", 3.5, 0.5, 0.5, "Packet Size");

		plcol0(9); // Blue
		plwind(xmin,xmax, y2min,y2max);
		plaxes(0,0,"",0,0,"cmstv",0,0);
		plmtex("r", 3.5, 0.5, 0.5, "Inter Time");

		plcol0(15);
		pllab("Packet Number","","N first packet statistics");
	}
	//--------------------------------------------------------------------


	// Plot all lines
	//--------------------------------------------------------------------
	for (node = selected_path; node != NULL; node = node->next) {
		// Get Index for Path
		path = (GtkTreePath *) node->data;
		gtk_tree_model_get_iter (tm,&iter,path);
		gtk_tree_model_get (tm, &iter, 0, &flow_index, -1);

		tuples = _separate_feature(flow_index,col_index, &numpackets);

		x1 = malloc(sizeof(PLFLT) * numpackets);
		x2 = malloc(sizeof(PLFLT) * numpackets);
		y1 = malloc(sizeof(PLFLT) * numpackets);
		y2 = malloc(sizeof(PLFLT) * numpackets);


		// Calculate the min and maxs
		// ---------------------------------------------------
		for (i = 0 ; i < numpackets ; i++) {

			x1[i] = i+1;
			y1[i] = (PLFLT) atoi(strtok(tuples[i],"_"));
			y2[i] = (PLFLT) atof(strtok(NULL,"_"));

			// Sum Time's
			if (i == 0 ) { x2[i] = y2[i];}
			else { x2[i] = x2[i-1] + y2[i];}

		}

		// Bytes
		// ---------------------------------------------------
		if (saved_combined_line) {// time/Packet Lenght
			// Plot Line
			plcol0(1); // Red

			plline(numpackets,x2,y1);
			plpoin(numpackets,x2,y1,9);
		} else {
			// interTime / Packet Lengh, two lines
			// Plot Lines
			plcol0(1); // Red
			plline(numpackets,x1,y1);
			plpoin(numpackets,x1,y1,9);

			plcol0(9); // Blue
			plline(numpackets,x1,y2);
			plpoin(numpackets,x1,y2,9);
		}

		// Clean Up
		// ---------------------------------------------------
		for (i = 0 ; i < numpackets ; i++) {
			free(tuples[i]);
		}

		free(tuples);
		free(y1);
		free(y2);
		free(x2);
		free(x1);
	}
	//--------------------------------------------------------------------


	// Clean Up
	//--------------------------------------------------------------------
	g_list_free(selected_path);

	// Save the current flow index so it can be redrawed
	//--------------------------------------------------------------------
	saved_xmin = xmin;
	saved_xmax = xmax;
	saved_ymin = ymin;
	saved_ymax = ymax;
	saved_y2min = y2min;
	saved_y2max = y2max;
}

/*
 * Plot values in a pie
 */
void _plot_pie(int column_index) {

#if DEBUG != 0
	printf("Ploting Chart Pie\n");
#endif


	// Setup the "other" aggregatopm entry
	// ---------------------------------------------------
	GtkFixed *optionpane_flow = GTK_FIXED(gtk_builder_get_object(builder,"coloptions"));

	// Reset if we have a new column
	if (column_index != saved_col_index) {
		saved_per_aggregation = 5; // Default is 5 %
	}


	// Entry Box for Number of Bins
	GtkAccelLabel * per_aggregation_label = GTK_ACCEL_LABEL(gtk_accel_label_new("% for \"Other\" Aggregation:"));

	char buffer[16];
	sprintf(buffer, "%f", saved_per_aggregation);

	GtkEntry * per_aggregation_textbox = GTK_ENTRY(gtk_entry_new());
	gtk_entry_set_text(GTK_ENTRY(per_aggregation_textbox),buffer);
	g_signal_connect (per_aggregation_textbox, "activate",G_CALLBACK(on_per_aggregation_textbox_activate), NULL);

	// Put them now on the optionpane
	gtk_fixed_put(GTK_FIXED(optionpane_flow), GTK_WIDGET(per_aggregation_textbox), 200, 10);
	gtk_fixed_put(GTK_FIXED(optionpane_flow), GTK_WIDGET(per_aggregation_label), 20, 15);


	gtk_widget_show_all(GTK_WIDGET(optionpane_flow));
	// ---------------------------------------------------

	int i,j;

	linkedlist_t * values = linkedlist_new();
	linkedlist_item_t *value;
	value_counter *v_counter;
	value_counter *new_v_counter;
	value_counter *other_v_counter;

	linkedlist_item_t *currentline= data_storage->head;
	linkedlist_item_t *column;
	linkedlist_t *dataline;

	int found = 0;

	char *tempvalue;

	// Count all Values
	// ---------------------------------------------------

	while (currentline != NULL) {
		if (filter_check(feature_filters, currentline)) {
			// Get the dataLine
			dataline = (linkedlist_t *) currentline->item;
			column = linkedlist_item_skipto(dataline,column_index);

			tempvalue = (char *) column->item;

			// Loop trough list and search for Value
			// If is already in List, increase the counter
			value = values->head;
			while(value != NULL && found == 0) {
				v_counter = (value_counter *) value->item;

				if(strcmp(tempvalue,v_counter->value) == 0) {
					found = 1;
					v_counter->count++;
				}

				value = value->next_item;
			}

			// If the Value wasn't found, add it to the list
			if (found == 0) {
				new_v_counter = malloc(sizeof(value_counter));
				new_v_counter->value = malloc(sizeof(char) * 32);

				strncpy(new_v_counter->value,tempvalue,32);
				new_v_counter->count = 1;

				linkedlist_add_item(values, linkedlist_item_new((void *) new_v_counter));
			}

			found = 0;
		}
		currentline = currentline->next_item;
	}

	// Add as last Element the Other Value's
	other_v_counter = malloc(sizeof(value_counter));
	other_v_counter->value = malloc(sizeof(char) * 32);

	strncpy(other_v_counter->value,"Other",32);
	other_v_counter->count = 0;
	linkedlist_add_item(values, linkedlist_item_new((void *) other_v_counter));

	// Sum all below a certain percentage and add to other values
	// ---------------------------------------------------
	double currentper;
	double maxper = saved_per_aggregation;
	value = values->head;

	if (maxper > 0) {
		while (value != NULL) {
			v_counter = (value_counter *) value->item;
			currentper = ((double)v_counter->count / (double) data_storage->size) * (double) 100;

			if (currentper < maxper) {
				other_v_counter->count += v_counter->count;
				v_counter->count = 0;
			}

			value = value->next_item;
		}
	}

	// Plot now the pie
	// ---------------------------------------------------

	int dthet, theta0, theta1, theta;
	PLFLT just, dx, dy;
	PLFLT x[500], y[500];
	double per[values->size];

	plvsta();
	plwind(-2,12,-2,12); // Setup Window from 0 to 10
	plcol0(2);

	theta0 = 0;
	dthet = 2;

	// Loop trought all values
	value = values->head;
	i = 0;
	while (value != NULL) {

		v_counter = (value_counter *) value->item;
		if (v_counter->count > 0) {

			// Calculate percentage
			per[i] = ((double)v_counter->count / (double) data_storage->size) * (double) 100;

	#if DEBUG != 0
			printf("Percentage of i=%d: %f -> Value: %s\n", i, per[i], v_counter->value);
	#endif

			// First Point centered
			j = 0;
			x[j] = 5.;
			y[j++] = 5.;

			/* n.b. the theta quantities multiplied by 2*G_PI/500 afterward so
			 * in fact per is interpreted as a percentage. */
			theta1 = (int)(theta0 + 5 * per[i]);

			if (value->next_item == NULL) { theta1 = 500; }

#if DEBUG != 0
			printf("Building Segments for the pies\n");
			printf("Theta0: %d, Theta1: %d\n", theta0, theta1);
#endif


			// Calculate each segment of the pie
			for (theta = theta0; theta <= theta1; theta += dthet) {
				x[j] = 5 + 4 * cos((2.*G_PI/500.)*theta);
				y[j] = 5 + 4 * sin((2.*G_PI/500.)*theta);

#if DEBUG != 0
				printf("x[%d]: %f\ty[%d]: %f\n", j, x[j], j, y[j]);
#endif

				j++;
			}


			// Odd Color 6, Even Color 8
			if (value->next_item == NULL) { plcol0(7);}
			else {
				if (i % 2 == 0) { plcol0(6);}
				else {plcol0(8); }
			}


			plpsty(0);
			plfill(j,x,y);

			plcol0(15);
			plline(j,x,y);

			// Position to plot the label
			just = (2.*G_PI/500.)*(theta0 + theta1)/2.;
			dx = .25 * cos(just);
			dy = .25 * sin(just);
			if ((theta0 + theta1)  < 250 || (theta0 + theta1) > 750)
				just = 0.;
			else
				just = 1.;

			plfont(2);
			plschr(0,0.8); // Font Size
			plptex((x[j / 2] + dx), (y[j / 2] + dy), 1.0, 0.0, just, v_counter->value);
			plschr(0,1); // Reset to default


			theta0 = theta - dthet;
			i++;
		}
		value = value->next_item;
	}

	// Clean up
	// ----------------------------------
	linkedlist_item_t * tempitem;
	value = values->head;
	tempitem = value->next_item;

	while (tempitem != NULL) {
		free(value->item);
		free(value);
		value = tempitem;
		tempitem = value->next_item;
	}
	free(values);
}


/*
 * Generate Bins and plot them
 * doHighlight = 1, display bin for dataToHightlight in a different color
 */
void _plot_histo(int column_index, double data_to_hightlight, int do_hightlight) {
#if DEBUG != 0
	printf("Ploting Histogramm\n");
#endif

	// Setup the log axis switcher
	// ---------------------------------------------------
	GtkFixed *optionpane_flow = GTK_FIXED(gtk_builder_get_object(builder,"coloptions"));

	GtkCheckButton* logaxis_switch = GTK_CHECK_BUTTON(gtk_check_button_new_with_label("logarithmical y-Axes"));

	// Reset if we have a new column.. default is log!
	if (column_index != saved_col_index) {
		saved_is_log_axis = 1;
		saved_bin_count = 40;
	}

	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(logaxis_switch),saved_is_log_axis);
	g_signal_connect (logaxis_switch, "toggled",G_CALLBACK (on_logaxis_switch_toogle), NULL);
	gtk_fixed_put(GTK_FIXED(optionpane_flow),GTK_WIDGET(logaxis_switch), 20,15);


	// Entry Box for Number of Bins
	GtkAccelLabel * bin_width_label = GTK_ACCEL_LABEL(gtk_accel_label_new("Num of Bins:"));

	char buffer[5];
	sprintf(buffer, "%i", saved_bin_count);

	GtkEntry * histo_bin_count_textbox = GTK_ENTRY(gtk_entry_new());
	gtk_entry_set_text(GTK_ENTRY(histo_bin_count_textbox),buffer);
	g_signal_connect (histo_bin_count_textbox, "activate",G_CALLBACK(on_histogramm_bin_count_textbox_activate), NULL);

	GtkButton *scotts_button = GTK_BUTTON(gtk_button_new_with_label("Scott's Rule"));
	g_signal_connect(scotts_button, "clicked", G_CALLBACK(_on_scotts_button_clicked), GINT_TO_POINTER(column_index));

	GtkButton *sturges_button = GTK_BUTTON(gtk_button_new_with_label("Sturges Rule"));
	g_signal_connect(sturges_button, "clicked", G_CALLBACK(_on_sturges_button_clicked), NULL);


	// Put them now on the optionpane
	gtk_fixed_put(GTK_FIXED(optionpane_flow), GTK_WIDGET(histo_bin_count_textbox), 350, 10);
	gtk_fixed_put(GTK_FIXED(optionpane_flow), GTK_WIDGET(bin_width_label), 250, 15);
	gtk_fixed_put(GTK_FIXED(optionpane_flow), GTK_WIDGET(scotts_button), 520, 8);
	gtk_fixed_put(GTK_FIXED(optionpane_flow), GTK_WIDGET(sturges_button), 620, 8);


	gtk_widget_show_all(GTK_WIDGET(optionpane_flow));
	// ---------------------------------------------------

	int nbins = saved_bin_count;

	PLFLT bins[nbins];
	PLFLT x[nbins];

	int n;
	double tempdata;

	linkedlist_item_t *currentline= data_storage->head;
	linkedlist_item_t *column;
	linkedlist_t *dataline;


	// Get min and max in the Datastorage
	// -----------------------------------

	double datamin = DBL_MAX;
	double datamax = DBL_MIN;
	double maxbins = DBL_MIN;


	while (currentline != NULL) {
		if (filter_check(feature_filters, currentline)) {

			// Get the dataLine
			dataline = (linkedlist_t *) currentline->item;
			column = linkedlist_item_skipto(dataline,column_index);
			tempdata = atof((char *) column->item);

			// Get Max and Min to set the bounds..
			if (tempdata < datamin) datamin = tempdata;
			if (tempdata > datamax) datamax = tempdata;
		}

		currentline = currentline->next_item;
	}

#if DEBUG != 0
			printf("Min & Max of Datastorage: Min: %f, Max: %f\n", datamin, datamax);
#endif

	double binwidth = (datamax - datamin) / nbins;

	// Set all bins to 0.. and x cord to left bin border
	for (n = 0 ; n < nbins ; n++) {
		bins[n] = 0;
		x[n] = n*binwidth + datamin;
	}


	// Increase the bins according to data
	// -----------------------------------
	currentline= data_storage->head;
	while (currentline != NULL) {
		if (filter_check(feature_filters, currentline)) {
			// Get the dataLine
			dataline = (linkedlist_t *) currentline->item;
			column = linkedlist_item_skipto(dataline,column_index);
			tempdata = atof((char *) column->item);

			for(n = 0 ; n < nbins ; n++) {

				if ((tempdata >= x[n] && tempdata < x[n] + binwidth)
						|| (n == nbins-1 && (tempdata >= x[n] && tempdata >= x[n] + binwidth))) {
					//If this is the last bin, right bound belongs to the bin too!
					bins[n]++;
				}

				if (bins[n] > maxbins) { maxbins = bins[n]; } // Get the max value

			}
		}
		currentline = currentline->next_item;
	}

#if DEBUG != 0
			printf("Max Bins: %f\n", maxbins);
#endif


	plvsta();
	plcol0(15);

	if (saved_is_log_axis) {
		if (datamin == datamax) {
			datamin--; datamax++;
		}
		if (log10(maxbins) == 0) {
			maxbins++;
		}
		plwind(datamin,datamax,0,log10(maxbins));
		plaxes(datamin,0,"fant",0,0,"lanfgth",0,0);
	} else {
		if (datamin == datamax) {
			datamin--; datamax++;
		}
		if (maxbins == 0) {
			maxbins++;
		}
		plwind(datamin,datamax,0,maxbins);
		plaxes(datamin,0,"fant",0,0,"anfgth",0,0);
	}


	// Plot the filled area and the border around the bins
	// -------------------------------------------------------------------

	PLFLT xfill[4], yfill[4]; // Polygon to fill the bins

	int dont_draw;
	for (n = 0 ; n < nbins ; n++) {

		dont_draw = 0;
		// Fill Polygon
		// ----------------------------
		xfill[0] = x[n];
		xfill[1] = x[n];
		xfill[2] = x[n]+binwidth;
		xfill[3] = x[n]+binwidth;



		// Don't Draw a bin with no content
		if (bins[n] == 0)
			dont_draw = 1;

		yfill[0] = 0;
		if (saved_is_log_axis) { // Log Axis Switch
			yfill[1] = log10(bins[n]);
			yfill[2] = log10(bins[n]);
		} else {
			yfill[1] = bins[n];
			yfill[2] = bins[n];
		}
		yfill[3] = 0;

#if DEBUG != 0
		printf("Log10(%f): %f\n", bins[n],log10(bins[n]));
#endif

		if ((do_hightlight == 1)	&& 	((data_to_hightlight >= x[n] && data_to_hightlight < x[n] + binwidth)
				|| (n == nbins-1 && (data_to_hightlight >= x[n] && data_to_hightlight >= x[n] + binwidth)))) {
			plcol0(8);
		} else {
			plcol0(6);
		}
		if (dont_draw == 0) {
			plfill(4,xfill, yfill);
		}

#if DEBUG != 0
	printf("Polygon Points for %d:\n", n);
	printf("x1: %f, y1: %f\n", xfill[0], yfill[0]);
	printf("x2: %f, y2: %f\n", xfill[1], yfill[1]);
	printf("x3: %f, y3: %f\n", xfill[2], yfill[2]);
	printf("x4: %f, y4: %f\n", xfill[3], yfill[3]);
#endif

		// Outline Border
		// ---------------------------
		if (dont_draw == 0) {
			plcol0(1);

			plline(4,xfill,yfill);


			if(yfill[1] == 0 && saved_is_log_axis) {
				yfill[1] += 0.001;
				yfill[2] += 0.001;

				pljoin(xfill[1],yfill[1],xfill[2],yfill[2]); // Top;
			}

		}
	}
}

void _on_sturges_button_clicked(GtkButton *button, gpointer user_data) {

	int bin_nums = 1 + (3.3 * log10(data_storage->size));

	// Only replot if valid..
	if (bin_nums > 0) {
		saved_bin_count = bin_nums;
		plot_col(saved_col_index,saved_data_to_highlight, saved_do_highlight);
	}

}

/*
 * Calculcate based on sturges Rule the number of bins
 */
void _on_scotts_button_clicked(GtkButton *button, gpointer user_data) {

	linkedlist_item_t *currentline= data_storage->head;
	linkedlist_item_t *column;
	linkedlist_t *dataline;

	int column_index = GPOINTER_TO_INT(user_data);

	double mean = 0;
	double var = 0;
	double std_dev = 0;

	double tempdata = 0;

	double bin_width = 0;
	int bin_nums = 0;

	double datamin = DBL_MAX;
	double datamax = DBL_MIN;


	// Get first the mean
	while (currentline != NULL) {
		// Get the dataLine
		dataline = (linkedlist_t *) currentline->item;
		column = linkedlist_item_skipto(dataline,column_index);

		tempdata = atof((char *) column->item);
		mean += tempdata;

		// Get Max and Min to set the bounds..
		if (tempdata < datamin) datamin = tempdata;
		if (tempdata > datamax) datamax = tempdata;


		currentline = currentline->next_item;
	}

	mean = mean / data_storage->size;


	// Get first the mean
	currentline= data_storage->head; // Reset
	while (currentline != NULL) {
		// Get the dataLine
		dataline = (linkedlist_t *) currentline->item;
		column = linkedlist_item_skipto(dataline,column_index);
		tempdata = atof((char *) column->item);

		var += pow(tempdata - mean,2);

		currentline = currentline->next_item;
	}

	var = var / (data_storage->size);
	std_dev = sqrt(var);

	bin_width = (3.49 * std_dev) / pow(data_storage->size,((double)1/(double)3));


	bin_nums = (int) ((datamax-datamin) / bin_width);

	// Only replot if valid..
	if (bin_nums > 0) {
		saved_bin_count = bin_nums;
		plot_col(saved_col_index,saved_data_to_highlight, saved_do_highlight);
	}

}

char **_separate_feature(int flow_index, int feature_index, int *count) {

	linkedlist_t *current_line = (linkedlist_t *) linkedlist_item_get(data_storage,flow_index);
	char *feature = (char *) linkedlist_item_get(current_line,feature_index);

	// Temporary Value for strtok (because strtok really toks..)
	char *tempfeature = malloc(sizeof(char) * strlen(feature)+1);
	strncpy(tempfeature,feature,strlen(feature));
	tempfeature[strlen(feature)] = '\0';

	char *currentfeature;

	*count = 0;
	int i = 0;

	// Get first just the count of values
	currentfeature = strtok(tempfeature,";");
	while(currentfeature != NULL) {
		*count = *count +1;
		currentfeature = strtok(NULL,";");
	}

	char **tuples = malloc(sizeof(char *) * *count);


	// Seperate by delim
	strncpy(tempfeature,feature,strlen(feature));
	tempfeature[strlen(feature)] = '\0';
	currentfeature = strtok(tempfeature,";");

	while(currentfeature != NULL) {

		tuples[i] = malloc(sizeof(char) * (strlen(currentfeature)+1));
		strncpy(tuples[i],currentfeature,strlen(currentfeature));
		tuples[i][strlen(currentfeature)] = '\0';
		i++;

		currentfeature = strtok(NULL,";");
	}

	free(tempfeature);
	free(currentfeature);

	return tuples;
}

/*
 * Clear all the Plots
 */
void clear_plots() {
	// Remove old option panel stuff
	//--------------------------------------------------------------------------------------
	GtkFixed *optionpane_col = GTK_FIXED(gtk_builder_get_object(builder,"coloptions"));
	GtkFixed *optionpane_flow = GTK_FIXED(gtk_builder_get_object(builder,"flowoptions"));

	// Remove all Childs in Container
	GList *node;
	GList *children;
	GtkWidget * current_widget;

	children = gtk_container_get_children(GTK_CONTAINER(optionpane_col));
	for (node = children; node != NULL; node = node->next) {
		current_widget = GTK_WIDGET((GtkWidget*) node->data);
		gtk_container_remove(GTK_CONTAINER(optionpane_col), GTK_WIDGET(current_widget));
	}
	g_list_free(children);


	children = gtk_container_get_children(GTK_CONTAINER(optionpane_flow));
	for (node = children; node != NULL; node = node->next) {
		current_widget = GTK_WIDGET((GtkWidget*) node->data);
		gtk_container_remove(GTK_CONTAINER(optionpane_flow), GTK_WIDGET(current_widget));
	}
	g_list_free(children);
	//--------------------------------------------------------------------------------------

}

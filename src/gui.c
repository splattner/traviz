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
 * This File handles all GTK Signals
 * So when a Signal is catched, the data's get's prepared and then the functions
 * for displaying or plotting data's is called.
 *
 */

#include "gui.h"

Plotarea * plotarea_col;
Plotarea * plotarea_flow;

linkedlist_t * feature_filters;
linkedlist_t * table_filters;


/*
 * Private Prototypes
 */

int _column_get_index(GtkTreeView *view, guint x, guint y);
gboolean on_button_event_flow(GtkWidget *widget, GdkEventButton *event, gpointer user_data);

typedef struct motionevent_s{
	double x;
	double y;
} motionevent_t;

motionevent_t start_event;
motionevent_t stop_event;

int motion_button_down = 0;
int turn_counter = 0;
int motion_start = 0;

gint ev_plotwindow_conf_col(GtkWidget *widget, GdkEventConfigure *ev, gpointer *data) {


	if (plotarea_col->plotWindow_pixmap) { gdk_pixmap_unref(plotarea_col->plotWindow_pixmap); }

	plotarea_col->plotWindow_pixmap = gdk_pixmap_new(widget->window, widget->allocation.width, widget->allocation.height, -1);

	gdk_draw_rectangle(plotarea_col->plotWindow_pixmap, widget->style->white_gc, TRUE, 0,0, widget->allocation.width, widget->allocation.height);

	return(TRUE);
}

gint ev_plotwindow_conf_flow(GtkWidget *widget, GdkEventConfigure *ev, gpointer *data) {


	if (plotarea_flow->plotWindow_pixmap) { gdk_pixmap_unref(plotarea_flow->plotWindow_pixmap); }

	plotarea_flow->plotWindow_pixmap = gdk_pixmap_new(widget->window, widget->allocation.width, widget->allocation.height, -1);

	gdk_draw_rectangle(plotarea_flow->plotWindow_pixmap, widget->style->white_gc, TRUE, 0,0, widget->allocation.width, widget->allocation.height);

	return(TRUE);
}

gint ev_plotwindow_expose_col(GtkWidget *widget, GdkEventExpose *ev, gpointer *data) {

	gdk_draw_pixmap(widget->window,
			widget->style->fg_gc[GTK_WIDGET_STATE(widget)],
			plotarea_col->plotWindow_pixmap, ev->area.x, ev->area.y, ev->area.x, ev->area.y,
			ev->area.width, ev->area.height);

	if (saved_col_index > 0) { // Replot if Valid Col Index
		plot_col(saved_col_index,saved_data_to_highlight,saved_do_highlight);
	}

	return(TRUE);
}


gint ev_plotwindow_expose_flow(GtkWidget *widget, GdkEventExpose *ev, gpointer *data) {


	gdk_draw_pixmap(widget->window,
			widget->style->fg_gc[GTK_WIDGET_STATE(widget)],
			plotarea_flow->plotWindow_pixmap, ev->area.x, ev->area.y, ev->area.x, ev->area.y,
			ev->area.width, ev->area.height);

	plot_flow(saved_plot_from_treeview);

	return(TRUE);
}

/*
 * init Some GUI based Stuff
 */
void gui_init() {

	GtkBox *box_flow = GTK_BOX(gtk_builder_get_object(builder,"conFlowGraphics"));
	GtkBox *box_col = GTK_BOX(gtk_builder_get_object(builder,"conColGraphics"));


	plotarea_col = malloc(sizeof(Plotarea));
	plotarea_flow = malloc(sizeof(Plotarea));

	// Create Drawing Areas
	plotarea_col->plotWindow = GTK_WIDGET(gtk_drawing_area_new());
	plotarea_col->plotWindow_pixmap = NULL;
	plotarea_flow->plotWindow = GTK_WIDGET(gtk_drawing_area_new());
	plotarea_flow->plotWindow_pixmap = NULL;

	// Pack the widgets in the boxes
	gtk_box_pack_end(GTK_BOX(box_flow),GTK_WIDGET(plotarea_flow->plotWindow),TRUE,TRUE,0);
	gtk_box_pack_end(GTK_BOX(box_col),GTK_WIDGET(plotarea_col->plotWindow),TRUE,TRUE,0);

	// The widget needs to trigger Button Press, Motion and Release Event, so the mouse movement in 3D mode works
	gtk_widget_add_events(GTK_WIDGET(plotarea_flow->plotWindow),GDK_BUTTON_PRESS_MASK);
	gtk_widget_add_events(GTK_WIDGET(plotarea_flow->plotWindow),GDK_BUTTON_RELEASE_MASK);
	gtk_widget_add_events(GTK_WIDGET(plotarea_flow->plotWindow),GDK_POINTER_MOTION_MASK);

	// Set the size request
	gtk_widget_set_size_request(GTK_WIDGET(plotarea_col->plotWindow),640,480);
	gtk_widget_set_size_request(GTK_WIDGET(plotarea_flow->plotWindow),640,480);


	// Connect all Signals
	g_signal_connect(G_OBJECT(plotarea_col->plotWindow), "configure-event",G_CALLBACK(ev_plotwindow_conf_col), NULL);
	g_signal_connect(G_OBJECT(plotarea_col->plotWindow), "expose-event",G_CALLBACK(ev_plotwindow_expose_col), NULL);
	g_signal_connect(G_OBJECT(plotarea_flow->plotWindow), "configure-event",G_CALLBACK(ev_plotwindow_conf_flow), NULL);
	g_signal_connect(G_OBJECT(plotarea_flow->plotWindow), "expose-event",G_CALLBACK(ev_plotwindow_expose_flow), NULL);

	g_signal_connect(GTK_WIDGET(plotarea_flow->plotWindow),"button-press-event",G_CALLBACK(on_button_event_flow),GINT_TO_POINTER(0));
	g_signal_connect(GTK_WIDGET(plotarea_flow->plotWindow),"button-release-event",G_CALLBACK(on_button_event_flow),GINT_TO_POINTER(1));
	g_signal_connect(GTK_WIDGET(plotarea_flow->plotWindow), "motion-notify-event", G_CALLBACK(on_mouse_motion_event),NULL);
}


gboolean on_mouse_motion_event (GtkWidget *widget, GdkEventMotion *event, gpointer user_data) {

	int mouse_x, mouse_y;

	gtk_widget_get_pointer(GTK_WIDGET(widget), &mouse_x, &mouse_y);

	if(saved_flow_plot == FLOW_CHART_TYPE_3DHISTO) {
		if (motion_button_down == 1) {

			if (motion_start == 0) {
				motion_start = 1;
				start_event.x = mouse_x;
				start_event.y = mouse_y;
			}

			if (turn_counter % 5 == 0) { // Redraw every 5 Steps
				stop_event.x = mouse_x;
				stop_event.y = mouse_y;

				int turnby_az = (int) ((stop_event.x - start_event.x) / 200 * 90);
				int turnby_alt = (int) ((stop_event.y - start_event.y) / 150 * 90);

//				// Max turn is 45 Degrees
//				if (turnby_az > 45) turnby_az = 45;
//				if (turnby_az < -45) turnby_az = -45;
//
//				if (turnby_alt > 45) turnby_alt = 45;
//				if (turnby_alt < -45) turnby_alt = -45;

				// Limit the angles to a max and min
				view_angle_az += turnby_az;
				if (view_angle_az < 0) view_angle_az = 0;
				if (view_angle_az > 90) view_angle_az = 90;

				view_angle_alt += turnby_alt;
				if (view_angle_alt < 0) view_angle_alt = 1;
				if (view_angle_alt > 90) view_angle_alt = 90;

				// Plot Again
				plot_flow(saved_plot_from_treeview);

				motion_start = 0;

			}

			turn_counter++;
		}
	}
	return (TRUE);
}

/*
 * Move the 3D Plot on Mouse Movement
 */
gboolean on_button_event_flow(GtkWidget *widget, GdkEventButton *event, gpointer user_data) {

	// Do this only when the 3D Histo is displayd
	if(saved_flow_plot == FLOW_CHART_TYPE_3DHISTO) {

		if (GPOINTER_TO_INT(user_data) == 0) {
			motion_button_down = 1;
		} else {
			motion_button_down = 0;
			motion_start = 0;
			turn_counter = 0;

			// Replot to show bins again
			plot_flow(saved_plot_from_treeview);

		}
	}

	return FALSE;
}



/*
 * Called when the LogAxis ToggleButton is toggled.
 * Change between Log Axis and Linear Axis
 */
void on_logaxis_switch_toogle(GtkCheckButton *logaxis_switch, gpointer user_data) {

	saved_is_log_axis = (int) gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(logaxis_switch));

	if (saved_col_index > 0) {
		plot_col(saved_col_index,saved_data_to_highlight, saved_do_highlight);
	}
}

/*
 * Called when a new value for the bin count is enteret in the Entry Box
 */
void on_histogramm_bin_count_textbox_activate (GtkEntry *textbox, gpointer user_data) {

	int bin_count = atoi(gtk_entry_get_text(GTK_ENTRY(textbox)));

	// Only replot if valid..
	if (bin_count > 0) {
		saved_bin_count = bin_count;
		plot_col(saved_col_index,saved_data_to_highlight, saved_do_highlight);
	}
}

/*
 * Called when a new value for the bin count is set in the Entry Box
 */
void on_per_aggregation_textbox_activate (GtkEntry *textbox, gpointer user) {

	double per_aggregation = atof(gtk_entry_get_text(GTK_ENTRY(textbox)));

	// Only replot if valid..
	if (per_aggregation > 0 && per_aggregation <= 100) {
		saved_per_aggregation = per_aggregation;
		plot_col(saved_col_index,saved_data_to_highlight, saved_do_highlight);
	}
}

void on_bound_update (GtkEntry *textbox, gpointer user_data) {

	double new_value;

	switch(GPOINTER_TO_INT(user_data)) {
	case 0:
		new_value = atof(gtk_entry_get_text(GTK_ENTRY(textbox)));
		saved_xmin = new_value;
		break;
	case 1:
		new_value = atof(gtk_entry_get_text(GTK_ENTRY(textbox)));
		saved_xmax = new_value;
		break;
	case 2:
		new_value = atof(gtk_entry_get_text(GTK_ENTRY(textbox)));
		saved_ymin = new_value;
		break;
	case 3:
		new_value = atof(gtk_entry_get_text(GTK_ENTRY(textbox)));
		saved_ymax = new_value;
		break;
	case 4:
		new_value = atof(gtk_entry_get_text(GTK_ENTRY(textbox)));
		saved_y2min = new_value;
		break;
	case 5:
		new_value = atof(gtk_entry_get_text(GTK_ENTRY(textbox)));
		saved_y2max = new_value;
		break;
	}

	plot_flow(saved_plot_from_treeview);
}

void on_bound_update_3d (GtkEntry *textbox, gpointer user_data) {

	double new_value;

	switch(GPOINTER_TO_INT(user_data)) {
	case 0:
		new_value = atof(gtk_entry_get_text(GTK_ENTRY(textbox)));
		saved_xmin_3d = new_value;
		break;
	case 1:
		new_value = atof(gtk_entry_get_text(GTK_ENTRY(textbox)));
		saved_xmax_3d = new_value;
		break;
	case 2:
		new_value = atof(gtk_entry_get_text(GTK_ENTRY(textbox)));
		saved_ymin_3d = new_value;
		break;
	case 3:
		new_value = atof(gtk_entry_get_text(GTK_ENTRY(textbox)));
		saved_ymax_3d = new_value;
		break;
	case 4:
		new_value = atof(gtk_entry_get_text(GTK_ENTRY(textbox)));
		saved_zmin_3d = new_value;
		break;
	case 5:
		new_value = atof(gtk_entry_get_text(GTK_ENTRY(textbox)));
		saved_zmax_3d = new_value;
		break;
	case 6:
		new_value = atof(gtk_entry_get_text(GTK_ENTRY(textbox)));
		saved_packet_size = new_value;
		break;
	case 7:
		new_value = atof(gtk_entry_get_text(GTK_ENTRY(textbox)));
		saved_iat_with = new_value;
		break;
	}

	plot_flow(saved_plot_from_treeview);
}

/*
 * Called when the Combined Line ToggleButton is toggled.
 * Change in Flow mode between a combined and a non combined XY-Line
 */
void on_combined_line_switch_toogle(GtkCheckButton *combined_line_switch, gpointer user_data) {


	saved_combined_line = (int) gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(combined_line_switch));
	saved_reset_bounds = 1;

	plot_flow(saved_plot_from_treeview);
}


/*
 * Quit the main Programm when the window is destroyed!
 */
void on_mainwindow_destroy (GtkObject *object, gpointer user_data) {
	g_object_unref (G_OBJECT (builder));
  	gtk_main_quit ();
}

/*
 * Show the open File Dialog to select the files for opening
 */
void on_mainmenu_open_button_clicked(GtkToolButton *button, gpointer user_data) {
	GtkWidget *dialog = GTK_WIDGET(gtk_builder_get_object(builder,"openFlowFileDialog"));
	gtk_widget_show(dialog);
}

/*
 * Clean all Stores and all Plots
 */
void on_mainmenu_new_button_clicked(GtkToolButton *button, gpointer user_data) {
	clean_stores();
	clear_plots();

	clear_linkedlist(table_filters,1);
	clear_linkedlist(feature_filters,1);
}

/*
 * Clean all Stores and all Plots
 */
void on_mainmenu_table_button_clicked(GtkToolButton *button, gpointer user_data) {
	GtkWidget *window_table = GTK_WIDGET(gtk_builder_get_object(builder,"window_table"));
	gtk_widget_show_all(window_table);
}

void on_table_close_button_clicked(GtkObject *object, gpointer user_data) {
	GtkWidget *window_table = GTK_WIDGET(gtk_builder_get_object(builder,"window_table"));
	gtk_widget_hide_all(window_table);
}

void on_table_saveas_button_clicked(GtkButton *button, gpointer user_data) {


	GtkWidget *dialog = GTK_WIDGET(gtk_builder_get_object(builder,"saveFlowFileDialog"));
	gtk_widget_show(dialog);
}

void on_table_filter_button_clicked(GtkObject *object, gpointer user_data) {


	if (header_storage != NULL) {
		open_filter_dialog(table_filters);
	} else {
		GtkWidget *dialog = gtk_message_dialog_new (GTK_WINDOW(window),
										GTK_DIALOG_DESTROY_WITH_PARENT,
										GTK_MESSAGE_ERROR,
			                            GTK_BUTTONS_CLOSE,
			                            "No Flow File loaded. You have to load one first!"
			                            );

		/* Destroy the dialog when the user responds to it (e.g. clicks a button) */
		g_signal_connect_swapped (dialog, "response", G_CALLBACK (gtk_widget_destroy), dialog);
		gtk_dialog_run(GTK_DIALOG(dialog));
	}


}

void on_feature_filter_button_clicked(GtkObject *object, gpointer user_data) {


	if (header_storage != NULL) {
		open_filter_dialog(feature_filters);
	} else {
		GtkWidget *dialog = gtk_message_dialog_new (GTK_WINDOW(window),
										GTK_DIALOG_DESTROY_WITH_PARENT,
										GTK_MESSAGE_ERROR,
			                            GTK_BUTTONS_CLOSE,
			                            "No Flow File loaded. You have to load one first!"
			                            );

		/* Destroy the dialog when the user responds to it (e.g. clicks a button) */
		g_signal_connect_swapped (dialog, "response", G_CALLBACK (gtk_widget_destroy), dialog);
		gtk_dialog_run(GTK_DIALOG(dialog));
	}


}

/*
 * Close Button in the open File Dialog, do nothing just close the widget
 */
void on_saveas_button_close_clicked(GtkObject *object, gpointer user_data) {
	GtkWidget *dialog =  GTK_WIDGET(gtk_builder_get_object(builder,"saveFlowFileDialog"));
	gtk_widget_hide(dialog);
}

/*
 * Close the open File Dialog, load the Files and build the ListStores
 */
void on_saveas_button_save_clicked(GtkObject *object, gpointer user_data) {

	// Get The FileChooser Dialog from the GUI
	GtkFileChooser *file_chooser =  GTK_FILE_CHOOSER(gtk_builder_get_object(builder,"saveFlowFileDialog"));
	gtk_widget_hide(GTK_WIDGET(file_chooser));

	gchar * file_chooser_filename = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(file_chooser));

	GtkTreeView *treeview = GTK_TREE_VIEW(gtk_builder_get_object(builder,"treeview_table"));

	extract_flows_to_new_file(treeview,file_chooser_filename);
}



gboolean on_window_table_delete_event (GtkWidget *widget,GdkEvent *event, gpointer user_data) {
	GtkWidget *window_table = GTK_WIDGET(gtk_builder_get_object(builder,"window_table"));
	gtk_widget_hide_all(window_table);
	return TRUE;
}


/*
 * Show Detail FlowData on selecting new FlowRow
 */
void on_treeDataFlow_cursor_changed(GtkTreeView *treeview, gpointer user_data) {

	saved_reset_bounds = 1;

	plot_flow(treeview);
	detaildata_set_flow(treeview);
}



gboolean on_treeDataFlow_select_all (GtkTreeView *treeview, gpointer user_data) {

	// Select all rows in TreeView
	GtkTreeSelection * tsel = gtk_tree_view_get_selection (treeview);
	gtk_tree_selection_select_all(tsel);

	saved_reset_bounds = 1;

	plot_flow(treeview);
	detaildata_set_flow(treeview);

	return TRUE;
}

/*
 *
 */
void on_flow_plot_combo_changed(GtkComboBox *combo_box, gpointer user_data) {

	// Get Active of the ComboBox to select the current Visualisation in Flow Mode
	saved_flow_plot = (int) gtk_combo_box_get_active(combo_box);

	if (saved_flow_plot == FLOW_CHART_TYPE_3DHISTO) { // Reset if change to 3DHisto
		view_angle_az = 45;
		view_angle_alt = 25;
	}

	saved_reset_bounds = 1;
	plot_flow(saved_plot_from_treeview);
}

/*
 * Show Detail ColData on selecting new FlowRow
 */
void on_treeColData_cursor_changed(GtkTreeView *treeview, gpointer user_data) {

	GtkComboBox *combobox = GTK_COMBO_BOX(gtk_builder_get_object(builder,"comboColSelect"));
	int active_Index = (int) gtk_combo_box_get_active(combobox);

	/*
	 * Get The position of the selected Row
	 */
	GtkTreeSelection * tsel = gtk_tree_view_get_selection (treeview);
	GtkTreeIter iter;
	GtkTreeModel * tm;


	int index;
	if ( gtk_tree_selection_get_selected ( tsel , &tm , &iter ) )
	{
		gtk_tree_model_get (tm, &iter, 0, &index, -1);
	}

	linkedlist_t *current_dataline = (linkedlist_t*) linkedlist_item_get(data_storage,index);
	linkedlist_item_t * current_item = linkedlist_item_skipto(current_dataline,active_Index);

	// Check if index is valid
	if (index < data_storage->size) {
		detaildata_set_col(index);
	}

	plot_col(active_Index, atof((char *)current_item->item),1);
}

/*
 * Close Button in the open File Dialog, do nothing just close the widget
 */
void on_closeButton_clicked(GtkObject *object, gpointer user_data) {
	GtkWidget *dialog =  GTK_WIDGET(gtk_builder_get_object(builder,"openFlowFileDialog"));
	gtk_widget_hide(dialog);
}

/*
 * Close the open File Dialog, load the Files and build the ListStores
 */
void on_loadButton_clicked(GtkObject *object, gpointer user_data) {

	// Get The FileChooser Dialog from the GUI
	GtkFileChooser *file_chooser = GTK_FILE_CHOOSER(gtk_builder_get_object(builder,"openFlowFileDialog"));
	gtk_widget_hide(GTK_WIDGET(file_chooser));

	gchar * file_chooser_filename = gtk_file_chooser_get_filename(file_chooser);

#if DEBUG != 0
	printf("on_loadButton_clicked: Filename: %s\n", file_chooser_filename);
#endif
	// Open and Parse the Files -> This creates the Data Storage
	if( open_files(file_chooser_filename) != -1) {


		// Clean old stuff
		clean_stores();
		clear_plots();

		clear_linkedlist(table_filters,1);
		table_filters = linkedlist_new();
		clear_linkedlist(feature_filters,1);
		feature_filters = linkedlist_new();

		// Build all List Store's for displaying in the GTK Window
		build_liststore_flows();
		build_liststore_flow_details();
		build_liststore_col_details();
		build_liststore_table();
		build_headercombobox_model();

		// Set the Flow Data
		flowdata_set();
		tabledata_set();

	}
}

/*
 * Change in the ComboBox
 */
void on_comboColSelect_changed(GtkObject *object, gpointer user_data) {

	// Get The ComboBox from the GUI
	GtkComboBox *combobox = GTK_COMBO_BOX(gtk_builder_get_object(builder,"comboColSelect"));

	int active_index = gtk_combo_box_get_active(combobox);

	build_liststore_cols(active_index);
	coldata_set(active_index);
	plot_col(active_index, 0,0);
}

/*
 * Change tab to FlowMode
 */
void on_flowmode_button(GtkToggleToolButton *button, gpointer user_data) {

	GtkToggleToolButton *button_flowmode = GTK_TOGGLE_TOOL_BUTTON(gtk_builder_get_object(builder,"button_flowmode"));


	if (gtk_toggle_tool_button_get_active(button_flowmode) == 1) {
		GtkToggleToolButton *button_colmode = GTK_TOGGLE_TOOL_BUTTON(gtk_builder_get_object(builder,"button_colmode"));
		GtkNotebook *nb = GTK_NOTEBOOK(gtk_builder_get_object(builder,"nbModes"));
		gtk_notebook_set_current_page(nb,0);
		gtk_toggle_tool_button_set_active(button_colmode,0);
	}
}

/*
 * Change Tab to ColMode
 */
void on_colmode_button(GtkToggleToolButton *button, gpointer user_data) {
	GtkToggleToolButton *button_colmode = GTK_TOGGLE_TOOL_BUTTON(gtk_builder_get_object(builder,"button_colmode"));

	if (gtk_toggle_tool_button_get_active(button_colmode) == 1) {
		GtkToggleToolButton *button_flowmode = GTK_TOGGLE_TOOL_BUTTON(gtk_builder_get_object(builder,"button_flowmode"));
		GtkNotebook *nb = GTK_NOTEBOOK(gtk_builder_get_object(builder,"nbModes"));
		gtk_notebook_set_current_page(nb,1);
		gtk_toggle_tool_button_set_active(button_flowmode,0);
	}
}

/*
 * Not used at the moment
 */
void on_treeDetailFlow_button_press_event (GtkWidget *view, GdkEventButton *bevent, gpointer data) {
	//int index = _column_get_index(GTK_TREE_VIEW(view), bevent->x, bevent->y);
}

/*
 * Return the index of a column in the GtkTreeView witch was at pos x/y of a click-Event in the TreeView
 */
int _column_get_index(GtkTreeView *view, guint x, guint y) {
	GtkTreeViewColumn *col = NULL;
	GList *node, *columns;
	int colx = 0;

	columns = gtk_tree_view_get_columns(view);

	int i = 0;

	for (node = columns;  node != NULL && col == NULL;  node = node->next)	{
		GtkTreeViewColumn *checkcol = (GtkTreeViewColumn*) node->data;

		if (x >= colx  &&  x < (colx + checkcol->width)) { col = checkcol;
		} else { colx += checkcol->width; }

		i++;
	}

	g_list_free(columns);

	return i-1;
}

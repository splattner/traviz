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
 * Filter Function for Feature Mode and Table View
 */

#include "filter.h"

gboolean filter_onButtonPressed (GtkWidget *treeview, GdkEventButton *event, gpointer userdata) {

	if (event->type == GDK_BUTTON_PRESS  &&  event->button == 3) {
		filter_popup_menu(treeview, event, userdata);
		return TRUE; /* we handled this */
	}

	return FALSE; /* we did not handle this */

}

gboolean filter_onPopupMenu(GtkWidget *treeview, gpointer userdata) {

	filter_popup_menu(treeview, NULL, userdata);
	return TRUE;
}

void filter_popup_menu(GtkWidget *treeview, GdkEventButton *event, gpointer userdata) {

	GtkWidget *menu, *menuitem;

	menu = gtk_menu_new();
	menuitem = gtk_menu_item_new_with_label("Delete selected filter");
	g_signal_connect(menuitem, "activate", (GCallback) filter_popup_menu_remove_selected, treeview);
	gtk_menu_shell_append(GTK_MENU_SHELL(menu), menuitem);

	menuitem = gtk_menu_item_new_with_label("Add new filter");
	g_signal_connect(menuitem, "activate", (GCallback) filter_popup_menu_new_filter, treeview);
	gtk_menu_shell_append(GTK_MENU_SHELL(menu), menuitem);

	gtk_widget_show_all(menu);

	gtk_menu_popup(GTK_MENU(menu),NULL,NULL,NULL,NULL,0,gdk_event_get_time((GdkEvent*)event));
}

void filter_popup_menu_new_filter (GtkWidget *menuitem, gpointer userdata) {

	GtkTreeView *treeview = GTK_TREE_VIEW(userdata);
	filter_t * filter;

	filter = malloc(sizeof(filter_t));
	set_filter_feature(filter, 0);
	filter->filter = FILTER_FUNC_EQ;

	linkedlist_add_item(current_filter_list, linkedlist_item_new((void*) filter));

	// Reload Treeview
	load_filter_in_treeview(current_filter_list, treeview);

}

void filter_popup_menu_remove_selected (GtkWidget *menuitem, gpointer userdata) {

	GtkTreeView *treeview = GTK_TREE_VIEW(userdata);


	GtkTreeSelection * tsel = gtk_tree_view_get_selection (treeview);
	GtkTreeIter iter;
	GtkTreeModel * tm;
	GtkTreePath * path ;

	int  *i;

	if ( gtk_tree_selection_get_selected ( tsel , &tm , &iter ) ) { // If a row is selected
		path = gtk_tree_model_get_path ( tm , &iter ) ;
		i = gtk_tree_path_get_indices ( path );

		// Clean filter
		linkedlist_item_t * current_filter_item = linkedlist_item_skipto(current_filter_list, *i);
		filter_t * current_filter = (filter_t *) current_filter_item->item;

		free(current_filter->value_string);
		free(current_filter_item->item);

		remove_linked_list_item_at_index(current_filter_list,*i);

		// Reload Filter list into Treeview
		load_filter_in_treeview(current_filter_list, treeview);
	}


}

void open_filter_dialog(linkedlist_t * filters) {
	GtkWidget *window_filter = GTK_WIDGET(gtk_builder_get_object(builder,"window_filter"));
	GtkTreeView *treeview_filter = GTK_TREE_VIEW(gtk_builder_get_object(builder,"treeview_filter"));

	// Save current used filter, so only one window can be used for all filters
	current_filter_list = filters;

	load_filter_in_treeview(filters, treeview_filter);

	gtk_widget_show_all(window_filter);
}

void on_filter_apply_button(GtkObject *object, gpointer user_data) {

	// Update the Table
	if (current_filter_list == table_filters) {
		tabledata_set();
	}

	// Update Feature Mode incl. Plot
	if (current_filter_list == feature_filters) {
		// Get The ComboBox from the GUI
		GtkComboBox *combobox = GTK_COMBO_BOX(gtk_builder_get_object(builder,"comboColSelect"));

		int active_index = gtk_combo_box_get_active(combobox);

		build_liststore_cols(active_index);
		coldata_set(active_index);
		plot_col(active_index, 0,0);

	}

}

void on_filter_close_button(GtkWidget *object, gpointer user_data) {
	GtkWidget *window_filter = GTK_WIDGET(gtk_builder_get_object(builder,"window_filter"));
	gtk_widget_hide_all(window_filter);
}


gboolean on_window_filter_delete_event (GtkWidget *widget,GdkEvent *event, gpointer user_data) {
	on_filter_close_button(widget, user_data);
	return TRUE;
}

void filter_edited_callback (GtkCellRendererText *cell,gchar *path_string, gchar *new_text,gpointer user_data) {

	GtkTreeView *treeview = GTK_TREE_VIEW(user_data);
	GtkTreeModel *tm = gtk_tree_view_get_model(treeview);
	GtkTreePath *path = gtk_tree_path_new_from_string(path_string);
	GtkTreeIter iter;

	gtk_tree_model_get_iter(tm, &iter, path);

	// Get edited Row
	int row = gtk_tree_path_get_indices (path)[0];

	linkedlist_item_t * current_filter_item = current_filter_list->head;
	filter_t * current_filter = (filter_t *) current_filter_item->item;

	int i;
	for (i = 0; i < row; i++) {
		current_filter_item = current_filter_item->next_item;
	}

	current_filter = (filter_t *) current_filter_item->item;

	if (strcmp(new_text, "Greater then") == 0) {
		gtk_list_store_set (GTK_LIST_STORE (tm), &iter, FILTER_TYPE_STRING, new_text, -1);
		current_filter->filter = FILTER_FUNC_GT;
	}
	if (strcmp(new_text, "Greater or equal then") == 0) {
		gtk_list_store_set (GTK_LIST_STORE (tm), &iter, FILTER_TYPE_STRING, new_text, -1);
		current_filter->filter = FILTER_FUNC_GET;
	}
	if (strcmp(new_text, "Smaller then") == 0) {
		gtk_list_store_set (GTK_LIST_STORE (tm), &iter, FILTER_TYPE_STRING, new_text, -1);
		current_filter->filter = FILTER_FUNC_ST;
	}
	if (strcmp(new_text, "Smaller or equal then") == 0) {
		gtk_list_store_set (GTK_LIST_STORE (tm), &iter, FILTER_TYPE_STRING, new_text, -1);
		current_filter->filter = FILTER_FUNC_SET;
	}
	if (strcmp(new_text, "Equal then") == 0) {
		gtk_list_store_set (GTK_LIST_STORE (tm), &iter, FILTER_TYPE_STRING, new_text, -1);
		current_filter->filter = FILTER_FUNC_EQ;
	}
	if (strcmp(new_text, "Not equal then") == 0) {
		gtk_list_store_set (GTK_LIST_STORE (tm), &iter, FILTER_TYPE_STRING, new_text, -1);
		current_filter->filter = FILTER_FUNC_NEQ;
	}



	gtk_tree_path_free (path);

}

void feature_edited_callback (GtkCellRendererText *cell,gchar *path_string, gchar *new_text,gpointer user_data) {
	GtkTreeView *treeview = GTK_TREE_VIEW(user_data);
	GtkTreeModel *tm = gtk_tree_view_get_model(treeview);
	GtkTreePath *path = gtk_tree_path_new_from_string(path_string);
	GtkTreeIter iter;

	// Get edited Column
	int column = GPOINTER_TO_INT(g_object_get_data(G_OBJECT(cell), "column"));

	gtk_tree_model_get_iter(tm, &iter, path);

	// Get edited Row
	int row = gtk_tree_path_get_indices (path)[0];

	linkedlist_item_t * current_filter_item = current_filter_list->head;
	filter_t * current_filter = (filter_t *) current_filter_item->item;

	int i;
	for (i = 0; i < row; i++) { current_filter_item = current_filter_item->next_item; }
	current_filter = (filter_t *) current_filter_item->item;

	i = get_index_by_columnname(new_text);

	switch (column) {
	case 0:
		gtk_list_store_set(GTK_LIST_STORE (tm), &iter, FILTER_FEATURE_STRING, new_text, -1);
		set_filter_feature(current_filter,i);
		break;
//	case 1: This is handled by filter_edited_callback
	case 2:
		set_filter_value(current_filter, new_text);
		break;
	}

	// Reload Value
	gtk_list_store_set(GTK_LIST_STORE (tm), &iter, FILTER_VALUE, get_filter_value_as_string(current_filter), -1);

	gtk_tree_path_free (path);

}

/*
 * Load a filter list in a treeview
 */
void load_filter_in_treeview(linkedlist_t * filters, GtkTreeView *treeview_filter ) {

	GtkTreeViewColumn *col;

	// Remove Model from treeview
	gtk_tree_view_set_model(GTK_TREE_VIEW(treeview_filter), NULL);

	/* Clean Old Treeview */
	GList *column_list;

	column_list = gtk_tree_view_get_columns(treeview_filter);
	while(column_list) {
		col = column_list->data;
		if (treeview_filter) gtk_tree_view_remove_column(treeview_filter, col);
		column_list = g_list_next(column_list);
	}

	/* Build Model */
	GtkTreeIter iter;
	GtkListStore * filter_store = gtk_list_store_new (COLUMN_NUMBER, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING);

	linkedlist_item_t * current_filter_item = filters->head;
	filter_t * current_filter;

	linkedlist_item_t * feature_item;
	column_type_t * feature;

	while (current_filter_item != NULL) {

		gtk_list_store_append(filter_store, &iter);

		current_filter = (filter_t *) current_filter_item->item;

		feature_item = linkedlist_item_skipto(header_storage, current_filter->column_index);
		feature = (column_type_t *) feature_item->item;

		// Feature
		gtk_list_store_set(filter_store,&iter, FILTER_FEATURE_STRING, feature->name, -1);

		switch (current_filter->filter) {
		case FILTER_FUNC_GT:
			gtk_list_store_set(filter_store,&iter, FILTER_TYPE_STRING, "Greater then", -1);
			break;
		case FILTER_FUNC_GET:
			gtk_list_store_set(filter_store,&iter, FILTER_TYPE_STRING, "Greater or equal then", -1);
			break;
		case FILTER_FUNC_ST:
			gtk_list_store_set(filter_store,&iter, FILTER_TYPE_STRING, "Smaller then", -1);
			break;
		case FILTER_FUNC_SET:
			gtk_list_store_set(filter_store,&iter, FILTER_TYPE_STRING, "Smaller or equal then", -1);
			break;
		case FILTER_FUNC_EQ:
			gtk_list_store_set(filter_store,&iter, FILTER_TYPE_STRING, "Equal then", -1);
			break;
		case FILTER_FUNC_NEQ:
			gtk_list_store_set(filter_store,&iter, FILTER_TYPE_STRING, "Not equal then", -1);
			break;
		}

		// Filter
		gtk_list_store_set(filter_store,&iter, FILTER_VALUE, get_filter_value_as_string(current_filter), -1);

		// Value
		current_filter_item = current_filter_item->next_item;
	}


	gtk_tree_view_set_model(GTK_TREE_VIEW(treeview_filter), GTK_TREE_MODEL(filter_store));

	// Build Model for Feature Combo Box
	GtkListStore *feature_combo_store = gtk_list_store_new(1, G_TYPE_STRING);

	linkedlist_item_t *current= header_storage->head;
	column_type_t *item;

	// Add each Header to the ComboBox Model
	while (current != NULL) {
		item = (column_type_t *) current->item;

		gtk_list_store_append(feature_combo_store, &iter);
		gtk_list_store_set(feature_combo_store,&iter, 0, item->name, -1);

		current = current->next_item;
	}

	// Build Model for Filter Combo Box
	GtkListStore *filter_combo_store = gtk_list_store_new(1, G_TYPE_STRING);

	gtk_list_store_append(filter_combo_store, &iter);
	gtk_list_store_set(filter_combo_store, &iter, 0, "Greater then", -1);
	gtk_list_store_append(filter_combo_store, &iter);
	gtk_list_store_set(filter_combo_store, &iter, 0, "Greater or equal then", -1);
	gtk_list_store_append(filter_combo_store, &iter);
	gtk_list_store_set(filter_combo_store, &iter, 0, "Smaller then", -1);
	gtk_list_store_append(filter_combo_store, &iter);
	gtk_list_store_set(filter_combo_store, &iter, 0, "Smaller or equal then", -1);
	gtk_list_store_append(filter_combo_store, &iter);
	gtk_list_store_set(filter_combo_store, &iter, 0, "Equal then", -1);
	gtk_list_store_append(filter_combo_store, &iter);
	gtk_list_store_set(filter_combo_store, &iter, 0, "Not equal then", -1);




	/* Build Colums */
	GtkCellRenderer *renderer;


	// Feature Col
	renderer = gtk_cell_renderer_combo_new();
	g_object_set(G_OBJECT(renderer),
					"model", GTK_TREE_MODEL(feature_combo_store),
					"text-column", 0,
					"editable", TRUE,
					"has-entry", FALSE,
					NULL);
	g_signal_connect(renderer, "edited", (GCallback) feature_edited_callback, treeview_filter);

	g_object_set_data(G_OBJECT (renderer), "column", GINT_TO_POINTER (0));

	col = gtk_tree_view_column_new_with_attributes("Feature",renderer,
													"text", FILTER_FEATURE_STRING,
													NULL);
	gtk_tree_view_append_column(GTK_TREE_VIEW(treeview_filter), col);



	// Filter Function
	renderer = gtk_cell_renderer_combo_new();
	g_object_set(G_OBJECT(renderer),
					"model", GTK_TREE_MODEL(filter_combo_store),
					"text-column", 0,
					"editable", TRUE,
					"has-entry", FALSE,
					NULL);
	g_signal_connect(renderer, "edited", (GCallback) filter_edited_callback, treeview_filter);

	g_object_set_data(G_OBJECT (renderer), "column", GINT_TO_POINTER (1));

	col = gtk_tree_view_column_new_with_attributes("Filter",renderer,
													"text", FILTER_TYPE_STRING,
													NULL);
	gtk_tree_view_append_column(GTK_TREE_VIEW(treeview_filter), col);



	//Value
	col = gtk_tree_view_column_new();
	gtk_tree_view_column_set_title(col,"Value");
	gtk_tree_view_append_column(GTK_TREE_VIEW(treeview_filter), col);

	renderer = gtk_cell_renderer_text_new();
	g_object_set(GTK_OBJECT(renderer),"editable", TRUE,
									NULL);
	g_object_set_data(G_OBJECT (renderer), "column", GINT_TO_POINTER (2));
	g_signal_connect(renderer, "edited", (GCallback) feature_edited_callback, treeview_filter);
	gtk_tree_view_column_pack_start(col, renderer, TRUE);
	gtk_tree_view_column_add_attribute(col, renderer,"text",2);
}


/*
 * Set the Feature for filter and reset filter value
 */
void set_filter_feature(filter_t * filter, int feature) {
	filter->column_index = feature;
	set_filter_value(filter, '\0');
}


/*
 * Set Value in filter
 * This function makes sure, that value is set according to feature type
 */
void set_filter_value(filter_t * filter, char * value) {

		column_type_t * c_type = (column_type_t *) linkedlist_item_skipto(header_storage,filter->column_index)->item;

		// Set according to type
		switch(c_type->type_short) {
		case TYPE_SHORT_DEC:
			if (value == '\0') filter->value_int = 0;
			else filter->value_int = atoi(value);

			filter->compare_with = compare_with_int;
			break;
		case TYPE_SHORT_FLOAT:
			if (value == '\0') filter->value_double = 0;
			else filter->value_double = atof(value);

			filter->compare_with = compare_with_double;
			break;
		case TYPE_SHORT_COMBINED:
		case TYPE_SHORT_HEX:
		case TYPE_SHORT_NONE:
		case TYPE_SHORT_STRING:
			free(filter->value_string);
			filter->value_string = malloc(sizeof(char) * 50);

			sprintf(filter->value_string, "%s", value);

			filter->compare_with = compare_with_string;
			break;
		}

}

/*
 * Return filter value as a String
 */
char * get_filter_value_as_string(filter_t * filter) {
	column_type_t * c_type = (column_type_t *) linkedlist_item_skipto(header_storage,filter->column_index)->item;

	char * value = malloc(sizeof(char) * 50);

	switch(c_type->type_short) {
	case TYPE_SHORT_DEC:
		sprintf(value,"%d", filter->value_int);
		break;
	case TYPE_SHORT_FLOAT:
		sprintf(value,"%f", filter->value_double);
		break;
	case TYPE_SHORT_COMBINED:
	case TYPE_SHORT_HEX:
	case TYPE_SHORT_NONE:
	case TYPE_SHORT_STRING:
		sprintf(value,"%s", filter->value_string);
		break;
	}


	return value;
}


/*
 * Function to compare filter with a int
 */
int compare_with_int(filter_t * filter, char * value_string) {

	int value = atoi(value_string);

	//printf("Compare with Int! Value: %d, compare to: %d\n", value, filter->value_int);

	switch (filter->filter) {
	case FILTER_FUNC_GT:
		if (value > filter->value_int) return 1;
		break;
	case FILTER_FUNC_GET:
		if (value >= filter->value_int) return 1;
		break;
	case FILTER_FUNC_ST:
		if (value < filter->value_int) return 1;
		break;
	case FILTER_FUNC_SET:
		if (value <= filter->value_int) return 1;
		break;
	case FILTER_FUNC_EQ:
		if (value == filter->value_int) return 1;
		break;
	case FILTER_FUNC_NEQ:
		if (value != filter->value_int) return 1;
		break;
	}

	return 0;

}

/*
 * Function to compare filter with a double
 */
int compare_with_double(filter_t * filter, char * value_string) {

	double value = atof(value_string);

	//printf("Compare with Double! Value: %f, compare to: %f\n", value, filter->value_double);

	switch (filter->filter) {
	case FILTER_FUNC_GT:
		if (value > filter->value_double) return 1;
		break;
	case FILTER_FUNC_GET:
		if (value >= filter->value_double) return 1;
		break;
	case FILTER_FUNC_ST:
		if (value < filter->value_double) return 1;
		break;
	case FILTER_FUNC_SET:
		if (value <= filter->value_double) return 1;
		break;
	case FILTER_FUNC_EQ:
		if (value == filter->value_double) return 1;
		break;
	case FILTER_FUNC_NEQ:
		if (value != filter->value_double) return 1;
		break;
	}

	return 0;

}

/*
 * Function to compare filter with a string
 */
int compare_with_string(filter_t *filter, char * value_string) {

	//printf("Compare with String! Value: %s, compare to: %s\n", value_string, filter->value_string);

	switch (filter->filter) {
	case FILTER_FUNC_GT:
	case FILTER_FUNC_GET:
	case FILTER_FUNC_ST:
	case FILTER_FUNC_SET:
		// These compare functions don't make sense to string.. so return 1 (won't affect other filters..)
		return 1;
		break;
	case FILTER_FUNC_EQ:
		if (strcmp( filter->value_string,value_string) == 0) return 1;
		break;
	case FILTER_FUNC_NEQ:
		if (strcmp(filter->value_string, value_string) != 0) return 1;
		break;
	}

	return 0;
}

/*
 * Check if filter matches the current flow
 */
int filter_check(linkedlist_t * filter_list, linkedlist_item_t * current_flow_item) {

	linkedlist_item_t * current_filter_item;
	filter_t *filter;

	linkedlist_t * current_flow;
	linkedlist_item_t * current_feature_item;

	int match = 1;

	// Reset stuff for filter
	current_filter_item = filter_list->head;

	// Check if filter Match
	while (current_filter_item != NULL && match == 1) { // Loop trought all filters


		filter = (filter_t *) current_filter_item->item;

		current_flow = (linkedlist_t *) current_flow_item->item; // Get current Flow from Datastorage
		current_feature_item = linkedlist_item_skipto(current_flow, filter->column_index); // Get the Feature form the Flow to compare with filter


		// All filter have to match, otherwise flow is not displayed!
		if (match == 1 && filter->compare_with(filter, (char *) current_feature_item->item) == 0) {	match = 0; }

		current_filter_item = current_filter_item->next_item;
	}

	return match;
}

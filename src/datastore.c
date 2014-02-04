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
 * This File handles all Stuff related to Data-Storage and Data-Handling
 * It parses the Output File of Tranalyzer and store them in the internal Data Structures.
 * It also builds the GTK Data Structur for displaying the Data Tables on the GUI
 */


#include "datastore.h"


/*
 * All List Store's needed to display the datas
 */
GtkListStore *liststore_flow;
GtkListStore *liststore_flow_detail;
GtkListStore *liststore_col;
GtkListStore *liststore_col_detail;
GtkListStore *liststore_header;
GtkListStore *liststore_table;

/*
 * Data Storage with the flow and Header Datas
 */
linkedlist_t * data_storage;
linkedlist_t * header_storage;

/*
 * Private Prototypes
 */

void _build_liststore_details(GtkTreeView *treeview, GtkListStore** liststore);
int _parse_binary_file(char filename_binary[]);
int _parse_flowfile(char filename_flow[]);
int _parse_headerfile(char filename_header[]);
void _parse_histofile(char filename_histo[]);
void _treeview_add_column(GtkTreeView * treeview, column_type_t * c_type, gint column, GtkCellRenderer * renderer);
void _detaildata_set(int index, GtkTreeView* treeview, GtkListStore** liststore);
void _clean_treeview(GtkTreeView * treeview);
void _headeritem_add(char *c_type, char *columnname, int index);

/*
 * Create Colums and listStore for the Data View in Flow Mode
 */
void build_liststore_flows() {

	// Get Tree View From GUI
	GtkTreeView *treeview_flow = GTK_TREE_VIEW(gtk_builder_get_object(builder,"treeDataFlow"));

	// Set Selection Mode to Multiple
	GtkTreeSelection * selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(treeview_flow));
	gtk_tree_selection_set_mode(selection, GTK_SELECTION_MULTIPLE);


	linkedlist_item_t * item;

	// Init Colum Types
	column_type_t *c_type;

	GType *types;
	int i = 0; // Column Index
	int n_cols = 8; // Number of Columns
	types = g_new0(GType, n_cols);


	// Build Columns for the first 7 Columns
	// -------------------------------------

	// Index
	_treeview_add_column(treeview_flow, NULL,i, NULL);
	types[i++] = G_TYPE_INT;

	// Direction
	item = linkedlist_item_skipto(header_storage,get_index_by_columnname("Flow direction"));
	c_type = (column_type_t *) item->item;
	_treeview_add_column(treeview_flow, c_type,i, NULL);
	types[i++] = c_type->type;

	// VLANID
	item = linkedlist_item_skipto(header_storage,get_index_by_columnname("VLAN ID"));
	c_type = (column_type_t *) item->item;
	_treeview_add_column(treeview_flow, c_type,i, NULL);
	types[i++] = c_type->type;

	// Source IP Address
	item = linkedlist_item_skipto(header_storage,get_index_by_columnname("Source IP address"));
	c_type = (column_type_t *) item->item;
	_treeview_add_column(treeview_flow, c_type,i, NULL);
	types[i++] = c_type->type;

	// Source Port
	item = linkedlist_item_skipto(header_storage,get_index_by_columnname("Source port"));
	c_type = (column_type_t *) item->item;
	_treeview_add_column(treeview_flow, c_type,i, NULL);
	types[i++] = c_type->type;

	// Destination IP address
	item = linkedlist_item_skipto(header_storage,get_index_by_columnname("Destination IP address"));
	c_type = (column_type_t *) item->item;
	_treeview_add_column(treeview_flow, c_type,i, NULL);
	types[i++] = c_type->type;

	// Destination Port
	item = linkedlist_item_skipto(header_storage,get_index_by_columnname("Destination port"));
	c_type = (column_type_t *) item->item;
	_treeview_add_column(treeview_flow, c_type,i, NULL);
	types[i++] = c_type->type;

	// Layer 4 Proto
	item = linkedlist_item_skipto(header_storage,get_index_by_columnname("Layer 4 protocol"));
	c_type = (column_type_t *) item->item;
	_treeview_add_column(treeview_flow, c_type,i, NULL);
	types[i++] = c_type->type;


	// Build List Store
	liststore_flow = gtk_list_store_newv (n_cols, types);
	g_free (types);

	// Set Model
	gtk_tree_view_set_model(GTK_TREE_VIEW(treeview_flow), GTK_TREE_MODEL(liststore_flow));
}


/*
 * Create Column and list Store in Col Mode
 */
void build_liststore_cols(int index) {
	// Get Tree View From GUI
	GtkTreeView *treeview_col = GTK_TREE_VIEW(gtk_builder_get_object(builder,"treeColData"));

	// Remove Old Column
	_clean_treeview(treeview_col);

	// Colum Types
	column_type_t *cType;

	GType *types;
	int n_cols = 2;
	types = g_new0(GType, n_cols);

	// Build Columns for current Index
	// -------------------------------
	linkedlist_item_t *item = linkedlist_item_skipto(header_storage,index);

	// Index Column
	_treeview_add_column(treeview_col, NULL,0, NULL);
	types[0] = G_TYPE_INT;

	// Data Column
	cType = (column_type_t *) item->item;
	_treeview_add_column(treeview_col, cType,1, NULL);
	types[1] = cType->type;

	// Build List Store
	liststore_col = gtk_list_store_newv (n_cols, types);
	g_free (types);

	// Set Model
	gtk_tree_view_set_model(GTK_TREE_VIEW(treeview_col), GTK_TREE_MODEL(liststore_col));
}

/*
 * Build a Model to be used in the combobox
 */
void build_headercombobox_model() {
	// Get ComboBox From GUI
	GtkComboBox *combobox = GTK_COMBO_BOX(gtk_builder_get_object(builder,"comboColSelect"));

	GtkTreeIter iter;
	linkedlist_item_t *current= header_storage->head;
	column_type_t *item;

	gtk_combo_box_set_model(GTK_COMBO_BOX(combobox),NULL);

	// Clean or Make a new ListStore
	if(liststore_header == NULL) {
		liststore_header = gtk_list_store_new(1, G_TYPE_STRING);
	} else {
		gtk_list_store_clear(liststore_header);
		gtk_cell_layout_clear(GTK_CELL_LAYOUT(combobox));
	}

	// Prepare the renderer
	GtkCellRenderer *renderer;
	renderer = gtk_cell_renderer_text_new();
	gtk_cell_layout_pack_start(GTK_CELL_LAYOUT(combobox), renderer, FALSE);
	gtk_cell_layout_add_attribute(GTK_CELL_LAYOUT(combobox), renderer, "text",0);
	g_object_set(GTK_OBJECT(renderer),"font", "Arial 8", NULL);

	char *tempname;

	// Add each Header to the ComboBox Model
	while (current != NULL) {
		item = (column_type_t *) current->item;

		// Cut to 40 chars.. otherwise the combobox is very big!
		tempname = malloc(sizeof(char) * 40);
		strncpy(tempname,item->name,39);
		tempname[39] = '\0';

		gtk_list_store_append(liststore_header, &iter);
		gtk_list_store_set(liststore_header,&iter, 0, tempname, -1);

		free(tempname);
		current = current->next_item;
	}

	// Set The Model
	gtk_combo_box_set_model(GTK_COMBO_BOX(combobox),GTK_TREE_MODEL(liststore_header));
}


/*
 * Create Colums and listStore for the Detail View
 */
void _build_liststore_details(GtkTreeView *treeview, GtkListStore** liststore) {

	column_type_t *c_type;
	GType *types;
	int n_cols = header_storage->size;
	types = g_new0 (GType, n_cols);

	linkedlist_item_t *item = linkedlist_item_skipto(header_storage,0);

	int i = 0;
	while(item != NULL) {
		c_type = (column_type_t *) item->item;
		_treeview_add_column(treeview, c_type,i, NULL);
		types[i] = c_type->type;

		item = item->next_item;
		i++;
	}

	// Build List Store
	*liststore = gtk_list_store_newv (n_cols, types);
	g_free (types);

	gtk_tree_view_set_model(GTK_TREE_VIEW(treeview), GTK_TREE_MODEL(*liststore));
}

/*
 * Create Colums and listStore for the Detail Data View Col Mode
 */
void build_liststore_flow_details() {
	// Get Tree View From GUI
	GtkTreeView *tree_flow_detail = GTK_TREE_VIEW(gtk_builder_get_object(builder,"tree_flow_detail"));
	_build_liststore_details(tree_flow_detail, &liststore_flow_detail);
}

/*
 * Create Colums and listStore for the Detail Data View Flow Mode
 */
void build_liststore_col_details() {
	// Get Tree View From GUI
	GtkTreeView *tree_col_detail = GTK_TREE_VIEW(gtk_builder_get_object(builder,"tree_col_detail"));
	_build_liststore_details(tree_col_detail, &liststore_col_detail);
}

void build_liststore_table() {
	// Get Tree View From GUI
	GtkTreeView *treeview_table = GTK_TREE_VIEW(gtk_builder_get_object(builder,"treeview_table"));

	// Set Selectio Mode to Multiple
	GtkTreeSelection * selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(treeview_table));
	gtk_tree_selection_set_mode(selection, GTK_SELECTION_MULTIPLE);

	column_type_t *c_type;
	GType *types;
	int n_cols = header_storage->size + 1;
	types = g_new0 (GType, n_cols);

	linkedlist_item_t *item = header_storage->head;

	// Index Column
	_treeview_add_column(treeview_table, NULL,0, NULL);
	types[0] = G_TYPE_INT;

	int i = 1;
	while(item != NULL) {
		c_type = (column_type_t *) item->item;
		_treeview_add_column(treeview_table, c_type,i, NULL);
		types[i] = c_type->type;

		item = item->next_item;
		i++;
	}

	// Build List Store
	liststore_table = gtk_list_store_newv (n_cols, types);
	g_free (types);

	gtk_tree_view_set_model(GTK_TREE_VIEW(treeview_table), GTK_TREE_MODEL(liststore_table));

}

/*
 * Add a new TreeView Column to treeview with name columnName at position column
 * Create a text Renderer and set Attribute to text
 */
void _treeview_add_column(GtkTreeView * treeview, column_type_t * c_type, gint column, GtkCellRenderer *renderer) {
	GtkTreeViewColumn *col = gtk_tree_view_column_new();
	gtk_tree_view_column_set_sort_column_id(col,column);
	gtk_tree_view_column_set_reorderable(col,1);


	char * columnname;

	if (c_type == NULL) {
		// Set a Empty Column Name;
		columnname = malloc(sizeof(char));
		columnname = "";
	} else {
		columnname = malloc(sizeof(char) * (strlen(c_type->name)+1));
		strncpy(columnname,c_type->name,strlen(c_type->name)+1);
	}

	gtk_tree_view_column_set_title(col,columnname);
	gtk_tree_view_append_column(GTK_TREE_VIEW(treeview), col);

	// Render Type is Text
	if(c_type == NULL || c_type->render_type == RENDER_TYPE_TEXT) {
		if (renderer == NULL) {
			renderer = gtk_cell_renderer_text_new();
			g_object_set(GTK_OBJECT(renderer),"font", "Arial 8", NULL);
		}

		gtk_tree_view_column_pack_start(col, renderer, TRUE);
		gtk_tree_view_column_add_attribute(col, renderer,"text",column);
	}

}


void _detaildata_set(int index, GtkTreeView* treeview, GtkListStore** listStore) {

	//Disconnect The model for performance
	gtk_tree_view_set_model(GTK_TREE_VIEW(treeview), NULL);

	GtkTreeIter iter;
	linkedlist_t *dataline = (linkedlist_t *) linkedlist_item_get(data_storage, index);
	linkedlist_item_t *current = linkedlist_item_skipto(dataline, 0);

	column_type_t *c_type;
	linkedlist_item_t *header = linkedlist_item_skipto(header_storage,0);

	int i = 0;

	//gtk_list_store_clear(*listStore);
	gtk_list_store_append(*listStore, &iter);

	//Fill List Store with Data
	while (current != NULL && header != NULL) {
		c_type = (column_type_t *) header->item;

		switch(c_type->type_short) {
		case TYPE_SHORT_DEC:
			gtk_list_store_set(*listStore,&iter, i, atoi(current->item), -1);
			break;
		case TYPE_SHORT_FLOAT:
			gtk_list_store_set(*listStore,&iter, i, atof(current->item), -1);
			break;
		case TYPE_SHORT_STRING:
		case TYPE_SHORT_COMBINED:
		case TYPE_SHORT_HEX:
		case TYPE_SHORT_NONE:
			gtk_list_store_set(*listStore,&iter, i, current->item, -1);
			break;
		}

		current = current->next_item;
		header = header->next_item;
		i++;
	}

	//Reconnect the model
	gtk_tree_view_set_model(GTK_TREE_VIEW(treeview), GTK_TREE_MODEL(*listStore));
}

/*
 * Set the Flow Detail Data of index'th Flow
 */
void detaildata_set_flow(GtkTreeView *treeview) {

	// Get Tree View From GUI
	GtkTreeView *treeview_flow_details = GTK_TREE_VIEW(gtk_builder_get_object(builder,"tree_flow_detail"));

	GtkTreeSelection * tsel = gtk_tree_view_get_selection (treeview);
	GtkTreeIter iter ;
	GtkTreeModel * tm ;
	GtkTreePath * path ;

	int flow_index;

	GList *node;
	GList * selected_path = gtk_tree_selection_get_selected_rows(tsel, &tm);

	gtk_list_store_clear(liststore_flow_detail);

	for (node = selected_path; node != NULL; node = node->next) {
		// Get Index for Path
		path = (GtkTreePath *) node->data;
		gtk_tree_model_get_iter (tm,&iter,path);
		gtk_tree_model_get (tm, &iter, 0, &flow_index, -1);

		_detaildata_set(flow_index, treeview_flow_details, &liststore_flow_detail);
	}
}

/*
 * Set the Flow Detail Data of index'th Flow
 */
void detaildata_set_col(int index) {

	// Get Tree View From GUI
	GtkTreeView *treeColDetail = GTK_TREE_VIEW(gtk_builder_get_object(builder,"tree_col_detail"));
	gtk_list_store_clear(liststore_col_detail);
	_detaildata_set(index, treeColDetail, &liststore_col_detail);
}


void tabledata_set() {
	GtkTreeView *treedata_table = GTK_TREE_VIEW(gtk_builder_get_object(builder, "treeview_table"));

	GtkTreeIter iter;

	linkedlist_item_t *currentline_data= data_storage->head;
	linkedlist_item_t *currentline_header= header_storage->head;
	linkedlist_item_t *currentdata;

	linkedlist_t *line_data;

	column_type_t *c_type;

	int i,j;

	gtk_tree_view_set_model(GTK_TREE_VIEW(treedata_table), NULL);
	if(liststore_table) { gtk_list_store_clear(GTK_LIST_STORE(liststore_table)); }


	for (i = 0 ; i < data_storage->size ; i++) {

		if (filter_check(table_filters, currentline_data)) { // If filter match, then show the flow
			gtk_list_store_append(liststore_table, &iter);
			gtk_list_store_set(liststore_table,&iter, 0, i, -1); // Index

			line_data = (linkedlist_t *) currentline_data->item;
			currentdata = line_data->head;

			for (j = 0 ; j < header_storage->size ; j++) {
				c_type = (column_type_t*) currentline_header->item;

				// Cast and set according to type
				switch(c_type->type_short) {
				case TYPE_SHORT_DEC:
					gtk_list_store_set(liststore_table,&iter, j+1, atoi((char *) currentdata->item), -1);
					break;
				case TYPE_SHORT_FLOAT:
					gtk_list_store_set(liststore_table,&iter, j+1, atof((char *) currentdata->item), -1);
					break;
				case TYPE_SHORT_COMBINED:
				case TYPE_SHORT_HEX:
				case TYPE_SHORT_NONE:
				case TYPE_SHORT_STRING:
					gtk_list_store_set(liststore_table,&iter, j+1, (char *) currentdata->item, -1);
					break;
				}


				currentdata = currentdata->next_item;
				currentline_header = currentline_header->next_item;
			}
		}


		currentline_header= header_storage->head; // Reset
		currentline_data = currentline_data->next_item;
	}

	gtk_tree_view_set_model(GTK_TREE_VIEW(treedata_table), GTK_TREE_MODEL(liststore_table));


}

/*
 * Set the General Datas of the Flow in FlowMode
 */
void flowdata_set() {

	// Get Tree View From GUI
	GtkTreeView *treedata_flow = GTK_TREE_VIEW(gtk_builder_get_object(builder,"treeDataFlow"));

	GtkTreeIter iter;
	linkedlist_item_t *currentline= data_storage->head;
	linkedlist_t *dataline;

	int i;

	//Disconnect The model for performance
	gtk_tree_view_set_model(GTK_TREE_VIEW(treedata_flow), NULL);

	for (i = 0 ; i < data_storage->size; i++) {

		//Add new Row
		gtk_list_store_append(liststore_flow, &iter);

		//Get the dataLine
		dataline = (linkedlist_t *) currentline->item;

		gtk_list_store_set(liststore_flow,&iter, 0, i, -1); // Index
		gtk_list_store_set(liststore_flow,&iter, 1, linkedlist_item_get(dataline, get_index_by_columnname("Flow direction")), -1);
		gtk_list_store_set(liststore_flow,&iter, 2, atoi(linkedlist_item_get(dataline, get_index_by_columnname("VLAN ID"))), -1);
		gtk_list_store_set(liststore_flow,&iter, 3, linkedlist_item_get(dataline, get_index_by_columnname("Source IP address")), -1);
		gtk_list_store_set(liststore_flow,&iter, 4, atoi(linkedlist_item_get(dataline, get_index_by_columnname("Source port"))), -1);
		gtk_list_store_set(liststore_flow,&iter, 5, linkedlist_item_get(dataline, get_index_by_columnname("Destination IP address")), -1);
		gtk_list_store_set(liststore_flow,&iter, 6, atoi(linkedlist_item_get(dataline, get_index_by_columnname("Destination port"))), -1);
		gtk_list_store_set(liststore_flow,&iter, 7, atoi(linkedlist_item_get(dataline, get_index_by_columnname("Layer 4 protocol"))), -1);

		currentline = currentline->next_item;
	}

	//Reconnect the modela
	gtk_tree_view_set_model(GTK_TREE_VIEW(treedata_flow), GTK_TREE_MODEL(liststore_flow));
}

/*
 * Set The Data in ColMode for current selected Index (from Combobox)
 */
void coldata_set(int index) {

	// Get Tree View From GUI
	GtkTreeView *treeview_coldata = GTK_TREE_VIEW(gtk_builder_get_object(builder,"treeColData"));

	GtkTreeIter iter;
	linkedlist_item_t *currentline= data_storage->head;
	linkedlist_t *dataline;


	linkedlist_item_t *current_header = linkedlist_item_skipto(header_storage,index);
	column_type_t * c_type = (column_type_t *) current_header->item;

	int i;

	//Disconnect The model for performance
	gtk_tree_view_set_model(GTK_TREE_VIEW(treeview_coldata), NULL);
	gtk_list_store_clear(liststore_col);

	for (i = 0 ; i < data_storage->size; i++) {

		if (filter_check(feature_filters, currentline)) {
			//Add new Row
			gtk_list_store_append(liststore_col, &iter);

			//Get the dataLine
			dataline = (linkedlist_t *) currentline->item;

			gtk_list_store_set(liststore_col,&iter, 0, i, -1); // Index

			switch(c_type->type_short) {
			case TYPE_SHORT_DEC:
				gtk_list_store_set(liststore_col,&iter, 1, atoi(linkedlist_item_get(dataline, index)), -1);
				break;
			case TYPE_SHORT_FLOAT:
				gtk_list_store_set(liststore_col,&iter, 1, atof(linkedlist_item_get(dataline, index)), -1);
				break;
			case TYPE_SHORT_STRING:
			case TYPE_SHORT_COMBINED:
			case TYPE_SHORT_HEX:
			case TYPE_SHORT_NONE:
				gtk_list_store_set(liststore_col,&iter, 1, linkedlist_item_get(dataline, index), -1);
				break;
			}
		}

		currentline = currentline->next_item;
	}

	//Reconnect the model
	gtk_tree_view_set_model(GTK_TREE_VIEW(treeview_coldata), GTK_TREE_MODEL(liststore_col));
}

/*
 * Clean all TreeView's and ListStore's
 */
void clean_stores() {
	// Get Tree View's From GUI
	GtkTreeView *treeview_flow = GTK_TREE_VIEW(gtk_builder_get_object(builder,"treeDataFlow"));
	GtkTreeView *treeview_flow_detail = GTK_TREE_VIEW(gtk_builder_get_object(builder,"tree_flow_detail"));
	GtkTreeView *treeview_col = GTK_TREE_VIEW(gtk_builder_get_object(builder,"treeColData"));
	GtkTreeView *treeview_col_detail = GTK_TREE_VIEW(gtk_builder_get_object(builder,"tree_flow_detail"));
	GtkTreeView *treeview_table = GTK_TREE_VIEW(gtk_builder_get_object(builder,"treeview_table"));
	GtkComboBox *combobox = GTK_COMBO_BOX(gtk_builder_get_object(builder,"comboColSelect"));

	//Flow Data
	gtk_tree_view_set_model(GTK_TREE_VIEW(treeview_flow), NULL);
	_clean_treeview(treeview_flow);
	if (liststore_flow) {
		gtk_list_store_clear(GTK_LIST_STORE(liststore_flow));
		gtk_tree_view_set_model(GTK_TREE_VIEW(treeview_flow), GTK_TREE_MODEL(liststore_flow));
	}

	//Flow Detail Data
	gtk_tree_view_set_model(GTK_TREE_VIEW(treeview_flow_detail), NULL);
	_clean_treeview(treeview_flow_detail);
	if (liststore_flow_detail) {
		gtk_list_store_clear(GTK_LIST_STORE(liststore_flow_detail));
		gtk_tree_view_set_model(GTK_TREE_VIEW(treeview_flow_detail), GTK_TREE_MODEL(liststore_flow_detail));
	}

	//Col Data
	gtk_tree_view_set_model(GTK_TREE_VIEW(treeview_col), NULL);
	_clean_treeview(treeview_col);
	if (liststore_col) {
		gtk_list_store_clear(GTK_LIST_STORE(liststore_col));
		gtk_tree_view_set_model(GTK_TREE_VIEW(treeview_col), GTK_TREE_MODEL(liststore_col));
	}

	//Col Detail Data
	gtk_tree_view_set_model(GTK_TREE_VIEW(treeview_col_detail), NULL);
	_clean_treeview(treeview_col_detail);
	if (liststore_col_detail) {
		gtk_list_store_clear(GTK_LIST_STORE(liststore_col_detail));
		gtk_tree_view_set_model(GTK_TREE_VIEW(treeview_col_detail), GTK_TREE_MODEL(liststore_col_detail));
	}

	// Table Data
	gtk_tree_view_set_model(GTK_TREE_VIEW(treeview_table), NULL);
	_clean_treeview(treeview_table);
	if (liststore_table) {
		gtk_list_store_clear(GTK_LIST_STORE(liststore_table));
		gtk_tree_view_set_model(GTK_TREE_VIEW(treeview_table), GTK_TREE_MODEL(liststore_table));
	}

	//Header ComboBox
	gtk_combo_box_set_model(GTK_COMBO_BOX(combobox),NULL);
	if(liststore_header) {
		gtk_list_store_clear(GTK_LIST_STORE(liststore_header));
		gtk_combo_box_set_model(GTK_COMBO_BOX(combobox),GTK_TREE_MODEL(liststore_header));
	}

}

/*
 * Remove all Columns in a GtkTreeView
 */
void _clean_treeview(GtkTreeView * treeView) {
	GList *column_list;
	GtkTreeViewColumn *treeview_column;

	column_list = gtk_tree_view_get_columns(treeView);
	while(column_list) {
		treeview_column = column_list->data;
		if (treeview_column) gtk_tree_view_remove_column(treeView, treeview_column);
		column_list = g_list_next(column_list);
	}
}

/*
 * Open Files for parsing
 */
int open_files(char * file_name) {
	return _parse_binary_file(file_name);
}


int _parse_binary_file(char filename_binary[]) {


	if (set_binary_file(filename_binary) == -1)
		return -1;

	// Build Header Storage
	// Clear Header Storage first
	if (header_storage != NULL) {
		clear_linkedlist(header_storage,1);
	}
	header_storage = linkedlist_new();
	parse_binary_header2storage(bv);


	// Build Data Storage
	//Clear Data Storage first
	if (data_storage != NULL) {
		clear_linkedlist(data_storage,2);
	}
	data_storage = linkedlist_new();
	binary2storage(bv);

	return 0;

}


/*
 * Generate a new Colum Type
 */
column_type_t * columntype_build(char *name, gint column, GType type, int type_short) {

	column_type_t * item = malloc(sizeof(column_type_t));

	item->name = malloc(sizeof(char) * (strlen(name)+1));
	strncpy(item->name,name, strlen(name) + 1);

	item->type_short = type_short;
	item->column = column;
	item->type = type;
	item->render_type = RENDER_TYPE_TEXT;

	// Set the Chart Type
	switch (item->type_short) {
	case TYPE_SHORT_STRING:
	case TYPE_SHORT_HEX:
		item->chart_type = COL_CHART_TYPE_PIE;
		break;
	case TYPE_SHORT_DEC:
	case TYPE_SHORT_FLOAT:
		item->chart_type = COL_CHART_TYPE_HISTO;
		break;
	default:
		item->chart_type = COL_CHART_TYPE_NONE;
	}

	return item;
}


/*
 * Return the Index of a Column by a given Column Name
 */
int get_index_by_columnname(char *columnName) {
	linkedlist_item_t *current= header_storage->head;
	column_type_t *item;

	int i = 0;

	while (current != NULL) {

		item = (column_type_t *) current->item;
		if(strcmp(item->name,columnName) == 0) {
			return i;
		}
		i++;
		current = current->next_item;
	}

	return -1; // Not found
}

column_type_t * get_column_type_by_columnindex(int index) {
	return (column_type_t *) linkedlist_item_skipto(header_storage,index)->item;
}

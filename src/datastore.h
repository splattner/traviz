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
 */


#ifndef DATASTORE_H_
#define DATASTORE_H_

/*
 * Includes
 */

#include "include.h"
#include "linkedList.h"
#include "gui.h"
#include "plot.h"
#include "binaryToText.h"


/*
 * Structs
 */

typedef struct column_type_s{
	gint column;
	GType type;
	int chart_type;
	int render_type;
	int type_short;
	char *name;
} column_type_t; // Contains all Data for one Column in a Tree View

/*
 * Enum's
 */

enum {
	RENDER_TYPE_TEXT,
}; // Types of Renderer


enum {
	TYPE_SHORT_NONE,
	TYPE_SHORT_STRING,
	TYPE_SHORT_DEC,
	TYPE_SHORT_FLOAT,
	TYPE_SHORT_HEX,
	TYPE_SHORT_COMBINED,
};

/*
 * Data Storage Objects
 */
extern linkedlist_t * data_storage;
extern linkedlist_t * header_storage;


/*
 * All List Store's needed to display the datas
 */
extern GtkListStore *liststore_flow;
extern GtkListStore *liststore_flow_detail;
extern GtkListStore *liststore_col;
extern GtkListStore *liststore_col_detail;
extern GtkListStore *liststore_header;
extern GtkListStore *liststore_table;



/*
 * Public Prototypes
 */

void build_liststore_flows();
void build_liststore_cols(int index);
void build_liststore_flow_details();
void build_liststore_col_details();
void build_liststore_table();
void build_headercombobox_model();
int get_index_by_columnname(char *columnname);
int open_files(char * file_name);
void flowdata_set();
void coldata_set();
void tabledata_set();
void detaildata_set_flow(GtkTreeView *treeview);
void detaildata_set_col(int index);
void clean_stores();
column_type_t * columntype_build(char *name, gint column, GType type, int type_short);
column_type_t * get_column_type_by_columnindex(int index);


#endif /* DATASTORE_H_ */

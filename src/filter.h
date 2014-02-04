/*
 * filter.h
 *
 *  Created on: 26.07.2010
 *      Author: sebastian
 */

#ifndef FILTER_H_
#define FILTER_H_

#include "include.h"
#include "datastore.h"
#include "linkedList.h"



/*
 * Structs
 */
typedef struct filter_s{
	int column_index;
	int filter;
	int value_int;
	double value_double;
	char * value_string;

	int (*compare_with)(struct filter_s *, char *);
} filter_t;

/*
 * Enum's
 */

enum {
	FILTER_FUNC_GT, // Greater then
	FILTER_FUNC_GET, // Greater or eqal then
	FILTER_FUNC_ST, // Smaller then
	FILTER_FUNC_SET, // Smaller or equal then
	FILTER_FUNC_EQ, // Equal then
	FILTER_FUNC_NEQ, // Not equal then
};

enum {
	FILTER_FEATURE_STRING,
	FILTER_TYPE_STRING,
	FILTER_VALUE,
	COLUMN_NUMBER
};



/*
 * Data Storage Objects
 */
extern linkedlist_t * feature_filters;
extern linkedlist_t * table_filters;

linkedlist_t * current_filter_list;



/*
 * Public Prototypes
 */
void open_filter_dialog(linkedlist_t * filters);
void close_filter_dialog();

void filter_popup_menu(GtkWidget *treeview, GdkEventButton *event, gpointer userdata);
gboolean filter_onPopupMenu(GtkWidget *treeview, gpointer userdata);
gboolean filter_onButtonPressed (GtkWidget *treeview, GdkEventButton *event, gpointer userdata);
void load_filter_in_treeview(linkedlist_t * filters, GtkTreeView *treeview_filter );
void filter_popup_menu_remove_selected (GtkWidget *menuitem, gpointer userdata);
void filter_popup_menu_new_filter (GtkWidget *menuitem, gpointer userdata);
void filter_edited_callback (GtkCellRendererText *cell,gchar *path_string, gchar *new_text,gpointer user_data);
void feature_edited_callback (GtkCellRendererText *cell,gchar *path_string, gchar *new_text,gpointer user_data);

void set_filter_feature(filter_t * filter, int type);
void set_filter_value(filter_t * filter, char * value);
char * get_filter_value_as_string(filter_t * filter);

int filter_check(linkedlist_t * filter_list, linkedlist_item_t * current_flow_item);

/*
 * Compare functions
 */
int compare_with_int(filter_t * filter, char * value);
int compare_with_double(filter_t * filter, char * value);
int compare_with_string(filter_t * filter, char * value);


#endif /* FILTER_H_ */

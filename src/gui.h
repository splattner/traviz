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


#ifndef GUI_H_
#define GUI_H_

/*
 * Includes
 */

#include "include.h"
#include "datastore.h"
#include "linkedList.h"
#include "plot.h"
#include "filter.h"


/*
 * Extern vars
 */
//extern PlplotCanvas* canvas_flow; // The Canvas containing the plots in Flow Mode
//extern PlplotCanvas* canvas_col; // The Canvas containing the plots in Col Mode

/*
 * Plotting Areas
 */

typedef struct Plotarea {
	GtkWidget *plotWindow;
	GdkPixmap *plotWindow_pixmap;
} Plotarea;

extern Plotarea * plotarea_flow;
extern Plotarea * plotarea_col;

extern int motion_button_down;


/*
 * Public Prototypes
 */
void gui_init();
int _column_get_index(GtkTreeView *view, guint x, guint y);

void on_logaxis_switch_toogle(GtkCheckButton *logaxis_switch, gpointer user_data);
void on_combined_line_switch_toogle(GtkCheckButton *combined_line_switch, gpointer user_data);
void on_histogramm_bin_count_textbox_activate (GtkEntry *textbox, gpointer user_data);
void on_per_aggregation_textbox_activate (GtkEntry *textbox, gpointer user);
gboolean on_resize(GtkWidget *widget, GdkEventExpose *event, gpointer user_data);
void on_mainwindow_destroy (GtkObject *object, gpointer user_data);
void on_menu_open(GtkObject *object, gpointer user_data);
void on_menu_new(GtkObject *object, gpointer user_data);
void on_treeDataFlow_cursor_changed(GtkTreeView *treeview, gpointer user_data);
void on_treeColData_cursor_changed(GtkTreeView *treeview, gpointer user_data);
void on_closeButton_clicked(GtkObject *object, gpointer user_data);
void on_loadButton_clicked(GtkObject *object, gpointer user_data);
void on_comboColSelect_changed(GtkObject *object, gpointer user_data);
void on_view_Col(GtkObject *object, gpointer user_data);
void on_view_Flow(GtkObject *object, gpointer user_data);
void on_treeDetailFlow_button_press_event (GtkWidget *view, GdkEventButton *bevent, gpointer data);
void on_flow_plot_combo_changed(GtkComboBox *treeview, gpointer user_data);

int _column_get_index(GtkTreeView *view, guint x, guint y);


gint ev_plotwindow_conf_flow(GtkWidget *widget, GdkEventConfigure *ev, gpointer *data);
gint ev_plotwindow_conf_col(GtkWidget *widget, GdkEventConfigure *ev, gpointer *data);
gint ev_plotwindow_expose_col(GtkWidget *widget, GdkEventExpose *ev, gpointer *data);
gint ev_plotwindow_expose_flow(GtkWidget *widget, GdkEventExpose *ev, gpointer *data);

void on_bound_update (GtkEntry *textbox, gpointer user_data);
void on_bound_update_3d (GtkEntry *textbox, gpointer user_data);

gboolean on_mouse_motion_event (GtkWidget *widget, GdkEventMotion *event, gpointer user_data);




#endif /* GUI_H_ */


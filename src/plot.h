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

#ifndef PLOT_H_
#define PLOT_H_

#include "include.h"
#include "main.h"
#include "datastore.h"
#include "gui.h"

enum {
	COL_CHART_TYPE_NONE,
	COL_CHART_TYPE_HISTO,
	COL_CHART_TYPE_PIE,
};

enum {
	FLOW_CHART_TYPE_STATS,
	FLOW_CHART_TYPE_3DHISTO,
};

typedef struct {
	int count;
	char *value;
} value_counter;

typedef struct {
	int packetsize;
	int iat;
	int count;
} histostat;

/*
 * Public Prototypes
 */

void plot_col(int activeIndex, double dataToHightlight, int doHightlight);
void plot_flow(GtkTreeView * treeview);
void clear_plots();


extern int saved_col_index;
extern double saved_data_to_highlight;
extern int saved_do_highlight;
extern int saved_is_log_axis;
extern int saved_bin_count;
extern double saved_per_aggregation;

extern int saved_flow_plot;
extern int saved_combined_line;
extern int saved_reset_bounds;

extern double saved_xmin, saved_xmax, saved_ymin, saved_ymax, saved_y2min, saved_y2max;
extern double saved_xmin_3d, saved_xmax_3d, saved_ymin_3d, saved_ymax_3d, saved_zmin_3d, saved_zmax_3d, saved_iat_with, saved_packet_size;

extern GtkTreeView * saved_plot_from_treeview;

extern int view_angle_az;
extern int view_angle_alt;


#endif /* PLOT_H_ */

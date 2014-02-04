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
 *
 *
 *
 */


#ifndef BINARYTOTEXT_H_
#define BINARYTOTEXT_H_

/*
 * Includes
 */

#include "include.h"
#include "binaryValue.h"
#include "datastore.h"



/*
 * Prototypes
 */
int extract_flows_to_new_file(GtkTreeView *treeview, char *new_file_name);
int set_binary_file(char * file_name);
void parse_binary_header2storage(binary_value_t *bv);
void parse_binary2storage(binary_value_t *bv, int index);

void parse_binary(binary_value_t *bv);
void binary2storage(binary_value_t *bv);

void parse_subval_to_buffer(binary_subvalue_t *sv, int depth, char * buffer);
void parse_subval(binary_subvalue_t *sv, int depth);

void parse_binary_value_to_buffer(uint32_t type, char * buffer);
void parse_binary_value(uint32_t type);

/*
 * External Vars
 */
extern binary_value_t *bv;
extern char * flow_file;




#endif /* BINARYTOTEXT_H_ */

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
 * Parse the Binary Output from Tranalyzer to the internal Traviz Data Structure
 *
 */

#include "binaryToText.h"


binary_value_t *bv;
FILE * input_file = NULL;
FILE * output_file = NULL;

char *flow_file;

uint32_t (*get_val_fkt)(void*, size_t, size_t);


/*
 * Read from input file and save to dest
 */
uint32_t get_val_from_input_file(void* dest, size_t size, size_t n) {
	return fread(dest,size,n,input_file);
}

/*
 * Read from input file and save to dest
 * Write dest to an output file, making a copy!
 */
uint32_t get_val_from_input_file_and_save_to_file(void* dest, size_t size, size_t n) {

	uint32_t retval = fread(dest,size,n,input_file);
	fwrite(dest, size, n, output_file);

	return retval;
}


/*
 * Create a new Flow File only with selected Flows
 */
int extract_flows_to_new_file(GtkTreeView *treeview, char *new_file_name) {


	// Open Original File to copy from
	input_file = fopen(flow_file,"r");
	if (input_file == NULL) {

		GtkWidget *dialog = gtk_message_dialog_new (GTK_WINDOW(window),
										GTK_DIALOG_DESTROY_WITH_PARENT,
										GTK_MESSAGE_ERROR,
			                            GTK_BUTTONS_CLOSE,
			                            "ERROR: Couldn't open Binary File: %s\n", flow_file
			                            );

		/* Destroy the dialog when the user responds to it (e.g. clicks a button) */
		g_signal_connect_swapped (dialog, "response", G_CALLBACK (gtk_widget_destroy), dialog);
		gtk_dialog_run(GTK_DIALOG(dialog));

		return -1;
	}

	// Create new flow
	output_file = fopen(new_file_name,"w");
	if (output_file == NULL) {

		GtkWidget *dialog = gtk_message_dialog_new (GTK_WINDOW(window),
										GTK_DIALOG_DESTROY_WITH_PARENT,
										GTK_MESSAGE_ERROR,
			                            GTK_BUTTONS_CLOSE,
			                            "ERROR: Couldn't create new  Binary File: %s\n", new_file_name
			                            );

		/* Destroy the dialog when the user responds to it (e.g. clicks a button) */
		g_signal_connect_swapped (dialog, "response", G_CALLBACK (gtk_widget_destroy), dialog);
		gtk_dialog_run(GTK_DIALOG(dialog));

		return -1;
	}

	uint32_t headerLength = 0;
	uint32_t value;

	for(;;) {
		if (fread(&value,sizeof(uint32_t),1,input_file) != 1) {

			GtkWidget *dialog = gtk_message_dialog_new (GTK_WINDOW(window),
											GTK_DIALOG_DESTROY_WITH_PARENT,
											GTK_MESSAGE_ERROR,
											GTK_BUTTONS_CLOSE,
											"ERROR: Incorrect header in file\n"
											);

			/* Destroy the dialog when the user responds to it (e.g. clicks a button) */
			g_signal_connect_swapped (dialog, "response", G_CALLBACK (gtk_widget_destroy), dialog);
			gtk_dialog_run(GTK_DIALOG(dialog));

			return -1;
		}
		headerLength++;
		if (value == UINT32_MAX) break;
		if (headerLength == UINT32_MAX) {

			GtkWidget *dialog = gtk_message_dialog_new (GTK_WINDOW(window),
											GTK_DIALOG_DESTROY_WITH_PARENT,
											GTK_MESSAGE_ERROR,
											GTK_BUTTONS_CLOSE,
											"ERROR: header is too long. Seems to be incorrect\n"
											);

			/* Destroy the dialog when the user responds to it (e.g. clicks a button) */
			g_signal_connect_swapped (dialog, "response", G_CALLBACK (gtk_widget_destroy), dialog);
			gtk_dialog_run(GTK_DIALOG(dialog));

			return -1;
		}
	}

	rewind(input_file);

	// Read Header from orig and write to the new file
	uint32_t headerBuffer[headerLength];
	fread(headerBuffer, sizeof(uint32_t), headerLength, input_file);
	fwrite(headerBuffer,sizeof(uint32_t), headerLength, output_file);


	// Get selected flows from treeview
	GtkTreeSelection * tsel = gtk_tree_view_get_selection (treeview);

	GtkTreeIter iter ;
	GtkTreeModel * tm ;
	GtkTreePath * path ;

	GList *node;
	GList * selected_path = gtk_tree_selection_get_selected_rows(tsel, &tm);


	int i = 0;
	int flow_index = -1;
	char temp_char;

	while(fread(&temp_char, sizeof(char), 1, input_file) != 0) {
		ungetc(temp_char, input_file);

		for (node = selected_path; flow_index != i && node != NULL; node = node->next) {
			// Get Index for Path
			path = (GtkTreePath *) node->data;
			gtk_tree_model_get_iter (tm,&iter,path);
			gtk_tree_model_get (tm, &iter, 0, &flow_index, -1);

		}

		if (flow_index == i) { get_val_fkt = get_val_from_input_file_and_save_to_file; } // Copy to output file
		else { get_val_fkt = get_val_from_input_file; } // Just read and continue

		parse_binary(bv);

		i++;
	}


	// Close files
	fclose(input_file);
	fclose(output_file);
}

/*
 * Load the binary file and analyze iz
 */
int set_binary_file(char * file_name) {

	flow_file = file_name;


	input_file = fopen(file_name,"r");
	if (input_file == NULL) {

		GtkWidget *dialog = gtk_message_dialog_new (GTK_WINDOW(window),
										GTK_DIALOG_DESTROY_WITH_PARENT,
										GTK_MESSAGE_ERROR,
			                            GTK_BUTTONS_CLOSE,
			                            "ERROR: Couldn't open Binary File: %s\n", file_name
			                            );

		/* Destroy the dialog when the user responds to it (e.g. clicks a button) */
		g_signal_connect_swapped (dialog, "response", G_CALLBACK (gtk_widget_destroy), dialog);
		gtk_dialog_run(GTK_DIALOG(dialog));

		return -1;
	}


	uint32_t headerLength = 0;
	uint32_t value;

	for(;;) {
		if (fread(&value,sizeof(uint32_t),1,input_file) != 1) {

			GtkWidget *dialog = gtk_message_dialog_new (GTK_WINDOW(window),
											GTK_DIALOG_DESTROY_WITH_PARENT,
											GTK_MESSAGE_ERROR,
				                            GTK_BUTTONS_CLOSE,
				                            "ERROR: Incorrect header in file\n"
				                            );

			/* Destroy the dialog when the user responds to it (e.g. clicks a button) */
			g_signal_connect_swapped (dialog, "response", G_CALLBACK (gtk_widget_destroy), dialog);
			gtk_dialog_run(GTK_DIALOG(dialog));

			return -1;
		}
		headerLength++;
		if (value == UINT32_MAX) break;
		if (headerLength == UINT32_MAX) {

			GtkWidget *dialog = gtk_message_dialog_new (GTK_WINDOW(window),
											GTK_DIALOG_DESTROY_WITH_PARENT,
											GTK_MESSAGE_ERROR,
			                                GTK_BUTTONS_CLOSE,
			                                "ERROR: header is too long. Seems to be incorrect\n"
			                                );

			/* Destroy the dialog when the user responds to it (e.g. clicks a button) */
			g_signal_connect_swapped (dialog, "response", G_CALLBACK (gtk_widget_destroy), dialog);
			gtk_dialog_run(GTK_DIALOG(dialog));

			return -1;
		}
	}

	rewind(input_file);
	uint32_t headerBuffer[headerLength];
	fread(headerBuffer, sizeof(uint32_t), headerLength, input_file);

	binary_header_t *header = malloc(sizeof(binary_header_t));
	header->header = headerBuffer;
	header->header_length = headerLength;

	bv = analyseHeader(header);

	// Set Val Function
	get_val_fkt = get_val_from_input_file;

	return 0;
}

/*
 * Parse the binary Header List and generate the HeaderStorage
 */
void parse_binary_header2storage(binary_value_t *bv) {
	int i = 0;

	linkedlist_item_t * header_item;

	while (bv) {
		header_item = linkedlist_item_new(NULL);

		if (bv->num_values > 1) {
			header_item->item = (void *) columntype_build(bv->name_value_long, i, G_TYPE_STRING, TYPE_SHORT_COMBINED);
		} else { // num_values == 1
			switch (bv->subval[0].value_type) {
				case bt_int_8:
				case bt_int_16:
				case bt_int_32:
				case bt_int_64:
				case bt_int_128:
				case bt_int_256:
				case bt_uint_8:
				case bt_uint_16:
				case bt_uint_32:
				case bt_uint_64:
				case bt_uint_128:
				case bt_uint_256:
					header_item->item = (void *) columntype_build(bv->name_value_long, i, G_TYPE_INT, TYPE_SHORT_DEC);
					break;
				case bt_hex_8:
				case bt_hex_16:
				case bt_hex_32:
				case bt_hex_64:
				case bt_hex_128:
				case bt_hex_256:
					header_item->item = (void *) columntype_build(bv->name_value_long, i, G_TYPE_STRING, TYPE_SHORT_HEX);
					break;
				case bt_float:
				case bt_double:
				case bt_long_double:
					header_item->item = (void *) columntype_build(bv->name_value_long, i, G_TYPE_DOUBLE, TYPE_SHORT_FLOAT);
					break;
				case bt_char:
				case bt_string:
				case bt_string_class:
					header_item->item = (void *) columntype_build(bv->name_value_long, i, G_TYPE_STRING, TYPE_SHORT_STRING);
					break;
				case bt_unix_time:
				case bt_time:
					header_item->item = (void *) columntype_build(bv->name_value_long, i, G_TYPE_DOUBLE, TYPE_SHORT_FLOAT);
					break;
				case bt_flow_direction:
				case bt_mac_addr:
				case bt_ip4_addr:
				case bt_ip6_addr:
					header_item->item = (void *) columntype_build(bv->name_value_long, i, G_TYPE_STRING, TYPE_SHORT_STRING);
					break;
			}
		}
		linkedlist_add_item(header_storage,header_item);

		bv = bv->next;
		i++;

	}
}

/*
 * Load all flows into datastorage
 */
void binary2storage(binary_value_t *bv) {

	char temp_char;

	int i = 0;

	while(fread(&temp_char, sizeof(char), 1, input_file) != 0) {
		ungetc(temp_char, input_file);
		parse_binary2storage(bv, i);

		i++;
	}

	fclose(input_file);

}

/*
 * Parse one Flow from binary
 * Only read from file and according to get_val_fkt write to second file
 */
void parse_binary(binary_value_t *bv) {
	binary_value_t *act_bv = bv;

	while(act_bv) {

		// check if output can be repeated. If yes, read amount of repeats, if no set repeat_val to 1
		uint32_t repeat_val;
		if (act_bv->is_repeating) {
			get_val_fkt(&repeat_val, sizeof(uint32_t), 1);
		} else {
			repeat_val = 1;
		}

		int repeat_i;

		for (repeat_i = 0; repeat_i < repeat_val; repeat_i++) {
			int num_val_i;

			// for each output val:
			// check type and write it out, if zero then it contains subvals
			for (num_val_i = 0; num_val_i < act_bv->num_values; num_val_i++) {
				if (act_bv->subval[num_val_i].value_type != 0) {
					parse_binary_value(act_bv->subval[num_val_i].value_type);
				} else {
					parse_subval(&act_bv->subval[num_val_i], 1);
				}
			}
		}

		act_bv = act_bv->next;
	}
}

/*
 * Parse one Flow from binary to datastorage
 */
void parse_binary2storage(binary_value_t *bv, int index) {
	binary_value_t *act_bv = bv;

	// Buffer to save one feature
	char *buffer = malloc(sizeof(char) * 100000);


	//init DataLine (LinkedList)
	linkedlist_t * data_line = linkedlist_new();
	linkedlist_item_t * data_item;


	while(act_bv) {
#if debug != 0
		if(act_bv != bv) {
			printf("\t");
		}
#endif

		// Reset Buffer
		buffer[0] = '\0';

		// check if output can be repeated. If yes, read amount of repeats, if no set repeat_val to 1
		uint32_t repeat_val;
		if (act_bv->is_repeating) {
			get_val_fkt(&repeat_val, sizeof(uint32_t), 1);
		} else {
			repeat_val = 1;
		}

		int repeat_i;

		for (repeat_i = 0; repeat_i < repeat_val; repeat_i++) {
			int num_val_i;

			// for each output val:
			// check type and write it out, if zero then it contains subvals
			for (num_val_i = 0; num_val_i < act_bv->num_values; num_val_i++) {
				if (act_bv->subval[num_val_i].value_type != 0) {
					parse_binary_value_to_buffer(act_bv->subval[num_val_i].value_type, buffer);
				} else {
					parse_subval_to_buffer(&act_bv->subval[num_val_i], 1, buffer);
				}

				// write value delimeter
				if (num_val_i < act_bv->num_values - 1) strcat(buffer,"_"); //printf("_");
			}
			// write repeat delimeter
			if (repeat_i < repeat_val - 1) strcat(buffer, ";"); //printf(";");
		}

#if debug != 0
		printf("%s", buffer);
#endif

		data_item = malloc(sizeof(linkedlist_item_t));
		data_item->item = malloc(sizeof(char) * strlen(buffer) + 1);
		strcpy((char *) data_item->item, buffer);
		data_item->next_item = NULL;

		linkedlist_add_item(data_line,data_item);

		act_bv = act_bv->next;
	}


	free(buffer);

	data_item = linkedlist_item_new((void *)data_line);
	linkedlist_add_item(data_storage, data_item);
}

/*
 * Parse Subvalue, just read it
 */
void parse_subval(binary_subvalue_t *sv, int depth) {

	// check if output can be repeated. If yes, read amount of repeats, if no set repeat_val to 1
	uint32_t repeat_val;
	if (sv->is_repeating)
		get_val_fkt(&repeat_val, sizeof(uint32_t), 1);
	else
		repeat_val = 1;

	int repeat_i;
	for (repeat_i = 0; repeat_i < repeat_val; repeat_i++) {
		int num_val_i;

		// for each output val:
		// check type and write it out, if zero then it contains subvals
		for (num_val_i = 0; num_val_i < sv->num_values; num_val_i++) {
			if (sv->subval[num_val_i].value_type != 0) {
				parse_binary_value(sv->subval[num_val_i].value_type);
			} else {
				parse_subval(&sv->subval[num_val_i], depth + 1);
			}
		}
	}
}

/*
 * Parse Subvalue to a buffer
 */
void parse_subval_to_buffer(binary_subvalue_t *sv, int depth, char * buffer) {

	// check if output can be repeated. If yes, read amount of repeats, if no set repeat_val to 1
	uint32_t repeat_val;
	if (sv->is_repeating)
		get_val_fkt(&repeat_val, sizeof(uint32_t), 1);
	else
		repeat_val = 1;

	int repeat_i;
	for (repeat_i = 0; repeat_i < repeat_val; repeat_i++) {
		int num_val_i;

		// for each output val:
		// check type and write it out, if zero then it contains subvals
		for (num_val_i = 0; num_val_i < sv->num_values; num_val_i++) {
			if (sv->subval[num_val_i].value_type != 0) {
				parse_binary_value_to_buffer(sv->subval[num_val_i].value_type, buffer);
			} else {
				parse_subval_to_buffer(&sv->subval[num_val_i], depth + 1, buffer);
			}

			// write value delimeter
			if (num_val_i < sv->num_values - 1) strcat(buffer, "_"); //printf("_");
		}
		// write repeat delimeter
		if (repeat_i < repeat_val - 1) strcat(buffer, ";"); //printf(";");
	}
}

/*
 * Parse Binary Value. Just read it
 */
void parse_binary_value(uint32_t type) {

	unsigned char val[16];

	switch (type) {
		case bt_int_8:
			// read value
			if (get_val_fkt(val, sizeof(int8_t), 1) == 0) {
				fprintf(stderr, "binaryToText: Error: Input is corrupted!\n");
				exit(1);
			}
			break;

		case bt_int_16:
			// read value
			if (get_val_fkt(val, sizeof(int16_t), 1) == 0) {
				fprintf(stderr, "binaryToText: Error: Input is corrupted!\n");
				exit(1);
			}
			break;

		case bt_int_32:
			// read value
			if (get_val_fkt(val, sizeof(int32_t), 1) == 0) {
				fprintf(stderr, "binaryToText: Error: Input is corrupted!\n");
				exit(1);
			}
			break;

		case bt_int_64:
			// read value
			if (get_val_fkt(val, sizeof(int64_t), 1) == 0) {
				fprintf(stderr, "binaryToText: Error: Input is corrupted!\n");
				exit(1);
			}
			break;

		case bt_uint_8:
			// read value
			if (get_val_fkt(val, sizeof(uint8_t), 1) == 0) {
				fprintf(stderr, "binaryToText: Error: Input is corrupted!\n");
				exit(1);
			}
			break;

		case bt_uint_16:
			// read value
			if (get_val_fkt(val, sizeof(uint16_t), 1) == 0) {
				fprintf(stderr, "binaryToText: Error: Input is corrupted!\n");
				exit(1);
			}
			break;

		case bt_uint_32:
			// read value
			if (get_val_fkt(val, sizeof(uint32_t), 1) == 0) {
				fprintf(stderr, "binaryToText: Error: Input is corrupted!\n");
				exit(1);
			}
			break;

		case bt_uint_64:
			// read value
			if (get_val_fkt(val, sizeof(uint64_t), 1) == 0) {
				fprintf(stderr, "binaryToText: Error: Input is corrupted!\n");
				exit(1);
			}
			break;

		case bt_hex_8:
			// read value
			if (get_val_fkt(val, sizeof(uint8_t), 1) == 0) {
				fprintf(stderr, "binaryToText: Error: Input is corrupted!\n");
				exit(1);
			}
			break;

		case bt_hex_16:
			// read value
			if (get_val_fkt(val, sizeof(uint16_t), 1) == 0) {
				fprintf(stderr, "binaryToText: Error: Input is corrupted!\n");
				exit(1);
			}
			break;

		case bt_hex_32:
			// read value
			if (get_val_fkt(val, sizeof(uint32_t), 1) == 0) {
				fprintf(stderr, "binaryToText: Error: Input is corrupted!\n");
				exit(1);
			}
			break;

		case bt_hex_64:
			// read value
			if (get_val_fkt(val, sizeof(uint64_t), 1) == 0) {
				fprintf(stderr, "binaryToText: Error: Input is corrupted!\n");
				exit(1);
			}
			break;

		case bt_float:
			// read value
			if (get_val_fkt(val, sizeof(float), 1) == 0) {
				fprintf(stderr, "binaryToText: Error: Input is corrupted!\n");
				exit(1);
			}
			break;

		case bt_double:
			// read value
			if (get_val_fkt(val, sizeof(double), 1) == 0) {
				fprintf(stderr, "binaryToText: Error: Input is corrupted!\n");
				exit(1);
			}
			break;

		case bt_long_double:
			// read value
			if (get_val_fkt(val, sizeof(long double), 1) == 0) {
				fprintf(stderr, "binaryToText: Error: Input is corrupted!\n");
				exit(1);
			}
			break;

		case bt_char:
			// read value
			if (get_val_fkt(val, sizeof(unsigned char), 1) == 0) {
				fprintf(stderr, "binaryToText: Error: Input is corrupted!\n");
				exit(1);
			}
			break;

		case bt_string_class:
			// read value
			if (get_val_fkt(val, sizeof(unsigned char), 1) == 0) {
				fprintf(stderr, "binaryToText: Error: Input is corrupted!\n");
				exit(1);
			}

			while (*val != '\0') {
				if (get_val_fkt(val, sizeof(unsigned char), 1) == 0) {
					fprintf(stderr, "binaryToText: Error: Input is corrupted!\n");
					exit(1);
				}
			}

			break;
		case bt_string:
			// read value
			if (get_val_fkt(val, sizeof(unsigned char), 1) == 0) {
				fprintf(stderr, "binaryToText: Error: Input is corrupted!\n");
				exit(1);
			}

			while (*val != '\0') {
				if (get_val_fkt(val, sizeof(unsigned char), 1) == 0) {
					fprintf(stderr, "binaryToText: Error: Input is corrupted!\n");
					exit(1);
				}
			}

			break;
		case bt_mac_addr:
			// read value
			if (get_val_fkt(val, l_bt_mac_addr * sizeof(u_int8_t), 1) == 0) {
				fprintf(stderr, "binaryToText: Error: Input is corrupted!\n");
				exit(1);
			}
			break;
		case bt_ip4_addr:
			// read value
			if (get_val_fkt(val, l_bt_ip4_addr * sizeof(u_int8_t), 1) == 0) {
				fprintf(stderr, "binaryToText: Error: Input is corrupted!\n");
				exit(1);
			}
			break;
		case bt_ip6_addr:
			// read value
			if (get_val_fkt(val, l_bt_ip6_addr * sizeof(u_int8_t), 1) == 0) {
				fprintf(stderr, "binaryToText: Error: Input is corrupted!\n");
				exit(1);
			}
			break;

		case bt_unix_time:
		case bt_time:
			// read seconds
			if (get_val_fkt(val, sizeof(u_int64_t), 1) == 0) {
				fprintf(stderr, "binaryToText: Error: Input is corrupted!\n");
				exit(1);
			}

			// read nanoseconds
			if (get_val_fkt(val, sizeof(u_int32_t), 1) == 0) {
				fprintf(stderr, "binaryToText: Error: Input is corrupted!\n");
				exit(1);
			}
			break;

		case bt_flow_direction:
			// read value
			if (get_val_fkt(val, sizeof(u_int8_t), 1) == 0) {
				fprintf(stderr, "binaryToText: Error: Input is corrupted!\n");
				exit(1);
			}
			break;

		default:
			fprintf(stderr, "ERROR: couldn't determine type of output or output type is not implemented!\nType is: %"PRIu32"\n", type);
			exit(1);
	}
}

/*
 * Parse Binary Value to a char Buffer
 */
void parse_binary_value_to_buffer(uint32_t type, char * buffer) {

	//printf("### parse binary value\n");
	unsigned char val[16];


	char * temp_buffer = malloc(sizeof(char)* 128);
	memset(temp_buffer, '\0', 128);


	char * time_temp_buffer;
	char * string_buffer;

	switch (type) {
		case bt_int_8:
			// read value
			if (get_val_fkt(val, sizeof(int8_t), 1) == 0) {
				fprintf(stderr, "binaryToText: Error: Input is corrupted!\n");
				exit(1);
			}
			// print value
			sprintf(temp_buffer,"%"PRId8, *((int8_t*)val));
			break;

		case bt_int_16:
			// read value
			if (get_val_fkt(val, sizeof(int16_t), 1) == 0) {
				fprintf(stderr, "binaryToText: Error: Input is corrupted!\n");
				exit(1);
			}
			// print value
			sprintf(temp_buffer,"%"PRId16, *((int16_t*)val));
			break;

		case bt_int_32:
			// read value
			if (get_val_fkt(val, sizeof(int32_t), 1) == 0) {
				fprintf(stderr, "binaryToText: Error: Input is corrupted!\n");
				exit(1);
			}
			// print value
			sprintf(temp_buffer,"%"PRId32, *((int32_t*)val));
			break;

		case bt_int_64:
			// read value
			if (get_val_fkt(val, sizeof(int64_t), 1) == 0) {
				fprintf(stderr, "binaryToText: Error: Input is corrupted!\n");
				exit(1);
			}
			// print value
			sprintf(temp_buffer,"%"PRId64, *((int64_t*)val));
			break;

		case bt_uint_8:
			// read value
			if (get_val_fkt(val, sizeof(uint8_t), 1) == 0) {
				fprintf(stderr, "binaryToText: Error: Input is corrupted!\n");
				exit(1);
			}
			// print value
			sprintf(temp_buffer,"%"PRIu8, *((uint8_t*)val));
			break;

		case bt_uint_16:
			// read value
			if (get_val_fkt(val, sizeof(uint16_t), 1) == 0) {
				fprintf(stderr, "binaryToText: Error: Input is corrupted!\n");
				exit(1);
			}
			// print value
			sprintf(temp_buffer,"%"PRIu16, *((uint16_t*)val));
			break;

		case bt_uint_32:
			// read value
			if (get_val_fkt(val, sizeof(uint32_t), 1) == 0) {
				fprintf(stderr, "binaryToText: Error: Input is corrupted!\n");
				exit(1);
			}
			// print value
			sprintf(temp_buffer,"%"PRIu32, *((uint32_t*)val));
			break;

		case bt_uint_64:
			// read value
			if (get_val_fkt(val, sizeof(uint64_t), 1) == 0) {
				fprintf(stderr, "binaryToText: Error: Input is corrupted!\n");
				exit(1);
			}
			// print value
			sprintf(temp_buffer,"%"PRIu64, *((uint64_t*)val));
			break;

		case bt_hex_8:
			// read value
			if (get_val_fkt(val, sizeof(uint8_t), 1) == 0) {
				fprintf(stderr, "binaryToText: Error: Input is corrupted!\n");
				exit(1);
			}
			// print value
			sprintf(temp_buffer,"0x%02"PRIx8, *((uint8_t*)val));
			break;

		case bt_hex_16:
			// read value
			if (get_val_fkt(val, sizeof(uint16_t), 1) == 0) {
				fprintf(stderr, "binaryToText: Error: Input is corrupted!\n");
				exit(1);
			}
			// print value
			sprintf(temp_buffer,"0x%04"PRIx16, *((uint16_t*)val));
			break;

		case bt_hex_32:
			// read value
			if (get_val_fkt(val, sizeof(uint32_t), 1) == 0) {
				fprintf(stderr, "binaryToText: Error: Input is corrupted!\n");
				exit(1);
			}
			// print value
			sprintf(temp_buffer,"0x%08"PRIx32, *((uint32_t*)val));
			break;

		case bt_hex_64:
			// read value
			if (get_val_fkt(val, sizeof(uint64_t), 1) == 0) {
				fprintf(stderr, "binaryToText: Error: Input is corrupted!\n");
				exit(1);
			}
			// print value
			sprintf(temp_buffer,"0x%016"PRIx64, *((uint64_t*)val));
			break;

		case bt_float:
			// read value
			if (get_val_fkt(val, sizeof(float), 1) == 0) {
				fprintf(stderr, "binaryToText: Error: Input is corrupted!\n");
				exit(1);
			}
			// print value
			sprintf(temp_buffer,"%f", *((float*)val));
			break;

		case bt_double:
			// read value
			if (get_val_fkt(val, sizeof(double), 1) == 0) {
				fprintf(stderr, "binaryToText: Error: Input is corrupted!\n");
				exit(1);
			}
			// print value
			sprintf(temp_buffer,"%f", *((double*)val));
			break;

		case bt_long_double:
			// read value
			if (get_val_fkt(val, sizeof(long double), 1) == 0) {
				fprintf(stderr, "binaryToText: Error: Input is corrupted!\n");
				exit(1);
			}
			// print value
			sprintf(temp_buffer,"%Lf", *((long double*)val));
			break;

		case bt_char:
			// read value
			if (get_val_fkt(val, sizeof(unsigned char), 1) == 0) {
				fprintf(stderr, "binaryToText: Error: Input is corrupted!\n");
				exit(1);
			}
			// print value
			sprintf(temp_buffer,"%c", *((unsigned char*)val));
			break;

		case bt_string:
			// read value
			if (get_val_fkt(val, sizeof(unsigned char), 1) == 0) {
				fprintf(stderr, "binaryToText: Error: Input is corrupted!\n");
				exit(1);
			}
			string_buffer = malloc(sizeof(unsigned char) * 2);


			while (*val != '\0') {
				// print value
				memset(string_buffer, '\0', 2);
				sprintf(string_buffer,"%c", *((unsigned char*)val));
				strcat(temp_buffer,string_buffer);
				if (get_val_fkt(val, sizeof(unsigned char), 1) == 0) {
					fprintf(stderr, "binaryToText: Error: Input is corrupted!\n");
					exit(1);
				}
			}

			free(string_buffer);
			break;
		case bt_string_class:
			// read value
			if (get_val_fkt(val, sizeof(unsigned char), 1) == 0) {
				fprintf(stderr, "binaryToText: Error: Input is corrupted!\n");
				exit(1);
			}
			string_buffer = malloc(sizeof(unsigned char) * 2);


			while (*val != '\0') {
				// print value
				memset(string_buffer, '\0', 2);
				sprintf(string_buffer,"%c", *((unsigned char*)val));
				strcat(temp_buffer,string_buffer);
				if (get_val_fkt(val, sizeof(unsigned char), 1) == 0) {
					fprintf(stderr, "binaryToText: Error: Input is corrupted!\n");
					exit(1);
				}
			}

			free(string_buffer);
			break;
		case bt_mac_addr:
			// read value
			if (get_val_fkt(val, l_bt_mac_addr * sizeof(u_int8_t), 1) == 0) {
				fprintf(stderr, "binaryToText: Error: Input is corrupted!\n");
				exit(1);
			}
			sprintf(temp_buffer,"%02"PRIx8":%02"PRIx8":%02"PRIx8":%02"PRIx8":%02"PRIx8":%02"PRIx8, val[0], val[1], val[2], val[3], val[4], val[5]);
			break;
		case bt_ip4_addr:
			// read value
			if (get_val_fkt(val, l_bt_ip4_addr * sizeof(u_int8_t), 1) == 0) {
				fprintf(stderr, "binaryToText: Error: Input is corrupted!\n");
				exit(1);
			}
			sprintf(temp_buffer,"%03"PRIu8".%03"PRIu8".%03"PRIu8".%03"PRIu8, val[0], val[1], val[2], val[3]);
			break;
		case bt_ip6_addr:
			// read value
			if (get_val_fkt(val, l_bt_ip6_addr * sizeof(u_int8_t), 1) == 0) {
				fprintf(stderr, "binaryToText: Error: Input is corrupted!\n");
				exit(1);
			}
			sprintf(temp_buffer,"%04"PRIx16":%04"PRIx16":%04"PRIx16":%04"PRIx16":%04"PRIx16":%04"PRIx16":%04"PRIx16":%04"PRIx16,
					((uint16_t*)val)[0], ((uint16_t*)val)[1], ((uint16_t*)val)[2], ((uint16_t*)val)[3],
					((uint16_t*)val)[4], ((uint16_t*)val)[5], ((uint16_t*)val)[6], ((uint16_t*)val)[7]);
			break;

		case bt_unix_time:
		case bt_time:
			time_temp_buffer = malloc(sizeof(char) * 32);
			memset(time_temp_buffer, '\0', 32);
			// read seconds
			if (get_val_fkt(val, sizeof(u_int64_t), 1) == 0) {
				fprintf(stderr, "binaryToText: Error: Input is corrupted!\n");
				exit(1);
			}
			sprintf(time_temp_buffer,"%"PRIu64".", *((uint64_t*)val));
			strcat(temp_buffer,time_temp_buffer);

			// read nanoseconds
			if (get_val_fkt(val, sizeof(u_int32_t), 1) == 0) {
				fprintf(stderr, "binaryToText: Error: Input is corrupted!\n");
				exit(1);
			}

			sprintf(time_temp_buffer,"%09"PRIu32, *((uint32_t*)val));
			strcat(temp_buffer,time_temp_buffer);

			free(time_temp_buffer);
			break;

		case bt_flow_direction:
			// read value
			if (get_val_fkt(val, sizeof(u_int8_t), 1) == 0) {
				fprintf(stderr, "binaryToText: Error: Input is corrupted!\n");
				exit(1);
			}
			if (*val == 0) sprintf(temp_buffer,"A:");
			else sprintf(temp_buffer,"B:");
			break;

		default:
			fprintf(stderr, "ERROR: couldn't determine type of output or output type is not implemented!\nType is: %"PRIu32"\n", type);
			exit(1);
	}

	//Add Value to buffer
	strcat(buffer,temp_buffer);
	free(temp_buffer);

}

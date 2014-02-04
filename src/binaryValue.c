/*
 * binaryParser.c
 *
 *  Created on: Apr 16, 2010
 *      Author: torben
 */

#include "binaryValue.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <math.h>

binary_header_t* rek_build_header(binary_subvalue_t *sv, binary_header_t *header, uint32_t *currHeaderAlloc);
void analyseSubvalues(uint32_t* header, uint32_t* headerpos, binary_subvalue_t* bv);
void rek_print_out_subheader(binary_subvalue_t *sv, int depth);


/*
 * Append a new binary value to a (list of) binary values
 */
binary_value_t* bv_append_bv(binary_value_t* dest, binary_value_t* new) {
	if (!dest) return new;
	binary_value_t *act_bv = dest;
	while (act_bv->next) act_bv = act_bv->next;
	act_bv->next = new;
	return dest;
}

/*
 * WTF?? add a new subvalue to an existing binary value
 */
binary_value_t* bv_add_sv_to_bv(binary_value_t* dest, uint32_t pos, uint32_t is_repeating, uint32_t num_values, ...) {
	if (!dest || dest->num_values <= pos) return NULL;
	dest->subval[pos].value_type = 0;
	dest->subval[pos].is_repeating = is_repeating;
	dest->subval[pos].num_values = num_values;
	dest->subval[pos].subval = malloc(num_values * sizeof(binary_subvalue_t));

	va_list argPtr;
	va_start(argPtr, num_values);

	uint32_t i;
	for (i = 0; i < num_values; i++) {
		dest->subval[pos].subval[i].value_type = va_arg(argPtr, uint32_t);
	}
	va_end(argPtr);
	return dest;
}

/*
 * add a new subvalue to an existing subvalue
 */
binary_subvalue_t* bv_add_sv_to_sv(binary_subvalue_t* dest, uint32_t pos, uint32_t is_repeating, uint32_t num_values, ...) {
	if (!dest || dest->num_values <= pos) return NULL;
	dest->subval[pos].value_type = 0;
	dest->subval[pos].is_repeating = is_repeating;
	dest->subval[pos].num_values = num_values;
	dest->subval[pos].subval = malloc(num_values * sizeof(binary_subvalue_t));

	va_list argPtr;
	va_start(argPtr, num_values);

	uint32_t i;
	for (i = 0; i < num_values; i++) {
		dest->subval[pos].subval[i].value_type = va_arg(argPtr, uint32_t);
	}
	va_end(argPtr);
	return dest;
}

/*
 * build a new binary value
 */
binary_value_t* bv_new_bv(char* name_long, char* name_short, uint32_t is_repeating, uint32_t num_values, ...) {
	binary_value_t *bv = malloc(sizeof(binary_value_t));
	memset(bv, '\0', sizeof(binary_value_t));
	strncpy(bv->name_value_long, name_long, BP_STRINGBUFFER_SIZE_LONG);
	strncpy(bv->name_value_short, name_short, BP_STRINGBUFFER_SIZE_SHORT);
	bv->num_values = num_values;
	bv->is_repeating = is_repeating;

	bv->subval = malloc(num_values * sizeof(binary_subvalue_t));

	va_list argPtr;
	va_start(argPtr, num_values);

	uint32_t i;
	for (i = 0; i < num_values; i++) {
		bv->subval[i].value_type = va_arg(argPtr, uint32_t);
	}
	va_end(argPtr);
	return bv;
}

/*
 * build a binary header from a (list of) binary_value(s)
 */
binary_header_t* build_header(binary_value_t *bv) {

	binary_value_t *bh_act_bv = bv;
	binary_header_t *header = malloc(sizeof(binary_header_t));
	header->header = malloc(3 * sizeof(uint32_t));
	uint32_t currHeaderAlloc = 3 * sizeof(uint32_t); // the header size to alloc in single bytes
	// write header prefix

	uint64_t magicVal = BV_MAGIC_VALUE_1;
	memcpy(&header->header[0], &magicVal, 4);
	magicVal = BV_MAGIC_VALUE_2;
	memcpy(&header->header[1], &magicVal, 4);
	// memcpy(&header->header[0], "ANTEATER", 8);
	// write version number
	uint32_t header_version = 1;
	memcpy(&header->header[2], &header_version, sizeof(uint32_t));
	header->header_length = 3; // the header size in uint32 portions

	while (bh_act_bv != NULL) {
		// realloc header
		// calc needed space for strings in uint32_t, remind offset and trailing '\0'
		uint32_t size_str_long = ceilf((float) (strlen(bh_act_bv->name_value_long) + 1) / 4.0);
		uint32_t size_str_short = ceilf((float) (strlen(bh_act_bv->name_value_short) + 1) / 4.0);

		uint32_t size_to_alloc = (size_str_long + size_str_short + bh_act_bv->num_values + 2) * sizeof(uint32_t);

		if ((header->header = realloc(header->header, currHeaderAlloc + size_to_alloc)) == NULL) {
			fprintf(stderr, "binaryValue.c: ERROR: realloc failed!\n");
			exit(1);
		}
		currHeaderAlloc += size_to_alloc;

		// write string long
		strncpy((char*) &header->header[header->header_length], bh_act_bv->name_value_long, size_str_long * sizeof(uint32_t));
		header->header_length += size_str_long;

		// write string short
		strncpy((char*) &header->header[header->header_length], bh_act_bv->name_value_short, size_str_short * sizeof(uint32_t));
		header->header_length += size_str_short;
		// write number of values
		header->header[header->header_length++] = bh_act_bv->num_values;

		// write type of values
		uint32_t i;
		for (i = 0; i < bh_act_bv->num_values; i++) {
			header->header[header->header_length++] = bh_act_bv->subval[i].value_type;
			if (bh_act_bv->subval[i].value_type == 0) {
				header = rek_build_header(&bh_act_bv->subval[i], header, &currHeaderAlloc);
			}
		}

		// write repeat flag
		header->header[header->header_length++] = bh_act_bv->is_repeating;

		bh_act_bv = bh_act_bv->next;
	}

	// write trailing end field
	header->header = realloc(header->header, currHeaderAlloc + sizeof(uint32_t));
	header->header[header->header_length++] = UINT32_MAX;

	return header;
}

binary_header_t* rek_build_header(binary_subvalue_t *sv, binary_header_t *header, uint32_t *currHeaderAlloc) {
	// realloc header size
	uint32_t size_to_alloc = (sv->num_values + 2) * sizeof(uint32_t);
	header->header = realloc(header->header, *currHeaderAlloc + size_to_alloc);
	*currHeaderAlloc += size_to_alloc;


	// write number of values
	header->header[header->header_length++] = sv->num_values;

	// write type of values
	uint32_t i;
	for (i = 0; i < sv->num_values; i++) {
		header->header[header->header_length++] = sv->subval[i].value_type;
		if (sv->subval[i].value_type == 0) {
			header = rek_build_header(&sv->subval[i], header, currHeaderAlloc);
		}
	}

	// write repeat flag
	header->header[header->header_length++] = sv->is_repeating;
	return header;
}


/*
 * Kill a binary tranalyzer header
 */
void killHeader(binary_value_t *header){
	unsigned int i;

	//free all subvalue data
	for(i=0;i<header->num_values;i++){
		killSubvalue(&(header->subval[i]));
	}

	//free all subvalues
	free(header->subval);
	header->num_values = 0;

	if(header->next){
		//proceed to the next element
		killHeader(header->next);
	}

	free(header); // free the header itself
}

void killSubvalue(binary_subvalue_t *subval){
	unsigned int i;

	if(subval->value_type == 0){ // contains other subvalues?
		for(i=0;i<subval->num_values;i++)
			killSubvalue(&(subval->subval[i]));
	}

	//free all the subvalues
	free(subval->subval);

	subval->num_values = 0;
}



/*
 * Analyse a binary tranalyzer header and builds a (list of) binary_value_t.
 * Args:
 * header_length: The length of the binary header, including the trailing header end sign (UINT32_MAX)
 * header: The binary header, including the trailing header end sign (UINT32_MAX)
 *
 * Return value:
 * a (list of) binary_value_t
 */
binary_value_t* analyseHeader(binary_header_t *header) {

	uint64_t string_length;
	binary_value_t *bv = NULL, *act_bv = NULL;
	uint32_t i = 0;
	// First check magic value at header start
	if (header->header[0] != BV_MAGIC_VALUE_1 || header->header[1] != BV_MAGIC_VALUE_2) {
		fprintf(stderr, "binaryValue.c: ERROR: header is no tranalyzer header.\n");
		exit(1);
	}

	// check header version. currently, only version 1 is supported
	if (header->header[2] != 1) {
		fprintf(stderr, "binaryValue.c: ERROR: header version %"PRIu32" is not supported.\n", header->header[2]);
		exit(1);
	}

	i = 3;

	while (i < header->header_length - 1) {
		if (!bv) {
			bv = malloc(sizeof(binary_value_t));
			act_bv = bv;
		} else {
			act_bv->next = malloc(sizeof(binary_value_t));
			act_bv = act_bv->next;
		}
		act_bv->next = NULL;

		// get string long
		string_length = strlen((char*) &header->header[i]) + 1;

		// security check
		if (i + ceilf((float) string_length / 4.0) >= header->header_length || string_length >= BP_STRINGBUFFER_SIZE_LONG) {
			printf("ERROR: header malformed 1!\n");
			exit(1);
		}

		strncpy(act_bv->name_value_long, (char*) &header->header[i], BP_STRINGBUFFER_SIZE_LONG);

		// place i at right pos. Because a char is shorter than uint32_t it can be padded
		i += ceilf((float) string_length / 4.0);


		// get string short
		string_length = strlen((char*) &header->header[i]) + 1;

		// security check
		if (i + ceilf((float) string_length / 4.0) >= header->header_length || string_length >= BP_STRINGBUFFER_SIZE_SHORT) {
			printf("ERROR: header malformed 2!\n");
			exit(1);
		}

		strncpy(act_bv->name_value_short, (char*) &header->header[i], BP_STRINGBUFFER_SIZE_SHORT);

		// place i at right pos. Because a char is shorter than uint32_t it can be padded
		i += ceilf((float) string_length / 4.0);

		// get number of subvalues
		if (header->header[i] != UINT32_MAX && header->header[i] != 0) {
			act_bv->num_values = header->header[i];
		} else {
			printf("ERROR: header malformed 3!\n");
			exit(1);
		}

		// build subvalue structs
		act_bv->subval = malloc(act_bv->num_values * sizeof(binary_subvalue_t));

		// get type of subvalues
		// increment i;
		i++;
		int ii;
		for (ii = 0; ii < act_bv->num_values; ii++) {
			// security check
			if (header->header[i] == UINT32_MAX) {
				printf("ERROR: header malformed 4!\n");
				exit(1);
			}

			//Init with default data
			act_bv->subval[ii].value_type = header->header[i];
			act_bv->subval[ii].num_values = 0;
			act_bv->subval[ii].is_repeating = 0;
			act_bv->subval[ii].subval = NULL;

			// check for more subvals
			if (act_bv->subval[ii].value_type == 0) {
				// next val is amount of subvalues
				i++;
				act_bv->subval[ii].num_values = header->header[i];
				// malloc space for subvalues
				act_bv->subval[ii].subval = malloc(act_bv->subval[ii].num_values * sizeof(binary_subvalue_t));
				// fill subvalues
				int iii;

				for (iii = 0; iii < act_bv->subval[ii].num_values; iii++) {
					i++;
					analyseSubvalues(header->header, &i, &act_bv->subval[ii].subval[iii]);
				}
				i++;
				// check if subvalues might be repeated
				act_bv->subval[ii].is_repeating = header->header[i];
			}
			i++;
		}

		// check if subvalues might be repeated
		act_bv->is_repeating = header->header[i];
		i++; // should now be at position of next column
	}

	return bv;
}


void analyseSubvalues(uint32_t* header, uint32_t* headerpos, binary_subvalue_t* bv) {

	// security check
	if (header[*headerpos] == UINT32_MAX) {
		printf("ERROR: header malformed 5!\n");
		exit(1);
	}

	// get type of subvalue
	bv->value_type = header[*headerpos];

	// check if there are more subvalues
	if(bv->value_type == 0) {
		*headerpos += 1;

		// security check
		if (header[*headerpos] == UINT32_MAX) {
			printf("ERROR: header malformed 6!\n");
			exit(1);
		}

		// get amount of subvalues and malloc space for them
		bv->num_values = header[*headerpos];
		bv->subval = malloc(bv->num_values * sizeof(binary_subvalue_t));

		// fill subvals
		int sub_i;
		for (sub_i = 0; sub_i < bv->num_values; sub_i++) {
			*headerpos += 1;
			analyseSubvalues(header, headerpos, &bv->subval[sub_i]);
		}
		*headerpos += 1;

		// security check
		if (header[*headerpos] == UINT32_MAX) {
			printf("ERROR: header malformed 7!\n");
			exit(1);
		}
		bv->is_repeating = header[*headerpos];
	}
}


/*
 * Prints a textual representation of the header to stdout
 */
void print_out_header(binary_value_t* bv) {
	int col = 0;
	while(bv) {
		printf(
				"Col %d:\n"
				"Name long : %s\n"
				"Name short: %s\n"
				"num values: %"PRIu32"\n"
				"repeating?: %"PRIu32"\n",
				col, bv->name_value_long, bv->name_value_short, bv->num_values, bv->is_repeating
		);

		int print_i;
		for (print_i = 0; print_i < bv->num_values; print_i++) {
			printf("Type of value %d: %"PRIu32"\n", print_i, bv->subval[print_i].value_type);
			if (bv->subval[print_i].value_type == 0) {
				rek_print_out_subheader(&(bv->subval[print_i]), 1);
			}
		}

		col++;
		bv = bv->next;
	}
}


void rek_print_out_subheader(binary_subvalue_t *sv, int depth) {
	int depth_i;
	for (depth_i = 0; depth_i < depth; depth_i++) printf("  ");
	printf("num values: %"PRIu32"\n", sv->num_values);
	for (depth_i = 0; depth_i < depth; depth_i++) printf("  ");
	printf("repeating?: %"PRIu32"\n", sv->is_repeating);

	int amount_i;
	for (amount_i = 0; amount_i < sv->num_values; amount_i++) {
		for (depth_i = 0; depth_i < depth; depth_i++) printf("  ");
		printf("Type of value %d: %"PRIu32"\n", amount_i, sv->subval[amount_i].value_type);
		if (sv->subval[amount_i].value_type == 0) {
			rek_print_out_subheader(&(sv->subval[amount_i]), depth + 1);
		}
	}
}

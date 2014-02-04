/*
 * binaryParser.h
 *
 *  Created on: Apr 16, 2010
 *      Author: torben
 */

#ifndef BINARYVALUE_H_
#define BINARYVALUE_H_

#define BP_STRINGBUFFER_SIZE_LONG 1024
#define BP_STRINGBUFFER_SIZE_SHORT 128

#define BV_MAGIC_VALUE_2 0x52455441 // the Magic Value that should be at the start of every tranalyzer binary save file, part 2
#define BV_MAGIC_VALUE_1 0x45544e41 // the Magic Value that should be at the start of every tranalyzer binary save file, part 1

#include <inttypes.h>

/*
 * Definition of types
 */
enum binary_types {
	// all signed integers
	bt_int_8 = 1,
	bt_int_16,
	bt_int_32,
	bt_int_64,
	bt_int_128,
	bt_int_256,
	// all unsigned integers
	bt_uint_8,
	bt_uint_16,
	bt_uint_32,
	bt_uint_64, // = 10
	bt_uint_128,
	bt_uint_256,
	// hex values
	bt_hex_8,
	bt_hex_16,
	bt_hex_32,
	bt_hex_64,
	bt_hex_128,
	bt_hex_256, // 32 bytes
	// floating point
	bt_float,
	bt_double, // = 20
	bt_long_double,
	// char and string
	bt_char,
	bt_string,
	// now the special types
	bt_flow_direction,
	bt_unix_time, 	// the time struct consists of one uint_64 value for the seconds
					// and a uint_32_t value for the nanosecs
	bt_time, 		// this is the same than above, but rather than unix_time, this
					// type is for time data where it makes no sense to calculate an
					// absolute date (e.g. time durations) but where a textual output
					// could be changed e.g. from seconds to minutes or hours.
	bt_mac_addr,
	bt_ip4_addr,
	bt_ip6_addr,
	bt_string_class,	// A string for classnames. CAUTION: textOutput doesn't escape control characters.
						// Advantage: We don't need '"' chars at the beginning and end of the string
						// Disadvantage: Usage of underscore, blank, semicolon, and every non-printable
						// ASCII are STRICTLY FORBIDDEN !!! If your classnames contain such chars, you'll
						// CORRUPT THE OUTPUT!!!
};

/*
 * Definition of typelengths in bytes
 */
enum binary_types_lengths {
	// all signed integers
	l_bt_int_8 = 1,
	l_bt_int_16 = 2,
	l_bt_int_32 = 4,
	l_bt_int_64 = 8,
	l_bt_int_128 = 16,
	l_bt_int_256 = 32,
	// all unsigned integers
	l_bt_uint_8 = 1,
	l_bt_uint_16 = 2,
	l_bt_uint_32 = 4,
	l_bt_uint_64 = 8,
	l_bt_uint_128 = 16,
	l_bt_uint_256 = 32,
	// hex values
	l_bt_hex_8 = 1,
	l_bt_hex_16 = 2,
	l_bt_hex_32 = 4,
	l_bt_hex_64 = 8,
	l_bt_hex_128 = 16,
	l_bt_hex_256 = 32,
	// floating point
	l_bt_float = 4,
	l_bt_double = 8,
	l_bt_long_double = 10,
	// char and string
	l_bt_char = 1,
	l_bt_string = 0, // because a string is a set of chars with variable amount, this has to be handled special
	// now the special types
	l_bt_flow_direction = 1,
	l_bt_unix_time = 12, // see def in enum binary types
	l_bt_time = 12,
	l_bt_mac_addr = 6,
	l_bt_ip4_addr = 4,
	l_bt_ip6_addr = 16,
	l_bt_string_class = 0, // because a string is a set of chars with variable amount, this has to be handled special
};

typedef struct binary_subvalue_s {
	uint32_t value_type; // the type of the value. If 0, it contains subvalues
	uint32_t num_values; // the amount of subvalues
	struct binary_subvalue_s *subval; // the definition of the subvalues
	uint32_t is_repeating; // are the subvalues repeating?
} binary_subvalue_t;


typedef struct binary_value_s {
	uint32_t num_values; // the amount of values in this column
	binary_subvalue_t *subval; // the definition of the subvalues
	uint32_t is_repeating; // are the subvalues repeating?
	char name_value_short[BP_STRINGBUFFER_SIZE_SHORT]; // a long string representation
	char name_value_long[BP_STRINGBUFFER_SIZE_LONG]; // a short string representation
	struct binary_value_s *next; // the next column
} binary_value_t;


typedef struct binary_header_s {
	uint32_t *header; // The header including the trailing terminate sign
	uint32_t header_length; // The length of the header (in uint32_t) including the trailing terminate sign
} binary_header_t;


binary_value_t* bv_append_bv(binary_value_t* dest, binary_value_t* new);
binary_value_t* bv_new_bv(char* name_long, char* name_short, uint32_t is_repeating, uint32_t num_values, ...);
binary_value_t* bv_add_sv_to_bv(binary_value_t* dest, uint32_t pos, uint32_t is_repeating, uint32_t num_values, ...);
binary_subvalue_t* bv_add_sv_to_sv(binary_subvalue_t* dest, uint32_t pos, uint32_t is_repeating, uint32_t num_values, ...);
binary_header_t* build_header(binary_value_t *bv);
binary_value_t* analyseHeader(binary_header_t *header);
void print_out_header(binary_value_t* bv);

/*
*	killHeader(header) : deconstruct (free) a binary tranalyzer-header
*	Arguments:
*		- header : The first headerelement of the header to kill
*/
void killHeader(binary_value_t *header);


/*
*	killSubvalue(header) : deconstruct (free) a binary tranalyzer-header-subvalue
*	Arguments:
*		- subval : The subvalue to kill
*/
void killSubvalue(binary_subvalue_t *subval);


#endif /* BINARYVALUE_H_ */

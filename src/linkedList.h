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


#ifndef LINKEDLIST_H_
#define LINKEDLIST_H_

#include "include.h"


typedef struct linkedlist_item_s{
	void * item;
	struct linkedlist_item_s *next_item;
} linkedlist_item_t;

typedef struct linkedlist_s{
	linkedlist_item_t * head;
	linkedlist_item_t * tail;
	linkedlist_item_t * current_item;
	int current_index;
	int size;
} linkedlist_t;

linkedlist_item_t * linkedlist_add_item(linkedlist_t *list, linkedlist_item_t *item);
linkedlist_t * linkedlist_new();
linkedlist_item_t * linkedlist_item_new(void * item);
void * linkedlist_item_get(linkedlist_t * list, int index);
linkedlist_item_t * linkedlist_item_skipto(linkedlist_t * list, int index);
void clear_linkedlist(linkedlist_t * list, int depth);
int remove_linked_list_item_at_index(linkedlist_t *list, int index);

#endif /* LINKEDLIST_H_ */

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


#include "linkedList.h"

/*
 * Adds a new Item to a Linked List
 */
linkedlist_item_t * linkedlist_add_item(linkedlist_t *list, linkedlist_item_t *item) {

	if(list->tail == NULL) {
		//First Entry
		list->head = item;
		list->tail = item;
	} else {
		//Add to tail and set new tail
		list->tail->next_item = item;
		list->tail = item;
	}

	list->size++;

	// Reset current pointer
	list->current_item = list->head;
	list->current_index = 0;

	return list->tail;
}


/*
 * Return a new Linked List
 */
linkedlist_t * linkedlist_new() {
	linkedlist_t * list = malloc(sizeof(linkedlist_t));
	list->head = NULL;
	list->tail = NULL;
	list->current_item = NULL;
	list->current_index = 0;
	list->size = 0;


	return list;
}

/*
 * Return a new linked List Item
 */
linkedlist_item_t * linkedlist_item_new(void * item) {
	linkedlist_item_t * new_item = malloc(sizeof(linkedlist_item_t));

	new_item->item = item;
	new_item->next_item = NULL;

	return new_item;
}

/*
 * Return the real Item (void Pointer) in list at Index
 */
void * linkedlist_item_get(linkedlist_t * list, int index) {
	return linkedlist_item_skipto(list, index)->item;
}

/*
 * Return linkedListItem of list at Index
 */
linkedlist_item_t * linkedlist_item_skipto(linkedlist_t * list, int index) {


	// Check if we can skip from current Position.
	// this just works when the requested Index is higher
	// than the current Index. If not we have to reset to the Head!
	if(list->current_item == NULL || index < list->current_index) {
		list->current_item = list->head;
		list->current_index = 0;
	}

	int i;
	for (i = list->current_index+1; i <= index; i++) {
		list->current_item = list->current_item->next_item;
		list->current_index = i;
	}


	return list->current_item;
}

/*
 * Clear the whole linked list and free all items
 *
 * It is possible to have linkedlists in a linkslist.
 * if so, it is possible to clear all linkedlist within.
 * depth is the level of linkedlist's to clear
 *
 */
void clear_linkedlist(linkedlist_t * list, int depth) {

	linkedlist_item_t * last = list->head;
	linkedlist_item_t * next;

	if(last != NULL) { // Are there any Elements at all?
		while (last != NULL) {

			if(depth > 1) { //Recursive call if we have Lists in Lists
				clear_linkedlist((linkedlist_t *) last->item, depth - 1);
			}

			next = last->next_item;
			free(last);
			last = next;

		}
	}

	free(list);
}

/*
 * Remove a linked_list_item at specified index
 *
 */
int remove_linked_list_item_at_index(linkedlist_t *list, int index) {

	// Check if index is valid
	if (index < 0 || index >= list->size)
		return -1;

	linkedlist_item_t * current_item = list->head;
	linkedlist_item_t * last_item = current_item;

	// Get Item to remove
	int i;
	for (i = 0; i < index; i++) {
		last_item = current_item;
		current_item = current_item->next_item;
	}

	if (list->size > 0) list->size -=1;


	if (current_item == list->head && current_item == list->tail) { // Only one Element
		list->tail = NULL;
		list->head = NULL;
		free(current_item);
	} else {
		if (current_item == list->head || current_item == list->tail) { // Is it first or last?
			if (current_item == list->head) {  // First
				list->head = current_item->next_item;
			} else { // Last
				last_item->next_item = NULL;
				list->tail = last_item;
			}
			free(current_item);
		} else { // Just a normal Element
			last_item->next_item = current_item->next_item;
			free(current_item);
		}
	}

	// Reset current pointer
	list->current_item = list->head;
	list->current_index = 0;


	return 0;
}

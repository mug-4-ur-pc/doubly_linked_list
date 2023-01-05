/*!
 * @file An implementation of doubly linked list.
 */

#include <assert.h>
#include <stdlib.h>
#include <memory.h>

#include "list.h"




/*!
 * @brief Print list element by bytes.
 */
static void list_print_bytes
(
	const void* elem,  /*!< [in]     pointer to a list element.              */
	size_t      size,  /*!< [in]     size of element.                        */
	FILE*       stream /*!< [in,out] output stream.                          */
)
{
	assert (elem);	assert (size);
	assert (stream);

	for (size_t i = 0; i < size; ++i)
		fprintf(stream, "%hhx", *((unsigned char*) elem + i));
}

/*!
 * @brief Write list dump to .dot file.
 */
static void list_write_dump_to_dot
(
	const list_t lst,       /*!< [in]     list.                              */
	FILE* dump,             /*!< [in,out] dump .dot file.                    */
	const char*  lst_name,  /*!< [in]     name of the list variable.         */
	size_t       line,      /*!< [in]     line where dump function
	                                      was called.                        */
	const char*  func_name, /*!< [in]     function name where dump function was
	                                      called.                            */
	const char*  file_name  /*!< [in]     file name where dump function was
	                                      called.                            */
)
{
	fprintf(dump, "digraph g {\n"
		"\tbgcolor = \"black\";\n"
		"\tnode [color = \"yellow\", fontcolor = \"indigo\", "
			"style = \"filled\", shape = \"record\"];\n"
		"\tfontcolor = \"white\";"
		"\n\tlabel = \"%s from %zd:%s:%s\\nCapacity = %zd\\nSize = %zd\\n"
			"Element size = %zd\\nFirst free = %zd\\n"
			"Head = %zd\\nTail = %zd\\n%s\\n"
			"Data pointer = %p\\nNext elements pointer = %p\\n"
			"Previous elements pointer = %p\";\n",
		lst_name, line, func_name, file_name,
		lst->capacity, lst->size, lst->elem_size, lst->first_free,
		lst->head, lst->tail,
		(lst->normalized) ? "Normalized" : "Not normalized",
		lst->data, (void*) lst->nexts, (void*) lst->prevs);

	if (!lst->data || !lst->nexts || !lst->prevs)
		return;

	fprintf(dump, "\n\tL0 [label = \"<LP0> %zd | {0 | ---} | <LN0> %zd\"];\n",
		lst->prevs[0], lst->nexts[0]);

	for (size_t i = 1; i < lst->capacity; ++i)
	{
		if (lst->prevs[i] == i)
		{
			fprintf(dump, "\tL%zd [color = \"orange\","
				"label = \"<LP%zd> %zd | {%zd | ---} | <LN%zd> %zd\"];\n",
				i, i, lst->prevs[i], i, i, lst->nexts[i]);
		}
		else
		{
			fprintf(dump, "\tL%zd [color = \"green\","
				"label = \"<LP%zd> %zd | {%zd | ",
				i, i, lst->prevs[i], i);

			if (lst->print_elem_func)
			{
				lst->print_elem_func((char*) lst->data + i * lst->elem_size,
				                     dump);
			}
			else
			{
				list_print_bytes((char*) lst->data + i * lst->elem_size,
				                 lst->elem_size, dump);
			}

			fprintf(dump, "} | <LN%zd> %zd\"];\n", i, lst->nexts[i]);
		}
	}

	fprintf(dump, "\tL%zd [label = \"Non-existent element\", "
		"color = \"red\"];\n",
		lst->capacity);

	fprintf(dump, "\n\tnode [color = \"black\",fontcolor = \"black\"];\n"
		"\tLH0");
	for (size_t i = 1; i <= lst->capacity; ++i)
		fprintf(dump, " -> LH%zd", i);
	fprintf(dump, " [weight = 100];\n\n");

	for (size_t i = 0; i <= lst->capacity; ++i)
	{
		fprintf(dump, "\t{rank = same; LH%zd; L%zd}\n", i, i);
	}

	for (size_t i = 0; i < lst->capacity; ++i)
	{
		fprintf(dump, "\tL%zd:<LN%zd> -> L%zd:<LN%zd> [color = %s];\n",
			i, i,
			(lst->nexts[i] < lst->capacity) ? lst->nexts[i] : lst->capacity,
			(lst->nexts[i] < lst->capacity) ? lst->nexts[i] : lst->capacity,
			(lst->prevs[i] == i) ? "\"white\", style = \"dotted\"" : "\"blue\"");

		if (lst->prevs[i] != i)
		{
			fprintf(dump, "\tL%zd:<LP%zd> -> L%zd:<LP%zd> [color = \"pink\"];\n",
				i, i,
				(lst->prevs[i] < lst->capacity) ? lst->prevs[i]
					: lst->capacity,
				(lst->prevs[i] < lst->capacity) ? lst->prevs[i]
					: lst->capacity);
		}
	}

	fprintf(dump, "}\n");
}

/*!
 * @brief Prepare first free element to making it used.
 *
 * @return Error code which has been occurred during performing this function.
 */
static list_error_t list_remove_first_free
(
	list_t           lst, /*!< [in]  list                                    */
	list_iterator_t* it   /*!< [out[ pointer to iterator in which prepared
	                                 element will be assigned.               */
)
{
	if (lst->size == lst->capacity)
	{
		list_error_t err = list_change_capacity(lst, lst->capacity
		                                             * CAPACITY_COEFF);
		if (err != LIST_NO_ERR)
			return err;
	}

	++lst->size;
	*it             = lst->first_free;
	lst->first_free = lst->nexts[lst->first_free];

	return LIST_NO_ERR;
}

/*!
 * @brief Swap two values in data array of the list.
 */
static void list_swap_vals
(
	list_t                lst, /*!< [in,out] list.                           */
	const list_iterator_t it1, /*!< [in]     first iterator.                 */
	const list_iterator_t it2  /*!< [in]     second iterator.                */
)
{
	memcpy(lst->data,
	       (char*) lst->data + it1 * lst->elem_size, lst->elem_size);
	memcpy((char*) lst->data + it1 * lst->elem_size,
	       (char*) lst->data + it2 * lst->elem_size, lst->elem_size);
	memcpy((char*) lst->data + it2 * lst->elem_size,
	       lst->data, lst->elem_size);
}


list_t list_create_func_ (size_t start_capacity,
                          void (*print_func) (const void*, FILE*),
                          size_t elem_size)
{
	if (!elem_size)
		return NULL;

	list_t lst = (list_t) calloc(1, sizeof *lst);
	if (!lst)
		return NULL;

	++start_capacity;
	lst->data  =           calloc(start_capacity, elem_size);
	lst->nexts = (size_t*) calloc(start_capacity, sizeof *lst->nexts);
	lst->prevs = (size_t*) calloc(start_capacity, sizeof *lst->prevs);
	if (!lst->data || !lst->nexts || !lst->prevs)
		return list_destroy(lst);

	lst->head            = 0;
	lst->tail            = 0;
	lst->size            = 1;
	lst->capacity        = start_capacity;
	lst->elem_size       = elem_size;
	lst->first_free      = 1;
	lst->head            = 0;
	lst->tail            = 0;
	lst->normalized      = true;
	lst->print_elem_func = print_func;
	lst->nexts[0]        = 0;
	lst->prevs[0]        = 0;
	for (size_t i = 1; i < lst->capacity; ++i)
	{
		lst->nexts[i] = (i + 1) % start_capacity;
		lst->prevs[i] = i;
	}

	return lst;
}


list_t list_destroy (list_t lst)
{
	if (!lst)
		return NULL;

	free(lst->data);
	free(lst->nexts);
	free(lst->prevs);
	free(lst);

	return NULL;
}


void* list_get (const list_t lst, const list_iterator_t it)
{
	assert (lst);
	assert (list_verify(lst) == LIST_NO_ERR);

	if (!list_check_iterator(lst, it))
		return NULL;

	return (char*) lst->data + it * lst->elem_size;
}


list_error_t list_insert_after (list_t lst,
                                const list_iterator_t it, const void* value)
{
	assert (lst);
	assert (value);
	assert (list_verify(lst) == LIST_NO_ERR);

	if (!list_check_iterator(lst, it))
		return LIST_BAD_ITERATOR;

	list_iterator_t place_to_insert;
	list_error_t err = list_remove_first_free(lst, &place_to_insert);
	if (err != LIST_NO_ERR)
		return err;

	memcpy((char*) lst->data + place_to_insert * lst->elem_size,
	       value, lst->elem_size);
	lst->nexts[place_to_insert]             = lst->nexts[it];
	lst->nexts[it]                          = place_to_insert;
	lst->prevs[place_to_insert]             = it;
	lst->prevs[lst->nexts[place_to_insert]] = place_to_insert;

	if (lst->nexts[place_to_insert] == 0)
		lst->tail = place_to_insert;
	else
		lst->normalized = false;

	if (lst->prevs[place_to_insert] == 0)
		lst->head = place_to_insert;

	return LIST_NO_ERR;
}


list_error_t list_insert_before (list_t lst, const list_iterator_t it,
                                 const void* value)
{
	assert (lst);
	assert (value);
	assert (list_verify(lst) == LIST_NO_ERR);

	return list_insert_after(lst, lst->prevs[it], value);
}


list_error_t list_insert_to_head (list_t lst, const void* value)
{
	assert (lst);
	assert (value);
	assert (list_verify(lst) == LIST_NO_ERR);

	return list_insert_before(lst, lst->head, value);
}


list_error_t list_insert_to_tail (list_t lst, const void* value)
{
	assert (lst);
	assert (value);
	assert (list_verify(lst) == LIST_NO_ERR);

	return list_insert_after(lst, lst->tail, value);
}


list_iterator_t list_next (const list_t lst, const list_iterator_t it)
{
	assert (lst);
	assert (list_verify(lst) == LIST_NO_ERR);

	if (!list_check_iterator(lst, it))
		return LIST_BAD_ITERATOR;

	return (it) ? lst->nexts[it] : 0;
}


list_iterator_t list_prev (const list_t lst, const list_iterator_t it)
{
	assert (lst);
	assert (list_verify(lst) == LIST_NO_ERR);

	if (!list_check_iterator(lst, it))
		return LIST_BAD_ITERATOR;

	return (it) ? lst->prevs[it] : 0;
}


#ifdef NDEBUG
#	define LIST_DUMP_RET(ERR_CODE_) return ERR_CODE_
#else
#	define LIST_DUMP_RET(ERR_CODE_)                                                  \
		do                                                                    \
		{                                                                     \
			list_perror(ERR_CODE_, stderr);                                   \
			list_dump_func_(lst, lst_name, line, func_name, file_name);       \
			return ERR_CODE_;                                                 \
		}                                                                     \
		while (false)
#endif // defined NDEBUG

list_error_t list_verify_func_ (const list_t lst, const char* lst_name,
                                size_t line, const char* func_name,
                                const char* file_name)
{
	if (!lst)
		return LIST_NO_ERR;

	if (!lst->data || !lst->nexts || !lst->prevs)
		LIST_DUMP_RET(LIST_BAD_MEMORY);

	if (!lst->size || lst->capacity < lst->size)
		LIST_DUMP_RET(LIST_BAD_CAPACITY);

	if (!lst->elem_size)
		LIST_DUMP_RET(LIST_BAD_ELEM_SIZE);

	if ((lst->first_free >= lst->capacity
	    || lst->prevs[lst->first_free] != lst->first_free)
	    && lst->capacity != 1 && lst->first_free)
		LIST_DUMP_RET(LIST_BAD_FIRST_FREE_ELEM);

	if (lst->head >= lst->capacity || (lst->size == 1 && lst->head))
		LIST_DUMP_RET(LIST_BAD_HEAD_ITERATOR);

	if (lst->tail >= lst->capacity || (lst->size == 1 && lst->tail))
		LIST_DUMP_RET(LIST_BAD_TAIL_ITERATOR);

	if (lst->capacity == 1)
		return LIST_NO_ERR;

	size_t free_amount = 0;
	for (list_iterator_t free_it = lst->first_free;
	     free_it;
	     free_it = lst->nexts[free_it])
	{
		if (free_amount++ > lst->capacity - lst->size
		    || lst->prevs[free_it] != free_it
		    || lst->nexts[free_it] == free_it)
			LIST_DUMP_RET(LIST_BAD_FREE_FIELDS);
	}

	size_t elems_amount = 0;
	for (list_iterator_t it = lst->head; it; it = lst->nexts[it])
	{
		if (elems_amount++ >= lst->size
		    || it != lst->nexts[lst->prevs[it]]
			|| it != lst->prevs[lst->nexts[it]])
			LIST_DUMP_RET(LIST_BAD_BUSY_FIELDS);
	}

	if (lst->prevs[0] != lst->tail)
		LIST_DUMP_RET(LIST_BAD_BUSY_FIELDS);

	return LIST_NO_ERR;
}

#undef LIST_DUMP_RET


list_error_t list_change_capacity (list_t lst, size_t new_capacity)
{
	assert (lst);
	assert (list_verify(lst) == LIST_NO_ERR);
	
	++new_capacity;
	if (new_capacity < lst->size)
		return LIST_BAD_CAPACITY;

	if (new_capacity < lst->capacity)
		list_normalize(lst);

	void*   new_data  = calloc(new_capacity, lst->elem_size);
	size_t* new_nexts = (size_t*) calloc(new_capacity, sizeof *lst->nexts);
	size_t* new_prevs = (size_t*) calloc(new_capacity, sizeof *lst->prevs);

	if (!new_data || !new_nexts || !new_prevs)
	{
		free(new_data);
		free(new_nexts);
		free(new_prevs);
		return LIST_ALLOC_ERR;
	}

	memcpy(new_data,  lst->data,  new_capacity * lst->elem_size);
	memcpy(new_nexts, lst->nexts, new_capacity * sizeof *lst->nexts);
	memcpy(new_prevs, lst->prevs, new_capacity * sizeof *lst->prevs);

	if (new_capacity > lst->capacity)
	{
		lst->first_free = (lst->first_free) ? lst->first_free : lst->size;

		list_iterator_t free = lst->first_free;
		for (list_iterator_t next = lst->nexts[free]; next; next = lst->nexts[free])
			free = next;

		new_nexts[free] = lst->size;
		for (size_t i = lst->size; i < new_capacity; ++i)
		{
			new_nexts[i] = (i + 1) % new_capacity;
			new_prevs[i] = i;
		}
	}

	free(lst->data);
	free(lst->nexts);
	free(lst->prevs);

	lst->data     = new_data;
	lst->nexts    = new_nexts;
	lst->prevs    = new_prevs;
	lst->capacity = new_capacity;

	return LIST_NO_ERR;
}


list_iterator_t list_head (const list_t lst)
{
	assert (lst);
	assert (list_verify(lst) == LIST_NO_ERR);

	return lst->head;
}


list_iterator_t list_tail (const list_t lst)
{
	assert (lst);
	assert (list_verify(lst) == LIST_NO_ERR);

	return lst->tail;
}


list_error_t list_erase (list_t lst, list_iterator_t* it)
{
	assert (lst);
	assert (it);
	assert (list_verify(lst) == LIST_NO_ERR);

	if (!list_check_iterator(lst, *it))
		return LIST_BAD_ITERATOR;

	if (!*it)
		return LIST_NO_ERR;

	list_iterator_t next = lst->nexts[*it];
	list_iterator_t prev = lst->prevs[*it];

	lst->nexts[prev] = next;
	lst->prevs[next] = prev;
	
	lst->nexts[*it] = lst->first_free;
	lst->prevs[*it] = *it;
	lst->first_free = *it;

	if (*it == lst->head)
		lst->head = next;

	if (*it == lst->tail)
		lst->tail = prev;
	else
		lst->normalized = false;

	--lst->size;
	*it = (next) ? next : prev;
	return LIST_NO_ERR;
}


list_error_t list_erase_by_value (list_t lst, const void* val)
{
	assert (lst);
	assert (val);
	assert (list_verify(lst) == LIST_NO_ERR);

	list_iterator_t it = list_find(lst, val);
	return list_erase(lst, &it);

}


list_error_t list_erase_by_index (list_t lst, size_t index)
{
	assert (lst);
	assert (list_verify(lst) == LIST_NO_ERR);

	list_iterator_t it = list_element_at(lst, index);
	return list_erase(lst, &it);
}


list_iterator_t list_find (const list_t lst, const void* value)
{
	assert (lst);
	assert (value);
	assert (list_verify(lst) == LIST_NO_ERR);

	for (list_iterator_t it = lst->head; it; it = lst->nexts[it])
	{
		if (!memcmp((char*) lst->data + it * lst->elem_size, value,
		           lst->elem_size))
		{
			return it;
		}
	}
	
	return 0;
}


list_iterator_t list_element_at (const list_t lst, size_t index)
{
	assert (lst);
	assert (list_verify(lst) == LIST_NO_ERR);

	if (++index >= lst->size)
		return LIST_BAD_INDEX;

	if (lst->normalized)
		return index;

	list_iterator_t it = list_head(lst);
	for (size_t i = 1; i < index; ++i)
		it = list_next(lst, it);

	return it;
}


list_error_t list_clear (list_t lst)
{
	assert (lst);
	assert (list_verify(lst) == LIST_NO_ERR);
	
	lst->normalized  = true;
	lst->size        = 1;
	list_error_t err = list_change_capacity(lst, 0);
	if (err != LIST_NO_ERR)
		return err;

	lst->nexts[0] = 0;
	lst->prevs[0] = 0;

	return LIST_NO_ERR;
}


bool list_check_iterator (const list_t lst, const list_iterator_t it)
{
	return !it || (it < lst->capacity && lst->prevs[it] != it);
}


bool list_iterator_on_element (const list_iterator_t it)
{
	return it;
}


void list_print (const list_t lst, FILE* stream)
{
	assert (lst);
	assert (stream);
	assert (list_verify(lst) == LIST_NO_ERR);

	fprintf(stream, "[ ");
	for (list_iterator_t it = list_head(lst); it; it = list_next(lst, it))
	{
		if (lst->print_elem_func)
			lst->print_elem_func(list_get(lst, it), stream);
		else
			list_print_bytes(list_get(lst, it), lst->elem_size, stream);
		fputc(' ', stream);
	}
	fputc(']', stream);
}


void list_dump_func_ (const list_t lst, const char* lst_name, size_t line,
                      const char* func_name, const char* file_name)
{
	assert (lst_name);
	assert (func_name);
	assert (file_name);

	char dump_fname[LIST_MAX_FNAME] = {0};
	sprintf(dump_fname, "%s_%zd_%s_%s.dot",
	        lst_name, line, func_name, file_name);

	FILE* dump = fopen(dump_fname, "w");
	if (!dump)
	{
		perror("List dump failed");
		return;
	}
	
	list_write_dump_to_dot(lst, dump, lst_name, line, func_name, file_name);
	fclose(dump);

	char command[LIST_MAX_FNAME * 2] = {0};
	sprintf(command, "dot %s_%zd_%s_%s.dot -Tpng -o %s_%zd_%s_%s.png",
	        lst_name, line, func_name, file_name,
	        lst_name, line, func_name, file_name);

	system(command);
}


void list_normalize (list_t lst)
{
	assert (lst);
	assert (list_verify(lst) == LIST_NO_ERR);

	if (lst->normalized)
		return;

	if (lst->size == 1)
	{
		lst->normalized = true;
		return;
	}

	lst->normalized    = true;
	list_iterator_t it = lst->head;
	for (size_t i = 1; i < lst->size; ++i)
	{
		if (i == it)
		{
			it = lst->nexts[it];
			lst->nexts[i] = (i + 1) % lst->size;
			lst->prevs[i] = i - 1;
			continue;
		}

		list_iterator_t tmp_it    = lst->nexts[it];
		lst->nexts[it]            = lst->nexts[i];
		list_swap_vals(lst, i, it);

		size_t* tmp = &lst->nexts[lst->prevs[i]];
		*tmp        = (*tmp > i) ? it : *tmp;

		lst->prevs[i] = i - 1;
		lst->nexts[i] = (i + 1) % lst->size;
		
		it = tmp_it;
	}

	lst->head       = 1;
	lst->tail       = lst->size - 1;
	lst->nexts[0]   = lst->head;
	lst->prevs[0]   = lst->tail;
	lst->first_free = (lst->size < lst->capacity || lst->size == 1)
	                  ? lst->size : 0;

	for (size_t i = lst->size; i < lst->capacity; ++i)
	{
		lst->nexts[i] = (i + 1) % lst->capacity;
		lst->prevs[i] = i;
	}
}


bool list_is_normalized (const list_t lst)
{
	assert (lst);
	assert (list_verify(lst) == LIST_NO_ERR);

	return lst->normalized;
}


#define LIST_PERROR_CASE(STR_)                                                     \
	fputs(STR_, stream); fputc('\n', stream); break

void list_perror (list_error_t err, FILE* stream)
{
	assert (stream);

	switch (err)
	{
		case LIST_NO_ERR: break;

		case LIST_ALLOC_ERR:    LIST_PERROR_CASE("allocation error");
		case LIST_BAD_ITERATOR: LIST_PERROR_CASE("list iterator invalid");
		case LIST_BAD_INDEX:    LIST_PERROR_CASE("index out of range");
		case LIST_EMPTY:        LIST_PERROR_CASE("list is empty");

		case LIST_BAD_CAPACITY:  LIST_PERROR_CASE("capacity less than size");
		case LIST_BAD_ELEM_SIZE: LIST_PERROR_CASE("size of element equals to 0");
		case LIST_BAD_MEMORY:    LIST_PERROR_CASE("bad pointer to list data");

		case LIST_BAD_FIRST_FREE_ELEM: LIST_PERROR_CASE("bad first free field");
		case LIST_BAD_HEAD_ITERATOR:   LIST_PERROR_CASE("bad head iterator");
		case LIST_BAD_TAIL_ITERATOR:   LIST_PERROR_CASE("bad tail iterator");

		case LIST_BAD_FREE_FIELDS: LIST_PERROR_CASE("bad some free fields");
		case LIST_BAD_BUSY_FIELDS: LIST_PERROR_CASE("bad some busy fields");
		default:                   LIST_PERROR_CASE("unknown error");
	}
}

#undef LIST_PERROR_CASE


size_t list_size (const list_t lst)
{
	return lst->size - 1;
}


size_t list_capacity (const list_t lst)
{
	return lst->capacity - 1;
}

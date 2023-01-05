/*!
 * @brief Header file with doubly linked list implementation.
 */


#ifndef LIST_H_
#define LIST_H_

#include <stddef.h>
#include <stdbool.h>
#include <stdio.h>




/*!
 * @brief Max dump file name.
 */
#define LIST_MAX_FNAME ((size_t) 8192)

/*!
 * @brief Coefficient that shows how many times will the value
 * of list capacity change.
 */
#define CAPACITY_COEFF ((size_t) 2)




/*!
 * @brief Iterator of list elements.
 */
typedef size_t list_iterator_t;

/*!
 * @brief Double linked list structure.
 */
typedef struct list_t_
{
	void*           data;       /*!< array with data.                        */
	size_t*         nexts;      /*!< array with indexes of next elements.    */
	size_t*         prevs;      /*!< array with indexes of previous elements.*/
	size_t          elem_size;  /*!< size of one element.                    */
	size_t          size;       /*!< amount of elements in list.             */
	size_t          capacity;   /*!< current capacity of list.               */
	list_iterator_t first_free; /*!< index of first free element.            */
	list_iterator_t head;       /*!< head of the list.                       */
	list_iterator_t tail;       /*!< tail of the list.                       */
	bool            normalized; /*!< Is elements in list has the order
	                                 like in array. It becomes false
	                                 when you erasing or inserting elements
	                                 not in tail.                            */

	void (*print_elem_func) (const void*, FILE*); /*!< function which prints
	                                                   one list element.     */
}
*list_t;

/*!
 * @brief Enum with possible list's errors.
 */
typedef enum
{
	LIST_NO_ERR              = 0,
	LIST_ALLOC_ERR           = 1,
	LIST_BAD_ITERATOR        = 2,
	LIST_BAD_INDEX           = 3,
	LIST_EMPTY               = 4,
	LIST_BAD_CAPACITY        = 5,
	LIST_BAD_ELEM_SIZE       = 6,
	LIST_BAD_MEMORY          = 7,
	LIST_BAD_FIRST_FREE_ELEM = 8,
	LIST_BAD_HEAD_ITERATOR   = 9,
	LIST_BAD_TAIL_ITERATOR   = 10,
	LIST_BAD_FREE_FIELDS     = 11,
	LIST_BAD_BUSY_FIELDS     = 12,
}
list_error_t;




/*!
 * @brief Create new list.
 *
 * @note Don't forget to free memory using list_destroy() function.
 */
#define list_create(START_CAPACITY_, PRINT_FUNC_, TYPE_) list_create_func_    \
	(START_CAPACITY_, PRINT_FUNC_, sizeof (TYPE_))

/*!
 * @brief Create new list.
 *
 * @note Don't forget to free memory using list_destroy function.
 * 
 * @note Use list_create() macro instead of this function.
 *
 * @return List which was created. If allocation error has been occurred
 * it returns NULL.
 */
list_t list_create_func_
(
	size_t start_capacity,                   /*!< [in] start capacity of
	                                                   creating list.        */
	void (*print_func) (const void*, FILE*), /*!< [in] function which prints
	                                                   one list element.
	                                                   If it equals to NULL
	                                                   elements will be
	                                                   printed by bytes.     */
	size_t elem_size                         /*!< [in] size of one element
	                                                   in creating list.     */
);

/*!
 * @brief Destroy list and deallocate memory.
 *
 * @return NULL
 */
list_t list_destroy
(
	list_t lst /*!< [in,out] list to destroy.                                */
);

/*!
 * @brief Get element from list.
 *
 * @return Pointer to value. IF some error occurred during performing
 * this function it returns NULL;
 */
void* list_get
(
	const list_t          lst, /*!< [in] list.                               */
	const list_iterator_t it   /*!< [in] list iterator.                      */
);

/*!
 * Insert element to list after current element.
 *
 * @return Error code which has been occurred during performing this function.
 */
list_error_t list_insert_after
(
	list_t                lst,  /*!< [in,out] list.                          */
	const list_iterator_t it,   /*!< [in]     iterator to current element.   */
	const void*           value /*!< [in]     value which will be inserted.  */
);

/*!
 * @brief Insert an element to list before current.
 *
 * @return Error code which has been occurred during performing this function.
 */
list_error_t list_insert_before
(
	list_t                lst,  /*!< [in,out] list.                          */
	const list_iterator_t it,   /*!< [in] iterator to current element.       */
	const void*           value /*!< [in] a value which will be inserted.    */
);

/*!
 * @brief Insert an element to the head of the list.
 *
 * @return Error code which has been occurred during performing this function.
 */
list_error_t list_insert_to_head
(
	list_t      lst,  /*!< [in,out] list.                                    */
	const void* value /*!< [in]     a value which will be inserted.          */
);

/*!
 * @brief Insert an element to the tail of the list.
 *
 * @return Error code which has been occurred during performing this function.
 */
list_error_t list_insert_to_tail
(
	list_t      lst,  /*!< [in,out] list.                                    */
	const void* value /*!< [in]     a value which will be inserted.          */
);

/*!
 * @brief Get next element of the list.
 *
 * @return iterator of an element.
 */
list_iterator_t list_next
(
	const list_t lst,        /*!< [in] list.                                 */
	const list_iterator_t it /*!< [in] iterator of current element.          */
);

/*!
 * @brief Get previous element of the list.
 *
 * @return iterator of an element.
 */
list_iterator_t list_prev
(
	const list_t lst,        /*!< [in] list.                                 */
	const list_iterator_t it /*!< [in] iterator of current element.          */
);

/*!
 * @brief Verify list.
 *
 * This function checks correctness of the state of the list.
 */
#define list_verify(LST_) list_verify_func_(LST_, #LST_,                      \
                                            __LINE__, __func__, __FILE__)
/*!
 * @brief Verify list.
 *
 * This function checks correctness of the state of the list.
 *
 * @return Error that shows the state of the list.
 */
list_error_t list_verify_func_
(
	const list_t lst,       /*!< [in] list.                                  */
	const char*  lst_name,  /*!< [in] name of the list variable.             */
	size_t       line,      /*!< [in] line where dump function was called.   */
	const char*  func_name, /*!< [in] function name where dump function was
	                                  called.                                */
	const char*  file_name  /*!< [in] file name where dump function was
	                                  called.                                */
);

/*!
 * @brief Change capacity of the list.
 *
 * @return Error code which has been occurred during performing this function.
 */
list_error_t list_change_capacity
(
	list_t lst,         /*!< [in,out] list.                                  */
	size_t new_capacity /*!< [in]     new capacity.                          */
);

/*!
 * @brief Get head of the list.
 *
 * @return An iterator of list's head.
 */
list_iterator_t list_head
(
	const list_t lst /*!< [in] list.                                         */
);

/*!
 * @brief Get tail of the list.
 *
 * @return An iterator of list's tail.
 */
list_iterator_t list_tail
(
	const list_t lst /*!< [in] list.                                         */
);

/*!
 * @brief Erase current element from the list.
 *
 * After processing this function iterator will point at next element.
 * If iterator points at last element it will point to previous element.
 *
 * @return Error code which has been occurred during performing this function.
 */
list_error_t list_erase
(
	list_t           lst, /*!< [in,out] list.                                */
	list_iterator_t* it   /*!< [in,out] an iterator.                         */
);

/*!
 * @brief Erase first element with particular value from the list.
 *
 * @return Error code which has been occurred during performing this function.
 */
list_error_t list_erase_by_value
(
	list_t      lst, /*!< [in,out] list.                                     */
	const void* val  /*!, [in]     value of erased element.                  */
);

/*!
 * @brief Erase element by its index from the list.
 *
 * @return Error code which has been occurred during performing this function.
 */
list_error_t list_erase_by_index
(
	list_t lst,  /*!< [in,out] list.                                         */
	size_t index /*!< [in]     index of erasing element.                     */
);

/*!
 * @brief Find element in list by its value.
 *
 * @return Iterator of finded element. If list doesn't contain an element
 * function returns iterator which doesn't point to nay elements.
 */
list_iterator_t list_find
(
	const list_t lst,  /*!< [in] list.                                       */
	const void*  value /*!< [in] pointer to value which will be founded.     */
);

/*!
 * @brief Get element by its position in list.
 *
 * @note Try to avoid using this function because it's too slow.
 *
 * @return iterator to an element.
 */
list_iterator_t list_element_at
(
	const list_t lst,  /*!< [in] list.                                       */
	size_t       index /*!< [in] number of an element in list.               */
);

/*!
 * @brief Delete all elements from list.
 *
 * @return Error code which has been occurred during performing this function.
 */
list_error_t list_clear
(
	list_t lst /*!< [in,out] list.                                           */
);

/*!
 * @brief Check iterator to valid state.
 *
 * @return Is iterator valid.
 */
bool list_check_iterator
(
	const list_t          lst, /*!< [in] list.                               */
	const list_iterator_t it   /*!< [in] iterator which will be checked.     */
);

/*!
 * @brief Check whether the iterator points to a list's element.
 *
 * @return Is the iterator points to a list's element.
 */
bool list_iterator_on_element
(
	const list_iterator_t it /*!< [in] iterator.                             */
);

/*!
 * @brief Print list.
 */
void list_print
(
	const list_t lst,   /*!< [in] list.                                      */
	FILE*        stream /*!< [in] stream where list will be printed.         */
);

/*!
* @brief Dump list to file "<list_name_line_func_file>.dot"
* and create .png file from it using GraphVis.
*/
#define list_dump(LST_) \
	list_dump_func_(LST_, #LST_, __LINE__, __func__, __FILE__)

/*!
 * @brief Dump list to file "<list_name_line_func_file>.dot"
 * and create .png file from it using GraphVis.
 */
void list_dump_func_
(
	const list_t lst,       /*!< [in] list.                                  */
	const char*  lst_name,  /*!< [in] name of the list variable.             */
	size_t       line,      /*!< [in] line where dump function was called.   */
	const char*  func_name, /*!< [in] function name where dump function was
	                                  called.                                */
	const char*  file_name  /*!< [in] file name where dump function was
	                                  called.                                */
);
/*!
 * @brief Normilize the order of elements in list.
 *
 * It will make order of elements like in an array.
 * That's why function list_element_at in normalized list is very fast.
 */
void list_normalize
(
	list_t lst /*!< [in] list.                                               */
);

/*!
 * @brief Check is list normalized.
 *
 * @return Is list normalized.
 */
bool list_is_normalized
(
	const list_t lst /*!< [in] list.                                         */
);

/*!
 * @brief Print info about list error.
 */
void list_perror
(
	list_error_t err,   /*!< [in]     error code.                            */
	FILE*        stream /*!< [in,out] output stream.                         */
);

/*!
 * @brief Get list size.
 *
 * @return List size.
 */
size_t list_size
(
	const list_t lst /*!< [in] list.                                         */
);

/*!
 * @brief Get list capacity.
 *
 * @return List capacity.
 */
size_t list_capacity
(
	const list_t lst /*!< [in] list.                                         */
);




#endif // undefined LIST_H_

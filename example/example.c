#include "../src/list.h"


void print_lst (list_t lst)
{
	list_print(lst, stdout);
	putchar('\n');

}


void print_int (const void* val, FILE* stream)
{
	fprintf(stream, "%d", *(const int*) val);
}


void test (list_t lst)
{
	int val = 1;
	list_insert_after(lst, list_head(lst), &val);
	print_lst(lst);

	val = 20;
	list_insert_after(lst, list_head(lst), &val);
	print_lst(lst);
	
	val = 4;
	list_insert_to_tail(lst, &val);
	print_lst(lst);
	
	val = 3;
	list_insert_before(lst, list_tail(lst), &val);
	print_lst(lst);

	val = 50;
	list_insert_to_tail(lst, &val);
	print_lst(lst);

	list_erase_by_index(lst, 4);
	print_lst(lst);

	val = 20;
	list_erase_by_value(lst, &val);
	print_lst(lst);
	
	list_dump(lst);
	list_normalize(lst);
	list_dump(lst);
}


int main (void)
{
	list_t lst = list_create(2, print_int, int);
	test(lst);
	lst = list_destroy(lst);

	return 0;
}

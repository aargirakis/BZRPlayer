#ifndef SLLHDR_H
#define SLLHDR_H

/*
Copyright (c) 2009-2010 Tero Lindeman (kometbomb)

Permission is hereby granted, free of charge, to any person
obtaining a copy of this software and associated documentation
files (the "Software"), to deal in the Software without
restriction, including without limitation the rights to use,
copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the
Software is furnished to do so, subject to the following
conditions:

The above copyright notice and this permission notice shall be
included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
OTHER DEALINGS IN THE SOFTWARE.
*/


#define SLLHDR \
	void *next;

typedef struct
{
	SLLHDR;
} SllHdr;


#define SLL_LIST(type)\
	struct\
	{\
		type *used;\
		type *reserve;\
	}

/*---------------*/

typedef SLL_LIST(void) SllList;

static inline SllHdr* sllhdr_pop(SllHdr **queue) 
{
	SllHdr *ptr = *queue;
	if (*queue != NULL)
		*queue = (*queue)->next;
	return ptr;
}

#define sllhdr_walk(ptr,head,block)\
{\
	ptr = (void*)head;\
	while (ptr)\
	{\
		block;\
		ptr = (void*)ptr->next;\
	}\
}

#define sllhdr_insert_after(queue, obj)\
	if (queue != NULL)\
	{\
		(obj)->next = (queue)->next;\
		(queue)->next = obj;\
	}\
	else\
	{\
		(queue) = obj;\
		(obj)->next = NULL;\
	}
	
#define sllhdr_insert_before(queue, obj)\
	if (queue != NULL)\
	{\
		(obj)->next = (void*)queue;\
		queue = (void*)obj;\
	}\
	else\
	{\
		(queue) = (void*)obj;\
		(obj)->next = NULL;\
	}

#define sllhdr_update_list(basetype, ptr, unused_head, in_use_head, foreach, kill_if, on_death)\
{\
	ptr = (void*)in_use_head;\
	basetype *prev = NULL;\
	while (ptr)\
	{\
		foreach;\
		basetype *next;\
		if (kill_if)\
		{\
			on_death;\
			if ((basetype*)in_use_head == (basetype*)ptr)\
			{\
				in_use_head = ptr->next;\
				next = ptr->next;\
			}\
			else\
			{\
				if (prev)\
				{\
					prev->next = ptr->next;\
				}\
				next = ptr->next;\
			}\
			ptr->next = unused_head;\
			unused_head = (basetype*)ptr;\
		}\
		else\
		{\
			prev = (basetype*)ptr;\
			next = ptr->next;\
		}\
		ptr = (void*)next;\
	}\
}

#define slllist_alloc(list) sllhdr_pop((SllHdr**)&(list).reserve);
#define slllist_insert_front(list, item) sllhdr_insert_before((list).used, (SllHdr*)item);

#define slllist_walk(ptr,list,block)\
	sllhdr_walk(ptr,list.used,block)

#define slllist_update(list, basetype, ptr, foreach, kill_if, on_death) \
	sllhdr_update_list(basetype, ptr, (list).reserve, (list).used, foreach, kill_if, on_death);

#define slllist_init(list, data, n_items)\
	{\
	int i;\
	for(i = 0 ; i < n_items-1 ; ++i) data[i].next = (SllHdr*)&data[i+1];\
	data[i].next = NULL;\
	list.reserve = (void*)&data[0];\
	list.used = NULL;\
	}
	
#endif

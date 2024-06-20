/* Extended Module Player
 * Copyright (C) 1996-2013 Claudio Matsuoka and Hipolito Carraro Jr
 *
 * This file is part of the Extended Module Player and is distributed
 * under the terms of the GNU Lesser General Public License. See COPYING.LIB
 * for more information.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "common.h"
#include "list.h"
#include "iff.h"

#include "loader.h"

struct iff_data {
	struct list_head iff_list;
	int id_size;
	int flags;
};

iff_handle iff_new()
{
	struct iff_data *data;

	data = malloc(sizeof(struct iff_data));
	if (data == NULL)
		return NULL;

	INIT_LIST_HEAD(&data->iff_list);
	data->id_size = 4;
	data->flags = 0;

	return (iff_handle) data;
}

void iff_chunk(iff_handle opaque, struct module_data *m, FILE *f, void *parm)
{
	struct iff_data *data = (struct iff_data *)opaque;
	long size;
	char id[17] = "";

	if (fread(id, 1, data->id_size, f) != data->id_size)
		return;

	if (data->flags & IFF_SKIP_EMBEDDED) {
		/* embedded RIFF hack */
		if (!strncmp(id, "RIFF", 4)) {
			read32b(f);
			read32b(f);
			/* read first chunk ID instead */
			fread(id, 1, data->id_size, f);
		}
	}

	size = (data->flags & IFF_LITTLE_ENDIAN) ? read32l(f) : read32b(f);

	if (data->flags & IFF_CHUNK_ALIGN2)
		size = (size + 1) & ~1;

	if (data->flags & IFF_CHUNK_ALIGN4)
		size = (size + 3) & ~3;

	if (data->flags & IFF_FULL_CHUNK_SIZE)
		size -= data->id_size + 4;

	iff_process(opaque, m, id, size, f, parm);
}

void iff_register(iff_handle opaque, char *id,
		  void (*loader)(struct module_data *, int, FILE *, void *))
{
	struct iff_data *data = (struct iff_data *)opaque;
	struct iff_info *f;

	f = malloc(sizeof(struct iff_info));
	strncpy(f->id, id, 5);
	f->loader = loader;

	list_add_tail(&f->list, &data->iff_list);
}

void iff_release(iff_handle opaque)
{
	struct iff_data *data = (struct iff_data *)opaque;
	struct list_head *tmp;
	struct iff_info *i;

	/* can't use list_for_each, we free the node before incrementing */
	for (tmp = (&data->iff_list)->next; tmp != (&data->iff_list);) {
		i = list_entry(tmp, struct iff_info, list);
		list_del(&i->list);
		tmp = tmp->next;
		free(i);
	}

	free(data);
}

int iff_process(iff_handle opaque, struct module_data *m, char *id, long size,
		FILE *f, void *parm)
{
	struct iff_data *data = (struct iff_data *)opaque;
	struct list_head *tmp;
	struct iff_info *i;
	int pos;

	pos = ftell(f);

	list_for_each(tmp, &data->iff_list) {
		i = list_entry(tmp, struct iff_info, list);
		if (id && !strncmp(id, i->id, data->id_size)) {
			i->loader(m, size, f, parm);
			break;
		}
	}

	fseek(f, pos + size, SEEK_SET);

	return 0;
}

/* Functions to tune IFF mutations */

void iff_id_size(iff_handle opaque, int n)
{
	struct iff_data *data = (struct iff_data *)opaque;

	data->id_size = n;
}

void iff_set_quirk(iff_handle opaque, int i)
{
	struct iff_data *data = (struct iff_data *)opaque;

	data->flags |= i;
}

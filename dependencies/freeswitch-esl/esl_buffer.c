/*
 * Copyright (c) 2010-2012, Anthony Minessale II
 * All rights reserved.
 * 
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 
 * * Redistributions of source code must retain the above copyright
 * notice, this list of conditions and the following disclaimer.
 * 
 * * Redistributions in binary form must reproduce the above copyright
 * notice, this list of conditions and the following disclaimer in the
 * documentation and/or other materials provided with the distribution.
 * 
 * * Neither the name of the original author; nor the names of any contributors
 * may be used to endorse or promote products derived from this software
 * without specific prior written permission.
 * 
 * 
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT OWNER
 * OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
 * LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */


#include "esl_buffer.h"

static unsigned buffer_id = 0;

struct esl_buffer {
	unsigned char *data;
	unsigned char *head;
	esl_size_t used;
	esl_size_t actually_used;
	esl_size_t datalen;
	esl_size_t max_len;
	esl_size_t blocksize;
	unsigned id;
	int loops;
};


ESL_DECLARE(esl_status_t) esl_buffer_create(esl_buffer_t **buffer, esl_size_t blocksize, esl_size_t start_len, esl_size_t max_len)
{
	esl_buffer_t *new_buffer;

	new_buffer = malloc(sizeof(*new_buffer));

	if (new_buffer) {
		memset(new_buffer, 0, sizeof(*new_buffer));

		if (start_len) {
			new_buffer->data = malloc(start_len);
			if (!new_buffer->data) {
				free(new_buffer);
				return ESL_FAIL;
			}
			memset(new_buffer->data, 0, start_len);
		}

		new_buffer->max_len = max_len;
		new_buffer->datalen = start_len;
		new_buffer->id = buffer_id++;
		new_buffer->blocksize = blocksize;
		new_buffer->head = new_buffer->data;

		*buffer = new_buffer;
		return ESL_SUCCESS;
	}

	return ESL_FAIL;
}

ESL_DECLARE(esl_size_t) esl_buffer_len(esl_buffer_t *buffer)
{

	esl_assert(buffer != NULL);

	return buffer->datalen;

}


ESL_DECLARE(esl_size_t) esl_buffer_freespace(esl_buffer_t *buffer)
{
	esl_assert(buffer != NULL);

	if (buffer->max_len) {
		return (esl_size_t) (buffer->max_len - buffer->used);
	}
	return 1000000;

}

ESL_DECLARE(esl_size_t) esl_buffer_inuse(esl_buffer_t *buffer)
{
	esl_assert(buffer != NULL);

	return buffer->used;
}

ESL_DECLARE(esl_size_t) esl_buffer_seek(esl_buffer_t *buffer, esl_size_t datalen)
{
	esl_size_t reading = 0;

	esl_assert(buffer != NULL);

	if (buffer->used < 1) {
		buffer->used = 0;
		return 0;
	} else if (buffer->used >= datalen) {
		reading = datalen;
	} else {
		reading = buffer->used;
	}

	buffer->used = buffer->actually_used - reading;
	buffer->head = buffer->data + reading;

	return reading;
}

ESL_DECLARE(esl_size_t) esl_buffer_toss(esl_buffer_t *buffer, esl_size_t datalen)
{
	esl_size_t reading = 0;

	esl_assert(buffer != NULL);

	if (buffer->used < 1) {
		buffer->used = 0;
		return 0;
	} else if (buffer->used >= datalen) {
		reading = datalen;
	} else {
		reading = buffer->used;
	}

	buffer->used -= reading;
	buffer->head += reading;

	return buffer->used;
}

ESL_DECLARE(void) esl_buffer_set_loops(esl_buffer_t *buffer, int loops)
{
	buffer->loops = loops;
}

ESL_DECLARE(esl_size_t) esl_buffer_read_loop(esl_buffer_t *buffer, void *data, esl_size_t datalen)
{
	esl_size_t len;
	if ((len = esl_buffer_read(buffer, data, datalen)) < datalen) {
		if (buffer->loops == 0) {
			return len;
		}
		buffer->head = buffer->data;
		buffer->used = buffer->actually_used;
		len = esl_buffer_read(buffer, (char*)data + len, datalen - len);
		buffer->loops--;
	}
	return len;
}

ESL_DECLARE(esl_size_t) esl_buffer_read(esl_buffer_t *buffer, void *data, esl_size_t datalen)
{
	esl_size_t reading = 0;

	esl_assert(buffer != NULL);
	esl_assert(data != NULL);


	if (buffer->used < 1) {
		buffer->used = 0;
		return 0;
	} else if (buffer->used >= datalen) {
		reading = datalen;
	} else {
		reading = buffer->used;
	}

	memcpy(data, buffer->head, reading);
	buffer->used -= reading;
	buffer->head += reading;

	/* if (buffer->id == 4) printf("%u o %d = %d\n", buffer->id, (unsigned)reading, (unsigned)buffer->used); */
	return reading;
}


ESL_DECLARE(esl_size_t) esl_buffer_packet_count(esl_buffer_t *buffer)
{
	char *pe, *p, *e, *head = (char *) buffer->head;
	esl_size_t x = 0;
	
	esl_assert(buffer != NULL);

	e = (head + buffer->used);

	for (p = head; p && *p && p < e; p++) {
		if (*p == '\n') {
			pe = p+1;
			if (*pe == '\r') pe++;
			if (pe <= e && *pe == '\n') {
				p = pe++;
				x++;
			}
		}
	}
	
	return x;
}

ESL_DECLARE(esl_size_t) esl_buffer_read_packet(esl_buffer_t *buffer, void *data, esl_size_t maxlen)
{
	char *pe, *p, *e, *head = (char *) buffer->head;
	esl_size_t datalen = 0;

	esl_assert(buffer != NULL);
	esl_assert(data != NULL);

	e = (head + buffer->used);

	for (p = head; p && *p && p < e; p++) {
		if (*p == '\n') {
			pe = p+1;
			if (*pe == '\r') pe++;
			if (pe <= e && *pe == '\n') {
				pe++;
				datalen = pe - head;
				if (datalen > maxlen) {
					datalen = maxlen;
				}
				break;
			}
		}
	}
	
	return esl_buffer_read(buffer, data, datalen);
}

ESL_DECLARE(esl_size_t) esl_buffer_write(esl_buffer_t *buffer, const void *data, esl_size_t datalen)
{
	esl_size_t freespace, actual_freespace;

	esl_assert(buffer != NULL);
	esl_assert(data != NULL);
	esl_assert(buffer->data != NULL);

	if (!datalen) {
		return buffer->used;
	}

	actual_freespace = buffer->datalen - buffer->actually_used;
	if (actual_freespace < datalen && (!buffer->max_len || (buffer->used + datalen <= buffer->max_len))) {
		memmove(buffer->data, buffer->head, buffer->used);
		buffer->head = buffer->data;
		buffer->actually_used = buffer->used;
	}

	freespace = buffer->datalen - buffer->used;

	/*
	  if (buffer->data != buffer->head) {
	  memmove(buffer->data, buffer->head, buffer->used);
	  buffer->head = buffer->data;
	  }
	*/
	
	if (freespace < datalen) {
		esl_size_t new_size, new_block_size;
		void *data1;
		
		new_size = buffer->datalen + datalen;
		new_block_size = buffer->datalen + buffer->blocksize;

		if (new_block_size > new_size) {
			new_size = new_block_size;
		}
		buffer->head = buffer->data;
		data1 = realloc(buffer->data, new_size);
		if (!data1) {
			return 0;
		}
		buffer->data = data1;
		buffer->head = buffer->data;
		buffer->datalen = new_size;
	}
	

	freespace = buffer->datalen - buffer->used;

	if (freespace < datalen) {
		return 0;
	} else {
		memcpy(buffer->head + buffer->used, data, datalen);
		buffer->used += datalen;
		buffer->actually_used += datalen;
	}
	/* if (buffer->id == 4) printf("%u i %d = %d\n", buffer->id, (unsigned)datalen, (unsigned)buffer->used); */

	return buffer->used;
}

ESL_DECLARE(void) esl_buffer_zero(esl_buffer_t *buffer)
{
	esl_assert(buffer != NULL);
	esl_assert(buffer->data != NULL);

	buffer->used = 0;
	buffer->actually_used = 0;
	buffer->head = buffer->data;
}

ESL_DECLARE(esl_size_t) esl_buffer_zwrite(esl_buffer_t *buffer, const void *data, esl_size_t datalen)
{
	esl_size_t w;
	
	if (!(w = esl_buffer_write(buffer, data, datalen))) {
		esl_buffer_zero(buffer);
		return esl_buffer_write(buffer, data, datalen);
	}

	return w;
}

ESL_DECLARE(void) esl_buffer_destroy(esl_buffer_t **buffer)
{
	if (*buffer) {
		free((*buffer)->data);
		(*buffer)->data = NULL;
		free(*buffer);
	}
	
	*buffer = NULL;
}

/* For Emacs:
 * Local Variables:
 * mode:c
 * indent-tabs-mode:t
 * tab-width:4
 * c-basic-offset:4
 * End:
 * For VIM:
 * vim:set softtabstop=4 shiftwidth=4 tabstop=4 noet:
 */

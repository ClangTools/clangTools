//
// Created by caesar kekxv on 2020/4/17.
//

#include "c_vector.h"

typedef struct c_vector {
    unsigned char *items;
    size_t total;
    size_t index;
} c_vector;


/**
 * init
 * @param cVector
 */
extern int c_vector_init(c_vector **cVector) {
    if (!cVector)return -1;
    if (NULL != (*cVector))return -2;
    *cVector = (c_vector *) malloc(sizeof(c_vector));
    (*cVector)->index = 0;
    (*cVector)->total = 0;
    (*cVector)->items = NULL;
    return 0;
}

/**
 * get c_vector size
 * @param cVector
 * @return
 */
extern size_t c_vector_size(c_vector *cVector) {
    if (!cVector)return 0;
    return cVector->total;
	if (!cVector)return 0;
	return cVector->total;
}

/**
 * resize c_vector
 * @param cVector
 */
extern int c_vector_resize(c_vector *cVector, size_t count) {
    unsigned char *items = NULL;
    if (!cVector)return -1;
    if (cVector->items) {
        items = (unsigned char *) realloc(cVector->items, sizeof(unsigned char) * count);
        if (items) {
            cVector->items = items;
            cVector->total = count;
            return 0;
        }
        return -1;
    } else {
        cVector->items = (unsigned char *) malloc(sizeof(unsigned char) * count);
        if (cVector->items == NULL) {
            return -1;
        } else {
            cVector->total = count;
            return 0;
        }
    }
}

/**
 * push back c_vector
 * @param cVector
 * @param data
 * @param offset
 * @param count
 */
extern int c_vector_push_back(c_vector *cVector, unsigned char *data, size_t offset, size_t count) {
    size_t index, new_size;
    index = cVector->total;
    new_size = count + cVector->total;
    if (c_vector_resize(cVector, new_size) < 0) {
        // error realloc
        return -1;
    }
    memcpy(cVector->items + index, data + offset, count);
    return 0;
}

/**
 * push zero to c_vector
 * @param cVector
 * @return
 */
extern int c_vector_push_back_zero(c_vector *cVector) {
    unsigned char data[1] = {0};
    return c_vector_push_back(cVector, data, 0, 1);
}

/**
 * set c_vector data
 * @param cVector
 * @param offset
 * @param data
 * @param count
 * @return
 */
extern size_t c_vector_set(c_vector *cVector, size_t index, unsigned char *data, size_t offset, size_t count) {
    if (cVector->total <= index || index + count >= cVector->total) {
        return 0;
    }
    memcpy(cVector->items + index, data + offset, count);
    return count;
}

/**
 * insert data
 * @param cVector
 * @param index
 * @param data
 * @param offset
 * @param count
 * @return
 */
extern size_t c_vector_insert(c_vector *cVector, size_t index, unsigned char *data, size_t offset, size_t count) {
    size_t last_total;
    size_t len;
    size_t i;
    size_t new_size;
    if (cVector->total <= index) {
        return 0;
    }
    last_total = cVector->total;
    new_size = count + cVector->total;
    if (c_vector_resize(cVector, new_size) < 0) {
        // error realloc
        return 0;
    }
    len = last_total - index;
    for (i = 1; i <= len; i++) {
        memcpy(cVector->items + new_size - i, cVector->items + last_total - i, 1);
    }
    memcpy(cVector->items + index, data + offset, count);
    return count;
}

/**
 * get c_vector data
 * @param cVector
 * @param offset
 * @return
 */
extern unsigned char *c_vector_get(c_vector *cVector, size_t offset) {
    if (cVector->total <= offset) {
        return NULL;
    }
    return cVector->items + offset;
}

/**
 * get c_vector data
 * @param cVector
 * @return
 */
extern unsigned char *c_vector_data(c_vector *cVector) {
    return c_vector_get(cVector, 0);
}

/**
 * c_vector delete
 * @param cVector
 * @param offset
 * @param count
 */
extern size_t c_vector_delete(c_vector *cVector, size_t offset, size_t count) {
    if (cVector->total <= offset) {
        return 0;
    }
    if (count + offset >= cVector->total) {
        cVector->total = cVector->total - offset;
        return count;
    }
    memcpy(cVector->items + offset, cVector->items + offset + count, cVector->total - count - offset);
    cVector->total = cVector->total - count;
    return count;
}

/**
 * free c_vector
 * @param cVector
 */
extern void c_vector_free(c_vector **cVector) {
    if (!cVector)return;
    if (!*cVector)return;
    if ((*cVector)->items)free((*cVector)->items);
    (*cVector)->items = NULL;
    free(*cVector);
    *cVector = NULL;
}

extern int c_vector_seekg(size_t offset, c_vector *cVector) {
    return c_vector_seek(cVector, offset, SEEK_SET);
}

extern int c_vector_skip(size_t offset, c_vector *cVector) {
    return c_vector_seek(cVector, offset, SEEK_CUR);
}

extern int c_vector_seek(c_vector *cVector, size_t offset, int whence) {
    if (whence != SEEK_SET && whence != SEEK_CUR && whence != SEEK_END) {
        return -1;
    }
    if (SEEK_SET == whence) {
        cVector->index = offset;
    } else if (SEEK_END == whence) {
        cVector->index = cVector->total - 1 - offset;
    } else if (SEEK_CUR == whence) {
        cVector->index = cVector->total + offset;
    }
    return -1;
}

extern size_t c_vector_read(unsigned char *p_buffer, size_t p_nb_bytes, c_vector *v) {
    if (v->total - v->index < p_nb_bytes) {
        p_nb_bytes = v->total - v->index;
    }
    memcpy(p_buffer, v->items + v->index, p_nb_bytes);
    v->index += p_nb_bytes;
    return p_nb_bytes;
}

extern size_t c_vector_write(unsigned char *p_buffer, size_t p_nb_bytes,
                      c_vector *v) {
    if (v->index + p_nb_bytes > v->total) {
        c_vector_resize(v, v->index + p_nb_bytes);
    }
    c_vector_set(v, v->index, p_buffer, 0, p_nb_bytes);
    v->index += p_nb_bytes;
    return p_nb_bytes;
}



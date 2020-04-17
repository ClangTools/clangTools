//
// Created by caesar kekxv on 2020/4/17.
//

#include "c_vector.h"

typedef struct c_vector {
    void *items;
    size_t total;
} c_vector;


/**
 * init
 * @param cVector
 */
int c_vector_init(c_vector **cVector) {
    if (!cVector)return -1;
    if (NULL != (*cVector))return -2;
    *cVector = (c_vector *) malloc(sizeof(c_vector));
    (*cVector)->total = 0;
    (*cVector)->items = NULL;
    return 0;
}

/**
 * get c_vector size
 * @param cVector
 * @return
 */
size_t c_vector_size(c_vector *cVector) {
    if (!cVector)return -1;
    return cVector->total;
}

/**
 * resize c_vector
 * @param cVector
 */
static int c_vector_resize(c_vector *cVector, size_t count) {
    if (!cVector)return -1;
    if (cVector->items) {
        void *items = (void *) realloc(cVector->items, sizeof(void) * count);
        if (items) {
            cVector->items = items;
            cVector->total = count;
            return 0;
        }
        return -1;
    } else {
        cVector->items = (void *) malloc(sizeof(void) * count);
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
int c_vector_push_back(c_vector *cVector, void *data, size_t offset, size_t count) {
    size_t index = cVector->total;
    size_t new_size = count + cVector->total;
    if (c_vector_resize(cVector, new_size) < 0) {
        // error realloc
        return -1;
    }
    memcpy(&(cVector->items[index]), &data[offset], count);
    return 0;
}

/**
 * push zero to c_vector
 * @param cVector
 * @return
 */
int c_vector_push_back_zero(c_vector *cVector) {
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
size_t c_vector_set(c_vector *cVector, size_t index, void *data, size_t offset, size_t count) {
    if (cVector->total <= index || index + count >= cVector->total) {
        return -1;
    }
    memcpy(&(cVector->items[index]), &data[offset], count);
    return 0;
}

/**
 * get c_vector data
 * @param cVector
 * @param offset
 * @return
 */
void *c_vector_get(c_vector *cVector, size_t offset) {
    if (cVector->total <= offset) {
        return NULL;
    }
    return &cVector->items[offset];
}

/**
 * get c_vector data
 * @param cVector
 * @return
 */
void *c_vector_data(c_vector *cVector) {
    return c_vector_get(cVector, 0);
}

/**
 * c_vector delete
 * @param cVector
 * @param offset
 * @param count
 */
size_t c_vector_delete(c_vector *cVector, size_t offset, size_t count) {
    if (cVector->total <= offset) {
        return -1;
    }
    if (count + offset >= cVector->total) {
        cVector->total = cVector->total - offset;
        return 0;
    }
    memcpy(&(cVector->items[offset]), &(cVector->items[offset + count]), cVector->total - count - offset);
    cVector->total = cVector->total - count;
    return 0;
}

/**
 * free c_vector
 * @param cVector
 */
void c_vector_free(c_vector **cVector) {
    if (!cVector)return;
    if (!*cVector)return;
    if ((*cVector)->items)free((*cVector)->items);
    (*cVector)->items = NULL;
    free(*cVector);
    *cVector = NULL;
}

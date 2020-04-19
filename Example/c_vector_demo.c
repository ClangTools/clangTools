//
// Created by caesar kekxv on 2020/4/17.
//

#include <stdio.h>
#include <stdlib.h>

#ifdef ENABLE_C_VECTOR
#include <c_vector.h>
#endif

int main(void) {
#ifdef ENABLE_C_VECTOR
    int i;
    c_vector *v = NULL;
    const char *hello = "hello c_vector";
    const char *bye = "bye c_vector";
    c_vector_init(&v);
    c_vector_push_back(v, (void *) hello, 0, strlen(hello) * sizeof(char));
    c_vector_push_back(v, "\n", 0, 1);

    c_vector_push_back(v, (void *) bye, 0, strlen(bye) * sizeof(char));
    c_vector_push_back(v, "\n", 0, 1);

    c_vector_push_back_zero(v);
    printf("%s\n", (const char *) c_vector_data(v));

    printf("%s\n", (const char *) c_vector_data(v));
    c_vector_insert(v, 2, "abcd", 0, 4);
    printf("%s\n", (const char *) c_vector_data(v));
    c_vector_delete(v, strlen(hello) + 1, 4);

    printf("%s\n", (const char *) c_vector_data(v));

    c_vector_free(&v);
    return 0;
#else
    printf("ENABLE_C_VECTOR == OFF");
    return 0;
#endif
}
/*
 Copyright (c) 2009 Dave Gamble

 Permission is hereby granted, free of charge, to any person obtaining a copy
 of this software and associated documentation files (the "Software"), to deal
 in the Software without restriction, including without limitation the rights
 to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 copies of the Software, and to permit persons to whom the Software is
 furnished to do so, subject to the following conditions:

 The above copyright notice and this permission notice shall be included in
 all copies or substantial portions of the Software.

 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 THE SOFTWARE.
 */

#ifndef cJson__h
#define cJson__h

#include <stdint.h>
#include <inttypes.h>

#ifndef PRId64
#define PRId64 "ld"
#endif

#ifdef __cplusplus
extern "C"
{
#endif
namespace neb {
    typedef int32_t int32;
    typedef uint32_t uint32;
    typedef int64_t int64;
    typedef uint64_t uint64;
/* cJson Types: */
#define cJson_False 0
#define cJson_True 1
#define cJson_NULL 2
#define cJson_Int 3
#define cJson_Double 4
#define cJson_String 5
#define cJson_Array 6
#define cJson_Object 7

#define cJson_IsReference 256

/* The cJson structure: */
    typedef struct cJson {
        struct cJson *next, *prev; /* next/prev allow you to walk array/object chains. Alternatively, use GetArraySize/GetArrayItem/GetObjectItem */
        struct cJson *child; /* An array or object item will have a child pointer pointing to a chain of the items in the array/object. */

        int type; /* The type of the item, as above. */

        char *valuestring; /* The item's string, if type==cJson_String */
        int64 valueint; /* The item's number, if type==cJson_Number */
        double valuedouble; /* The item's number, if type==cJson_Number */
        int sign;   /* sign of valueint, 1(unsigned), -1(signed) */

        char *string; /* The item's name string, if this item is the child of, or is in the list of subitems of an object. */
    } cJson;

    typedef struct cJson_Hooks {
        void *(*malloc_fn)(size_t sz);

        void (*free_fn)(void *ptr);
    } cJson_Hooks;

/* Supply malloc, realloc and free functions to cJson */
    extern void cJson_InitHooks(cJson_Hooks *hooks);

/* Supply a block of JSON, and this returns a cJson object you can interrogate. Call cJson_Delete when finished. */
    extern cJson *cJson_Parse(const char *value, const char **ep);

/* Render a cJson entity to text for transfer/storage. Free the char* when finished. */
    extern char *cJson_Print(cJson *item);

/* Render a cJson entity to text for transfer/storage without any formatting. Free the char* when finished. */
    extern char *cJson_PrintUnformatted(cJson *item);

/* Delete a cJson entity and all subentities. */
    extern void cJson_Delete(cJson *c);

/* Returns the number of items in an array (or object). */
    extern int cJson_GetArraySize(cJson *array);

/* Retrieve item number "item" from array "array". Returns NULL if unsuccessful. */
    extern cJson *cJson_GetArrayItem(cJson *array, int item);

/* Get item "string" from object. Case insensitive. */
    extern cJson *cJson_GetObjectItem(cJson *object, const char *string);

/* remove gloal variable for thread safe. --by Bwar on 2020-11-15 */
/* For analysing failed parses. This returns a pointer to the parse error. You'll probably need to look a few chars back to make sense of it. Defined when cJson_Parse() returns 0. 0 when cJson_Parse() succeeds. */
/* extern const char *cJson_GetErrorPtr(); */

/* These calls create a cJson item of the appropriate type. */
    extern cJson *cJson_CreateNull();

    extern cJson *cJson_CreateTrue();

    extern cJson *cJson_CreateFalse();

    extern cJson *cJson_CreateBool(int b);

    extern cJson *cJson_CreateDouble(double num, int sign);

    extern cJson *cJson_CreateInt(uint64 num, int sign);

    extern cJson *cJson_CreateString(const char *string);

    extern cJson *cJson_CreateArray();

    extern cJson *cJson_CreateObject();

/* These utilities create an Array of count items. */
    extern cJson *cJson_CreateIntArray(int *numbers, int sign, int count);

    extern cJson *cJson_CreateFloatArray(float *numbers, int count);

    extern cJson *cJson_CreateDoubleArray(double *numbers, int count);

    extern cJson *cJson_CreateStringArray(const char **strings, int count);

/* Append item to the specified array/object. */
    extern void cJson_AddItemToArray(cJson *array, cJson *item);

    extern void cJson_AddItemToArrayHead(cJson *array, cJson *item);    /* add by Bwar on 2015-01-28 */
    extern void cJson_AddItemToObject(cJson *object, const char *string,
                                      cJson *item);

/* Append reference to item to the specified array/object. Use this when you want to add an existing cJson to a new cJson, but don't want to corrupt your existing cJson. */
    extern void cJson_AddItemReferenceToArray(cJson *array, cJson *item);

    extern void cJson_AddItemReferenceToObject(cJson *object, const char *string,
                                               cJson *item);

/* Remove/Detatch items from Arrays/Objects. */
    extern cJson *cJson_DetachItemFromArray(cJson *array, int which);

    extern void cJson_DeleteItemFromArray(cJson *array, int which);

    extern cJson *cJson_DetachItemFromObject(cJson *object, const char *string);

    extern void cJson_DeleteItemFromObject(cJson *object, const char *string);

/* Update array items. */
    extern void cJson_ReplaceItemInArray(cJson *array, int which, cJson *newitem);

    extern void cJson_ReplaceItemInObject(cJson *object, const char *string,
                                          cJson *newitem);

#define cJson_AddNullToObject(object, name)    cJson_AddItemToObject(object, name, cJson_CreateNull())
#define cJson_AddTrueToObject(object, name)    cJson_AddItemToObject(object, name, cJson_CreateTrue())
#define cJson_AddFalseToObject(object, name)        cJson_AddItemToObject(object, name, cJson_CreateFalse())
#define cJson_AddNumberToObject(object, name, n)    cJson_AddItemToObject(object, name, cJson_CreateNumber(n))
#define cJson_AddStringToObject(object, name, s)    cJson_AddItemToObject(object, name, cJson_CreateString(s))


}

#ifdef __cplusplus
}
#endif

#endif

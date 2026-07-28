#ifndef DYARRAY_H_
#define DYARRAY_H_

#include <stddef.h>

#include <stdio.h>

typedef struct
{
    void* buf;
    size_t bufCapacity; //Allocated memory for buf
    size_t elementSize; //Size in bytes of each element
    size_t elementCount; //Number of stored elements
} dyarray;

//Element size is the size in bytes of each element stored in the array (must be at least 1).
//Initial element capacity indicates the initial per-element capacity allocated for the array (must be at least 1).
//Returns false on failure.
bool DyArrayCreate(dyarray* arr, size_t elementSize, size_t initialElementCapacity);
void DyArrayFree(dyarray* arr);

//Copies the element for N bytes, where N is the singular element size.
//Returns false on failure.
bool DyArrayAddElement(dyarray* arr, void* element);

//Returns NULL on failure (example: the index is out of bounds).
void* DyArrayGetElement(dyarray* arr, size_t idx);

//Change an existing element's value with a new value.
bool DyArraySetElement(dyarray* arr, size_t idx, void* newVal);

//Removes an element by swapping it with the last one, and then removing the last element,
//to achieve constant time complexity for the operation.
//This means that the array will NOT be ordered after.
//Returns false on failure.
bool DyArrayRemoveElementSP(dyarray* arr, size_t idx);

bool DyArrayClone(dyarray* original, dyarray* new);

//Serializes the passed dyarray in the specified file stream, by appending the data at the current cursor position.
void DyArraySerialize(dyarray* arr, FILE* file);

//Deserialize a dyarray stored in a file stream, into a destination array, the dyarray must be freed after use.
void DyArrayDeserialize(FILE* file, dyarray* dest);

#ifdef DYARRAY_IMPL

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define DYARRAY_EXPECT(condition, ...)\
do\
{\
    if (!condition)\
    {\
        printf("\033[31m" "ASSERTION FAILED at %s -- line %d\n", __FILE__, __LINE__);\
        __VA_OPT__(printf("Message: " "\033[39m", __VA_ARGS__);)\
        exit(1);\
    }\
} while (0)

#define DYARRAY_REALLOC_FACTOR 2

bool DyArrayCreate(dyarray* arr, size_t elementSize, size_t initialElementCapacity)
{
    if (!arr) {printf("DyArrayCreate ERROR: arr is NULL.\n"); return false;}
    if (!elementSize) {printf("DyArrayCreate ERROR: elementSize is 0.\n"); return false;}
    if (!initialElementCapacity) {printf("DyArrayCreate ERROR: initialElementCapacity is 0.\n"); return false;}

    arr->buf = malloc(elementSize * initialElementCapacity);
    if (!arr->buf) {printf("DyArrayCreate ERROR: malloc failed.\n"); return false;}

    arr->elementSize = elementSize;
    arr->elementCount = 0;
    arr->bufCapacity = initialElementCapacity * elementSize;

    return true;
}

void DyArrayFree(dyarray *arr)
{
    if (!arr) {printf("DyArrayFree ERROR: arr is NULL.\n"); return;}
    if (!arr->buf) {printf("DyArrayFree ERROR: arr.buf is NULL.\n"); return;}

    free(arr->buf);
    arr->buf = NULL;
    arr->bufCapacity = 0;
    arr->elementCount = 0;
    arr->elementSize = 0;
}

bool DyArrayAddElement(dyarray* arr, void* element)
{
    if (!arr) {printf("DyArrayAddElement ERROR: arr is NULL.\n"); return false;}
    if (!arr->buf) {printf("DyArrayAddElement ERROR: arr.buf is NULL.\n"); return false;}
    if (arr->elementSize == 0) {printf("DyArrayAddElement ERROR: arr.elementSize is 0.\n"); return false;}
    if (!element) {printf("DyArrayAddElement ERROR: element is NULL.\n"); return false;}

    //Realloc if necessary
    if (arr->elementCount * arr->elementSize >= arr->bufCapacity)
    {
        void* ptr = realloc(arr->buf, arr->bufCapacity * DYARRAY_REALLOC_FACTOR);
        if (!ptr) {printf("DyArrayAddElement ERROR: realloc failed.\n"); return false;}
        arr->buf = ptr;
        arr->bufCapacity *= DYARRAY_REALLOC_FACTOR;
    }

    memcpy(((char*)arr->buf) + arr->elementCount * arr->elementSize, element, arr->elementSize);
    arr->elementCount++;
    return true;
}

bool DyArrayRemoveElementSP(dyarray *arr, size_t idx)
{
    if (!arr) {printf("DyArrayRemoveElementSP ERROR: arr is NULL.\n"); return false;}
    if (!arr->buf) {printf("DyArrayRemoveElementSP ERROR: arr.buf is NULL.\n"); return false;}
    if (arr->elementSize == 0) {printf("DyArrayRemoveElementSP ERROR: arr.elementSize is 0.\n"); return false;}

    //Check if idx is out of bounds
    if (idx >= arr->elementCount) {printf("DyArrayRemoveElementSP ERROR: index is out of bounds\n"); return false;}

    memcpy(
        ((char*)arr->buf) + idx * arr->elementSize,
        ((char*)arr->buf) + arr->elementSize * (arr->elementCount - 1),
        arr->elementSize);

    arr->elementCount--;
    return true;
}

void* DyArrayGetElement(dyarray* arr, size_t idx)
{
    if (!arr) {printf("DyArrayGetElement ERROR: arr is NULL.\n"); return NULL;}
    if (!arr->buf) {printf("DyArrayGetElement ERROR: arr.buf is NULL.\n"); return NULL;}
    if (arr->elementSize == 0) {printf("DyArrayGetElement ERROR: arr.elementSize is 0.\n"); return NULL;}

    if (idx >= arr->elementCount) {printf("DyArrayGetElement ERROR: index is out of bounds\n"); return NULL;}

    return ((char*)arr->buf) + idx * arr->elementSize;
}

bool DyArraySetElement(dyarray* arr, size_t idx, void* newVal)
{
    if (!arr) {printf("DyArraySetElement ERROR: arr is NULL.\n"); return false;}
    if (!arr->buf) {printf("DyArraySetElement ERROR: arr.buf is NULL.\n"); return false;}
    if (arr->elementSize == 0) {printf("DyArraySetElement ERROR: arr.elementSize is 0.\n"); return false;}

    if (idx >= arr->elementCount) {printf("DyArraySetElement ERROR: index is out of bounds\n"); return false;}

    memcpy(((char*)arr->buf) + idx * arr->elementSize, newVal, arr->elementSize);
    return true;
}

bool DyArrayClone(dyarray* original, dyarray* new)
{
    if (!original) {printf("DyArrayClone ERROR: original is NULL.\n"); return false;}
    if (!original->buf) {printf("DyArrayClone ERROR: original.buf is NULL.\n"); return false;}
    if (original->elementSize == 0) {printf("DyArrayClone ERROR: original.elementSize is 0.\n"); return false;}
    if (original->bufCapacity == 0) {printf("DyArrayClone ERROR: original.bufCapacity is 0.\n"); return false;}

    if (!new) {printf("DyArrayClone ERROR: new is NULL.\n"); return false;}

    new->buf = malloc(original->bufCapacity);
    if (!new->buf) {printf("DyArrayClone ERROR: malloc failed.\n"); return false;}
    memcpy(new->buf, original->buf, original->bufCapacity);

    new->elementSize = original->elementSize;
    new->elementCount = original->elementCount;
    new->bufCapacity = original->bufCapacity;

    return true;
}

void DyArraySerialize(dyarray* arr, FILE* file)
{
    if (!arr) {
        printf("DyArraySerialize ERROR: arr is NULL.\n");
        return;
    }

    if (!file) {
        printf("DyArraySerialize ERROR: file is NULL.\n");
        return;
    }

    if (!arr->buf) {
        printf("DyArraySerialize ERROR: arr.buf is NULL.\n");
        return;
    }

    /*
     * Follow this structure:
     */

    typedef struct
    {
        size_t bufCapacity; //Allocated memory for buf
        size_t elementSize; //Size in bytes of each element
        size_t elementCount; //Number of stored elements
        void* buf;
    } dyarray;

    fwrite(&arr->bufCapacity, 1, sizeof(size_t), file);
    fwrite(&arr->elementSize, 1, sizeof(size_t), file);
    fwrite(&arr->elementCount, 1, sizeof(size_t), file);
    fwrite(arr->buf, 1, arr->bufCapacity, file);
}

void DyArrayDeserialize(FILE* file, dyarray* dest)
{
    if (!dest) {
        printf("DyArrayDeserialize ERROR: dest is NULL.\n");
        return;
    }

    if (!file) {
        printf("DyArrayDeserialize ERROR: file is NULL.\n");
        return;
    }

    /*
     * Follow this structure:
     */

    typedef struct
    {
        size_t bufCapacity; //Allocated memory for buf
        size_t elementSize; //Size in bytes of each element
        size_t elementCount; //Number of stored elements
        void* buf;
    } dyarray;

    fread(&dest->bufCapacity, 1, sizeof(size_t), file);
    fread(&dest->elementSize, 1, sizeof(size_t), file);
    fread(&dest->elementCount, 1, sizeof(size_t), file);

    dest->buf = malloc(dest->bufCapacity);
    if (!dest->buf) {
        printf("DyArrayDeserialize ERROR: malloc failed.\n");
        return;
    }

    fread(dest->buf, 1, dest->bufCapacity, file);
}

#endif //Impl

#endif //Include guard

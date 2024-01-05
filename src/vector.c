#include "vector.h"
#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

int DEFAULT_CAPACITY = 16;

vector vector_empty()
{
  return vector_with_cap(DEFAULT_CAPACITY);
}

vector vector_with_cap(int capacity)
{
  return (vector){
      .cap = capacity,
      .len = 0,
      .data = calloc(sizeof(void *), capacity)};
}

vector vector_with_data(int len, void **data)
{
  return (vector){
      .cap = len,
      .len = len,
      .data = data};
}

int vector_length(vector *v)
{
  return v->len;
}

void resize_vector(vector *v, int cap)
{
  assert(v->len <= cap);
  void **old = v->data;
  v->data = memcpy(calloc(sizeof(void *), cap), v->data, sizeof(void *) * v->len);
  free(old);
  v->cap = cap;
}

void vector_append(vector *v, void *element)
{
  if (!v->cap)
    resize_vector(v, DEFAULT_CAPACITY);
  if (v->cap < v->len + 1)
    resize_vector(v, v->cap < 1024 ? v->cap * 2 : v->cap + 1024);
  v->data[v->len++] = element;
}

void *vector_pop(vector *v)
{
  assert(v->len > 0);
  return v->data[--v->len];
}

void vector_shrink(vector *v)
{
  resize_vector(v, v->len);
}

void *vector_at(vector *v, int i)
{
  assert(0 <= i && i < v->len);
  return v->data[i];
}

void *vector_set(vector *v, int i, void *element)
{
  assert(0 <= i && i < v->len);
  void *out = v->data[i];
  v->data[i] = element;
  return out;
}

void *vector_remove(vector *v, int i)
{
  assert(0 <= i && i < v->len);
  void *out = v->data[i];
  memmove(&v[i], &v[i + 1], sizeof(void *) * (--v->len - i));
  return out;
}

void vector_free(vector v)
{
  free(v.data);
}

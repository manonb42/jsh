#ifndef VECTOR_H_
#define VECTOR_H_

typedef struct vector
{
  int len;
  int cap;
  void **data;
} vector;

vector vector_empty();
vector vector_with_cap(int capacity);
vector vector_with_data(int len, void **data);

int vector_length(vector *v);

void vector_append(vector *v, void *element);
void *vector_pop(vector *v);
void vector_shrink(vector *v);
void *vector_at(vector *v, int i);
void *vector_set(vector *v, int i, void *element);
void *vector_remove(vector *v, int i);

void vector_free(vector v);

#endif // VECTOR_H_

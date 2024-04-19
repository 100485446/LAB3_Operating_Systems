//SSOO-P3 23/24

#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <string.h>
#include "queue.h"

//To create a queue
queue* queue_init(int size){
  queue* q = malloc(sizeof(queue));
  q->elements = malloc(size * sizeof(struct element));
  q->size = size;
  q->start = 0;
  q->end = 0;
  return q;
}

int queue_destroy(queue* q) {
  free(q->elements);
  free(q);
  return 0;
}

int queue_put(queue* q, struct element* elem) {
  if (queue_full(q)) {
    return -1;  // Queue is full
  }
  q->elements[q->end] = *elem;
  q->end = (q->end + 1) % q->size;
  return 0;
}

struct element* queue_get(queue* q) {
  if (queue_empty(q)) {
    return NULL;  // Queue is empty
  }
  struct element* elem = &q->elements[q->start];
  q->start = (q->start + 1) % q->size;
  return elem;
}

int queue_empty(queue* q) {
  return q->start == q->end;
}

int queue_full(queue* q) {
  return (q->end + 1) % q->size == q->start;
}

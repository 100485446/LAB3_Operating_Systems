//SSOO-P3 23/24

#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <string.h>
#include "queue.h"

// Initialize a queue with a given size
queue* queue_init(int size){
  queue* q = malloc(sizeof(queue));
  q->size = size;
  // Allocate memory for the elements
  q->elements = malloc(size * sizeof(struct element));
  // At first, the queue is empty
  q->start = 0;
  q->end = 0;
  return q;
}

// Destroy a queue
int queue_destroy(queue* q) {
  // Free the memory allocated for the elements
  free(q->elements);
  // Free the memory allocated for the queue
  free(q);
  return 0;
}

int queue_put(queue* q, struct element* elem) {
  // If the queue is full, we cannot add elements
  if (queue_full(q)) {
    return -1;
  }
  // Copy the element to the queue
  q->elements[q->end] = *elem;
  // Update the end index
  q->end = (q->end + 1) % q->size;
  return 0;
}

struct element* queue_get(queue* q) {
  // If the queue is empty, we cannot get elements
  if (queue_empty(q)) {
    return NULL;
  }
  // Get the element at the start index
  struct element* elem = &q->elements[q->start];
  // Update the start index
  q->start = (q->start + 1) % q->size;
  return elem;
}

int queue_empty(queue* q) {
  // If the start index is equal to the end index, the queue is empty
  return q->start == q->end;
}

int queue_full(queue* q) {
  // If the end index is one position before the start index, the queue is full
  return (q->end + 1) % q->size == q->start;
}

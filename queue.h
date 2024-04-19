//SSOO-P3 23/24

#ifndef HEADER_FILE
#define HEADER_FILE

// Structure to hold the elements of the queue
struct element {
  int product_id; //Product identifier
  int op;         //Operation
  int units;      //Product units
};

// Structure to hold the queue
typedef struct queue {
    struct element* elements;  // Array to hold the elements
    int size;  // Maximum size of the queue
    int start;  // Index of the first element
    int end;  // Index of where the next element will be inserted
}queue;

// Structure to hold the input data for the producer
struct producer_data {
  int start;
  int end;
  queue * circular_buffer;
};

// Structure to hold the output data for the consumer
struct consumer_return {
  int partial_profits;
	int partial_product_stock [5];
  };

queue* queue_init (int size);
int queue_destroy (queue *q);
int queue_put (queue *q, struct element* elem);
struct element * queue_get(queue *q);
int queue_empty (queue *q);
int queue_full(queue *q);

#endif

//SSOO-P3 23/24

#ifndef HEADER_FILE
#define HEADER_FILE


struct element {
  int product_id; //Product identifier
  int op;         //Operation
  int units;      //Product units
};

struct producer_data {
  int start;
  int end;
  queue * circular_buffer;
};


struct consumer_return {
  int partial_profits;
	int partial_product_stock [5];
  };


typedef struct queue {
  // Define the struct yourself
  int size;
}queue;


queue* queue_init (int size);
int queue_destroy (queue *q);
int queue_put (queue *q, struct element* elem);
struct element * queue_get(queue *q);
int queue_empty (queue *q);
int queue_full(queue *q);

#endif

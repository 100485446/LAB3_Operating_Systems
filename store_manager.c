//SSOO-P3 23/24

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <stddef.h>
#include <sys/stat.h>
#include <pthread.h>
#include "queue.h"
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>

pthread_mutex_t mutex; /* mutex to access shared buffer */
pthread_cond_t non_full; /* can we add more elements? */
pthread_cond_t non_empty; /* can we remove elements? */
int n_elements; /* number of elements in buffer */
struct element * operations;
int all_producers_finished = 0;


void * producer(struct producer_data * data) { /* Producer code */
	int i;
	for(i = data->start; i <= data->end; i++ ) {
		pthread_mutex_lock(&mutex); /* access to buffer*/

		while (queue_full(data->circular_buffer)){
			// when buffer is full
			pthread_cond_wait(&non_full, &mutex);
		} 
			
		queue_put(data->circular_buffer, &operations[i]);

		pthread_cond_signal(&non_empty); /* buffer is not empty */
		pthread_mutex_unlock(&mutex);
	}
	pthread_exit(0);
}

void * consumer(queue *  circular_buffer) { /* consumer code */
	int partial_profits = 0;
	int partial_product_stock [5] = {0};
	for (;;){
		pthread_mutex_lock(&mutex); /* access to buffer */

		while (queue_empty(circular_buffer)){
			/* when buffer empty */
			if (all_producers_finished) {
			pthread_mutex_unlock(&mutex);
				struct consumer_return * my_return = (struct consumer_return *) malloc(sizeof(struct consumer_return));
				my_return -> partial_profits = partial_profits;
				memcpy(my_return -> partial_product_stock, partial_product_stock, sizeof(partial_product_stock));
                pthread_exit(my_return);
            }
			pthread_cond_wait(&non_empty, &mutex);
		}   

		struct element operation = * queue_get(circular_buffer);

		int purchase_cost, sales_price;

		switch (operation.product_id) {
			case 1:
				if (strcmp(operation.op, "Purchase") == 0){
					partial_profits += 2;
					partial_product_stock[0] += 1;
				}
				else if (strcmp(operation.op, "Sale") == 0){
					partial_profits -= 3;
					partial_product_stock[0] -= 1;
				}
				else {
					printf("Invalid operation\n");
					return -1;
				}
				break;
			case 2:
				if (strcmp(operation.op, "Purchase") == 0){
					partial_profits += 5;
					partial_product_stock[1] += 1;
				}
				else if (strcmp(operation.op, "Sale") == 0){
					partial_profits -= 10;
					partial_product_stock[1] -= 1;
				}
				else {
					printf("Invalid operation\n");
					return -1;
				}
				break;
			case 3:
				if (strcmp(operation.op, "Purchase") == 0){
					partial_profits += 15;
					partial_product_stock[2] += 1;
				}
				else if (strcmp(operation.op, "Sale") == 0){
					partial_profits -= 20;
					partial_product_stock[2] -= 1;
				}
				else {
					printf("Invalid operation\n");
					return -1;
				}
				break;
			case 4:
				if (strcmp(operation.op, "Purchase") == 0){
					partial_profits += 25;
					partial_product_stock[3] += 1;
				}
				else if (strcmp(operation.op, "Sale") == 0){
					partial_profits -= 40;
					partial_product_stock[3] -= 1;
				}
				else {
					printf("Invalid operation\n");
					return -1;
				}
				break;
			case 5:
				if (strcmp(operation.op, "Purchase") == 0){
					partial_profits += 100;
					partial_product_stock[4] += 1;
				}
				else if (strcmp(operation.op, "Sale") == 0){
					partial_profits -= 125;
					partial_product_stock[4] -= 1;
				}
				else {
					printf("Invalid operation\n");
					return -1;
				}
				break;
			default:
				printf("Invalid product id\n");
				return -1;
		}

		pthread_cond_signal(&non_full); /* buffer is not full */
		pthread_mutex_unlock(&mutex);
	}
}

int main (int argc, const char * argv[]){
	// If less than five arguments print an error and return -1
	if(argc < 5)
	{
		printf("Too few arguments\n");
		return -1;
	}
	// if more arguments than expected are given, we return -1 
	else if(argc > 5){
		printf("Too many arguments\n");
		return -1;
	}
	else{

		int profits = 0;
		int product_stock [5] = {0};

		int n_producers = atoi(argv[2]);
		int n_consumers = atoi(argv[3]);
		queue * my_queue = queue_init(atoi(argv[4]));

		// 'fd' will store the file descriptor of the opened file
		int fd;
		if ((fd = open(argv[1], O_RDONLY)) < 0){
			// if open returns -1, there has been an error and we return -1
			printf("Error while opening\n");
			return -1;
		}

		char n_operations_buff[50];
		int bytes_read = read(fd, n_operations_buff, sizeof(n_operations_buff) - 1);
		if (bytes_read < 0) {
			printf("Error while reading\n");
			return -1;
		}

		n_operations_buff[bytes_read] = '\0';  // Null-terminate the string

		int n_operations;
		if (sscanf(n_operations_buff, "%d", &n_operations) < 0){
			printf("Failed to read number of operations\n");
			return -1;
		}

		// Allocate memory for the array of pointers
		operations = (struct element*) malloc(n_operations * sizeof(struct element));
		if (operations == NULL) {
			printf("Failed to allocate memory\n");
			return -1;
		}

		char element_buffer[50];
		int bytes_read;
		int byte_counter = 0;
		int elem_counter = 0;
		int struct_counter = 0;
		char ch;
		while (((bytes_read = read(fd, &ch, 1)) > 0) && struct_counter < n_operations){
			if (ch == ' '){	
				element_buffer[byte_counter] = '\0';
				if (elem_counter == 0){
					operations[struct_counter].product_id = atoi(element_buffer);
				}
				if (elem_counter == 1){
					operations[struct_counter].op = (char *) malloc(byte_counter + 1);
					strcpy(operations[struct_counter].op, element_buffer);
        		}
				byte_counter = 0;
				elem_counter = (elem_counter + 1) % 3;
			}
			else if (ch == '\n'){
				element_buffer[byte_counter] = '\0';
				if (elem_counter == 2){
					operations[struct_counter].units = atoi(element_buffer);
				}
				byte_counter = 0;
				elem_counter = (elem_counter + 1) % 3;
				struct_counter++;
			}

			element_buffer[byte_counter++] = ch;
		}
		if (bytes_read < 0) {
			printf("Error while reading\n");
			return -1;
		}






		pthread_mutex_init(&mutex, NULL);
		pthread_cond_init(&non_full, NULL);
		pthread_cond_init(&non_empty, NULL);

		//Distribution of operations among the producers
		int operations_per_producer = n_operations / n_producers;
		int spare_operations = n_operations % n_producers;
		
		pthread_t * producer_threads = (pthread_t *) malloc(n_producers * sizeof(pthread_t));
		if (producer_threads == NULL) {
   			fprintf(stderr, "Failed to allocate memory for producer_threads\n");
    		return -1;  
		}
		for (int i = 0; i < n_producers; i++){
			struct producer_data my_producer_data;
			my_producer_data.circular_buffer = my_queue;
			my_producer_data.start = i * operations_per_producer;
			my_producer_data.end = my_producer_data.start + operations_per_producer - 1;
			/*
			if (i == n_producers - 1){
				my_producer_data.end += spare_operations;
			}
			*/
			// Create a new thread for each producer
			pthread_create(&producer_threads[i], NULL, producer, (void *) &my_producer_data);
		}

		pthread_t * consumer_threads = (pthread_t *) malloc(n_producers * sizeof(pthread_t));
		if (consumer_threads == NULL) {
   			fprintf(stderr, "Failed to allocate memory for consumer_threads\n");
    		return -1;  
		}
		for (int i = 0; i < n_consumers; i++){
			pthread_create(&consumer_threads[i], NULL, consumer, (void *) my_queue);
		}

		//Wait for the threads to finish
		struct consumer_return * c_return;
		for (int i = 0; i < n_producers; i++){
			pthread_join(producer_threads[i], (void **) &c_return);
			profits += c_return -> partial_profits;
			for (int j = 0; j < 5; j++){
				product_stock[j] += c_return -> partial_product_stock[j];
			}
			free(c_return);
		}
		all_producers_finished = 1;
		for (int i = 0; i < n_consumers; i++){
			pthread_join(consumer_threads[i], NULL);
		}

		pthread_mutex_destroy(&mutex);
		pthread_cond_destroy(&non_full);
		pthread_cond_destroy(&non_empty);





		if (close(fd) < 0){
			// if close returns -1, there has been an error and we return -1
			printf("Error while closing\n");
			return -1;
		}

		queue_destroy(my_queue);

		for (int i = 0; i < n_operations; i++) {
			free(operations[i].op);
		}
		free(operations);

		free(producer_threads);
		free(consumer_threads);

		// Output
		printf("Total: %d euros\n", profits);
		printf("Stock:\n");
		printf("  Product 1: %d\n", product_stock[0]);
		printf("  Product 2: %d\n", product_stock[1]);
		printf("  Product 3: %d\n", product_stock[2]);
		printf("  Product 4: %d\n", product_stock[3]);
		printf("  Product 5: %d\n", product_stock[4]);

		return 0;
}


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


void * producer(void * arg) { /* Producer code */
	struct producer_data * data = (struct producer_data *) arg;
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

void * consumer(void * arg) { /* consumer code */
	queue * circular_buffer = (queue *) arg;
	int partial_profits = 0;
	int partial_product_stock [5] = {0};
	struct element operation;
	for (;;){
		
		pthread_mutex_lock(&mutex); /* access to buffer */

		while (queue_empty(circular_buffer)){
			/* when buffer empty */
			if (all_producers_finished) {
				pthread_mutex_unlock(&mutex);
				struct consumer_return * my_return = (struct consumer_return *) malloc(sizeof(struct consumer_return));
				my_return->partial_profits = partial_profits;
				memcpy(my_return->partial_product_stock, partial_product_stock, sizeof(partial_product_stock));
                pthread_exit(my_return);
            }
			pthread_cond_wait(&non_empty, &mutex);
		}   

		operation = * queue_get(circular_buffer);

		switch (operation.product_id) {
			// Purchase is op = 0; Sale is op = 1
			case 1:
				if (operation.op == 0){
					partial_profits += 2;
					partial_product_stock[0] += 1;
				}
				else if (operation.op == 1){
					partial_profits -= 3;
					partial_product_stock[0] -= 1;
				}
				else {
					printf("Invalid operation\n");
				}
				break;
			case 2:
				if (operation.op == 0){
					partial_profits += 5;
					partial_product_stock[1] += 1;
				}
				else if (operation.op == 1){
					partial_profits -= 10;
					partial_product_stock[1] -= 1;
				}
				else {
					printf("Invalid operation\n");
				}
				break;
			case 3:
				if (operation.op == 0){
					partial_profits += 15;
					partial_product_stock[2] += 1;
				}
				else if (operation.op == 1){
					partial_profits -= 20;
					partial_product_stock[2] -= 1;
				}
				else {
					printf("Invalid operation\n");
				}
				break;
			case 4:
				if (operation.op == 0){
					partial_profits += 25;
					partial_product_stock[3] += 1;
				}
				else if (operation.op == 1){
					partial_profits -= 40;
					partial_product_stock[3] -= 1;
				}
				else {
					printf("Invalid operation\n");
				}
				break;
			case 5:
				if (operation.op == 0){
					partial_profits += 100;
					partial_product_stock[4] += 1;
				}
				else if (operation.op == 1){
					partial_profits -= 125;
					partial_product_stock[4] -= 1;
				}
				else {
					printf("Invalid operation\n");
				}
				break;
			default:
				printf("Invalid product id\n");
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
	






		char buffer[50];
		char ch;
		int bytes_read;
		int byte_counter = 0;

		while ((bytes_read = read(fd, &ch, 1)) > 0 && ch != '\n') {
			if (byte_counter < sizeof(buffer) - 1) {
				buffer[byte_counter++] = ch;
			}
		}
		if (bytes_read < 0) {
			printf("Error while reading\n");
			return -1;
		}
		buffer[byte_counter] = '\0';  // Null-terminate the string

		int n_operations;
		if (sscanf(buffer, "%d", &n_operations) < 0){
			printf("Failed to read number of operations\n");
			return -1;
		}

		memset(buffer, 0, sizeof(buffer));
		// Allocate memory for the array of pointers
		operations = (struct element*) malloc(n_operations * sizeof(struct element));
		if (operations == NULL) {
			printf("Failed to allocate memory\n");
			return -1;
		}







		bytes_read = 0;
		byte_counter = 0;
		int elem_counter = 0;
		int struct_counter = 0;
		while (((bytes_read = read(fd, &ch, 1)) > 0) && struct_counter < n_operations){
			if (ch == ' '){	
				buffer[byte_counter] = '\0';
				if (elem_counter == 0){
					operations[struct_counter].product_id = atoi(buffer);
				}
				if (elem_counter == 1){
					if (strcmp("PURCHASE", buffer) == 0){
						operations[struct_counter].op = 0;
					}
					else if (strcmp("SALE", buffer) == 0){
						operations[struct_counter].op = 1;
					}
					else{
						printf("Invalid operation found in file\n");
						return -1;
					}
        		}
				byte_counter = 0;
				elem_counter = (elem_counter + 1) % 3;
			}
			else if (ch == '\n'){
				buffer[byte_counter] = '\0';
				if (elem_counter == 2){
					operations[struct_counter].units = atoi(buffer);
				}
				byte_counter = 0;
				elem_counter = (elem_counter + 1) % 3;
				struct_counter++;
			}
			else{
				buffer[byte_counter++] = ch;
			}
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
		int spare_operations_copy = spare_operations;
		
		pthread_t * producer_threads = (pthread_t *) malloc(n_producers * sizeof(pthread_t));
		if (producer_threads == NULL) {
   			fprintf(stderr, "Failed to allocate memory for producer_threads\n");
    		return -1;  
		}
		struct producer_data * producers_data = (struct producer_data *) malloc(n_producers * sizeof(struct producer_data));
		if (producers_data == NULL) {
   			fprintf(stderr, "Failed to allocate memory for producers_data\n");
    		return -1;  
		}
		for (int i = 0; i < n_producers; i++){
			producers_data[i].circular_buffer = my_queue;
			if (spare_operations > 0){
				producers_data[i].start = (i * operations_per_producer) + i;
				producers_data[i].end = producers_data[i].start + operations_per_producer;
				spare_operations--;
			}
			else{
				producers_data[i].start = (i * operations_per_producer) + spare_operations_copy;
				producers_data[i].end = producers_data[i].start + operations_per_producer - 1;
			}
			// Create a new thread for each producer
			pthread_create(&producer_threads[i], NULL, producer, (void *) &producers_data[i]);
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
		for (int i = 0; i < n_producers; i++){
			pthread_join(producer_threads[i], NULL);
		}

		pthread_mutex_lock(&mutex);
		all_producers_finished = 1;
		pthread_cond_broadcast(&non_empty);
		pthread_mutex_unlock(&mutex);

		struct consumer_return * c_return = (struct consumer_return * ) malloc(n_consumers * sizeof(struct consumer_return));
		for (int i = 0; i < n_consumers; i++){
			pthread_join(consumer_threads[i], (void **) &c_return[i]);
			printf("partial profits of consumer thread %d: %d\n", i, c_return[i].partial_profits);
			profits += c_return[i].partial_profits;
			for (int j = 0; j < 5; j++){
				printf("partial product stock of product %d of consumer thread %d: %d\n", j, i, c_return[i].partial_profits);
				product_stock[j] += c_return[i].partial_product_stock[j];
			}
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

		free(operations);
		free(producer_threads);
		free(consumer_threads);
		free(producers_data);
		free(c_return);

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
}


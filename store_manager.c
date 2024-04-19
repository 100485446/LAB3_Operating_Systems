
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
// Array of pointers to the struct that stores the operations
struct element * operations; 
// To check if all producers have finished, it will be used by consumers
int all_producers_finished = 0;


void * producer(void * arg) {
	// We cast the argument to a struct producer_data pointer
	struct producer_data * data = (struct producer_data *) arg;
	for(int i = data->start; i <= data->end; i++ ) {
		pthread_mutex_lock(&mutex); /* access to buffer*/

		// If the buffer is full, we wait until it is not full
		while (queue_full(data->circular_buffer)){
			pthread_cond_wait(&non_full, &mutex);
		} 
		
		// We add the operation to the circular buffer
		queue_put(data->circular_buffer, &operations[i]);

		// We signal the consumers that the buffer is not empty
		pthread_cond_signal(&non_empty); 
		// We unlock the mutex
		pthread_mutex_unlock(&mutex);
	}
	// We exit the thread
	pthread_exit(0);
}

void * consumer(void * arg) {
	// We cast the argument to a queue pointer
	queue * circular_buffer = (queue *) arg;
	// Variables to store the profits and the product stock recoreded by this 
	// particulae consumer
	int profits = 0;
	int product_stock [5] = {0};
	// Variable to store the operation that will be retrieved from the circular buffer
	struct element operation;
	// Infinite loop
	for (;;){
		
		// We lock the mutex
		pthread_mutex_lock(&mutex);

		// If the buffer is empty, we wait until it is not empty
		while (queue_empty(circular_buffer)){
			// If all producers have finished and the buffer is empty, we exit the loop
			if (all_producers_finished) {
				pthread_mutex_unlock(&mutex);
				// We create a struct to store the partial profits and the partial product stock
				struct consumer_return * my_return = (struct consumer_return *) malloc(sizeof(struct consumer_return));
				my_return->partial_profits = profits;
				memcpy(my_return->partial_product_stock, product_stock, sizeof(product_stock));
				// We exit the thread and return that struct
                pthread_exit(my_return);
            }
			pthread_cond_wait(&non_empty, &mutex);
		}   

		// We consume the operation from the circular buffer
		operation = * queue_get(circular_buffer);

		// We update the profits and the product stock
		// Purchase is op = 0 
		// Sale is op = 1
		switch (operation.product_id) {
			case 1:
				if (operation.op == 1){
					profits += 3 * operation.units;
					product_stock[0] -=  operation.units;
				}
				else if (operation.op == 0){
					profits -= 2 * operation.units;
					product_stock[0] +=  operation.units;
				}
				else {
					printf("Invalid operation\n");
				}
				break;
			case 2:
				if (operation.op == 1){
					profits += 10 * operation.units;
					product_stock[1] -= operation.units;
				}
				else if (operation.op == 0){
					profits -= 5 * operation.units;
					product_stock[1] += operation.units;
				}
				else {
					printf("Invalid operation\n");
				}
				break;
			case 3:
				if (operation.op == 1){
					profits += 20 * operation.units;
					product_stock[2] -= operation.units;
				}
				else if (operation.op == 0){
					profits -= 15 * operation.units;
					product_stock[2] += operation.units;
				}
				else {
					printf("Invalid operation\n");
				}
				break;
			case 4:
				if (operation.op == 1){
					profits += 40 * operation.units;
					product_stock[3] -= operation.units;
				}
				else if (operation.op == 0){
					profits -= 25 * operation.units;
					product_stock[3] += operation.units;
				}
				else {
					printf("Invalid operation\n");
				}
				break;
			case 5:
				if (operation.op == 1){
					profits += 125 * operation.units;
					product_stock[4] -= operation.units;
				}
				else if (operation.op == 0){
					profits -= 100 * operation.units;
					product_stock[4] += operation.units;
				}
				else {
					printf("Invalid operation\n");
				}
				break;
			default:
				printf("Invalid product id\n");
		}
		// We signal the producers that the buffer is not full
		pthread_cond_signal(&non_full);
		// We unlock the mutex
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
		// If the number of arguments is correct, we proceed to declare 
		// some variables that will be used in the program
		int profits = 0;
		int product_stock [5] = {0};

		int n_producers = atoi(argv[2]);
		int n_consumers = atoi(argv[3]);
		// 'my_queue' will store a pointer to the queue (circular buffer) of 
		// the specified size
		queue * my_queue = queue_init(atoi(argv[4]));

		// 'fd' will store the file descriptor of the opened file
		int fd;
		if ((fd = open(argv[1], O_RDONLY)) < 0){
			// if open returns -1, there has been an error and we return -1
			printf("Error while opening\n");
			return -1;
		}

		// ------------------------------------------------------------------------------
		// Read the file

		// Some variables that will be used to read the file
		char buffer[50];
		char ch;
		int bytes_read;
		int byte_counter = 0;
		// Read the first line of the file to get the number of operations
		while ((bytes_read = read(fd, &ch, 1)) > 0 && ch != '\n') {
			if (byte_counter < sizeof(buffer) - 1) {
				// If the buffer is not full, we store a character one by one in it
				buffer[byte_counter++] = ch;
			}
		}
		// If there has been an error while reading, we return -1
		if (bytes_read < 0) {
			printf("Error while reading the number of operations\n");
			return -1;
		}
		// Null-terminate the string
		buffer[byte_counter] = '\0';

		// Convert the string to an integer using sscanf
		int n_operations;
		if (sscanf(buffer, "%d", &n_operations) < 0){
			printf("Failed to read number of operations\n");
			return -1;
		}
		// Allocate memory for the array of pointers to operations 
		// (global variable), now that we know the number of operations
		operations = (struct element*) malloc(n_operations * sizeof(struct element));
		// If there has been an error while allocating memory, we return -1
		if (operations == NULL) {
			printf("Failed to allocate memory for the array of pointers to operations\n");
			return -1;
		}
		
		bytes_read = 0;
		byte_counter = 0;
		int elem_counter = 0;
		int struct_counter = 0;
		// Read the rest of the file to get the operations
		while (((bytes_read = read(fd, &ch, 1)) > 0) && struct_counter < n_operations){
			// We read the file character by character
			// When we find a space, we store the buffer in either the 1st 
			// or the 2nd field of the struct
			if (ch == ' '){	
				buffer[byte_counter] = '\0';
				// We store the buffer in the corresponding field of the struct
				// (the field depends on the value of elem_counter)
				if (elem_counter == 0){
					operations[struct_counter].product_id = atoi(buffer);
				}
				if (elem_counter == 1){
					// We check if the operation is PURCHASE or SALE
					if (strcmp("PURCHASE", buffer) == 0){
						// PURCHASE is 0
						operations[struct_counter].op = 0;
					}
					else if (strcmp("SALE", buffer) == 0){
						// SALE is 1
						operations[struct_counter].op = 1;
					}
					else{
						printf("Invalid operation found in file\n");
						return -1;
					}
        		}
				// Once we read a whole element of the struct, we reset the byte_counter 
				// and increment the elem_counter (that only goes from 0 to 2)
				byte_counter = 0;
				elem_counter = (elem_counter + 1) % 3;
			}
			else if (ch == '\n'){
				// When we find a newline, we store the buffer in the last field of the struct
				buffer[byte_counter] = '\0';
				if (elem_counter == 2){
					operations[struct_counter].units = atoi(buffer);
				}
				byte_counter = 0;
				elem_counter = (elem_counter + 1) % 3;
				// We increment the struct_counter to move to the next struct
				struct_counter++;
			}
			else{
				// If the character is not a space or a newline, we store it in the buffer
				buffer[byte_counter++] = ch;
			}
		}
		// If there has been an error while reading, we return -1
		if (bytes_read < 0) {
			printf("Error while reading the operations\n");
			return -1;
		}

		// ------------------------------------------------------------------------------
		// Create the threads

		// Initialize mutexes and condition variables
		pthread_mutex_init(&mutex, NULL);
		// non_full represents the condition that the circular buffer is not full
		pthread_cond_init(&non_full, NULL);
		// non_empty represents the condition that the circular buffer is not empty
		pthread_cond_init(&non_empty, NULL);

		// Distribution of operations among the PRODUCERS

		// We calculate the number of operations that each producer will have to do
		int operations_per_producer = n_operations / n_producers;
		// We calculate the number of operations that will be left over
		int spare_operations = n_operations % n_producers;
		// We create a copy of the number of spare operations to use it later
		int spare_operations_copy = spare_operations;
		
		// We create an array of threads for the producers
		pthread_t * producer_threads = (pthread_t *) malloc(n_producers * sizeof(pthread_t));
		if (producer_threads == NULL) {
   			fprintf(stderr, "Failed to allocate memory for producer_threads\n");
    		return -1;  
		}
		// We create an array of structs to store the data that will be passed to the producers
		struct producer_data * producers_data = (struct producer_data *) malloc(n_producers * sizeof(struct producer_data));
		if (producers_data == NULL) {
   			fprintf(stderr, "Failed to allocate memory for producers_data\n");
    		return -1;  
		}
		for (int i = 0; i < n_producers; i++){
			// We assign the circular buffer and the range of operations to each producer
			producers_data[i].circular_buffer = my_queue;
			// We calculate the range of operations that each producer will have to do
			// If there are spare operations left, we assign one to the current producer
			if (spare_operations > 0){
				producers_data[i].start = (i * operations_per_producer) + i;
				producers_data[i].end = producers_data[i].start + operations_per_producer;
				// We decrement the number of spare operations
				spare_operations--;
			}
			else{
				// If there are no spare operations left, we assign the default range of operations
				// (taking into account the spare operations that have been assigned to other producers)
				producers_data[i].start = (i * operations_per_producer) + spare_operations_copy;
				producers_data[i].end = producers_data[i].start + operations_per_producer - 1;
			}
			// Create a new thread for each producer
			pthread_create(&producer_threads[i], NULL, producer, (void *) &producers_data[i]);
		}

		// CONSUMERS
		
		// We create an array of threads for the consumers
		pthread_t * consumer_threads = (pthread_t *) malloc(n_consumers * sizeof(pthread_t));
		if (consumer_threads == NULL) {
   			fprintf(stderr, "Failed to allocate memory for consumer_threads\n");
    		return -1;  
		}
		for (int i = 0; i < n_consumers; i++){
			// Create a new thread for each consumer
			pthread_create(&consumer_threads[i], NULL, consumer, (void *) my_queue);
		}


		//-------------------------------------------------------------------------------
		// Wait for the threads to finish

		// We wait for all the producer threads to finish
		for (int i = 0; i < n_producers; i++){
			pthread_join(producer_threads[i], NULL);
		}
		
		// We signal the consumers that all producers have finished
		all_producers_finished = 1;
		pthread_cond_broadcast(&non_empty);

		// We create an array of pointers to the structs that store the return values of the consumers
		struct consumer_return ** c_return = (struct consumer_return ** ) malloc(n_consumers * sizeof(struct consumer_return*));
		if (c_return == NULL) {
   			fprintf(stderr, "Failed to allocate memory for the the consumer returns \n");
    		return -1;  
		}
		for (int i = 0; i < n_consumers; i++){
			// We wait for all the consumer threads to finish
			// We store the return values of the consumers in the array of pointers
			pthread_join(consumer_threads[i], (void **) &c_return[i]);
			// We add the partial profits and the partial product stock of each consumer
			//  to the total profits and product stock
			profits += c_return[i]->partial_profits;
			for (int j = 0; j < 5; j++){
				product_stock[j] += c_return[i]->partial_product_stock[j];
			}
		}

		// ------------------------------------------------------------------------------
		// Destroy mutexes and condition variables, close the file, free memory

		// We destroy the mutexes and condition variables
		pthread_mutex_destroy(&mutex);
		pthread_cond_destroy(&non_full);
		pthread_cond_destroy(&non_empty);

		// We close the file
		if (close(fd) < 0){
			// if close returns -1, there has been an error and we return -1
			printf("Error while closing\n");
			return -1;
		}
		// We free the memory that has been allocated for the queue
		queue_destroy(my_queue);

		// We free the memory that has been dynamically allocated
		free(operations);
		free(producer_threads);
		free(consumer_threads);
		free(producers_data);
		// As c_return is a pointer to an array of pointers, we have to free each pointer
		for (int i = 0; i < n_consumers; i++){
			free(c_return[i]);
		}
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


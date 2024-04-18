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

#define DATA_SIZE 100000 /* number of data items to be produced*/
pthread_mutex_t mutex; /* mutex to access shared buffer */
pthread_cond_t non_full; /* can we add more elements? */
pthread_cond_t non_empty; /* can we remove elements? */
int n_elements; /* number of elements in buffer */
typedef struct {
    int product_id;
    char * transaction;
    int number_units;
} Operation;

void producer() { /* Producer code */
	int data, i ,pos = 0;
	for(i=0; i < DATA_SIZE; i++ ) {
		data= i; /* generate data */
		pthread_mutex_lock(&mutex); /* access to buffer*/

		while (n_elements == MAX_BUFFER) /* when buffer is full*/
			pthread_cond_wait(&non_full, &mutex);

		buffer[pos] = data;
		pos = (pos + 1) % MAX_BUFFER;
		n_elements ++;

		pthread_cond_signal(&non_empty); /* buffer is not empty */
		pthread_mutex_unlock(&mutex);
	}
	pthread_exit(0);
}

void consumer() { /* consumer code */
	int data, i ,pos = 0;
	for(i=0; i < DATA_SIZE; i++ ) {
		pthread_mutex_lock(&mutex); /* access to buffer */

		while (n_elements == 0) /* when buffer empty */
			pthread_cond_wait(&non_empty, &mutex);

		data = buffer[pos];
		pos = (pos + 1) % MAX_BUFFER;
		n_elements --;

		pthread_cond_signal(&non_full); /* buffer is not full */
		pthread_mutex_unlock(&mutex);
		printf("Consumed %d \n", data); /* Use data*/
	}
	pthread_exit(0);
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
		int buffer[atoi(argv[4])]; /* common buffer */

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
		Operation * operations[] = malloc(n_operations * sizeof(Operation *));
		if (operations == NULL) {
			printf("Failed to allocate memory\n");
			return -1;
		}

		// Allocate memory for each struct
		for (int i = 0; i < n_operations; i++) {
			operations[i] = malloc(sizeof(Operation));
			if (operations[i] == NULL) {
				printf("Failed to allocate memory\n");
			}
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
					operations[struct_counter]->product_id = atoi(element_buffer);
				}
				if (elem_counter == 1){
					operations[struct_counter]->transaction = malloc(byte_counter + 1);
					strcpy(operations[struct_counter]->transaction, element_buffer);
        		}
				byte_counter = 0;
				elem_counter = (elem_counter + 1) % 3;
			}
			else if (ch == '\n'){
				element_buffer[byte_counter] = '\0';
				if (elem_counter == 2){
					operations[struct_counter]->number_units = atoi(element_buffer);
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

		pthread_t th1, th2;
		pthread_mutex_init(&mutex, NULL);
		pthread_cond_init(&non_full, NULL);
		pthread_cond_init(&non_empty, NULL);

		pthread_create(&th1, NULL, producer, NULL);
		pthread_create(&th2, NULL, consumer, NULL);

		pthread_join(th1, NULL);
		pthread_join(th2, NULL);

		pthread_mutex_destroy(&mutex);
		pthread_cond_destroy(&non_full);
		pthread_cond_destroy(&non_empty);

		if (close(fd) < 0){
			// if close returns -1, there has been an error and we return -1
			printf("Error while closing\n");
			return -1;
		}

		for (int i = 0; i < n_operations; i++) {
			free(operations[i]->product_id);
			free(operations[i]->transaction);
			free(operations[i]->number_units);
			free(operations[i]);
		}
		free(operations);

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


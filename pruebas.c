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

#define MAX_FILENAME_LENGTH 100
#define PURCHASE 1
#define SALE 2

// Precios unitarios y de compra de productos
int unit_price[5] = {2, 5, 15, 25, 100};
int purchase_price[5] = {3, 10, 20, 40, 125};

// Estructura para pasar argumentos al hilo productor
struct ProducerArgs 
{
    queue *cola;
    FILE *file;
};

// Estructura para pasar argumentos al hilo consumidor
struct ConsumerArgs 
{
    queue *cola;
    int *profits;
    int *product_stock;
};

void *producer(void *arg);
void *consumer(void *arg);

int main(int argc, const char *argv[]) 
{
    if (argc != 5) 
    {
        fprintf(stderr, "Incorrect structure: store_manager <file_name><num_producers><num_consumers><buff_size>");
        exit(EXIT_FAILURE);
    }

    int num_producers = atoi(argv[2]);
    int num_consumers = atoi(argv[3]);
    // int buff_size = atoi(argv[4]);

    FILE *file = fopen(argv[1], "r");
    if (file == NULL) 
    {
        perror("Error opening file\n");
        exit(EXIT_FAILURE);
    }

    // Create the queue
    struct queue *cola = queue_init(MAX_FILENAME_LENGTH);
    if (cola == NULL) 
    {
        perror("Error creating queue\n");
        exit(EXIT_FAILURE);
    }
    
    // Create the producer threads
    pthread_t producer_threads[num_producers];
    struct ProducerArgs producer_args[num_producers];
    for (int i = 0; i < num_producers; i++) 
    {
        producer_args[i].cola = cola;
        producer_args[i].file = file;
        if (pthread_create(&producer_threads[i], NULL, producer, (void *)&producer_args[i]) != 0) 
        {
            perror("Error creating producer thread\n");
            exit(EXIT_FAILURE);
        }
    }

    // Create the consumer threads
    pthread_t consumer_threads[num_consumers];
    int profits = 0;
    int product_stock[5] = {0};
    struct ConsumerArgs consumer_args[num_consumers];
    for (int i = 0; i < num_consumers; i++) 
    {
        consumer_args[i].cola = cola;
        consumer_args[i].profits = &profits;
        consumer_args[i].product_stock = product_stock;
        if (pthread_create(&consumer_threads[i], NULL, consumer, (void *)&consumer_args[i]) != 0) 
        {
            perror("Error creating consumer thread\n");
            exit(EXIT_FAILURE);
        }
    }

    // Read operations from the file and distribute them among producers and consumers
    struct element elem;
    while (fscanf(file, "%d %d %d", &elem.product_id, &elem.op, &elem.units) == 3) 
    {
        // Verificar si es una operación de compra o venta
        if (elem.op == PURCHASE) 
        {
            // Enviar la operación al hilo productor
            queue_put(cola, &elem);
        } 
        else if (elem.op == SALE) 
        {
            // Enviar la operación al hilo consumidor
            queue_put(cola, &elem);
        }
        else 
        {
            // Operación desconocida o inválida
            printf("Operación desconocida: %d\n", elem.op);
        }
    }

    //print queue
    printf("Queue: \n");

    for (int i = 0; i < cola->capacity; i++) 
    {
        printf("%d ", cola->elements[i].product_id);
    }

    // Wait for producer threads to finish
    for (int i = 0; i < num_producers; i++) 
    {
        pthread_join(producer_threads[i], NULL);
    }

    // Send termination signal to consumers
    for (int i = 0; i < num_consumers; i++) 
    {
        queue_put(cola, NULL);
    }

    // Wait for consumer threads to finish
    for (int i = 0; i < num_consumers; i++) 
    {
        pthread_join(consumer_threads[i], NULL);
    }

    // Destroy the queue
    queue_destroy(cola);

    fclose(file);



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

void *producer(void *arg) 
{
    struct ProducerArgs *args = (struct ProducerArgs *)arg;
    queue *cola = args->cola;
    FILE *file = args->file;

    struct element elem;
    while (fscanf(file, "%d %d %d", &elem.product_id, &elem.op, &elem.units) == 3) 
    {
        queue_put(cola, &elem);
    }

    pthread_exit(NULL);
}

void *consumer(void *arg) 
{
    struct ConsumerArgs *args = (struct ConsumerArgs *)arg;
    queue *cola = args->cola;
    int *profits = args->profits;
    int *product_stock = args->product_stock;

    while (1) 
    {
        struct element *elem = queue_get(cola);
        if (elem == NULL) 
        {
            break;
        }

        // Process the element
        if (elem->op == PURCHASE) 
        {
            product_stock[elem->product_id - 1] += elem->units;
        } else if (elem->op == SALE) {
            if (product_stock[elem->product_id - 1] >= elem->units) 
            {
                product_stock[elem->product_id - 1] -= elem->units;
                // Adjust profits accordingly
                // Assume unit price and purchase price arrays are defined and initialized
                *profits += elem->units * (unit_price[elem->product_id - 1] - purchase_price[elem->product_id - 1]);
            } 
            else 
            {
                // Handle insufficient stock
            }
        }

        free(elem); // Free the memory allocated for the element
    }

    pthread_exit(NULL);
}



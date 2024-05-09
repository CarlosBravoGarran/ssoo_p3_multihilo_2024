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


// Estructura para pasar los argumentos al hilo productor
typedef struct ProducerArgs 
{
    int start_index;
    struct element *operations;
    int max_ops;
}t_producer_args;

// Estructura para pasar argumentos al hilo consumidor
typedef struct ConsumerResults 
{
    int profit;            // Profit que devolverá
    int product_stock[5];   // Stock que devolverá
}t_consumer_results;

// Precios unitarios y de compra de productos
int purchase_price[5] = {2, 5, 15, 25, 100};
int unit_price[5] = {3, 10, 20, 40, 125};
int num_producers, num_consumers;


void *producer(void *arg);
void *consumer(void *arg);

pthread_mutex_t mutex;
pthread_cond_t condProducers;
pthread_cond_t condConsumers;

queue *buffer = NULL;

int main(int argc, const char *argv[]) 
{
    // Variables
    int profits = 0;
    int product_stock[5] = {0};

    // Comprobación de argumentos
    if (argc != 5)
    {
        fprintf(stderr, "Incorrect structure: store_manager <file_name><num_producers><num_consumers><buff_size>");
        exit(EXIT_FAILURE);

    }

    if (atoi(argv[2]) <= 0 || atoi(argv[3]) <= 0 || atoi(argv[4]) <= 0)
    {
        fprintf(stderr, "Error: number of producers, consumers and buffer size must be greater than 0\n");
        exit(EXIT_FAILURE);
    }

    // Variables de argumentos
    num_producers = atoi(argv[2]);
    num_consumers = atoi(argv[3]);
    int buff_size = atoi(argv[4]);

    // Creación de la cola
    queue *queue = queue_init(buff_size);
    if (queue == NULL)
    {
        perror("Error creating queue\n");
        exit(EXIT_FAILURE);
    }

    // Abrir el archivo para lectura
    FILE *file = fopen(argv[1], "r");
    if (file == NULL) 
    {
        perror("Error opening file\n");
        exit(EXIT_FAILURE);
    }

    // Leer el número de operaciones del archivo
    int num_operations;
    if (fscanf(file, "%d", &num_operations) != 1) 
    {
        perror("Error reading number of operations\n");
        exit(EXIT_FAILURE);
    }


    // Reservar memoria para todas las operaciones
    t_producer_args * argumentos_prod = malloc(num_producers * sizeof(t_producer_args));
    struct element *operations = malloc(num_operations * sizeof(struct element));
    if (operations == NULL) 
    {
        perror("Error allocating memory\n");
        exit(EXIT_FAILURE);
    }

    // Almacenar las operaciones en el array
    char op[10];
    for (int i = 0; i < num_operations; i++) 
    {
        if (fscanf(file, "%d %s %d", &operations[i].product_id, op, &operations[i].units) !=3)
        {
            perror("Error reading operation\n");
            exit(EXIT_FAILURE);
        }
        if (strcmp(op, "PURCHASE") == 0)
        {
            operations[i].op = 1;
        }
        else if (strcmp(op, "SALE") == 0)
        {
            operations[i].op = 2;
        }
        else 
        {
            perror("Error type operation");
            exit(EXIT_FAILURE);
        }        
            
    }

    for (int j = 0; j < num_producers; j++)
    {
        argumentos_prod[j].operations = operations;
    }

    if (num_operations < num_producers)
    {
        perror("Error: number of operations is less than the number of producers\n");
        exit(EXIT_FAILURE);
    }

    fclose(file); // Cerramos el archivo
    

    //Calcular número de operaciones por productor
    int ops_per_prod = num_operations / num_producers;
    int extra_ops_prod= num_operations % num_producers;
        
    // Creación de hilos productores
    buffer = queue_init(buff_size);

    pthread_t producer_threads[num_producers];
    for (int i = 0; i < num_producers; i++)
    {
        argumentos_prod[i].start_index = i;
        argumentos_prod[i].max_ops = ops_per_prod;
        if (extra_ops_prod != 0)
        {
            argumentos_prod[i].max_ops += 1; 
            extra_ops_prod -= 1;
        }

        if (pthread_create(&producer_threads[i], NULL, producer, &argumentos_prod[i]) != 0)
        {
            perror("ERROR creating producer thread\n");
            exit(EXIT_FAILURE);
        }
    }
    
    // Calcular numero de operaciones por consumidor
    int ops_per_cons = num_operations / num_consumers;
    int extra_ops_cons = num_operations % num_consumers;
    int max_ops_per_cons;
    
    // Creación de hilos consumidores
    pthread_t consumer_threads[num_consumers];
    for (int i = 0; i < num_consumers; i++)
    {

        max_ops_per_cons = ops_per_cons;
        if (extra_ops_cons != 0)
        {
            max_ops_per_cons += 1;
            extra_ops_cons -= 1;
        }

        int *max_ops_ptr = malloc(sizeof(int));
        *max_ops_ptr = max_ops_per_cons;

        if (pthread_create(&consumer_threads[i], NULL, consumer, max_ops_ptr) != 0)
        {
            perror("ERROR creating consumer thread\n");
            exit(EXIT_FAILURE);
        }
    }


    // Esperar a que los hilos productores terminen
    for (int i = 0; i < num_producers; i++)
    {
        pthread_join(producer_threads[i], NULL);
    }
 
    // Esperar a que los hilos consumidores terminen y recoger los resultados
    void *thread_result;
    for (int i = 0; i < num_consumers; i++)
    {

        pthread_join(consumer_threads[i], &thread_result);
        t_consumer_results *result = (t_consumer_results *)thread_result;
        profits += result->profit;                // Sumar el profit de cada consumidor
        for (int j = 0; j < 5; j++)
        {
            product_stock[j] += result->product_stock[j];       // Sumar el stock de cada producto
        }
        free(result); // Liberar memoria
    }


    // Liberar la memoria asignada para las operaciones
    free(operations);
    free(argumentos_prod);

    // Liberar la cola
    queue_destroy(queue);
    pthread_mutex_destroy(&mutex);
    pthread_cond_destroy(&condConsumers);
    pthread_cond_destroy(&condProducers);


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

void *consumer(void *arg)
{
    // Variables
    int max_op = *((int*)arg);
    int ops_realizadas = 0;
    t_consumer_results *result = malloc(sizeof(t_consumer_results));
    result->profit = 0;
    for (int i = 0; i < 5; i++)
    {
        result->product_stock[i] = 0;
    }
    
    // Bucle de operaciones
    while (ops_realizadas < max_op)
    {
        // Entramos en la seccion critica
        pthread_mutex_lock(&mutex);
        while(queue_empty(buffer))
        {
            // Esperamos a la señal del productor
            pthread_cond_wait(&condConsumers, &mutex);
        }
        struct element *operation;
        operation = queue_get(buffer);
        int aux_profit = 0;

        // Comprobamos si es una compra o una venta
        if (operation->op == 1)
        {
            int price = purchase_price[operation->product_id-1];
            aux_profit -=  operation->units * price;
            result->product_stock[operation->product_id-1] += operation->units;
        }
        else
        {
            int price = unit_price[operation->product_id-1];
            aux_profit += operation->units * price;
            result->product_stock[operation->product_id-1] -= operation->units;
        }

        result->profit += aux_profit;      // Sumamos el profit de la operación

        pthread_cond_signal(&condProducers);    // Manda señal al productor de que ha consumido datos
        pthread_mutex_unlock(&mutex);          // Salimos de la sección crítica

        ops_realizadas ++;
    }
    pthread_exit((void *)result);
}

void *producer(void *arg)
{
    // Variables
    int ops_realizadas = 0;
    t_producer_args *args = (t_producer_args *) arg;
    int max_op = args->max_ops;
    int index = args->start_index;
    struct element *operation = malloc(sizeof(struct element));
    
    while(ops_realizadas < max_op )
    {
        // Entramos en la seccion critica
        pthread_mutex_lock(&mutex);

        while(queue_full(buffer))
        {
            // Espera la señal de los consumidores
            pthread_cond_wait(&condProducers, &mutex);
        }
        operation = &args->operations[index];
        queue_put(buffer, operation);           // Agremamos datos al buffer circular

        index += num_producers;
        ops_realizadas ++;

        pthread_cond_signal(&condConsumers);    // Manda señal al consumidor de que ha añadido datos
        pthread_mutex_unlock(&mutex);           // Salimos de la sección crítica
    }
    pthread_exit(NULL);
}


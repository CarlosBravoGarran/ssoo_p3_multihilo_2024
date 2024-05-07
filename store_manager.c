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


// Array para almacenar el numero de operaciones de estructuras de tipo element para pasarselo a cada hilo 
// Estructura para pasar argumentos a los hilos con los ídices de los productos y el array



// Estructura para pasar los argumentos al hilo productor
typedef struct ProducerArgs {
    int start_index;
    struct element *operations;
    int max_op;
}t_producer_args;

// Estructura para pasar argumentos al hilo consumidor
typedef struct ConsumerArgs {
    queue *queue;      // Cola compartida
    int *profits;        // Puntero a las ganancias
    int *product_stock;  // Puntero al stock de productos
}t_consumer_arg;

queue *buffer;
pthread_mutex_t mutex;
pthread_cond_t condProducers;
pthread_cond_t condConsumers;
int num_producers;

// Precios unitarios y de compra de productos
int unit_price[5] = {2, 5, 15, 25, 100};
int purchase_price[5] = {3, 10, 20, 40, 125};


void *producer(void *arg);
void *consumer(void *arg);

int main(int argc, const char *argv[]) 
{
    // Variablencls
    int profits = 0;
    int product_stock[5] = {0};

    // Comprobación de argumentos
    if (argc != 5)
    {
        fprintf(stderr, "Incorrect structure: store_manager <file_name><num_producers><num_consumers><buff_size>");
        exit(EXIT_FAILURE);

    }

    // Variables de argumentos
    int num_producers = atoi(argv[2]);
    //int num_consumers = atoi(argv[3]);
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
    printf("%d \n", num_operations);


    // Reservar memoria para todas las operaciones
    t_producer_args * argumentos = malloc(num_producers * sizeof(t_producer_args));
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
        if (strcmp(op, "PURCHASE") == 0){
            operations[i].op = 1;
        }
        else if (strcmp(op, "SALE") == 0){
            operations[i].op = 2;
        }
        else {
            perror("Error type operation");
            exit(EXIT_FAILURE);
        }        
            
    }
    for (int j = 0; j < num_producers; j++){
        argumentos[j].operations = operations;
    
         /*for (int i = 0; i < num_operations; i++){
            printf("%d, %d, %d\n", argumentos->operations[i].product_id, argumentos->operations[i].op, argumentos->operations[i].units);}*/
    }
    
    //Calcular número de operaciones por productor
    int ops_per_prod = num_operations / num_producers;
    int extra_ops= num_operations % num_producers;

    //Inicializar cola
    buffer = queue_init(buff_size);
    
    // Creación de hilos productores
    pthread_t producer_threads[num_producers];
    for (int i = 0; i < num_producers; i++){
        printf("he entrado\n");
        argumentos[i].start_index = i;
        argumentos[i].max_op = ops_per_prod;
        if (extra_ops != 0){
            argumentos[i].max_op += extra_ops;
            extra_ops = 0;
        }
        if (pthread_create(&producer_threads[i], NULL, producer, &argumentos[i]) != 0){
            perror("ERROR creating producer thread\n");
            exit(EXIT_FAILURE);
        }
        
    }
    for (int i = 0; i < num_producers; i++){
        pthread_join(producer_threads[i], NULL);
    }









    fclose(file);

    // Liberar la memoria asignada para las operaciones
    free(operations);

    // Liberar la cola
    queue_destroy(queue);


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
    pthread_exit(NULL);
}

void *producer(void *arg){
    printf("Hilo productor creado\n");
    int ops_realizadas = 0;
    t_producer_args *args = (t_producer_args *) arg;
    int max_op = args->max_op;
    int index = args->start_index;
    struct element *operation = malloc(sizeof(operation));
    //hacer los mutex
    while(ops_realizadas < max_op )
    {
        pthread_mutex_lock(&mutex);
        while (queue_full(buffer))
        {
            pthread_cond_wait(&condProducers, &mutex);
        }
    operation = &args->operations[index];
    queue_put(buffer, operation);
    ops_realizadas++;
    index += num_producers;
    pthread_cond_signal(&condConsumers);
    pthread_mutex_unlock(&mutex);
        
         
    }
    pthread_exit(NULL);
}


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
//#define PURCHASE 1
//#define SALE 2


// Array para almacenar el numero de operaciones de estructuras de tipo element para pasarselo a cada hilo 
// Estructura para pasar argumentos a los hilos con los ídices de los productos y el array



// Estructura para pasar los argumentos al hilo productor
struct ProducerArgs {
    int start_index;
    queue *queue; // Cola compartida
    FILE *file;     // Archivo de entrada
};

// Estructura para pasar argumentos al hilo consumidor
struct ConsumerArgs {
    queue *queue;      // Cola compartida
    int *profits;        // Puntero a las ganancias
    int *product_stock;  // Puntero al stock de productos
};

// Precios unitarios y de compra de productos
int unit_price[5] = {2, 5, 15, 25, 100};
int purchase_price[5] = {3, 10, 20, 40, 125};


void *producer(void *arg);
void *consumer(void *arg);

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

    // Variables de argumentos
    int num_producers = atoi(argv[2]);
    int num_consumers = atoi(argv[3]);
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
    struct element *operations = malloc(num_operations * sizeof(struct element));
    if (operations == NULL) 
    {
        perror("Error allocating memory\n");
        exit(EXIT_FAILURE);
    }
    
    // Almacenar las operaciones en el array
    for (int i = 0; i < num_operations; i++) 
    {
        if (fscanf(file, "%d", &operations[i].product_id) != 1) 
        {
            perror("Error reading product id\n");
            exit(EXIT_FAILURE);
        }
        char operation[10];
        if (fscanf(file, "%10s", operation) != 1) 
        {
            perror("Error reading operation\n");
            exit(EXIT_FAILURE);
        }
        strcpy(operations[i].op, operation);

        if (fscanf(file, "%d", &operations[i].units) != 1) 
        {
            perror("Error reading units\n");
            exit(EXIT_FAILURE);
        }
        
    }
    
    //Calcular número de operaciones por productor
    int ops_per_prod = num_operations / num_producers;
    //int extra_ops= num_operations % num_producers;
    
    // Creación de hilos productores
    pthread_t producer_threads[num_producers];
    struct ProducerArgs producer_args[num_producers];
    int start_index = 0;
    for (int i = 0; i < num_producers; i++)
    {
        // Asignar argumentos al hilo
        producer_args[i].start_index = start_index;
        producer_args[i].end_index = start_index + ops_per_prod - 1; // Restar 1 para incluir el último índice
        producer_args[i].queue = queue;
        producer_args[i].file = file;

        // Calcular índice final
        int end_index = start_index + ops_per_prod;
        
        // Crear el hilo
        if (pthread_create(&producer_threads[i], NULL, producer, (void *)&producer_args[i]) != 0) 
        {
            perror("Error creating producer thread\n");
            exit(EXIT_FAILURE);
        }

        // Actualizar el índice de inicio para el siguiente productor
        start_index = end_index;
    }

    // Esperar a que los hilos productores terminen
    for (int i = 0; i < num_producers; i++) 
    {
        pthread_join(producer_threads[i], NULL);
    }

    // Crear hilos consumidores
    pthread_t consumer_threads[num_consumers];
    for (int i = 0; i < num_consumers; i++) 
    {
        if (pthread_create(&consumer_threads[i], NULL, consumer, (void *)queue) != 0) 
        {
            perror("Error creating consumer thread\n");
            exit(EXIT_FAILURE);
        }
    }

    // Esperar a que los hilos consumidores terminen
    for (int i = 0; i < num_consumers; i++) 
    {
        pthread_join(consumer_threads[i], NULL);
    }


    // Liberar la memoria asignada para las operaciones
    free(operations);

    // Liberar la cola
    queue_destroy(queue);

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

void *producer(void *arg) {
    // Obtener parámetros del hilo
    struct ProducerArgs *args = (struct ProducerArgs *)arg;
    int start_index = args->start_index;
    int end_index = args->end_index;
    FILE *file = args->file;
    queue *queue = args->queue;

    // Bucle para procesar las operaciones asignadas
    for (int i = start_index; i <= end_index; i++) {
        // Obtener los datos de la operación del archivo
        int product_id;
        char op[10]; 
        int units;
        fscanf(file, "%d %s %d", &product_id, op, &units);

        // Crear un elemento con los datos de la operación
        struct element *elem = (struct element *)malloc(sizeof(struct element));
        if (elem == NULL) {
            perror("Error allocating memory for element\n");
            pthread_exit(NULL);
        }
        elem->product_id = product_id;
        //elem->op = op;
        strcpy(elem->op, op);
        elem->units = units;

        // Insertar el elemento en la cola
        if (!queue_put(queue, elem)) {
            fprintf(stderr, "Error putting element into queue\n");
            free(elem); // Liberar la memoria asignada para el elemento
            pthread_exit(NULL);
        }
    }

    // Finalizar el hilo
    pthread_exit(NULL);
}


void *consumer(void *arg) {
    // Obtener parámetros del hilo
    struct ConsumerArgs *args = (struct ConsumerArgs *)arg;
    queue *queue = args->queue;
    int *profits = args->profits;
    int *product_stock = args->product_stock;

    // Variables para almacenar el beneficio y el stock parcial
    int partial_profits = 0;
    int partial_stock[5] = {0};

    // Bucle para procesar los elementos de la cola
    while (1) {
        // Extraer elemento de la cola
        struct element *elem = queue_get(queue);
        
        // Verificar si la cola está vacía (señal de finalización)
        if (elem == NULL) {
            break;
        }

        // Procesar la operación contenida en el elemento
        int product_id = elem->product_id;
        int units = elem->units;
        // Calcular el beneficio y actualizar el stock parcial
        if (strcmp(elem->op, "PURCHASE") == 0) {
            // Actualizar el stock parcial
            partial_stock[product_id - 1] += units;
        } else if (strcmp(elem->op, "SALE") == 0) {
            // Verificar si hay suficiente stock para realizar la venta
            if (partial_stock[product_id - 1] >= units) {
                // Actualizar el stock parcial
                partial_stock[product_id - 1] -= units;
                // Calcular el beneficio y acumularlo
                partial_profits += units * (unit_price[product_id - 1] - purchase_price[product_id - 1]);
            } else {
                // Manejar caso de stock insuficiente
                printf("Error: Stock insuficiente para realizar la venta de %d unidades del producto %d\n", units, product_id);
            }
        }

        // Liberar la memoria asignada para el elemento
        free(elem);
    }

    // Actualizar los beneficios y el stock totales
    // No es necesario mutex para actualizar estas variables, ya que cada hilo consumidor
    // trabaja con su propio conjunto de variables parciales
    *profits += partial_profits;
    for (int i = 0; i < 5; i++) {
        product_stock[i] += partial_stock[i];
    }

    // Finalizar el hilo y devolver los beneficios y el stock parcial
    pthread_exit(NULL);
}


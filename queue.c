//SSOO-P3 23/24

#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <string.h>
#include "queue.h"

//To create a queue
queue *queue_init(int size) {
    queue *q = (queue *)malloc(sizeof(queue));
    if (q == NULL) {
        fprintf(stderr, "Error: Could not allocate memory for the queue\n");
        return NULL;
    }

    // Inicializar los miembros de la estructura t_queue
    q->elements = (struct element *)malloc(size * sizeof(struct element));
    if (q->elements == NULL) {
        fprintf(stderr, "Error: Could not allocate memory for the queue elements\n");
        free(q); // Liberar la memoria asignada para la estructura de la cola
        return NULL;
    }
    q->capacity = size;
    q->head = 0;
    q->tail = -1; // Inicializar a -1 ya que la cola está vacía inicialmente
    q->count = 0;

    return q;
}


// To Enqueue an element
int queue_put(queue *q, struct element* x)
{
  if (queue_full(q))
  {
      fprintf(stderr, "Queue is full\n");
      return -1; // Error: cola llena
  }

  q->tail = (q->tail + 1) % q->capacity;
  q->elements[q->tail] = *x;
  q->count++; 
  return 0;
}

// To Dequeue an element.
struct element* queue_get(queue *q)
{
  struct element *element;
  if (queue_empty(q))
  {
      fprintf(stderr, "Queue is empty\n");
      return NULL; // Error: cola vacía
  }

  element = &q->elements[q->head];
  q->head = (q->head + 1) % q->capacity;
  q->count--;
  return element;
}

//To check queue state
int queue_empty(queue *q)
{
  return (q->count == 0);
}

int queue_full(queue *q)
{
  return (q->count == q->capacity);
}

//To destroy the queue and free the resources
int queue_destroy(queue *q)
{
  free(q->elements); // Libera la memoria asignada para los elementos de la cola
  free(q); // Libera la memoria asignada para la estructura de cola
  return 0;
}



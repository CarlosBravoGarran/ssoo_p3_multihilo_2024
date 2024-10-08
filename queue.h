//SSOO-P3 23/24

#ifndef HEADER_FILE
#define HEADER_FILE


struct element {
  int product_id;   //Product identifier
  int op;         //Operation
  int units;        //Product units
};

typedef struct s_queue {
  // Define the struct yourself
  struct element *elements;
  int capacity;
  int head;
  int tail;
  int count;
}queue;

queue* queue_init (int size);
int queue_destroy (queue *q);
int queue_put (queue *q, struct element* elem);
struct element * queue_get(queue *q);
int queue_empty (queue *q);
int queue_full(queue *q);
int queue_print(queue *q);

#endif

## Introducción
El propósito principal de esta práctica es implementar un sistema multi-hilo concurrente en C sobre el sistema operativo UNIX/Linux. El sistema debe calcular el beneficio y gestionar el stock de una tienda a partir de un conjunto de operaciones proporcionadas en un archivo específico. Para lograr esto, se utilizan llamadas al sistema POSIX para la gestión de hilos, como pthread_create, pthread_join y pthread_exit.

Además, se requiere aplicar mecanismos de sincronización, como mutex y variables condicionales, para garantizar la coherencia de los datos y prevenir problemas de concurrencia. Esto implica el diseño de distintas estructuras y condiciones para controlar el acceso a recursos compartidos y coordinar la ejecución de los hilos de manera segura.

## Estructuras y funciones base
### Estructuras de datos
En el archivo queue.h definimos dos estructuras struct element, que representa los elementos individuales que se almacenarán en la cola, y queue, que define la estructura de la propia cola. Estas estructuras proporcionan la capacidad de almacenar y manipular los datos de manera eficiente durante la ejecución del programa.
La estructura del elemento contiene los siguientes campos:
- product_id: identificador único del producto.
- op: tipo de operación a realizar (ejemplo: compra o venta).
- units: cantidad de unidades del producto involucradas en la operación.
  
La estructura queue presenta:
- elements: un puntero a un arreglo dinámico de elementos, que almacena los elementos de la cola.
- capacity: la capacidad máxima de la cola, es decir, el número máximo de elementos que puede contener. 
- head: el índice del primer elemento en la cola.
- tail: el índice del último elemento en la cola.
- count: El número actual de elementos en la cola.

### Funciones
En el archivo queue.c implementamos un conjunto de funciones para inicializar una cola, insertar y extraer elementos, verificar el estado de la cola, si se encuentra llena o vacía. Estas funciones son esenciales para el funcionamiento adecuado de la cola y facilitan su integración en el sistema multi-hilo.

### Variables globales
En nuestro caso, definimos varias variables globales que son utilizadas para diversos propósitos, algunas de las declaradas las utilizaremos para almacenar información sobre los precios de los productos, el número de hilos productores y consumidores, y las estructuras de datos necesarias para la sincronización entre los hilos. 

## Descripción del código
### Función principal
La función `main` de este programa desempeña un papel central en la ejecución del sistema de gestión de almacén. Comienza inicializando las variables necesarias, como `profits` para rastrear el beneficio total y `product_stock` para almacenar el inventario de cada producto. Luego, verifica si se proporcionan los argumentos necesarios para la ejecución, es decir, el nombre del archivo de entrada, el número de productores, consumidores y el tamaño del búfer. Si no se proporcionan los argumentos esperados, el programa termina con un mensaje de error.

Una vez que se asegura de que se proporcionan los argumentos necesarios, la función `main` abre el archivo de entrada para leer las operaciones que deben procesarse. Lee el número total de operaciones del archivo y reserva memoria para almacenarlas. Luego, almacena las operaciones en un array de estructuras, asigna estas operaciones a los productores y crea hilos separados para cada uno de ellos, pasando como argumento el número máximo de operaciones que debe realizar cada uno.

Asimismo, asigna operaciones a los consumidores, crea hilos separados para cada uno de ellos y les pasa igualmente el número máximo de operaciones que deben procesar. Una vez que los productores han terminado su trabajo, la función espera a que los consumidores terminen y recoge los resultados de cada uno de ellos, incluyendo el beneficio total y el stock final de cada producto, sumándolos en las variables ‘profit’ y ‘product_stock’ según van saliendo.

Finalmente, la función libera la memoria asignada para las operaciones, destruye la cola utilizada para la comunicación entre productores y consumidores, y muestra los resultados finales, incluyendo el beneficio total y el inventario actualizado de cada producto. En resumen, la función `main` coordina todas las actividades del programa, desde la inicialización hasta la finalización, manejando los errores necesarios y generando los resultados finales.

### Función hilo productor
La función ‘producer’ implementa la lógica de cada hilo productor para agregar operaciones a la cola de operaciones. Comienza inicializando variables, como el número de operaciones que debe realizar, el índice de inicio y la estructura de argumentos proporcionada.

Dentro del bucle principal, el hilo productor entra en una sección crítica utilizando un mutex para garantizar la exclusión mutua y evitar condiciones de carrera. Dentro de esta sección crítica, el hilo verifica si la cola está llena utilizando una variable de condición. Si la cola está llena, el hilo espera hasta que haya espacio disponible en la cola antes de continuar.

Una vez que hay espacio disponible en la cola, el hilo extrae una operación del array de operaciones utilizando el índice proporcionado y la agrega a la cola. Luego, actualiza el índice para apuntar a la siguiente operación que debe agregar en el siguiente ciclo.

Después de agregar una operación a la cola, el hilo emite una señal al consumidor para indicar que ha agregado datos y libera el mutex para permitir que otros hilos accedan a la cola.

El hilo continúa este proceso hasta que ha alcanzado el número máximo de operaciones que debe agregar a la cola. Una vez que se completan todas las operaciones, el hilo sale del bucle, emite una señal de terminación y sale de la función. 

### Función hilo consumidor
La función ‘consumer’ representa la lógica que sigue cada hilo consumidor para procesar las operaciones en la cola de operaciones, con una estructura muy similar a los hilos productores. Comienza recibiendo igualmente el número máximo de operaciones que debe realizar como argumento, inicializa un contador para realizar un seguimiento de las operaciones realizadas y reserva memoria para almacenar los resultados finales.

Dentro del bucle principal, el hilo consumidor entra en una sección crítica utilizando un mutex para garantizar la exclusión mutua y evitar condiciones de carrera al igual que en la función anterior. Dentro de esta sección crítica, el hilo espera hasta que haya operaciones en la cola para procesar utilizando una variable de condición. Una vez que hay operaciones disponibles, el hilo las extrae de la cola y las procesa.

Para cada operación, el hilo verifica si es una compra o una venta y calcula el beneficio asociado. Luego, actualiza el inventario del producto correspondiente en la estructura de resultados y suma el beneficio de la operación al beneficio total.

Después de procesar una operación, el hilo emite una señal al productor para indicar que ha consumido datos y libera el mutex para permitir que otros hilos accedan a la cola. Esto garantiza que los productores puedan reanudar su trabajo y llenar la cola si es necesario.

Una vez que el hilo ha alcanzado el número máximo de operaciones a procesar, sale del bucle, emite una señal y devuelve los resultados finales. En resumen, la función sirve para procesar operaciones de compra y venta utilizando mecanismos de sincronización para evitar condiciones de carrera y garantizar la integridad de los datos.

## Conclusión
El desarrollo de esta práctica me ha permitido afianzar conocimientos sobre programación concurrente y la implementación de sistemas multi-hilo en C. He aprendido cómo coordinar la comunicación y sincronización entre múltiples hilos para realizar tareas de manera eficiente y segura, evitando condiciones de carrera y asegurando la integridad de los datos.

Además, adquirido experiencia en el diseño e implementación de estructuras de datos fundamentales, como las colas circulares, y en el manejo de variables de condición y mutex para garantizar la exclusión mutua y la cooperación entre hilos.

Finalmente, los resultados obtenidos han sido muy satisfactorio y el código implementado cubre todos los aspectos necesarios de una forma legible y bien estructurada.

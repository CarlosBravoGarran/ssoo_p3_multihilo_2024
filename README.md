# Sistema Multi-Hilo Concurrente en C

Este proyecto implementa un sistema multi-hilo concurrente en C sobre UNIX/Linux. El objetivo es gestionar el stock y calcular el beneficio de una tienda a partir de un conjunto de operaciones, utilizando hilos POSIX (`pthread`) y mecanismos de sincronización como mutex y variables de condición para prevenir condiciones de carrera.

## Requisitos

- Sistema operativo UNIX/Linux.
- Compilador `gcc`.
- Biblioteca POSIX de hilos (`pthread`).

## Compilación

Para compilar el proyecto, utiliza el siguiente comando:

```bash
gcc -pthread -o multihilo main.c queue.c -lm
```

## Ejecución

El programa se ejecuta con el siguiente comando, proporcionando el archivo de operaciones, el número de hilos productores, consumidores y el tamaño del búfer:

```bash
./multihilo <archivo_operaciones> <n_productores> <n_consumidores> <tamaño_bufer>
```

## Descripción General

El sistema utiliza hilos productores y consumidores para manejar operaciones de compra y venta sobre productos. La comunicación entre ellos se realiza mediante una cola circular sincronizada con mutex y variables de condición, garantizando la integridad de los datos en un entorno concurrente.

### Estructuras de Datos

- **element**: Representa una operación (producto, tipo de operación y unidades involucradas).
- **queue**: Cola circular utilizada para almacenar las operaciones.

### Función Principal

La función `main()` coordina la ejecución de los hilos productores y consumidores, manejando el procesamiento de las operaciones y calculando el beneficio final y el inventario de productos. Al final, muestra los resultados y limpia los recursos utilizados.

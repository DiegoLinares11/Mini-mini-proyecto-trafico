#include <stdio.h>
#include <stdlib.h>
#include <time.h>

// Estados de luz: 0=ROJO, 1=VERDE, 2=AMARILLO
typedef struct {
    int id;
    int estado;         // 0,1,2
    int ticks_en_estado;
    int dur_rojo, dur_verde, dur_amarillo;
} TrafficLight;

typedef struct {
    int id;
    int posicion;       // celda discreta 0..RUTA_MAX
    int velocidad;      // celdas/iteración cuando hay verde
    int destino;        // posición objetivo (simple)
    int luz_asociada;   // índice de semáforo que controla su paso
} Vehicle;

typedef struct {
    TrafficLight *lights;
    int num_lights;
    Vehicle *vehicles;
    int num_vehicles;
    int ruta_max;       // largo de la ruta discreta
} Interseccion;


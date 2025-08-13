#include <stdio.h>
#include <stdlib.h>
#include <time.h>

//Paso 1
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
//Paso 2 
void init_lights(TrafficLight *L, int n){
    for (int i=0;i<n;i++){
        L[i].id = i;
        L[i].ticks_en_estado = 0;
        L[i].dur_rojo = 3; L[i].dur_verde = 3; L[i].dur_amarillo = 1;
        //  patrón para que al inicio no todas estén en ROJO:
        if (i % 3 == 0) L[i].estado = 0;       // rojo los 3, 6 etc
        else if (i % 3 == 1) L[i].estado = 1;  // verde los 1, 4, 7 etc
        else L[i].estado = 2;                  // amarillo los 2, 5, 8 etc
    }
}


//  variedad en vehículos: que no todos dependan de la misma luz
void init_vehicles(Vehicle *V, int n, int ruta_max, int num_luces){
    srand(42);
    for (int i=0;i<n;i++){
        V[i].id = i;
        V[i].posicion = 0;
        V[i].velocidad = 1 + (i % 2); // 1 o 2 celdas/iteración
        V[i].destino = ruta_max;
        V[i].luz_asociada = i % num_luces; // distribuye vehículos entre luces
    }
}

//Paso 3 Comportamiento de semáforos

// Cambia estados con contadores de duración: 0=ROJO, 1=VERDE, 2=AMARILLO
void update_traffic_lights(TrafficLight *L, int n){
    for (int i = 0; i < n; i++) {
        L[i].ticks_en_estado++;  // avanza un "tick" en el estado actual

        if (L[i].estado == 0) { // ROJO
            if (L[i].ticks_en_estado >= L[i].dur_rojo) {
                L[i].estado = 1;                // pasa a VERDE
                L[i].ticks_en_estado = 0;       // reinicia contador en el nuevo estado
            }
        } else if (L[i].estado == 1) { // VERDE
            if (L[i].ticks_en_estado >= L[i].dur_verde) {
                L[i].estado = 2;                // pasa a AMARILLO
                L[i].ticks_en_estado = 0;
            }
        } else { // estado == 2  (AMARILLO)
            if (L[i].ticks_en_estado >= L[i].dur_amarillo) {
                L[i].estado = 0;                // vuelve a ROJO
                L[i].ticks_en_estado = 0;
            }
        }
    }
}

//Paso 4 Comportamiento de vehículos
// nv = número de vehículos
// nl = número de luces o semaforos
void move_vehicles(Vehicle *V, int nv, TrafficLight *L, int nl, int ruta_max){
    for (int i=0;i<nv;i++){
        int luz = V[i].luz_asociada;
        if (luz>=0 && luz<nl && L[luz].estado==1){
            if (V[i].posicion < V[i].destino){
                V[i].posicion += V[i].velocidad;
                if (V[i].posicion>ruta_max) V[i].posicion=ruta_max;
            }
        }
    }
}

int main(int argc, char **argv){
    int num_iters   = (argc>1)? atoi(argv[1]) : 50;
    int num_luces   = (argc>2)? atoi(argv[2]) : 4;
    int num_autos   = (argc>3)? atoi(argv[3]) : 100000;
    int ruta_max    = (argc>4)? atoi(argv[4]) : 100;

    TrafficLight *L = malloc(sizeof(TrafficLight)*num_luces);
    Vehicle *V = malloc(sizeof(Vehicle)*num_autos);
    if(!L || !V){ fprintf(stderr,"memoria insuficiente\n"); return 1; }

    init_lights(L, num_luces);
    init_vehicles(V, num_autos, ruta_max, num_luces);
    clock_t start = clock();

    for (int t=0; t<num_iters; t++){
        if (t < 5) { // imprime las 5 primeras iteraciones como en las slides
            printf("\nIteración %d\n", t+1);
            // muestra los primeros 20 vehículos
            int mostrar = (num_autos < 20 ? num_autos : 20);
            for (int i=0; i<mostrar; i++){
                printf("Vehículo %d - Posición: %d\n", i, V[i].posicion);
            }
            // muestra los semáforos
            for (int j=0; j<num_luces; j++){
                printf("Semáforo %d - Estado: %d\n", j, L[j].estado);
            }
        }
        update_traffic_lights(L, num_luces);
        move_vehicles(V, num_autos, L, num_luces, ruta_max);
    }
    clock_t end = clock();
    double tiempo = (double)(end - start) / CLOCKS_PER_SEC;
    long suma=0; for(int i=0;i<num_autos;i++) suma+=V[i].posicion;
    printf("Checksum posiciones: %ld\n", suma);
    printf("Tiempo secuencial: %f s\n", tiempo);

    free(L); free(V);    

    return 0;
}

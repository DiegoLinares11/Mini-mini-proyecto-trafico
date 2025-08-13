#include <stdio.h>
#include <stdlib.h>
#include <omp.h>

//Paso 1
// Estados: 0=ROJO, 1=VERDE, 2=AMARILLO
typedef struct {
    int id, estado, ticks_en_estado, dur_rojo, dur_verde, dur_amarillo;
} TrafficLight;

typedef struct {
    int id, posicion, velocidad, destino, luz_asociada;
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

void update_traffic_lights(TrafficLight *L, int n){
    #pragma omp parallel for schedule(static)
    for (int i=0;i<n;i++){
        L[i].ticks_en_estado++;
        if (L[i].estado==0 && L[i].ticks_en_estado>=L[i].dur_rojo){
            L[i].estado=1; L[i].ticks_en_estado=0;
        } else if (L[i].estado==1 && L[i].ticks_en_estado>=L[i].dur_verde){
            L[i].estado=2; L[i].ticks_en_estado=0;
        } else if (L[i].estado==2 && L[i].ticks_en_estado>=L[i].dur_amarillo){
            L[i].estado=0; L[i].ticks_en_estado=0;
        }
    }
}


//Paso 4 Comportamiento de vehículos
void move_vehicles(Vehicle *V, int nv, TrafficLight *L, int nl, int ruta_max){
    // Trabajo masivo → paralelismo con for + scheduling dinámico
    #pragma omp parallel for schedule(dynamic,1024)
    for (int i=0;i<nv;i++){
        int luz = V[i].luz_asociada;
        if (luz>=0 && luz<nl && L[luz].estado==1){
            int pos = V[i].posicion + V[i].velocidad;
            if (pos>ruta_max) pos=ruta_max;
            V[i].posicion = pos;
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

    // Paralelismo dinámico (ajusta #hilos según la carga)
    omp_set_dynamic(1);
    // (Opcional) anidado:
    // omp_set_nested(1);

    init_lights(L, num_luces);
    init_vehicles(V, num_autos, ruta_max, num_luces);

    double t0 = omp_get_wtime();
    for (int t=0; t<num_iters; t++){
        // Heurística simple: más autos => más hilos
        int nthreads = num_autos/20000 + 2;
        if (nthreads > 32) nthreads = 32; // cap razonable
        omp_set_num_threads(nthreads);

        // Como en las slides: actualizar luces y mover vehículos en paralelo
        //Avca hay paralelismo de tareas
        #pragma omp parallel
        {
            #pragma omp single
            update_traffic_lights(L, num_luces);
            
            #pragma omp single  
            move_vehicles(V, num_autos, L, num_luces, ruta_max);
        }


        // Impresión estilo “Iteración N” (solo primeras 4–5 para no saturar)
        if (t < 5){
            printf("\nIteración %d\n", t+1);
            int mostrar = (num_autos < 20 ? num_autos : 20);
            for (int i=0; i<mostrar; i++)
                printf("Vehículo %d - Posición: %d\n", i, V[i].posicion);
            for (int j=0; j<num_luces; j++)
                printf("Semáforo %d - Estado: %d\n", j, L[j].estado);
        }
    }
    double t1 = omp_get_wtime();

    long checksum = 0;
    #pragma omp parallel for reduction(+:checksum) schedule(static)
    for (int i=0; i<num_autos; i++) checksum += V[i].posicion;

    printf("\nChecksum posiciones: %ld\n", checksum);
    printf("Tiempo paralelo: %.6f s\n", t1 - t0);

    free(L); free(V);
    return 0;
}

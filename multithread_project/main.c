#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>
#include <time.h>

#define CAPACITY 4
#define NUM_PASSENGERS 12

typedef struct {
    int capacity;
    int boarded;
    pthread_mutex_t board_mutex;
    
    sem_t board_queue;
    sem_t unboard_queue;
    
    sem_t all_aboard;
    sem_t all_ashore;
} RollerCoasterData;

// Tipo necessario para receber a função createThread
void* passenger(void* arg);
// Tipo necessario para receber a função createThread
void* roller_coaster(void* arg);
void board(RollerCoasterData* rc, int passenger_id);
void unboard(RollerCoasterData* rc, int passenger_id);

RollerCoasterData rc;

void init_roller_coaster(RollerCoasterData* rc, int capacity) {
    rc->capacity = capacity;
    rc->boarded = 0;

    // Inicializar mutex e semáforos
    pthread_mutex_init(&rc->board_mutex, NULL);
    
    // todos os lugares prontos para embarcar
    sem_init(&rc->board_queue, 0, capacity);
    // nenhum lugar pronto para desembarcar
    sem_init(&rc->unboard_queue, 0, 0);
    
    // Ninguem embarcou
    sem_init(&rc->all_aboard, 0, 0);
    // Todos já desembarcaram
    sem_init(&rc->all_ashore, 0, 0);
}

// Cada passageiro tenta embarcar e desembarcar
void* passenger(void* arg) {
    int passenger_id = *(int*)arg;
    printf("Passenger %d is waiting to board.\n", passenger_id);
    
    // Esperar o embarque
    sem_wait(&rc.board_queue);
    board(&rc, passenger_id);
    
    // Esperar o desembarque
    sem_wait(&rc.unboard_queue);
    unboard(&rc, passenger_id);
    
    return NULL;
}

// Para cada oassageiro
void board(RollerCoasterData* rc, int passenger_id) {
    // Tornar o embarque atomico
    pthread_mutex_lock(&rc->board_mutex);
    rc->boarded++;
    printf("Passenger %d boarded. (%d/%d)\n", passenger_id, rc->boarded, rc->capacity);
    
    // Adicionar animação aqui
    printf("🎢");
    for (int i = 0; i < rc->boarded; i++) printf("🚶");
    for (int i = 0; i < rc->capacity - rc->boarded; i++) printf(" _");
    printf("🎢\n");
    
    usleep(500000 + (rand() % 1000000));
    
    if (rc->boarded == rc->capacity) {
        printf("Roller coaster is full. Starting the ride!\n");
        // Ultimo passageiro avisa que todos embarcaram
        sem_post(&rc->all_aboard);
    }
    // Liberar para outras threads embarque
    pthread_mutex_unlock(&rc->board_mutex);
}

void unboard(RollerCoasterData* rc, int passenger_id) {
    // Tornar o desembarque atomico
    pthread_mutex_lock(&rc->board_mutex);
    rc->boarded--;

    // Adiciona animação aqui
    usleep(500000 + (rand() % 1000000));
    
    printf("🎢");
    for (int i = 0; i < rc->capacity - rc->boarded; i++) printf(" _");
    for (int i = 0; i < rc->boarded; i++) printf("🚶");
    printf("🎢\n");
    
    printf("Passenger %d unboarded. (%d/%d)\n", passenger_id, rc->capacity - rc->boarded, rc->capacity);
    
    if (rc->boarded == 0) {
        printf("All passengers have unboarded. Ready for the next ride!\n");
        // Ultimo passajeiro avisa que todos desembarcaram
        sem_post(&rc->all_ashore);
    }
    // Liberar para outras threads desembarque
    pthread_mutex_unlock(&rc->board_mutex);
}

void* roller_coaster(void* arg) {
    while (1) {
        printf("Roller coaster is ready for boarding.\n");
        
        // esperar o ultimo passageiro embarcar
        sem_wait(&rc.all_aboard);
        
        printf("Roller coaster is running...\n");
        sleep(2); // Simulate the ride
        printf("Roller coaster ride finished.\n");
        
        // Liberar os espacos do desembarque
        for (int i = 0; i < CAPACITY; i++) {
            sem_post(&rc.unboard_queue);
        }
        
        // esperar até que o ultimo passageiro desembarque
        sem_wait(&rc.all_ashore);

        // Liberar para os espacos do embarque
        for (int i = 0; i < CAPACITY; i++) {
            sem_post(&rc.board_queue);
        }
    }
    return NULL;
}

int main() {
    srand(time(NULL));
    init_roller_coaster(&rc, CAPACITY);
    
    pthread_t passengers[NUM_PASSENGERS];
    pthread_t coaster_thread;
    int passenger_ids[NUM_PASSENGERS];
    
    // Criar as threads
    for (int i = 0; i < NUM_PASSENGERS; i++) {
        passenger_ids[i] = i;
        pthread_create(&passengers[i], NULL, passenger, &passenger_ids[i]);
    }
    
    // Create roller coaster thread
    pthread_create(&coaster_thread, NULL, roller_coaster, NULL);
    
    // Depois de 30 segundos de execução, o programa é encerrado
    sleep(30);
    printf("Shutting down the program after 30 seconds.\n");

    // Limpar recursos
    pthread_mutex_destroy(&rc.board_mutex);
    sem_destroy(&rc.board_queue);
    sem_destroy(&rc.unboard_queue);
    sem_destroy(&rc.all_aboard);
    sem_destroy(&rc.all_ashore);
    
    return 0;
}
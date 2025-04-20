#include <SDL2/SDL.h>
#include <SDL2/SDL2_gfxPrimitives.h>
#include <SDL2/SDL_image.h>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>

#define WIDTH 900
#define HEIGHT 600
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

RollerCoasterData rc;
SDL_Window *window;
SDL_Renderer *renderer;

SDL_Texture* cart_texture = NULL;
SDL_Texture* passenger_textures[3]; // 0: waiting, 1: boarded, 2: unboarded

int passenger_states[NUM_PASSENGERS]; // 0: waiting, 1: boarded, 2: unboarded
int ride_state = 0;
float cart_position = 300;
float cart_velocity = 0;
pthread_mutex_t gui_mutex = PTHREAD_MUTEX_INITIALIZER;

float get_cart_y(float x) {
    float a = 0.002;
    float h = WIDTH/2 ;
    float k = 200;
    return a * (x - h) + k;
}

SDL_Texture* load_texture(const char* path) {
    SDL_Surface* surface = IMG_Load(path);
    if (!surface) {
        printf("Erro ao carregar imagem: %s\n", path);
        return NULL;
    }
    SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
    SDL_FreeSurface(surface);
    return texture;
}

void draw_passenger(int x, int y, int state) {
    SDL_Rect dst = { x-20, y, 50, 50 };
    SDL_RenderCopy(renderer, passenger_textures[state], NULL, &dst);
}

void draw_cart(float x, float y) {
    SDL_Rect dst = { (int)x, (int)y, 120, 60 };
    SDL_RenderCopy(renderer, cart_texture, NULL, &dst);
    filledCircleColor(renderer, x + 20, y + 60, 10, 0x000000FF);
    filledCircleColor(renderer, x + 100, y + 60, 10, 0x000000FF);
}

void draw_legend() {
    stringRGBA(renderer, 20, 20, "Legenda:", 255, 255, 255, 255);
    SDL_Rect d0 = { 30, 45, 20, 20 }; SDL_RenderCopy(renderer, passenger_textures[0], NULL, &d0);
    stringRGBA(renderer, 60, 50, "Esperando", 255, 255, 255, 255);
    SDL_Rect d1 = { 30, 75, 20, 20 }; SDL_RenderCopy(renderer, passenger_textures[1], NULL, &d1);
    stringRGBA(renderer, 60, 80, "Embarcado", 255, 255, 255, 255);
    SDL_Rect d2 = { 30, 105, 20, 20 }; SDL_RenderCopy(renderer, passenger_textures[2], NULL, &d2);
    stringRGBA(renderer, 60, 110, "Desembarcado", 255, 255, 255, 255);
}

void draw_scene() {
    SDL_SetRenderDrawColor(renderer, 30, 30, 30, 255);
    SDL_RenderClear(renderer);

    pthread_mutex_lock(&gui_mutex);
    cart_position += cart_velocity;
    if (cart_position > WIDTH - 150) cart_position = WIDTH - 150;
    float cart_y = get_cart_y(cart_position);
    draw_cart(cart_position, cart_y);

    int onboard = 0;
    for (int i = 0; i < NUM_PASSENGERS && onboard < CAPACITY; i++) {
        if (passenger_states[i] == 1) {
            draw_passenger(cart_position + 20 + onboard * 20, cart_y - 20, 1);
            onboard++;
        }
    }

    for (int i = 0; i < NUM_PASSENGERS; i++) {
        int px = 100 + (i % 6) * 60;
        int py = 400 + (i / 6) * 60;
        draw_passenger(px, py, passenger_states[i]);
    }

    draw_legend();
    pthread_mutex_unlock(&gui_mutex);

    SDL_RenderPresent(renderer);
}

void init_roller_coaster(RollerCoasterData* rc, int capacity) {
    rc->capacity = capacity;
    rc->boarded = 0;
    pthread_mutex_init(&rc->board_mutex, NULL);
    sem_init(&rc->board_queue, 0, capacity);
    sem_init(&rc->unboard_queue, 0, 0);
    sem_init(&rc->all_aboard, 0, 0);
    sem_init(&rc->all_ashore, 0, 0);
}

void board(RollerCoasterData* rc, int id) {
    pthread_mutex_lock(&rc->board_mutex);
    rc->boarded++;
    pthread_mutex_lock(&gui_mutex);
    passenger_states[id] = 1;
    pthread_mutex_unlock(&gui_mutex);
    usleep(300000 + rand() % 1000000);
    if (rc->boarded == rc->capacity) sem_post(&rc->all_aboard);
    pthread_mutex_unlock(&rc->board_mutex);
}

void unboard(RollerCoasterData* rc, int id) {
    pthread_mutex_lock(&rc->board_mutex);
    rc->boarded--;
    pthread_mutex_lock(&gui_mutex);
    passenger_states[id] = 2;
    pthread_mutex_unlock(&gui_mutex);
    usleep(300000 + rand() % 1000000);
    if (rc->boarded == 0) sem_post(&rc->all_ashore);
    pthread_mutex_unlock(&rc->board_mutex);
}

void* passenger(void* arg) {
    int id = *(int*)arg;
    free(arg);
    passenger_states[id] = 0;
    sem_wait(&rc.board_queue);
    board(&rc, id);
    sem_wait(&rc.unboard_queue);
    unboard(&rc, id);
    return NULL;
}

void* roller_coaster(void* arg) {
    while (1) {
        sem_wait(&rc.all_aboard);
        pthread_mutex_lock(&gui_mutex);
        cart_velocity = 3;
        pthread_mutex_unlock(&gui_mutex);
        sleep(3);
        pthread_mutex_lock(&gui_mutex);
        cart_velocity = 0;
        cart_position = 300;
        pthread_mutex_unlock(&gui_mutex);
        for (int i = 0; i < CAPACITY; i++) sem_post(&rc.unboard_queue);
        sem_wait(&rc.all_ashore);
        for (int i = 0; i < CAPACITY; i++) sem_post(&rc.board_queue);
    }
    return NULL;
}

int main() {
    srand(time(NULL));
    init_roller_coaster(&rc, CAPACITY);

    SDL_Init(SDL_INIT_VIDEO);
    IMG_Init(IMG_INIT_PNG);
    window = SDL_CreateWindow("🎢 Montanha-Russa SDL2", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, WIDTH, HEIGHT, 0);
    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

    cart_texture = load_texture("carrinho.png");
    passenger_textures[0] = load_texture("twitch.png");
    passenger_textures[1] = load_texture("gragas2.png");
    passenger_textures[2] = load_texture("malphite.png");

    pthread_t coaster_thread;
    pthread_create(&coaster_thread, NULL, roller_coaster, NULL);

    for (int i = 0; i < NUM_PASSENGERS; i++) {
        int *id = malloc(sizeof(int));
        *id = i;
        pthread_t t;
        pthread_create(&t, NULL, passenger, id);
    }

    SDL_Event event;
    int running = 1;
    while (running) {
        while (SDL_PollEvent(&event)) if (event.type == SDL_QUIT) running = 0;
        draw_scene();
        SDL_Delay(1000 / 60);
    }

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}


#include "raylib.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

// create sth of interest.
// anything of interest will spawn problems
// if you solve these problems well,
// you may create sth useful on the scale
// of these problems if not the whole thing
// but above all: create sth of interest.

// also do it "headless"
// also use clay for nice UI

#define GRAVITY 10
#define SCREEN_WIDTH 1440
#define SCREEN_HEIGHT 900
#define MAX_PLANETS 100

typedef struct Planet {
  char name[30];
  float mass;
  float diameter;
  Vector2 pos;
  Vector2 velo;
  Vector2 accel;
} Planet;

typedef struct {
    size_t size;
    size_t used;
    char *memory;
} Arena;

void InitArena(Arena *arena, size_t size) {
    arena->size = size;
    arena->used = 0;
    arena->memory = (char *)malloc(size);
}

Arena *ArenaAllocate(Arena *arena, size_t size) {
    if (arena->used + size > arena->size) {
        return NULL;
    }
    void *ptr = arena->memory + arena->used;
    arena->used += size;
    return ptr;
}

typedef struct {
    Planet *pool_array[MAX_PLANETS];
    size_t pool_size;
    Arena *arena;
} PlanetPool;

int GetTotalPoolMemory() {
    return (MAX_PLANETS * sizeof(Planet)) + sizeof(PlanetPool) + sizeof(Arena);
}

PlanetPool *InitPlanetPool() {
    PlanetPool *pool = (PlanetPool *)malloc(sizeof(PlanetPool));
    if (!pool) {
        printf("ERROR: Failed to allocate the planet pool. This should not be happening.");
        return NULL;
    }
    pool->pool_size = 0;
    pool->arena = (Arena *)malloc(sizeof(Arena)); // this is just the pointer to the arena, not the mem
    if (!pool->arena) {
        printf("ERROR: Failed to allocate the arena. This should not be happening.");
        free(pool);
        return NULL;
    }
    size_t arena_size = MAX_PLANETS * sizeof(Planet);
    InitArena(pool->arena, arena_size); // here we set the actual mem

    printf("INFO: Successfully init'ed planet pool. Total memory allocation: %d bytes\n", GetTotalPoolMemory());
    return pool;
}

void FreePlanetPool(PlanetPool *pool) {
    if (pool) {
        free(pool->arena->memory);
        free(pool->arena);
        free(pool);
    }
    printf("INFO: Successfully free'd the planet pool.\n");
}

int AddPlanet(PlanetPool *pool, Planet *planet) {
    if (pool->pool_size >= MAX_PLANETS) {
        printf("ERROR: Too many planets. Max allowed is %d. Pool's closed!\n", MAX_PLANETS);
        return -1;
    }

    Planet *new_planet = (Planet *)ArenaAllocate(pool->arena, sizeof(Planet));
    if (!new_planet) {
        printf("ERROR: Failed to allocate new planet in arena. This should not be happening.\n");
        return -1;
    }
    
    strlcpy(new_planet->name, planet->name, sizeof(new_planet->name));
    new_planet->mass = planet->mass;
    new_planet->diameter =  planet->diameter;
    new_planet->pos =  planet->pos;
    new_planet->velo =  planet->velo;
    new_planet->accel =  planet->accel;

    pool->pool_array[pool->pool_size++] = new_planet;
    return 1; 
}

void DrawPlanet(Planet *planet, Vector2 pos) {
  DrawCircleSector(pos, planet->diameter, 0.0, 360.0, 50, RED);
}

void UpdatePairwise(Planet *planet1, Planet *planet2) {
  Vector2 distance = {planet2->pos.x - planet1->pos.x,
                      planet2->pos.y - planet1->pos.y};

  float forceMagnitude = GRAVITY * planet1->mass * planet2->mass /
                         (distanceMagnitude * distanceMagnitude);

  Vector2 distanceUnit = {distance.x / distanceMagnitude,
                          distance.y / distanceMagnitude};

  Vector2 accel1 = {forceMagnitude * distanceUnit.x / planet1->mass,
                    forceMagnitude * distanceUnit.y / planet1->mass};

  Vector2 accel2 = {-forceMagnitude * distanceUnit.x / planet2->mass,
                    -forceMagnitude * distanceUnit.y / planet2->mass};

  planet1->velo.x += accel1.x;
  planet1->velo.y += accel1.y;

  planet2->velo.x += accel2.x;
  planet2->velo.y += accel2.y;

  planet1->pos.x += planet1->velo.x;
  planet1->pos.y += planet1->velo.y;

  planet2->pos.x += planet2->velo.x;
  planet2->pos.y += planet2->velo.y;
}

// we need to check for circle collision
void CheckCollision(Planet *p1, Planet *p2) {
    (void) p1;
    (void) p2;
    assert(false && "TODO: Not Implemented");
}

// TODO: add collision test
void UpdateTrajectories(PlanetPool *planet_pool) {
  for (size_t i = 0; i < planet_pool->pool_size; i++) {
    for (size_t j = i + 1; j < planet_pool->pool_size; j++) {
      UpdatePairwise(planet_pool->pool_array[i], planet_pool->pool_array[j]);
      CheckCollision(planet_pool->pool_array[i], planet_pool->pool_array[j]);
    }
  }
}

int main() {

    float posX = (float)SCREEN_WIDTH / 2;
    float posY = (float)SCREEN_HEIGHT / 2;

    // for now: define all planet configs here
    // how do we define planet configs en masse?
    // need some settings where I can place "stable" clusters.
    Vector2 posEarth = {posX + 150, posY + 150};
    Vector2 posSun = {posX, posY};
    Vector2 posMars = {posX - 150, posY - 150};
    
    Planet earth = {"Earth", 20, 30, posEarth, {-2.5f, 3.5f}, {0.0f, 0.0f}};
    Planet sun = {"Sun", 500, 75, posSun, {0.0f, 0.0f}, {0.0f, 0.0f}};
    Planet mars = {"Mars", 10, 25, posMars, {-1.5f, +2.5f}, {0.0f, 0.0f}};

    PlanetPool *planet_pool = InitPlanetPool();
    AddPlanet(planet_pool, &earth);
    AddPlanet(planet_pool, &sun);
    AddPlanet(planet_pool, &mars);

    int total_mem = GetTotalPoolMemory(); 

    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "N-Body Simulation");
    SetTargetFPS(60);

    Camera2D camera = {0};
    camera.rotation = 0.0f;
    camera.zoom = 1.0f;
    
    bool pause = false;

  while (!WindowShouldClose()) {

    if (IsKeyDown(KEY_W)) {
      camera.target.y -= 10.0f;
    }

    if (IsKeyDown(KEY_S)) {
      camera.target.y += 10.0f;
    }

    if (IsKeyDown(KEY_A)) {
      camera.target.x -= 10.0f;
    }

    if (IsKeyDown(KEY_D)) {
      camera.target.x += 10.0f;
    }

    if (IsKeyPressed(KEY_I)) {
      camera.zoom -= 0.1f;
    }

    if (IsKeyPressed(KEY_O)) {
      camera.zoom += 0.1f;
    }

    if (IsKeyPressed(KEY_SPACE)) {
       pause = !pause; 
    }
    
    BeginDrawing();

    ClearBackground(RAYWHITE);

    BeginMode2D(camera);
    
    // TODO: mouse pos is off for some reason?
    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
        Vector2 mousePos = GetMousePosition();
        Planet defaultPlanet = {"Default", 20, 30, mousePos, {-0.5f, 1.5f}, {0.0f, 0.0f}};
        AddPlanet(planet_pool, &defaultPlanet); 
    }

    if (!pause) {
        UpdateTrajectories(planet_pool);
    }

    for (size_t i = 0; i < planet_pool->pool_size; i++) {
        DrawPlanet(planet_pool->pool_array[i], planet_pool->pool_array[i]->pos);
        DrawText((char *)planet_pool->pool_array[i]->name, planet_pool->pool_array[i]->pos.x, planet_pool->pool_array[i]->pos.y, 15, BLACK);
    }

    EndMode2D();

    DrawFPS(10, 5);
    DrawText(TextFormat("Maximum allowed planets: %d\n", MAX_PLANETS), 10, 25, 15, BLACK);
    DrawText(TextFormat("Total allocated pool memory: %d Bytes\n", total_mem), 10, 45, 15, BLACK);

    EndDrawing();
  }

    CloseWindow();
    FreePlanetPool(planet_pool);
    return 0;
}

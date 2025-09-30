#include "world/chunk_system.h"
#include "core/math3d.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

// Global block blueprints
static BlockBlueprint* g_block_blueprints = NULL;

// Block blueprint system
BlockBlueprint* create_block_blueprints() {
    if (g_block_blueprints) {
        return g_block_blueprints; // Already created
    }
    
    // Allocate array for all block types - Expandido: 16 tipos
    g_block_blueprints = (BlockBlueprint*)safe_calloc(16, sizeof(BlockBlueprint)); // 16 block types
    if (!g_block_blueprints) return NULL;
    
    // Initialize each block type - SIMPLIFICADO: Solo 4 tipos básicos
    // VOXEL_AIR
    g_block_blueprints[VOXEL_AIR].type = VOXEL_AIR;
    g_block_blueprints[VOXEL_AIR].baseColor = (Color){0, 0, 0}; // Transparent
    g_block_blueprints[VOXEL_AIR].isSolid = FALSE;
    g_block_blueprints[VOXEL_AIR].isTransparent = TRUE;
    g_block_blueprints[VOXEL_AIR].durability = 0;
    g_block_blueprints[VOXEL_AIR].hardness = 0.0f;
    g_block_blueprints[VOXEL_AIR].isFlammable = FALSE;
    g_block_blueprints[VOXEL_AIR].isGravityAffected = FALSE;
    g_block_blueprints[VOXEL_AIR].friction = 0.0f;
    strcpy(g_block_blueprints[VOXEL_AIR].name, "Air");
    
    // VOXEL_GRASS - Pasto verde (suelo)
    g_block_blueprints[VOXEL_GRASS].type = VOXEL_GRASS;
    g_block_blueprints[VOXEL_GRASS].baseColor = (Color){34, 139, 34}; // Verde
    g_block_blueprints[VOXEL_GRASS].isSolid = TRUE;
    g_block_blueprints[VOXEL_GRASS].isTransparent = FALSE;
    g_block_blueprints[VOXEL_GRASS].durability = 2;
    g_block_blueprints[VOXEL_GRASS].hardness = 0.2f;
    g_block_blueprints[VOXEL_GRASS].isFlammable = FALSE;
    g_block_blueprints[VOXEL_GRASS].isGravityAffected = FALSE;
    g_block_blueprints[VOXEL_GRASS].friction = 0.6f;
    strcpy(g_block_blueprints[VOXEL_GRASS].name, "Grass");
    
    // VOXEL_WOOD - Madera café (troncos)
    g_block_blueprints[VOXEL_WOOD].type = VOXEL_WOOD;
    g_block_blueprints[VOXEL_WOOD].baseColor = (Color){101, 67, 33}; // Café
    g_block_blueprints[VOXEL_WOOD].isSolid = TRUE;
    g_block_blueprints[VOXEL_WOOD].isTransparent = FALSE;
    g_block_blueprints[VOXEL_WOOD].durability = 8;
    g_block_blueprints[VOXEL_WOOD].hardness = 0.6f;
    g_block_blueprints[VOXEL_WOOD].isFlammable = TRUE;
    g_block_blueprints[VOXEL_WOOD].isGravityAffected = FALSE;
    g_block_blueprints[VOXEL_WOOD].friction = 0.5f;
    strcpy(g_block_blueprints[VOXEL_WOOD].name, "Wood");
    
    // VOXEL_LEAVES - Hojas verde oscuro (árboles)
    g_block_blueprints[VOXEL_LEAVES].type = VOXEL_LEAVES;
    g_block_blueprints[VOXEL_LEAVES].baseColor = (Color){0, 80, 0}; // Verde oscuro
    g_block_blueprints[VOXEL_LEAVES].isSolid = TRUE;
    g_block_blueprints[VOXEL_LEAVES].isTransparent = TRUE;
    g_block_blueprints[VOXEL_LEAVES].durability = 1;
    g_block_blueprints[VOXEL_LEAVES].hardness = 0.1f;
    g_block_blueprints[VOXEL_LEAVES].isFlammable = TRUE;
    g_block_blueprints[VOXEL_LEAVES].isGravityAffected = FALSE;
    g_block_blueprints[VOXEL_LEAVES].friction = 0.3f;
    strcpy(g_block_blueprints[VOXEL_LEAVES].name, "Leaves");
    
    // VOXEL_STONE - Piedra gris
    g_block_blueprints[VOXEL_STONE].type = VOXEL_STONE;
    g_block_blueprints[VOXEL_STONE].baseColor = (Color){128, 128, 128}; // Gris
    g_block_blueprints[VOXEL_STONE].isSolid = TRUE;
    g_block_blueprints[VOXEL_STONE].isTransparent = FALSE;
    g_block_blueprints[VOXEL_STONE].durability = 30;
    g_block_blueprints[VOXEL_STONE].hardness = 1.5f;
    g_block_blueprints[VOXEL_STONE].isFlammable = FALSE;
    g_block_blueprints[VOXEL_STONE].isGravityAffected = FALSE;
    g_block_blueprints[VOXEL_STONE].friction = 0.8f;
    strcpy(g_block_blueprints[VOXEL_STONE].name, "Stone");
    
    // VOXEL_DIRT - Tierra marrón
    g_block_blueprints[VOXEL_DIRT].type = VOXEL_DIRT;
    g_block_blueprints[VOXEL_DIRT].baseColor = (Color){139, 69, 19}; // Marrón
    g_block_blueprints[VOXEL_DIRT].isSolid = TRUE;
    g_block_blueprints[VOXEL_DIRT].isTransparent = FALSE;
    g_block_blueprints[VOXEL_DIRT].durability = 2;
    g_block_blueprints[VOXEL_DIRT].hardness = 0.2f;
    g_block_blueprints[VOXEL_DIRT].isFlammable = FALSE;
    g_block_blueprints[VOXEL_DIRT].isGravityAffected = FALSE;
    g_block_blueprints[VOXEL_DIRT].friction = 0.6f;
    strcpy(g_block_blueprints[VOXEL_DIRT].name, "Dirt");
    
    // VOXEL_SAND - Arena amarilla
    g_block_blueprints[VOXEL_SAND].type = VOXEL_SAND;
    g_block_blueprints[VOXEL_SAND].baseColor = (Color){238, 203, 173}; // Amarillo claro
    g_block_blueprints[VOXEL_SAND].isSolid = TRUE;
    g_block_blueprints[VOXEL_SAND].isTransparent = FALSE;
    g_block_blueprints[VOXEL_SAND].durability = 1;
    g_block_blueprints[VOXEL_SAND].hardness = 0.1f;
    g_block_blueprints[VOXEL_SAND].isFlammable = FALSE;
    g_block_blueprints[VOXEL_SAND].isGravityAffected = TRUE;
    g_block_blueprints[VOXEL_SAND].friction = 0.4f;
    strcpy(g_block_blueprints[VOXEL_SAND].name, "Sand");
    
    // VOXEL_WATER - Agua azul
    g_block_blueprints[VOXEL_WATER].type = VOXEL_WATER;
    g_block_blueprints[VOXEL_WATER].baseColor = (Color){0, 100, 200}; // Azul
    g_block_blueprints[VOXEL_WATER].isSolid = FALSE;
    g_block_blueprints[VOXEL_WATER].isTransparent = TRUE;
    g_block_blueprints[VOXEL_WATER].durability = 0;
    g_block_blueprints[VOXEL_WATER].hardness = 0.0f;
    g_block_blueprints[VOXEL_WATER].isFlammable = FALSE;
    g_block_blueprints[VOXEL_WATER].isGravityAffected = TRUE;
    g_block_blueprints[VOXEL_WATER].friction = 0.1f;
    strcpy(g_block_blueprints[VOXEL_WATER].name, "Water");
    
    // VOXEL_LAVA - Lava roja
    g_block_blueprints[VOXEL_LAVA].type = VOXEL_LAVA;
    g_block_blueprints[VOXEL_LAVA].baseColor = (Color){255, 100, 0}; // Rojo-naranja
    g_block_blueprints[VOXEL_LAVA].isSolid = FALSE;
    g_block_blueprints[VOXEL_LAVA].isTransparent = TRUE;
    g_block_blueprints[VOXEL_LAVA].durability = 0;
    g_block_blueprints[VOXEL_LAVA].hardness = 0.0f;
    g_block_blueprints[VOXEL_LAVA].isFlammable = FALSE;
    g_block_blueprints[VOXEL_LAVA].isGravityAffected = TRUE;
    g_block_blueprints[VOXEL_LAVA].friction = 0.1f;
    strcpy(g_block_blueprints[VOXEL_LAVA].name, "Lava");
    
    // VOXEL_IRON - Hierro gris oscuro
    g_block_blueprints[VOXEL_IRON].type = VOXEL_IRON;
    g_block_blueprints[VOXEL_IRON].baseColor = (Color){105, 105, 105}; // Gris oscuro
    g_block_blueprints[VOXEL_IRON].isSolid = TRUE;
    g_block_blueprints[VOXEL_IRON].isTransparent = FALSE;
    g_block_blueprints[VOXEL_IRON].durability = 50;
    g_block_blueprints[VOXEL_IRON].hardness = 3.0f;
    g_block_blueprints[VOXEL_IRON].isFlammable = FALSE;
    g_block_blueprints[VOXEL_IRON].isGravityAffected = FALSE;
    g_block_blueprints[VOXEL_IRON].friction = 0.9f;
    strcpy(g_block_blueprints[VOXEL_IRON].name, "Iron");
    
    // VOXEL_GOLD - Oro amarillo
    g_block_blueprints[VOXEL_GOLD].type = VOXEL_GOLD;
    g_block_blueprints[VOXEL_GOLD].baseColor = (Color){255, 215, 0}; // Dorado
    g_block_blueprints[VOXEL_GOLD].isSolid = TRUE;
    g_block_blueprints[VOXEL_GOLD].isTransparent = FALSE;
    g_block_blueprints[VOXEL_GOLD].durability = 30;
    g_block_blueprints[VOXEL_GOLD].hardness = 3.0f;
    g_block_blueprints[VOXEL_GOLD].isFlammable = FALSE;
    g_block_blueprints[VOXEL_GOLD].isGravityAffected = FALSE;
    g_block_blueprints[VOXEL_GOLD].friction = 0.9f;
    strcpy(g_block_blueprints[VOXEL_GOLD].name, "Gold");
    
    // VOXEL_DIAMOND - Diamante azul claro
    g_block_blueprints[VOXEL_DIAMOND].type = VOXEL_DIAMOND;
    g_block_blueprints[VOXEL_DIAMOND].baseColor = (Color){0, 191, 255}; // Azul claro
    g_block_blueprints[VOXEL_DIAMOND].isSolid = TRUE;
    g_block_blueprints[VOXEL_DIAMOND].isTransparent = FALSE;
    g_block_blueprints[VOXEL_DIAMOND].durability = 100;
    g_block_blueprints[VOXEL_DIAMOND].hardness = 5.0f;
    g_block_blueprints[VOXEL_DIAMOND].isFlammable = FALSE;
    g_block_blueprints[VOXEL_DIAMOND].isGravityAffected = FALSE;
    g_block_blueprints[VOXEL_DIAMOND].friction = 0.9f;
    strcpy(g_block_blueprints[VOXEL_DIAMOND].name, "Diamond");
    
    // VOXEL_COAL - Carbón negro
    g_block_blueprints[VOXEL_COAL].type = VOXEL_COAL;
    g_block_blueprints[VOXEL_COAL].baseColor = (Color){64, 64, 64}; // Negro
    g_block_blueprints[VOXEL_COAL].isSolid = TRUE;
    g_block_blueprints[VOXEL_COAL].isTransparent = FALSE;
    g_block_blueprints[VOXEL_COAL].durability = 15;
    g_block_blueprints[VOXEL_COAL].hardness = 1.0f;
    g_block_blueprints[VOXEL_COAL].isFlammable = TRUE;
    g_block_blueprints[VOXEL_COAL].isGravityAffected = FALSE;
    g_block_blueprints[VOXEL_COAL].friction = 0.7f;
    strcpy(g_block_blueprints[VOXEL_COAL].name, "Coal");
    
    // VOXEL_GLASS - Vidrio transparente
    g_block_blueprints[VOXEL_GLASS].type = VOXEL_GLASS;
    g_block_blueprints[VOXEL_GLASS].baseColor = (Color){200, 200, 255}; // Azul muy claro
    g_block_blueprints[VOXEL_GLASS].isSolid = TRUE;
    g_block_blueprints[VOXEL_GLASS].isTransparent = TRUE;
    g_block_blueprints[VOXEL_GLASS].durability = 1;
    g_block_blueprints[VOXEL_GLASS].hardness = 0.3f;
    g_block_blueprints[VOXEL_GLASS].isFlammable = FALSE;
    g_block_blueprints[VOXEL_GLASS].isGravityAffected = FALSE;
    g_block_blueprints[VOXEL_GLASS].friction = 0.1f;
    strcpy(g_block_blueprints[VOXEL_GLASS].name, "Glass");
    
    // VOXEL_BRICK - Ladrillo rojo
    g_block_blueprints[VOXEL_BRICK].type = VOXEL_BRICK;
    g_block_blueprints[VOXEL_BRICK].baseColor = (Color){178, 34, 34}; // Rojo
    g_block_blueprints[VOXEL_BRICK].isSolid = TRUE;
    g_block_blueprints[VOXEL_BRICK].isTransparent = FALSE;
    g_block_blueprints[VOXEL_BRICK].durability = 20;
    g_block_blueprints[VOXEL_BRICK].hardness = 1.0f;
    g_block_blueprints[VOXEL_BRICK].isFlammable = FALSE;
    g_block_blueprints[VOXEL_BRICK].isGravityAffected = FALSE;
    g_block_blueprints[VOXEL_BRICK].friction = 0.8f;
    strcpy(g_block_blueprints[VOXEL_BRICK].name, "Brick");
    
    // VOXEL_CONCRETE - Concreto gris
    g_block_blueprints[VOXEL_CONCRETE].type = VOXEL_CONCRETE;
    g_block_blueprints[VOXEL_CONCRETE].baseColor = (Color){169, 169, 169}; // Gris claro
    g_block_blueprints[VOXEL_CONCRETE].isSolid = TRUE;
    g_block_blueprints[VOXEL_CONCRETE].isTransparent = FALSE;
    g_block_blueprints[VOXEL_CONCRETE].durability = 40;
    g_block_blueprints[VOXEL_CONCRETE].hardness = 2.0f;
    g_block_blueprints[VOXEL_CONCRETE].isFlammable = FALSE;
    g_block_blueprints[VOXEL_CONCRETE].isGravityAffected = FALSE;
    g_block_blueprints[VOXEL_CONCRETE].friction = 0.8f;
    strcpy(g_block_blueprints[VOXEL_CONCRETE].name, "Concrete");
    
    printf("Block blueprints creados: %d tipos de bloques (expandido)\n", 16);
    return g_block_blueprints;
}

void destroy_block_blueprints(BlockBlueprint* blueprints) {
    if (blueprints) {
        safe_free(blueprints);
        g_block_blueprints = NULL;
        printf("Block blueprints destruidos\n");
    }
}

BlockBlueprint* get_block_blueprint(BlockBlueprint* blueprints, VoxelType type) {
    if (!blueprints || type < 0 || type >= 4) return NULL; // Solo 4 tipos
    return &blueprints[type];
}

void initialize_block_from_blueprint(VoxelBlock* block, BlockBlueprint* blueprint) {
    if (!block || !blueprint) return;
    
    block->type = blueprint->type;
    block->color = blueprint->baseColor;
    block->isVisible = (blueprint->type != VOXEL_AIR);
    block->currentDurability = blueprint->durability;
    
    // Initialize face visibility
    block->hasTopFace = block->isVisible;
    block->hasBottomFace = block->isVisible;
    block->hasLeftFace = block->isVisible;
    block->hasRightFace = block->isVisible;
    block->hasFrontFace = block->isVisible;
    block->hasBackFace = block->isVisible;
}

// Create chunk manager with memory pool
ChunkManager* create_chunk_manager(int maxChunks, int renderDistance) {
    ChunkManager* manager = (ChunkManager*)safe_malloc(sizeof(ChunkManager));
    if (!manager) return NULL;
    
    manager->chunks = (VoxelChunk**)safe_calloc(maxChunks, sizeof(VoxelChunk*));
    if (!manager->chunks) {
        safe_free(manager);
        return NULL;
    }
    
    // Create memory pool for chunks - OPTIMIZADO PARA 3x3 CENTRADO
    size_t pool_size = maxChunks * sizeof(VoxelChunk) * 3; // Triple tamaño para seguridad
    manager->chunkPool = create_memory_pool(pool_size, sizeof(VoxelChunk));
    if (!manager->chunkPool) {
        safe_free(manager->chunks);
        safe_free(manager);
        return NULL;
    }
    
    manager->maxChunks = maxChunks;
    manager->loadedChunks = 0;
    manager->renderDistance = renderDistance;
    manager->chunkSize = 16;
    manager->blockSize = 1.0f;
    
    printf("ChunkManager creado: %d chunks máximos, distancia de render: %d\n", maxChunks, renderDistance);
    
    return manager;
}

// Destroy chunk manager
void destroy_chunk_manager(ChunkManager* manager) {
    if (!manager) return;
    
    printf("Destruyendo ChunkManager...\n");
    
    // Free all chunks
    for (int i = 0; i < manager->maxChunks; i++) {
        if (manager->chunks[i]) {
            // Chunks are allocated from pool, so no individual free needed
            manager->chunks[i] = NULL;
        }
    }
    
    // Destroy memory pool
    if (manager->chunkPool) {
        destroy_memory_pool(manager->chunkPool);
    }
    
    safe_free(manager->chunks);
    safe_free(manager);
    
    printf("ChunkManager destruido.\n");
}

// Get or create chunk with optimized memory management
VoxelChunk* get_or_create_chunk(ChunkManager* manager, int chunkX, int chunkY, int chunkZ) {
    if (!manager) return NULL;
    
    // Find existing chunk
    for (int i = 0; i < manager->maxChunks; i++) {
        if (manager->chunks[i] && 
            manager->chunks[i]->chunkX == chunkX &&
            manager->chunks[i]->chunkY == chunkY &&
            manager->chunks[i]->chunkZ == chunkZ) {
            return manager->chunks[i];
        }
    }
    
    // Check if we can create a new chunk - OPTIMIZADO
    if (manager->loadedChunks >= manager->maxChunks) {
        // Try to unload distant chunks first
        printf("Límite de chunks alcanzado (%d/%d). Liberando chunks distantes...\n", 
               manager->loadedChunks, manager->maxChunks);
        
        // Force unload oldest chunks to make space (FIFO)
        int chunksToUnload = manager->loadedChunks - (manager->maxChunks / 2); // Liberar la mitad
        for (int i = 0; i < manager->maxChunks && chunksToUnload > 0; i++) {
            if (manager->chunks[i]) {
                printf("Liberando chunk (%d, %d, %d) para espacio\n", 
                       manager->chunks[i]->chunkX, manager->chunks[i]->chunkY, manager->chunks[i]->chunkZ);
                manager->chunks[i] = NULL;
                manager->loadedChunks--;
                chunksToUnload--;
            }
        }
        
        // Reset pool if still no space
        if (manager->loadedChunks >= manager->maxChunks) {
            printf("Reset completo del pool de memoria...\n");
            pool_reset(manager->chunkPool);
            manager->loadedChunks = 0;
        }
    }
    
    // Allocate chunk from memory pool
    VoxelChunk* chunk = (VoxelChunk*)pool_alloc(manager->chunkPool);
    if (!chunk) {
        printf("ERROR: No se pudo asignar chunk desde el pool de memoria\n");
        return NULL;
    }
    
    // Initialize chunk
    memset(chunk, 0, sizeof(VoxelChunk));
    chunk->chunkX = chunkX;
    chunk->chunkY = chunkY;
    chunk->chunkZ = chunkZ;
    chunk->isGenerated = FALSE;
    chunk->isVisible = TRUE;
    chunk->distanceToCamera = 0.0f;
    
    // Initialize all blocks as air using blueprints
    BlockBlueprint* blueprints = create_block_blueprints();
    if (blueprints) {
        BlockBlueprint* airBlueprint = get_block_blueprint(blueprints, VOXEL_AIR);
        for (int x = 0; x < 16; x++) {
            for (int y = 0; y < 16; y++) {
                for (int z = 0; z < 16; z++) {
                    initialize_block_from_blueprint(&chunk->blocks[x][y][z], airBlueprint);
                }
            }
        }
    }
    
    // Add to manager
    for (int i = 0; i < manager->maxChunks; i++) {
        if (!manager->chunks[i]) {
            manager->chunks[i] = chunk;
            manager->loadedChunks++;
            printf("Chunk creado: (%d, %d, %d) - Total: %d/%d\n", 
                   chunkX, chunkY, chunkZ, manager->loadedChunks, manager->maxChunks);
            break;
        }
    }
    
    return chunk;
}

// Terrain generation with procedural matrix
TerrainGenerator create_terrain_generator(int seed) {
    TerrainGenerator generator;
    generator.seed = seed;
    generator.frequency = 0.01f;
    generator.amplitude = 10.0f;
    generator.octaves = 4;
    generator.persistence = 0.5f;
    generator.lacunarity = 2.0f;
    
    // Initialize procedural matrix (1000x1000 for large world)
    generator.matrix_size = 1000;
    generator.noise_matrix = (float*)safe_calloc(generator.matrix_size * generator.matrix_size, sizeof(float));
    
    if (generator.noise_matrix) {
        printf("Matriz de ruido creada: %dx%d (seed: %d)\n", generator.matrix_size, generator.matrix_size, seed);
        
        // Generate noise matrix
        for (int x = 0; x < generator.matrix_size; x++) {
            for (int y = 0; y < generator.matrix_size; y++) {
                int index = y * generator.matrix_size + x;
                generator.noise_matrix[index] = noise_2d(x, y, seed);
            }
        }
    }
    
    return generator;
}

// Get voxel color based on type - SIMPLIFICADO: Solo 4 tipos
Color get_voxel_color(VoxelType type) {
    switch (type) {
        case VOXEL_GRASS: return (Color){34, 139, 34};   // Verde
        case VOXEL_WOOD: return (Color){101, 67, 33};    // Café
        case VOXEL_LEAVES: return (Color){0, 80, 0};     // Verde oscuro
        default: return (Color){255, 255, 255};          // Blanco
    }
}

// Generate random color based on position and seed
Color generate_random_color(int x, int y, int z, int seed) {
    // Use position and seed to generate consistent random colors
    int colorSeed = (x * 31 + y * 17 + z * 13 + seed) % 1000000;
    
    // Generate RGB values with some variation
    int r = (colorSeed * 7) % 256;
    int g = (colorSeed * 11) % 256;
    int b = (colorSeed * 13) % 256;
    
    // Ensure minimum brightness for visibility
    if (r < 50) r = 50 + (r % 100);
    if (g < 50) g = 50 + (g % 100);
    if (b < 50) b = 50 + (b % 100);
    
    return (Color){r, g, b};
}

// Generate color variation for terrain types
Color get_terrain_color(VoxelType type, int x, int y, int z, int seed) {
    Color baseColor = get_voxel_color(type);
    Color randomColor = generate_random_color(x, y, z, seed);
    
    // Mix base color with random color (70% base, 30% random)
    Color finalColor;
    finalColor.r = (baseColor.r * 0.7f) + (randomColor.r * 0.3f);
    finalColor.g = (baseColor.g * 0.7f) + (randomColor.g * 0.3f);
    finalColor.b = (baseColor.b * 0.7f) + (randomColor.b * 0.3f);
    
    return finalColor;
}

// Simple 2D noise function
float noise_2d(float x, float z, int seed) {
    int n = (int)(x + z * 57 + seed * 131);
    n = (n << 13) ^ n;
    return (1.0f - ((n * (n * n * 15731 + 789221) + 1376312589) & 0x7fffffff) / 1073741824.0f);
}

// Fractal noise
float fractal_noise(float x, float z, TerrainGenerator* generator) {
    float value = 0.0f;
    float amplitude = generator->amplitude;
    float frequency = generator->frequency;
    
    for (int i = 0; i < generator->octaves; i++) {
        value += noise_2d(x * frequency, z * frequency, generator->seed + i) * amplitude;
        amplitude *= generator->persistence;
        frequency *= generator->lacunarity;
    }
    
    return value;
}

// Get terrain height at world position using procedural matrix
int get_terrain_height(float x, float y, TerrainGenerator* generator) {
    if (!generator || !generator->noise_matrix) {
        return 32; // Default height
    }
    
    // Map world coordinates to matrix coordinates
    int matrix_x = (int)(x + generator->matrix_size / 2) % generator->matrix_size;
    int matrix_y = (int)(y + generator->matrix_size / 2) % generator->matrix_size;
    
    if (matrix_x < 0) matrix_x += generator->matrix_size;
    if (matrix_y < 0) matrix_y += generator->matrix_size;
    
    // Get noise value from matrix
    int index = matrix_y * generator->matrix_size + matrix_x;
    float noise_value = generator->noise_matrix[index];
    
    // Convert to height (0-64 range)
    int height = (int)((noise_value + 1.0f) * 32.0f); // Scale from [-1,1] to [0,64]
    
    return height;
}

// Get block type based on height - SUELO EN Z=0 (DONDE PISA EL JUGADOR)
VoxelType get_terrain_block_type(int x, int y, int z, int height) {
    (void)x; // Suppress unused parameter warning
    (void)y; // Suppress unused parameter warning
    (void)height; // Suppress unused parameter warning
    
    // Solo la capa del suelo (z=0) es pasto, todo lo demás es aire
    if (z == 0) {
        return VOXEL_GRASS;
    }
    return VOXEL_AIR; // Todo lo demás es aire
}

// Global array to track tree positions for safe zone checking
struct {
    int x, y, z;
    int radius;
} g_tree_positions[1000]; // Maximum 1000 trees tracked
int g_tree_count = 0;

// Generate chunk terrain - OPTIMIZED: Only generate top layer with procedural matrix
void generate_chunk_terrain(VoxelChunk* chunk, TerrainGenerator* generator) {
    if (!chunk || chunk->isGenerated) return;
    
    int worldChunkX = chunk->chunkX * 16;
    int worldChunkY = chunk->chunkY * 16;
    
    printf("Generando terreno para chunk (%d, %d, %d) usando matriz procedural\n", 
           chunk->chunkX, chunk->chunkY, chunk->chunkZ);
    
    // Get block blueprints
    BlockBlueprint* blueprints = create_block_blueprints();
    if (!blueprints) return;
    
    // Generate only the ground layer (z = 0) for grass floor - DONDE PISA EL JUGADOR
    for (int x = 0; x < 16; x++) {
        for (int y = 0; y < 16; y++) {
            int worldX = worldChunkX + x;
            int worldY = worldChunkY + y;
            
            // Generate only the ground layer (z = 0) - DONDE PISA EL JUGADOR
            int z = 0;
            int worldZ = chunk->chunkZ * 16 + z;
            VoxelType blockType = get_terrain_block_type(worldX, worldY, worldZ, 0);
            
            // Initialize block from blueprint
            BlockBlueprint* blueprint = get_block_blueprint(blueprints, blockType);
            if (blueprint) {
                initialize_block_from_blueprint(&chunk->blocks[x][y][z], blueprint);
                
                // Add color variation for non-air blocks
                if (blockType != VOXEL_AIR) {
                    chunk->blocks[x][y][z].color = get_terrain_color(blockType, worldX, worldY, worldZ, generator->seed);
                }
            }
            
            // All other layers remain as air
            BlockBlueprint* airBlueprint = get_block_blueprint(blueprints, VOXEL_AIR);
            for (int z_other = 1; z_other < 16; z_other++) {
                if (airBlueprint) {
                    initialize_block_from_blueprint(&chunk->blocks[x][y][z_other], airBlueprint);
                }
            }
        }
    }
    
    // Generate trees in this chunk - ÁRBOLES CON DIMENSIONES EXACTAS
    TreeGenerator treeGen = create_tree_generator(generator->seed);
    int treesGenerated = 0;
    
    for (int x = 0; x < 16; x++) {
        for (int y = 0; y < 16; y++) {
            int worldX = worldChunkX + x;
            int worldY = worldChunkY + y;
            int worldZ = chunk->chunkZ * 16 + 0; // Ground level (z=0)
            
            // Check if we should generate a tree here
            if (should_generate_tree_at(worldX, worldY, worldZ, &treeGen)) {
                // Make sure there's solid grass at this position (z=0)
                if (chunk->blocks[x][y][0].type == VOXEL_GRASS) {
                    Tree tree = generate_tree_at_position(x, y, 0, &treeGen);
                    
                    // Verificar que el árbol cabe completamente en el chunk
                    if (can_tree_fit_in_chunk(&tree, chunk)) {
                        place_tree_in_chunk(chunk, &tree);
                        
                        // Registrar posición del árbol para zona segura
                        int worldTreeX = worldChunkX + tree.x;
                        int worldTreeY = worldChunkY + tree.y;
                        int worldTreeZ = chunk->chunkZ * 16 + tree.z;
                        
                        // Usar radio de hojas como zona segura
                        if (g_tree_count < 1000) {
                            g_tree_positions[g_tree_count].x = worldTreeX;
                            g_tree_positions[g_tree_count].y = worldTreeY;
                            g_tree_positions[g_tree_count].z = worldTreeZ;
                            g_tree_positions[g_tree_count].radius = tree.leafRadius + 2; // Buffer adicional
                            g_tree_count++;
                        }
                        
                        treesGenerated++;
                    } else {
                        printf("Árbol omitido: no cabe en chunk (%d,%d,%d)\n", 
                               chunk->chunkX, chunk->chunkY, chunk->chunkZ);
                    }
                }
            }
        }
    }
    
    // Calculate face visibility for all layers (including trees)
    for (int x = 0; x < 16; x++) {
        for (int y = 0; y < 16; y++) {
            for (int z = 0; z < 16; z++) {
                if (chunk->blocks[x][y][z].type != VOXEL_AIR) {
                    calculate_block_faces(chunk, x, y, z);
                }
            }
        }
    }
    
    chunk->isGenerated = TRUE;
    printf("Terreno generado para chunk (%d, %d, %d) con %d árboles\n", 
           chunk->chunkX, chunk->chunkY, chunk->chunkZ, treesGenerated);
}

// Calculate which faces of a block are visible
void calculate_block_faces(VoxelChunk* chunk, int x, int y, int z) {
    VoxelBlock* block = &chunk->blocks[x][y][z];
    
    // Check adjacent blocks
    block->hasTopFace = !is_block_adjacent(chunk, x, y, z, 0, 1, 0);
    block->hasBottomFace = !is_block_adjacent(chunk, x, y, z, 0, -1, 0);
    block->hasLeftFace = !is_block_adjacent(chunk, x, y, z, -1, 0, 0);
    block->hasRightFace = !is_block_adjacent(chunk, x, y, z, 1, 0, 0);
    block->hasFrontFace = !is_block_adjacent(chunk, x, y, z, 0, 0, 1);
    block->hasBackFace = !is_block_adjacent(chunk, x, y, z, 0, 0, -1);
}

// Check if there's a block adjacent to the given position
BOOL is_block_adjacent(VoxelChunk* chunk, int x, int y, int z, int dx, int dy, int dz) {
    int newX = x + dx;
    int newY = y + dy;
    int newZ = z + dz;
    
    // Check bounds
    if (newX < 0 || newX >= 16 || newY < 0 || newY >= 16 || newZ < 0 || newZ >= 16) {
        return FALSE; // Assume no block outside chunk bounds
    }
    
    return chunk->blocks[newX][newY][newZ].type != VOXEL_AIR;
}

// Get block type at position
VoxelType get_block_type(VoxelChunk* chunk, int x, int y, int z) {
    if (x < 0 || x >= 16 || y < 0 || y >= 16 || z < 0 || z >= 16) {
        return VOXEL_AIR;
    }
    return chunk->blocks[x][y][z].type;
}

// Convert chunk coordinates to world position
Vect3 chunk_to_world_pos(int chunkX, int chunkY, int chunkZ, int blockX, int blockY, int blockZ) {
    Vect3 pos;
    pos.x = chunkX * 16 + blockX;
    pos.y = chunkY * 16 + blockY;
    pos.z = chunkZ * 16 + blockZ;
    return pos;
}

// Convert world position to chunk coordinates
void world_to_chunk_pos(float worldX, float worldY, float worldZ, int* chunkX, int* chunkY, int* chunkZ, int* blockX, int* blockY, int* blockZ) {
    *chunkX = (int)floor(worldX / 16.0f);
    *chunkY = (int)floor(worldY / 16.0f);
    *chunkZ = (int)floor(worldZ / 16.0f);
    
    *blockX = (int)worldX - (*chunkX * 16);
    *blockY = (int)worldY - (*chunkY * 16);
    *blockZ = (int)worldZ - (*chunkZ * 16);
}


// Get voxel opacity
float get_voxel_opacity(VoxelType type) {
    switch (type) {
        case VOXEL_LEAVES: return 0.8f; // Hojas semi-transparentes
        default: return 1.0f; // Sólido
    }
}

// Update chunk visibility based on camera position
void update_chunk_visibility(VoxelChunk* chunk, Vect3 cameraPosition) {
    if (!chunk) return;
    
    // Calculate chunk center
    Vect3 chunkCenter;
    chunkCenter.x = chunk->chunkX * 16 + 8;
    chunkCenter.y = chunk->chunkY * 16 + 8;
    chunkCenter.z = chunk->chunkZ * 16 + 8;
    
    // Calculate distance to camera
    Vect3 distance = vect3_subtract(chunkCenter, cameraPosition);
    chunk->distanceToCamera = vect3_length(distance);
    
    // Simple distance culling
    chunk->isVisible = (chunk->distanceToCamera < 100.0f);
}

// Render a single chunk - OPTIMIZED: Only render top layer
void render_chunk(VoxelChunk* chunk, Vect3 cameraPosition, Vect3 cameraForward) {
    if (!chunk || !chunk->isVisible || !chunk->isGenerated) return;
    
    // Suppress unused parameter warnings
    (void)cameraForward;
    
    // Calculate distance from camera to chunk center
    Vect3 chunkCenter = {
        chunk->chunkX * 16 + 8,
        chunk->chunkY * 16 + 8,
        chunk->chunkZ * 16 + 8
    };
    
    Vect3 distance = vect3_subtract(chunkCenter, cameraPosition);
    float distanceToChunk = vect3_length(distance);
    
    // Suppress unused variable warning
    (void)distanceToChunk;
    
    // Render only the top layer (z = 15) for ground
    for (int x = 0; x < 16; x++) {
        for (int y = 0; y < 16; y++) {
            int z = 15; // Only top layer
            VoxelBlock* block = &chunk->blocks[x][y][z];
            
            if (block->type == VOXEL_AIR || !block->isVisible) continue;
            
            // Suppress unused variable warnings
            (void)chunk;
            (void)x;
            (void)y;
            (void)z;
            (void)block;
            
            // Simple rendering - just draw a quad for the top face
            if (block->hasTopFace) {
                // This would be replaced with actual OpenGL rendering
                // For now, just mark as rendered
            }
        }
    }
}

// Render chunk manager
void render_chunk_manager(ChunkManager* manager, Vect3 cameraPosition, Vect3 cameraForward) {
    if (!manager) return;
    
    // Update visibility for all chunks
    for (int i = 0; i < manager->maxChunks; i++) {
        if (manager->chunks[i]) {
            update_chunk_visibility(manager->chunks[i], cameraPosition);
        }
    }
    
    // Render visible chunks
    for (int i = 0; i < manager->maxChunks; i++) {
        if (manager->chunks[i] && manager->chunks[i]->isVisible) {
            render_chunk(manager->chunks[i], cameraPosition, cameraForward);
        }
    }
}

// Update chunk loading system
void update_chunk_loading(ChunkManager* manager, Vect3 playerPosition, ChunkLoadingConfig config) {
    if (!manager || !config.enableChunkLoading) return;
    
    // Load chunks around player
    load_chunks_around_player(manager, playerPosition, config.loadDistance);
    
    // Unload distant chunks
    unload_distant_chunks(manager, playerPosition, config.unloadDistance);
}

// Load chunks around player position - PATRÓN 3x3 CENTRADO
void load_chunks_around_player(ChunkManager* manager, Vect3 playerPosition, int distance) {
    if (!manager) return;
    
    // Convert player position to chunk coordinates
    int playerChunkX, playerChunkY, playerChunkZ;
    int blockX, blockY, blockZ;
    world_to_chunk_pos(playerPosition.x, playerPosition.y, playerPosition.z, 
                      &playerChunkX, &playerChunkY, &playerChunkZ, 
                      &blockX, &blockY, &blockZ);
    
    // PATRÓN 3x3 CENTRADO EN EL JUGADOR - OPTIMIZADO
    // Cargar chunks en un cuadrado 3x3 centrado en la posición del jugador
    int centerX = playerChunkX;
    int centerY = playerChunkY;
    
    // Cargar solo chunks esenciales (3x3 máximo)
    int maxDistance = (distance > 1) ? 1 : distance; // Limitar a 3x3 máximo
    
    for (int x = centerX - maxDistance; x <= centerX + maxDistance; x++) {
        for (int y = centerY - maxDistance; y <= centerY + maxDistance; y++) {
            // Solo cargar chunks en el nivel del suelo (z = 0)
            VoxelChunk* chunk = get_or_create_chunk(manager, x, y, 0);
            if (chunk && !chunk->isGenerated) {
                // Usar el mismo generador de terreno para consistencia
                TerrainGenerator generator = create_terrain_generator(12345);
                generate_chunk_terrain(chunk, &generator);
                printf("Chunk cargado (3x3): (%d, %d, 0)\n", x, y);
            }
        }
    }
}

// Unload chunks that are too far from player
void unload_distant_chunks(ChunkManager* manager, Vect3 playerPosition, int maxDistance) {
    if (!manager) return;
    
    // Convert player position to chunk coordinates
    int playerChunkX, playerChunkY, playerChunkZ;
    int blockX, blockY, blockZ;
    world_to_chunk_pos(playerPosition.x, playerPosition.y, playerPosition.z, 
                      &playerChunkX, &playerChunkY, &playerChunkZ, 
                      &blockX, &blockY, &blockZ);
    
    // Check all loaded chunks
    for (int i = 0; i < manager->maxChunks; i++) {
        if (manager->chunks[i]) {
            VoxelChunk* chunk = manager->chunks[i];
            
            // Calculate distance from player chunk (solo X,Y para 2D)
            int dx = chunk->chunkX - playerChunkX;
            int dy = chunk->chunkY - playerChunkY;
            
            // Usar distancia Manhattan para mejor rendimiento
            int distance = abs(dx) + abs(dy);
            
            // Unload if too far (solo chunks del suelo z=0)
            if (distance > maxDistance && chunk->chunkZ == 0) {
                printf("Chunk descargado: (%d, %d, %d) - distancia: %d\n", 
                       chunk->chunkX, chunk->chunkY, chunk->chunkZ, distance);
                manager->chunks[i] = NULL;
                
                // Only decrement if loadedChunks > 0
                if (manager->loadedChunks > 0) {
                    manager->loadedChunks--;
                }
            }
        }
    }
}

// Emergency memory cleanup - force unload all chunks
void emergency_cleanup_chunks(ChunkManager* manager) {
    if (!manager) return;
    
    printf("EMERGENCIA: Limpiando todos los chunks para liberar memoria...\n");
    
    for (int i = 0; i < manager->maxChunks; i++) {
        if (manager->chunks[i]) {
            manager->chunks[i] = NULL;
        }
    }
    
    manager->loadedChunks = 0;
    
    // Reset memory pool
    if (manager->chunkPool) {
        pool_reset(manager->chunkPool);
    }
    
    printf("EMERGENCIA: Limpieza completada. Chunks liberados: %d\n", manager->maxChunks);
}

// World persistence system
void save_world_data(TerrainGenerator* generator, const char* filename) {
    if (!generator || !generator->noise_matrix) return;
    
    FILE* file = fopen(filename, "wb");
    if (!file) {
        printf("ERROR: No se pudo abrir archivo para guardar: %s\n", filename);
        return;
    }
    
    // Save generator metadata
    fwrite(&generator->seed, sizeof(int), 1, file);
    fwrite(&generator->matrix_size, sizeof(int), 1, file);
    
    // Save noise matrix
    size_t matrix_size = generator->matrix_size * generator->matrix_size;
    fwrite(generator->noise_matrix, sizeof(float), matrix_size, file);
    
    fclose(file);
    printf("Datos del mundo guardados en: %s\n", filename);
}

BOOL load_world_data(TerrainGenerator* generator, const char* filename) {
    if (!generator) return FALSE;
    
    FILE* file = fopen(filename, "rb");
    if (!file) {
        printf("Archivo de mundo no encontrado: %s\n", filename);
        return FALSE;
    }
    
    // Load generator metadata
    fread(&generator->seed, sizeof(int), 1, file);
    fread(&generator->matrix_size, sizeof(int), 1, file);
    
    // Allocate noise matrix
    generator->noise_matrix = (float*)safe_calloc(generator->matrix_size * generator->matrix_size, sizeof(float));
    if (!generator->noise_matrix) {
        fclose(file);
        return FALSE;
    }
    
    // Load noise matrix
    size_t matrix_size = generator->matrix_size * generator->matrix_size;
    fread(generator->noise_matrix, sizeof(float), matrix_size, file);
    
    fclose(file);
    printf("Datos del mundo cargados desde: %s (seed: %d, matriz: %dx%d)\n", 
           filename, generator->seed, generator->matrix_size, generator->matrix_size);
    
    return TRUE;
}

void delete_world_data(const char* filename) {
    if (remove(filename) == 0) {
        printf("Datos del mundo eliminados: %s\n", filename);
    } else {
        printf("No se pudo eliminar archivo: %s\n", filename);
    }
}

void destroy_terrain_generator(TerrainGenerator* generator) {
    if (generator && generator->noise_matrix) {
        safe_free(generator->noise_matrix);
        generator->noise_matrix = NULL;
        printf("Generador de terreno destruido\n");
    }
}

// ============================================================================
// 3D MATRIX BLOCK SYSTEM - Matriz 3D para mapa de bloques
// ============================================================================

// Create 3D block matrix
BlockMatrix3D* create_block_matrix_3d(int width, int height, int depth, int chunkSize) {
    BlockMatrix3D* matrix = (BlockMatrix3D*)safe_malloc(sizeof(BlockMatrix3D));
    if (!matrix) return NULL;
    
    matrix->width = width;
    matrix->height = height;
    matrix->depth = depth;
    matrix->chunkSize = chunkSize;
    matrix->isInitialized = FALSE;
    
    // Allocate 3D array
    matrix->blocks = (int***)safe_malloc(width * sizeof(int**));
    if (!matrix->blocks) {
        safe_free(matrix);
        return NULL;
    }
    
    for (int x = 0; x < width; x++) {
        matrix->blocks[x] = (int**)safe_malloc(height * sizeof(int*));
        if (!matrix->blocks[x]) {
            // Cleanup on failure
            for (int i = 0; i < x; i++) {
                safe_free(matrix->blocks[i]);
            }
            safe_free(matrix->blocks);
            safe_free(matrix);
            return NULL;
        }
        
        for (int y = 0; y < height; y++) {
            matrix->blocks[x][y] = (int*)safe_calloc(depth, sizeof(int));
            if (!matrix->blocks[x][y]) {
                // Cleanup on failure
                for (int i = 0; i < x; i++) {
                    for (int j = 0; j < height; j++) {
                        safe_free(matrix->blocks[i][j]);
                    }
                    safe_free(matrix->blocks[i]);
                }
                safe_free(matrix->blocks);
                safe_free(matrix);
                return NULL;
            }
        }
    }
    
    // Initialize all blocks to air
    for (int x = 0; x < width; x++) {
        for (int y = 0; y < height; y++) {
            for (int z = 0; z < depth; z++) {
                matrix->blocks[x][y][z] = VOXEL_AIR;
            }
        }
    }
    
    matrix->isInitialized = TRUE;
    printf("BlockMatrix3D creada: %dx%dx%d (chunk size: %d)\n", width, height, depth, chunkSize);
    return matrix;
}

// Destroy 3D block matrix
void destroy_block_matrix_3d(BlockMatrix3D* matrix) {
    if (!matrix) return;
    
    if (matrix->blocks) {
        for (int x = 0; x < matrix->width; x++) {
            if (matrix->blocks[x]) {
                for (int y = 0; y < matrix->height; y++) {
                    safe_free(matrix->blocks[x][y]);
                }
                safe_free(matrix->blocks[x]);
            }
        }
        safe_free(matrix->blocks);
    }
    
    safe_free(matrix);
    printf("BlockMatrix3D destruida\n");
}

// Get block type at position
int get_block_at(BlockMatrix3D* matrix, int x, int y, int z) {
    if (!matrix || !matrix->isInitialized || !is_valid_position(matrix, x, y, z)) {
        return VOXEL_AIR; // Default to air if invalid
    }
    
    return matrix->blocks[x][y][z];
}

// Set block type at position
void set_block_at(BlockMatrix3D* matrix, int x, int y, int z, int blockType) {
    if (!matrix || !matrix->isInitialized || !is_valid_position(matrix, x, y, z)) {
        printf("Error: Invalid position (%d,%d,%d) or matrix not initialized\n", x, y, z);
        return;
    }
    
    matrix->blocks[x][y][z] = blockType;
    printf("Block set at (%d,%d,%d) = %d\n", x, y, z, blockType);
}

// Check if position is valid
BOOL is_valid_position(BlockMatrix3D* matrix, int x, int y, int z) {
    if (!matrix || !matrix->isInitialized) return FALSE;
    
    return (x >= 0 && x < matrix->width &&
            y >= 0 && y < matrix->height &&
            z >= 0 && z < matrix->depth);
}

// Check if block type is solid
BOOL is_solid_block(int blockType) {
    // All blocks except air, water, and lava are solid
    return (blockType != VOXEL_AIR && 
            blockType != VOXEL_WATER && 
            blockType != VOXEL_LAVA);
}

// Render terrain from matrix (simplified version)
void render_terrain_from_matrix(BlockMatrix3D* matrix, int startX, int startY, int startZ, int endX, int endY, int endZ) {
    if (!matrix || !matrix->isInitialized) return;
    
    printf("Rendering terrain from matrix: (%d,%d,%d) to (%d,%d,%d)\n", 
           startX, startY, startZ, endX, endY, endZ);
    
    // Clamp coordinates to valid range
    startX = (startX < 0) ? 0 : startX;
    startY = (startY < 0) ? 0 : startY;
    startZ = (startZ < 0) ? 0 : startZ;
    endX = (endX >= matrix->width) ? matrix->width - 1 : endX;
    endY = (endY >= matrix->height) ? matrix->height - 1 : endY;
    endZ = (endZ >= matrix->depth) ? matrix->depth - 1 : endZ;
    
    // Render blocks in the specified range
    for (int x = startX; x <= endX; x++) {
        for (int y = startY; y <= endY; y++) {
            for (int z = startZ; z <= endZ; z++) {
                int blockType = matrix->blocks[x][y][z];
                if (blockType != VOXEL_AIR) {
                    // Here you would call the actual rendering function
                    // For now, just log the block
                    printf("Rendering block at (%d,%d,%d) = %d\n", x, y, z, blockType);
                }
            }
        }
    }
}

// Regenerate terrain below a position (for surface block breaking)
void regenerate_terrain_below(BlockMatrix3D* matrix, int x, int y, int z) {
    if (!matrix || !matrix->isInitialized || !is_valid_position(matrix, x, y, z)) {
        return;
    }
    
    printf("Regenerating terrain below (%d,%d,%d)\n", x, y, z);
    
    // Check if this is a surface block (no solid blocks above)
    BOOL isSurface = TRUE;
    for (int checkZ = z + 1; checkZ < matrix->depth; checkZ++) {
        if (is_solid_block(matrix->blocks[x][y][checkZ])) {
            isSurface = FALSE;
            break;
        }
    }
    
    if (isSurface) {
        // Regenerate terrain below this position
        for (int newZ = z - 1; newZ >= 0; newZ--) {
            if (matrix->blocks[x][y][newZ] == VOXEL_AIR) {
                // Fill with appropriate terrain based on depth
                if (newZ >= 12) {
                    matrix->blocks[x][y][newZ] = VOXEL_GRASS; // Surface layer
                } else if (newZ >= 8) {
                    matrix->blocks[x][y][newZ] = VOXEL_DIRT;  // Dirt layer
                } else if (newZ >= 4) {
                    matrix->blocks[x][y][newZ] = VOXEL_STONE; // Stone layer
                } else {
                    matrix->blocks[x][y][newZ] = VOXEL_COAL;  // Deep layer
                }
            }
        }
        
        printf("Terrain regenerated below surface block\n");
    }
}

// ============================================================================
// TREE GENERATION SYSTEM
// ============================================================================

// Simple pseudo-random number generator for trees
static int tree_random(int* seed) {
    *seed = (*seed * 1103515245 + 12345) & 0x7fffffff;
    return *seed;
}

// Create tree generator
TreeGenerator create_tree_generator(int seed) {
    TreeGenerator generator;
    generator.seed = seed;
    generator.treeDensity = 0.02f; // 2% chance per chunk
    generator.minHeight = 4; // Altura mínima de 4 bloques (cabe en chunk)
    generator.maxHeight = 8; // Altura máxima de 8 bloques (cabe en chunk)
    generator.minLeafRadius = 2; // Hojas más pequeñas para caber
    generator.maxLeafRadius = 3; // Hojas más pequeñas para caber
    printf("Tree generator creado con seed: %d, densidad: %.2f, altura: %d-%d, hojas: %d-%d\n", 
           seed, generator.treeDensity, generator.minHeight, generator.maxHeight, 
           generator.minLeafRadius, generator.maxLeafRadius);
    return generator;
}

// Check if position is in safe zone of any existing tree
static BOOL is_in_tree_safe_zone(int x, int y, int z, int safeRadius) {
    for (int i = 0; i < g_tree_count; i++) {
        int dx = x - g_tree_positions[i].x;
        int dy = y - g_tree_positions[i].y;
        int dz = z - g_tree_positions[i].z;
        
        // Calculate distance (3D)
        float distance = sqrtf(dx * dx + dy * dy + dz * dz);
        
        // Check if within safe radius
        if (distance < (g_tree_positions[i].radius + safeRadius)) {
            return TRUE; // Too close to existing tree
        }
    }
    return FALSE; // Safe to place tree
}

// Check if a tree should be generated at this position
BOOL should_generate_tree_at(int x, int y, int z, TreeGenerator* generator) {
    if (!generator) return FALSE;
    
    // ZONA SEGURA: Verificar que no esté muy cerca de otros árboles
    int safeRadius = 8; // Radio de seguridad (ajustable)
    if (is_in_tree_safe_zone(x, y, z, safeRadius)) {
        return FALSE; // Demasiado cerca de otro árbol
    }
    
    // Use position-based seeding for consistent results
    int localSeed = generator->seed + x * 1000 + y * 100 + z;
    int randomValue = tree_random(&localSeed);
    
    // Convert to float [0, 1]
    float chance = (float)(randomValue % 10000) / 10000.0f;
    
    // Crear zonas de concentración usando ruido simple
    // Usar coordenadas del mundo para crear patrones de bosque
    float worldX = (float)x;
    float worldY = (float)y;
    
    // Crear zonas de concentración usando función seno/coseno
    float forestZone1 = sinf(worldX * 0.1f) * cosf(worldY * 0.1f);
    float forestZone2 = sinf(worldX * 0.05f + 1.0f) * cosf(worldY * 0.05f + 1.0f);
    float forestZone3 = sinf(worldX * 0.03f + 2.0f) * cosf(worldY * 0.03f + 2.0f);
    
    // Combinar zonas para crear patrones más complejos
    float forestPattern = (forestZone1 + forestZone2 + forestZone3) / 3.0f;
    
    // Aumentar densidad en zonas de concentración (reducido)
    float adjustedDensity = generator->treeDensity;
    if (forestPattern > 0.3f) {
        adjustedDensity *= 2.0f; // 2x más denso en zonas de bosque (reducido de 3x)
    } else if (forestPattern < -0.3f) {
        adjustedDensity *= 0.2f; // 5x menos denso en zonas despejadas (reducido de 10x)
    }
    
    return chance < adjustedDensity;
}

// Generate a tree at a specific position
Tree generate_tree_at_position(int x, int y, int z, TreeGenerator* generator) {
    Tree tree;
    tree.x = x;
    tree.y = y;
    tree.z = z;
    tree.trunkType = VOXEL_WOOD;
    tree.leafType = VOXEL_LEAVES;
    
    // Use position-based seeding for consistent tree properties
    int localSeed = generator->seed + x * 1000 + y * 100 + z;
    
    // Generate variable height (6-12 blocks) - más altos
    int heightRange = generator->maxHeight - generator->minHeight + 1;
    tree.height = generator->minHeight + (tree_random(&localSeed) % heightRange);
    
    // Generate trunk width (1-2 blocks) - más variado
    tree.trunkWidth = 1 + (tree_random(&localSeed) % 2);
    
    // Generate leaf radius (3-5 blocks) - más grandes
    int leafRange = generator->maxLeafRadius - generator->minLeafRadius + 1;
    tree.leafRadius = generator->minLeafRadius + (tree_random(&localSeed) % leafRange);
    
    // Generate leaf height (3-6 blocks) - más altas
    tree.leafHeight = 3 + (tree_random(&localSeed) % 4);
    
    printf("Árbol generado en (%d,%d,%d): altura=%d, tronco=%d, hojas_radio=%d, hojas_altura=%d\n",
           x, y, z, tree.height, tree.trunkWidth, tree.leafRadius, tree.leafHeight);
    
    return tree;
}

// Check if a tree can fit completely within a chunk
BOOL can_tree_fit_in_chunk(Tree* tree, VoxelChunk* chunk) {
    if (!tree || !chunk) return FALSE;
    
    // Verificar que el tronco cabe
    int trunkEndX = tree->x + tree->trunkWidth - 1;
    int trunkEndY = tree->y + tree->trunkWidth - 1;
    int trunkEndZ = tree->z + tree->height - 1;
    
    if (trunkEndX >= 16 || trunkEndY >= 16 || trunkEndZ >= 16) {
        return FALSE; // Tronco se sale del chunk
    }
    
    // Verificar que las hojas caben
    int leafStartZ = tree->z + tree->height - 2; // Hojas empiezan antes del final del tronco
    int leafEndZ = leafStartZ + tree->leafHeight - 1;
    
    if (leafEndZ >= 16) {
        return FALSE; // Hojas se salen del chunk por arriba
    }
    
    // Verificar que las hojas no se salen por los lados
    int leafMaxRadius = tree->leafRadius;
    int leafMinX = tree->x - leafMaxRadius;
    int leafMaxX = tree->x + leafMaxRadius;
    int leafMinY = tree->y - leafMaxRadius;
    int leafMaxY = tree->y + leafMaxRadius;
    
    if (leafMinX < 0 || leafMaxX >= 16 || leafMinY < 0 || leafMaxY >= 16) {
        return FALSE; // Hojas se salen del chunk por los lados
    }
    
    return TRUE; // El árbol cabe completamente en el chunk
}

// Place a tree in a chunk - ÁRBOLES CON DIMENSIONES EXACTAS DE BLOQUES
void place_tree_in_chunk(VoxelChunk* chunk, Tree* tree) {
    if (!chunk || !tree) return;
    
    // Get block blueprints
    BlockBlueprint* blueprints = create_block_blueprints();
    if (!blueprints) return;
    
    BlockBlueprint* woodBlueprint = get_block_blueprint(blueprints, VOXEL_WOOD);
    BlockBlueprint* leavesBlueprint = get_block_blueprint(blueprints, VOXEL_LEAVES);
    
    if (!woodBlueprint || !leavesBlueprint) return;
    
    // Calcular posición del árbol en el mundo
    int worldTreeX = chunk->chunkX * 16 + tree->x;
    int worldTreeY = chunk->chunkY * 16 + tree->y;
    int worldTreeZ = chunk->chunkZ * 16 + tree->z;
    
    printf("Colocando árbol en mundo (%d,%d,%d) chunk (%d,%d,%d) local (%d,%d,%d)\n",
           worldTreeX, worldTreeY, worldTreeZ, chunk->chunkX, chunk->chunkY, chunk->chunkZ, tree->x, tree->y, tree->z);
    
    // Place trunk - DIMENSIONES EXACTAS (1x1 o 2x2 bloques)
    for (int h = 0; h < tree->height; h++) {
        int trunkZ = tree->z + h; // Start from tree base
        
        // Solo colocar tronco si está dentro del chunk actual
        if (trunkZ >= 0 && trunkZ < 16) {
            // Place trunk blocks con dimensiones exactas
            for (int tx = 0; tx < tree->trunkWidth; tx++) {
                for (int ty = 0; ty < tree->trunkWidth; ty++) {
                    int blockX = tree->x + tx;
                    int blockY = tree->y + ty;
                    
                    // Check bounds del chunk
                    if (blockX >= 0 && blockX < 16 && blockY >= 0 && blockY < 16) {
                        VoxelBlock* block = &chunk->blocks[blockX][blockY][trunkZ];
                        if (block->type == VOXEL_AIR || block->type == VOXEL_GRASS) {
                            initialize_block_from_blueprint(block, woodBlueprint);
                        }
                    }
                }
            }
        }
    }
    
    // Place leaves - FORMA ESFÉRICA COMPACTA CON DIMENSIONES EXACTAS
    int trunkTopZ = tree->z + tree->height - 1;
    int leafStartZ = trunkTopZ - 1; // Empezar hojas un poco antes del final del tronco
    
    // Usar seed para crear variación consistente en las hojas
    int leafSeed = 12345 + worldTreeX * 1000 + worldTreeY * 100 + worldTreeZ;
    
    // Colocar hojas en forma de esfera compacta
    for (int lz = 0; lz < tree->leafHeight; lz++) {
        int currentZ = leafStartZ + lz;
        
        // Solo procesar si está dentro del chunk actual
        if (currentZ >= 0 && currentZ < 16) {
            // Calcular radio para este nivel (forma esférica)
            float levelProgress = (float)lz / (float)(tree->leafHeight - 1);
            float sphereRadius = tree->leafRadius * sinf(levelProgress * 3.14159f); // Esfera perfecta
            
            // Colocar hojas en patrón cuadrado para dimensiones exactas
            int radius = (int)sphereRadius;
            if (radius < 1) radius = 1; // Mínimo 1 bloque de radio
            
            for (int lx = -radius; lx <= radius; lx++) {
                for (int ly = -radius; ly <= radius; ly++) {
                    int blockX = tree->x + lx;
                    int blockY = tree->y + ly;
                    
                    // Check bounds del chunk
                    if (blockX >= 0 && blockX < 16 && blockY >= 0 && blockY < 16) {
                        // Distancia desde el centro del tronco
                        float distance = sqrtf(lx*lx + ly*ly);
                        
                        // Solo colocar si está dentro del radio esférico
                        if (distance <= sphereRadius + 0.5f) {
                            // Agregar pequeña variación orgánica
                            int noiseSeed = leafSeed + lx * 100 + ly * 10 + lz;
                            int randomChance = tree_random(&noiseSeed) % 100;
                            
                            // 90% de chance de colocar hoja (más compacto)
                            if (randomChance < 90) {
                                VoxelBlock* block = &chunk->blocks[blockX][blockY][currentZ];
                                if (block->type == VOXEL_AIR) {
                                    initialize_block_from_blueprint(block, leavesBlueprint);
                                }
                            }
                        }
                    }
                }
            }
        }
    }
    
    printf("Árbol colocado: altura=%d, tronco=%dx%d, hojas_radio=%d, hojas_altura=%d\n", 
           tree->height, tree->trunkWidth, tree->trunkWidth, tree->leafRadius, tree->leafHeight);
}

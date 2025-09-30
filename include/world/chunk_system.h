#ifndef CHUNK_SYSTEM_H
#define CHUNK_SYSTEM_H

#include "core/types.h"
#include "core/memory.h"

// Voxel block types - SIMPLIFICADO: Solo bloques básicos
typedef enum {
    VOXEL_AIR = 0,
    VOXEL_GRASS = 1,    // Pasto verde (suelo)
    VOXEL_WOOD = 2,     // Madera café (troncos)
    VOXEL_LEAVES = 3    // Hojas verde oscuro (árboles)
} VoxelType;

// Block blueprint - defines properties of each block type
typedef struct {
    VoxelType type;
    Color baseColor;        // Color base del bloque
    BOOL isSolid;           // Si tiene colisión
    BOOL isTransparent;     // Si es transparente
    int durability;         // Cuántos golpes aguanta antes de romperse
    float hardness;         // Qué tan difícil es de romper (0.0-1.0)
    BOOL isFlammable;       // Si se puede quemar
    BOOL isGravityAffected; // Si es afectado por la gravedad
    float friction;         // Fricción del bloque (0.0-1.0)
    char name[32];          // Nombre del bloque
} BlockBlueprint;

// Voxel block structure
typedef struct {
    VoxelType type;
    Color color;        // Color actual del bloque (puede variar)
    BOOL isVisible;
    BOOL hasTopFace;
    BOOL hasBottomFace;
    BOOL hasLeftFace;
    BOOL hasRightFace;
    BOOL hasFrontFace;
    BOOL hasBackFace;
    int currentDurability; // Durabilidad actual (se reduce al golpear)
} VoxelBlock;

// Chunk structure (16x16x16 blocks)
typedef struct {
    int chunkX, chunkY, chunkZ;  // Chunk coordinates
    VoxelBlock blocks[16][16][16];  // 16x16x16 voxel grid
    BOOL isGenerated;
    BOOL isVisible;
    float distanceToCamera;
} VoxelChunk;

// Chunk manager with optimized memory management
typedef struct {
    VoxelChunk** chunks;
    int maxChunks;
    int loadedChunks;
    int renderDistance;
    int chunkSize;
    float blockSize;
    MemoryPool* chunkPool;  // Memory pool for chunks
} ChunkManager;

// Terrain generation with procedural matrix
typedef struct {
    int seed;
    float frequency;
    float amplitude;
    int octaves;
    float persistence;
    float lacunarity;
    
    // Procedural matrix for large world generation
    int matrix_size;
    float* noise_matrix;
} TerrainGenerator;

// Tree structure for procedural generation
typedef struct {
    int x, y, z;           // Position
    int height;            // Tree height (4-8 blocks)
    int trunkWidth;        // Trunk width (1-2 blocks)
    int leafRadius;        // Leaf radius (2-4 blocks)
    int leafHeight;        // Leaf height (3-6 blocks)
    VoxelType trunkType;   // Wood type
    VoxelType leafType;    // Leaves type
} Tree;

// Tree generation system
typedef struct {
    int seed;
    float treeDensity;     // Trees per chunk (0.0-1.0)
    int minHeight;         // Minimum tree height
    int maxHeight;         // Maximum tree height
    int minLeafRadius;     // Minimum leaf radius
    int maxLeafRadius;     // Maximum leaf radius
} TreeGenerator;

// Chunk loading configuration
typedef struct {
    int renderDistance;
    int loadDistance;
    int unloadDistance;
    BOOL enableChunkLoading;
} ChunkLoadingConfig;

// Tree generation functions
TreeGenerator create_tree_generator(int seed);
Tree generate_tree_at_position(int x, int y, int z, TreeGenerator* generator);
void place_tree_in_chunk(VoxelChunk* chunk, Tree* tree);
BOOL should_generate_tree_at(int x, int y, int z, TreeGenerator* generator);

// Block blueprint system
BlockBlueprint* create_block_blueprints();
void destroy_block_blueprints(BlockBlueprint* blueprints);
BlockBlueprint* get_block_blueprint(BlockBlueprint* blueprints, VoxelType type);
void initialize_block_from_blueprint(VoxelBlock* block, BlockBlueprint* blueprint);

// Function declarations
ChunkManager* create_chunk_manager(int maxChunks, int renderDistance);
void destroy_chunk_manager(ChunkManager* manager);
VoxelChunk* get_or_create_chunk(ChunkManager* manager, int chunkX, int chunkY, int chunkZ);
void generate_chunk_terrain(VoxelChunk* chunk, TerrainGenerator* generator);
void update_chunk_visibility(VoxelChunk* chunk, Vect3 cameraPosition);
void render_chunk(VoxelChunk* chunk, Vect3 cameraPosition, Vect3 cameraForward);
void render_chunk_manager(ChunkManager* manager, Vect3 cameraPosition, Vect3 cameraForward);

// Terrain generation functions
TerrainGenerator create_terrain_generator(int seed);
float noise_2d(float x, float z, int seed);
float fractal_noise(float x, float z, TerrainGenerator* generator);
int get_terrain_height(float x, float y, TerrainGenerator* generator);
VoxelType get_terrain_block_type(int x, int y, int z, int height);

// Block face culling
void calculate_block_faces(VoxelChunk* chunk, int x, int y, int z);
BOOL is_block_adjacent(VoxelChunk* chunk, int x, int y, int z, int dx, int dy, int dz);
VoxelType get_block_type(VoxelChunk* chunk, int x, int y, int z);

// Utility functions
Vect3 chunk_to_world_pos(int chunkX, int chunkY, int chunkZ, int blockX, int blockY, int blockZ);
void world_to_chunk_pos(float worldX, float worldY, float worldZ, int* chunkX, int* chunkY, int* chunkZ, int* blockX, int* blockY, int* blockZ);
Color get_voxel_color(VoxelType type);
float get_voxel_opacity(VoxelType type);

// Chunk loading system
void update_chunk_loading(ChunkManager* manager, Vect3 playerPosition, ChunkLoadingConfig config);
void load_chunks_around_player(ChunkManager* manager, Vect3 playerPosition, int distance);
void unload_distant_chunks(ChunkManager* manager, Vect3 playerPosition, int maxDistance);
void emergency_cleanup_chunks(ChunkManager* manager);

// World persistence system
void save_world_data(TerrainGenerator* generator, const char* filename);
BOOL load_world_data(TerrainGenerator* generator, const char* filename);
void delete_world_data(const char* filename);
void destroy_terrain_generator(TerrainGenerator* generator);

#endif // CHUNK_SYSTEM_H

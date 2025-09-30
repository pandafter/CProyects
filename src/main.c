#include "world/chunk_system.h"  // Must be included before renderer.h
#include "graphics/window.h"
#include "core/memory.h"
#include <stdio.h>
#include <stdlib.h>

int main() {
    printf("=== VOXEL ENGINE - REFACTORED ARCHITECTURE ===\n");
    printf("Iniciando motor de voxels con arquitectura limpia...\n");
    
    // Initialize memory system
    printf("Inicializando sistema de memoria...\n");
    
    // Initialize game
    if (!initialize_game()) {
        printf("ERROR: Fallo al inicializar el juego\n");
        return -1;
    }
    
    printf("Juego inicializado correctamente. Iniciando bucle principal...\n");
    
    // Run game loop
    run_game_loop();
    
    // Cleanup
    cleanup_game();
    
    // Print memory statistics
    print_memory_stats();
    
    printf("Juego terminado correctamente.\n");
    return 0;
}

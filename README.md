# Voxel Engine - Refactored Architecture

Un motor de voxels 3D desarrollado en C con OpenGL, diseñado para ser escalable y mantenible.

## 🏗️ Arquitectura del Proyecto

### Estructura de Directorios

```
Etapa1C/
├── src/                          # Código fuente
│   ├── core/                     # Sistemas fundamentales
│   │   ├── memory.c              # Gestión de memoria
│   │   └── math3d.c              # Matemáticas 3D
│   ├── graphics/                 # Sistema de renderizado
│   │   ├── opengl/               # Implementación OpenGL
│   │   │   └── simple_opengl.c   # Contexto OpenGL principal
│   │   ├── shaders/              # Sistema de shaders
│   │   │   └── shaders.c         # Gestión de shaders GLSL
│   │   ├── effects/              # Efectos visuales
│   │   │   ├── Skybox.c          # Cielo procedural
│   │   │   ├── Shadow.c          # Sistema de sombras
│   │   │   └── Volumetrics.c     # Fog volumétrico
│   │   ├── renderer.c            # Renderizador principal
│   │   └── window.c              # Gestión de ventana
│   ├── world/                    # Sistema de mundo
│   │   └── chunk_system.c        # Gestión de chunks
│   └── main.c                    # Punto de entrada
├── include/                      # Headers
│   ├── core/                     # Headers de sistemas core
│   ├── graphics/                 # Headers de gráficos
│   │   ├── opengl/               # Headers OpenGL
│   │   ├── shaders/              # Headers de shaders
│   │   └── effects/              # Headers de efectos
│   └── world/                    # Headers del mundo
├── build/                        # Archivos compilados
├── Makefile                      # Sistema de build
└── README.md                     # Este archivo
```

## 🎮 Características

### Sistema de Renderizado
- **OpenGL 3.3+** con shaders modernos
- **Skybox procedural** con gradiente azul y sol
- **Iluminación direccional** (Lambert + Blinn-Phong)
- **Sistema de sombras** (preparado para implementación)
- **Fog volumétrico** (preparado para implementación)

### Sistema de Mundo
- **Generación procedural** de terreno
- **Sistema de chunks** 3x3 centrado en el jugador
- **Generación de árboles** con zonas seguras
- **Gestión de memoria** optimizada

### Sistema de Física
- **Movimiento del jugador** con gravedad
- **Sistema de vuelo** (doble espacio)
- **Detección de colisiones** básica
- **Física de salto** realista

### Sistema de Input
- **Controles WASD** para movimiento
- **Mouse look** para cámara
- **Teclas de función** para debug
- **Sistema de vuelo** con controles verticales

## 🚀 Compilación y Ejecución

### Requisitos
- **Windows 10/11**
- **MinGW-w64** o **MSYS2**
- **OpenGL 3.3+** compatible

### Compilación
```bash
# Compilación normal
make

# Compilación con debug
make debug

# Compilación optimizada
make release

# Limpiar archivos de build
make clean

# Compilar y ejecutar
make run
```

### Ejecución
```bash
./voxel_engine.exe
```

## 🎯 Controles

### Movimiento
- **W, A, S, D** - Movimiento horizontal
- **Mouse** - Mirar alrededor
- **Espacio** - Saltar (en tierra)
- **Doble Espacio** - Activar/desactivar vuelo
- **Espacio (mantener)** - Subir (en vuelo)
- **Shift (mantener)** - Bajar (en vuelo)

### Debug
- **F** - Toggle fog
- **G** - Toggle vuelo
- **L** - Test de iluminación
- **ESC** - Salir

## 🔧 Sistemas Técnicos

### Core Systems
- **Memory Management**: Pool de memoria para chunks
- **Math3D**: Vectores, matrices, transformaciones
- **Types**: Definiciones de tipos básicos

### Graphics Systems
- **OpenGL Context**: Gestión del contexto OpenGL
- **Shader System**: Compilación y enlace de shaders
- **Skybox**: Cielo procedural con gradiente
- **Lighting**: Iluminación direccional realista

### World Systems
- **Chunk System**: Generación y gestión de chunks
- **Terrain Generation**: Generación procedural
- **Tree Generation**: Árboles con zonas seguras

## 📈 Escalabilidad

### Estructura Modular
- **Separación de responsabilidades** clara
- **Interfaces bien definidas** entre módulos
- **Fácil adición** de nuevos sistemas

### Optimizaciones
- **Pool de memoria** para chunks
- **Culling de chunks** no visibles
- **Shaders optimizados** para rendimiento
- **Gestión eficiente** de recursos OpenGL

### Extensiones Futuras
- **Sistema de audio** (carpeta `src/audio/`)
- **Sistema de red** (carpeta `src/network/`)
- **Sistema de IA** (carpeta `src/ai/`)
- **Sistema de UI** (carpeta `src/ui/`)

## 🐛 Debugging

### Información en Pantalla
- **FPS** en tiempo real
- **Uso de memoria** del proceso
- **Estado del día/noche**
- **Intensidad del sol** en lúmenes
- **Estado de chunks** cargados

### Logs de Consola
- **Inicialización** de sistemas
- **Errores de shaders** con detalles
- **Estado de chunks** y memoria
- **Eventos de input** importantes

## 🔮 Roadmap

### Próximas Características
- [ ] **Sistema de audio** con OpenAL
- [ ] **Sistema de red** multijugador
- [ ] **Sistema de UI** con ImGui
- [ ] **Sistema de guardado** de mundos
- [ ] **Sistema de mods** con scripting

### Mejoras Técnicas
- [ ] **Vulkan backend** como alternativa
- [ ] **Sistema de LOD** para chunks
- [ ] **Occlusion culling** avanzado
- [ ] **Sistema de partículas**
- [ ] **Post-processing** effects

## 📝 Notas de Desarrollo

### Convenciones de Código
- **C99 standard** para compatibilidad
- **Naming convention**: snake_case para funciones y variables
- **Documentación**: Comentarios en español para claridad
- **Error handling**: Verificación de punteros y valores de retorno

### Performance
- **Target**: 60 FPS en 1080p con GPU GTX 1060
- **Memory**: Pool de memoria para evitar fragmentación
- **GPU**: Shaders optimizados para hardware moderno
- **CPU**: Algoritmos eficientes para generación procedural

## 🤝 Contribución

### Estructura de Commits
- **feat**: Nueva característica
- **fix**: Corrección de bug
- **refactor**: Refactorización de código
- **docs**: Documentación
- **perf**: Mejora de rendimiento

### Testing
- **Compilación** en diferentes configuraciones
- **Testing** de rendimiento en hardware objetivo
- **Validación** de shaders en diferentes GPUs
- **Testing** de memoria y leaks

---

**Desarrollado con ❤️ en C y OpenGL**
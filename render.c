#include "render.h"

// --- FUNCION DE PRUEBA 2D ---
void render_2d_test(HDC hdc, int width, int height, int mouseX, int mouseY) { // entonces width y heigth son los parametros de tamaño de la pantalla
    //Colores basicos
    Color red = {255, 0, 0};
    Color green = {0, 255, 0};
    Color blue = {0, 0, 255};
    Color white = {255, 255, 255};
    Color black = {0, 0, 0};

    COLORREF rgbWhite = 0x00FFFFFF;

    //Dibujar ejers coordenados
    draw_line(hdc, 0, mouseY, width, mouseY, rgbWhite); // Eje x
    draw_line(hdc, mouseX, 0, mouseX, height, rgbWhite); // Eje y
}

void render_3d_test(HDC hdc, int width, int height, int mouseX, int mouseY) {
    // fondo
    FillRect(hdc, &(RECT){0,0,width,height}, (HBRUSH)GetStockObject(BLACK_BRUSH));

    float fov = 70.0f;

    // algunos cubos (sin rotación)
    Cube3D cubes[] = {
        create_cube(vect3_create( 0, 0, 0), 1.2f, (Color){  0,220,180}),
        create_cube(vect3_create( 2, 0, 0), 1.0f, (Color){220, 80,  0}),
        create_cube(vect3_create(-2, 1, 0), 0.8f, (Color){ 80,120,255}),
        create_cube(vect3_create( 0,-1, 1), 0.6f, (Color){255,255,255}),
    };

    for (int i=0; i<4; i++){
        draw_cube_wire(hdc, width, height, fov, cubes[i], 0.0f, 0.0f, 0.0f); // Sin rotación
    }

    // tu cruz del mouse encima:
    draw_line(hdc, 0, mouseY, width, mouseY, RGB(255,255,255));
    draw_line(hdc, mouseX, 0, mouseX, height, RGB(255,255,255));
}

void draw_cube_wireframe(HDC hdc, Cube3D cube, Camera camera, int screenWidth, int screenHeight) {
    Vect3 corners[8] = {
        vect3_create(cube.position.x - cube.size/2, cube.position.y - cube.size/2, cube.position.z - cube.size/2),
        vect3_create(cube.position.x + cube.size/2, cube.position.y - cube.size/2, cube.position.z - cube.size/2),
        vect3_create(cube.position.x + cube.size/2, cube.position.y + cube.size/2, cube.position.z - cube.size/2),
        vect3_create(cube.position.x - cube.size/2, cube.position.y + cube.size/2, cube.position.z - cube.size/2),
        vect3_create(cube.position.x - cube.size/2, cube.position.y - cube.size/2, cube.position.z + cube.size/2),
        vect3_create(cube.position.x + cube.size/2, cube.position.y - cube.size/2, cube.position.z + cube.size/2),
        vect3_create(cube.position.x + cube.size/2, cube.position.y + cube.size/2, cube.position.z + cube.size/2),
        vect3_create(cube.position.x - cube.size/2, cube.position.y + cube.size/2, cube.position.z + cube.size/2)
    };

    // Transformar puntos segun la camara
    Point2D projected[8];
    for (int i = 0; i < 8; i++) {
        Vect3 transformed = camera_transform_point(camera, corners[i]);
        projected[i] = project_3d_to_2d(transformed, screenWidth, screenHeight, camera.fov);
    };

    // Dibujar las 12 aristas del cubo
    int edges[12][2] = {
        {0,1}, {1,2}, {2,3}, {3,0},  // Cara frontal
        {4,5}, {5,6}, {6,7}, {7,4},  // Cara trasera
        {0,4}, {1,5}, {2,6}, {3,7}   // Aristas que conectan
    };

    for (int i = 0; i < 12; i++) {
        Point2D start = projected[edges[i][0]];
        Point2D end = projected[edges[i][1]];
        
        // Solo dibujar si ambos puntos son válidos
        if (start.x != -1 && end.x != -1) {
            draw_line(hdc, start.x, start.y, end.x, end.y, 
                     RGB(cube.color.r, cube.color.g, cube.color.b));
        }
    }
}

void render_3d_wireframe(HDC hdc, int width, int height, int mouseX, int mouseY) {
    // Colores
    Color red = {255, 0, 0};
    Color green = {0, 255, 0};
    Color blue = {0, 0, 255};
    Color white = {255, 255, 255};
    Color yellow = {255, 255, 0};
    
    // Crear cámara
    Camera camera = create_camera(
        vect3_create(0, 0, -5),  // Posición de la cámara
        vect3_create(0, 0, 0),   // Mira hacia el origen
        3.14159f / 4.0f          // FOV de 45 grados
    );
    
    // Crear cubos 3D
    Cube3D cubes[] = {
        create_cube(vect3_create(0, 0, 0), 2.0f, red),
        create_cube(vect3_create(4, 0, 0), 1.5f, green),
        create_cube(vect3_create(-4, 0, 0), 1.5f, blue),
        create_cube(vect3_create(0, 4, 0), 1.0f, white),
        create_cube(vect3_create(0, -4, 0), 1.0f, yellow)
    };
    
    // Renderizar cada cubo
    for (int i = 0; i < 5; i++) {
        draw_cube_wireframe(hdc, cubes[i], camera, width, height);
    }
}

void draw_cube_wire(HDC hdc, int W, int H, float fov_deg, Cube3D cube, float yaw, float pitch, float roll) {
    // 8 esquinas locales
    float h = cube.size * 0.5f;
    Vect3 local[8] = {
        {-h,-h,-h},{+h,-h,-h},{+h,+h,-h},{-h,+h,-h},
        {-h,-h,+h},{+h,-h,+h},{+h,+h,+h},{-h,+h,+h}
    };

    // transformar: rotaciones y luego trasladar
    Vect3 world[8];
    for (int i=0; i<8; i++){
        Vect3 p = local[i];
        p = v3_rotY(p, yaw);
        p = v3_rotX(p, pitch);
        p = v3_rotZ(p, roll);
        p = vect3_add(p, cube.position);
        p.z += 3.0f; // empuja hacia delante
        world[i] = p;
    }

    // proyectar y dibujar aristas
    COLORREF col = RGB(cube.color.r, cube.color.g, cube.color.b);
    int sx[8], sy[8], vis[8];
    for (int i=0; i<8; i++) vis[i] = project_persp(world[i], W, H, fov_deg, &sx[i], &sy[i]);

    // Definir aristas del cubo
    int edges[12][2] = {
        {0,1},{1,2},{2,3},{3,0},
        {4,5},{5,6},{6,7},{7,4},
        {0,4},{1,5},{2,6},{3,7}
    };

    for (int e=0; e<12; e++){
        int a = edges[e][0], b = edges[e][1];
        if (!vis[a] || !vis[b]) continue;
        draw_line(hdc, sx[a], sy[a], sx[b], sy[b], col);
    }
}

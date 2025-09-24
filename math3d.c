#include "math3d.h"

// --- FUNCIONES MATEMATICAS 3D ---

// CREAR UN VECTOR 3D
Vect3 vect3_create(float x, float y, float z) {
    Vect3 v = {x, y, z};
    return v;
}

Vect3 vect3_add(Vect3 a, Vect3 b) {
    return vect3_create(a.x + b.x, a.y + b.y, a.z + b.z);
}

Vect3 vect3_subtract(Vect3 a, Vect3 b) {
    return vect3_create(a.x - b.x, a.y - b.y, a.z - b.z);
}

// Producto cruz - es importante para la camara
Vect3 vect3_cross(Vect3 a, Vect3 b) {
    return vect3_create(
        a.y * b.z - a.z * b.y,
        a.z * b.x - a.x * b.z,
        a.x * b.y - a.y * b.x
    );
}

Vect3 vect3_scale(Vect3 v, float scalar) {
    return vect3_create(v.x * scalar, v.y * scalar, v.z * scalar);
}

// Producto punto - importante para el raycasting
float vect3_dot(Vect3 a, Vect3 b) {
    return a.x * b.x + a.y * b.y + a.z * b.z;
}

// Magnitud del vector
float vect3_magnitude(Vect3 v) {
    return sqrt(v.x * v.x + v.y * v.y + v.z * v.z);
}

// Normalizar el vectror 3D se convierte a unitario
Vect3 vect3_normalize(Vect3 v) {
    float mag = vect3_magnitude(v);
    if (mag == 0) return vect3_create(0, 0, 0);
    return vect3_create(v.x / mag, v.y / mag, v.z / mag);
}

// CREAR UN CUBO
Cube3D create_cube(Vect3 position, float size, Color color) {
    Cube3D cube = {position, size, color};
    return cube;
}

// CREAR CAMARA
Camera create_camera(Vect3 position, Vect3 target, float fov) {
    Camera cam = {
        position,
        target,
        vect3_create(0, 1, 0),
        fov,
        0.1f,
        1000.0f
    };
    return cam;
}

// CALCULAR MATRIZ DE VISTA
Vect3 camera_transform_point(Camera cam, Vect3 point) {
    Vect3 forward = vect3_normalize(vect3_subtract(cam.target, cam.position));

    Vect3 right = vect3_normalize(vect3_cross(forward, cam.up));

    Vect3 up = vect3_cross(right, forward);

    Vect3 translated = vect3_subtract(point, cam.position);

    Vect3 rotated = vect3_create(
        vect3_dot(translated, right),
        vect3_dot(translated, up),
        vect3_dot(translated, forward)
    );

    return rotated;
}


// Proyectar punto 3D a coordenadas de pantalla 2D
Point2D project_3d_to_2d(Vect3 point3d, int screenWidth, int screenHeight, float fov) {
    //Verifica si el pinto está detras de la camara
    if (point3d.z <= 0) {
        return (Point2D) {-1, -1};
    }
    
    // FOV (Field of View) - ángulo de visión
    float scale = screenWidth / (2.0f * tan(fov / 2.0f));
    
    // Proyección perspectiva simple
    float x2d = (point3d.x * scale) / point3d.z + screenWidth / 2;
    float y2d = (point3d.y * scale) / point3d.z + screenHeight / 2;
    
    // Verificar si el punto está dentro de la pantalla
    if (x2d < 0 || x2d >= screenWidth || y2d < 0 || y2d >= screenHeight) {
        return (Point2D){-1, -1}; // Punto fuera de pantalla
    }

    Point2D point2d = {(int)x2d, (int)y2d};
    return point2d;
}

// Funciones de rotación
Vect3 v3_rotX(Vect3 p, float a) {
    float c = cosf(a), s = sinf(a);
    return vect3_create(p.x, p.y*c - p.z*s, p.y*s + p.z*c);
}

Vect3 v3_rotY(Vect3 p, float a) {
    float c = cosf(a), s = sinf(a);
    return vect3_create(p.x*c + p.z*s, p.y, -p.x*s + p.z*c);
}

Vect3 v3_rotZ(Vect3 p, float a) {
    float c = cosf(a), s = sinf(a);
    return vect3_create(p.x*c - p.y*s, p.x*s + p.y*c, p.z);
}

// Proyección perspectiva mejorada
int project_persp(Vect3 pc, int W, int H, float fov_deg, int* sx, int* sy) {
    const float nearZ = 0.1f;
    if (pc.z <= nearZ) return 0;
    float f = 1.0f / tanf((fov_deg * 0.5f) * (float)M_PI / 180.0f);
    float aspect = (float)W / (float)H;
    float ndcX = (pc.x * (f / aspect)) / pc.z;
    float ndcY = (pc.y * f) / pc.z;
    *sx = (int)((ndcX * 0.5f + 0.5f) * (float)W);
    *sy = (int)((-ndcY * 0.5f + 0.5f) * (float)H);
    return 1;
}

#include "math2d.h"

// --- FUNCIONES MATEMÁTICAS 2D ---

// CREAR UN VECTOR 2D
Vect2 vect2_create(float x, float y) {
    Vect2 v = {x, y};
    return v;
}

// --- SUMAR DOS VECTORES ---
Vect2 vect2_add(Vect2 a, Vect2 b){
    return vect2_create(a.x + b.x, a.y + b.y);
};

// -- MULTIPLICAR VECTOR POR ESCALAR (el escalar es el tamaño o la magnitud del vector sin cambiar su direccion, como hacer el rayo del tracing) ---
Vect2 vect2_scale(Vect2 v, float scalar) {
    return vect2_create(v.x * scalar, v.y * scalar);
}

// VERIFICAR LA DISTANCIA ENTRE VECTOR A Y B O ENTRE 2 PUNTOS EN OTRAS PALABRAS
float vec2_distance(Vect2 a, Vect2 b) {
    float dx = a.x - b.x;
    float dy = a.y - b.y;
    return sqrt((dx * dx) + (dy * dy));
}

// --- FUNCINOES DE RENDERIZADO 2D ---
// Dibujar un punto
void draw_point(HDC hdc, int x, int y, Color color) {
    SetPixel(hdc, x, y, RGB(color.r, color.g, color.b));
}

//Dubujar una linea (algoritmo de Bresenham - Low Level!)
void draw_line(HDC hdc, int x1, int y1, int x2, int y2, COLORREF color) {
    HPEN hPen = CreatePen(PS_SOLID, 1, color);
    HPEN hOldPen = SelectObject(hdc, hPen);
    
    MoveToEx(hdc, x1, y1, NULL);
    LineTo(hdc, x2, y2);
    
    SelectObject(hdc, hOldPen);
    DeleteObject(hPen);
}

// Dibujar un circulo (algoritmo de breshenham para circulos ahora)
void draw_circle(HDC hdc, int centerX, int centerY, int radius, Color color) {
    int x = 0;
    int y = radius;
    int d = 3 - 2 * radius;

    while (x <= y) {
        draw_point(hdc, centerX + x, centerY + y, color);
        draw_point(hdc, centerX - x, centerY + y, color);
        draw_point(hdc, centerX + x, centerY - y, color);
        draw_point(hdc, centerX - x, centerY - y, color);
        draw_point(hdc, centerX + y, centerY + x, color);
        draw_point(hdc, centerX - y, centerY + x, color);
        draw_point(hdc, centerX + y, centerY - x, color);
        draw_point(hdc, centerX - y, centerY - x, color);

        if (d <= 0) {
            d = d + 4 * x + 6;
        } else {
            d = d + 4 * (x - y) + 10;
            y--;
        }
        x++;
    }
}

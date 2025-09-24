#include <windows.h>
#include <math.h>

HWND mainW;
int mouseX, mouseY;

typedef struct {
    LPWSTR lpWindowName;
    DWORD dwExStyle;
    DWORD dwStyle;
    int x, y, width, height;
    HWND parent;
} WindowParams;


// COLOR RGB (para renderizado)
typedef struct {
    unsigned char r, g, b;
} Color;


// --- ESTRUCTURA 3D ---

// VECTOR 3D
typedef struct {
    float x, y, z;
} Vect3;

// CUBO 3D
typedef struct {
    Vect3 position;
    float size;
    Color color;
} Cube3D;

// ESFERA 3D
typedef struct {
    Vect3 position;
    float size;
    Color color;
} Sphere3D;


// --- ESTRUCTURAS 2D ---

// VECTOR 2D
typedef struct {
    float x, y;
} Vect2;

// PUNTO EN PANTALLA
typedef struct {
    int x, y;
} Point2D;



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



// --- FUNCIONES MATEMATICAS 3D ---

// CREAR UN VECTOR 3D
Vect3 vect3_create(float x, float y, float z) {
    Vect3 v = {x, y, z};
    return v;
}

Vect3 vect3_add(Vect3 a, Vect3 b) {
    return vect3_create(a.x + b.x, a.y + b.y, a.z + b.z);
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



// Proyectar punto 3D a coordenadas de pantalla 2D
Point2D project_3d_to_2d(Vect3 point3d, int screenWidth, int screenHeight, float fov) {
    // FOV (Field of View) - ángulo de visión
    float scale = screenWidth / (2.0f * tan(fov / 2.0f));
    
    // Proyección perspectiva simple
    float x2d = (point3d.x * scale) / point3d.z + screenWidth / 2;
    float y2d = (point3d.y * scale) / point3d.z + screenHeight / 2;
    
    Point2D point2d = {(int)x2d, (int)y2d};
    return point2d;
}




// --- FUNCINOES DE RENDERIZADO 2D ---


void render_3d_test(HDC hdc, int width, int height, int mouseX, int mouseY) {
    //Colores basicos
    Color red = {255, 0, 0};
    Color green = {0, 255, 0};
    Color blue = {0, 0, 255};
    Color white = {255, 255, 255};
    Color black = {0, 0, 0};

    // FOV o field of view en radianes
    float fov = 3.14159f / 4.0f; //45grados

    Cube3D cubes[] =  {
        create_cube(vect3_create(0, 0, 5), 1.0f, red),
        create_cube(vect3_create(2, 0, 5), 1.0f, green),
        create_cube(vect3_create(-2, 0, 5), 1.0f, blue),
        create_cube(vect3_create(0, 2, 5), 1.0f, white)
    };

    for (int i = 0; i < 4; i++) {
        // Proyectar las esquinas del cubo a 2D
        Vect3 corners[] = {
            vect3_create(cubes[i].position.x - cubes[i].size/2, cubes[i].position.y - cubes[i].size/2, cubes[i].position.z),
            vect3_create(cubes[i].position.x + cubes[i].size/2, cubes[i].position.y - cubes[i].size/2, cubes[i].position.z),
            vect3_create(cubes[i].position.x + cubes[i].size/2, cubes[i].position.y + cubes[i].size/2, cubes[i].position.z),
            vect3_create(cubes[i].position.x - cubes[i].size/2, cubes[i].position.y + cubes[i].size/2, cubes[i].position.z)
        };

        // Dibujar las líneas del cubo
        for (int j = 0; j < 4; j++) {
            Point2D start = project_3d_to_2d(corners[j], width, height, fov);
            Point2D end = project_3d_to_2d(corners[(j + 1) % 4], width, height, fov);
            
            draw_line(hdc, start.x, start.y, end.x, end.y, RGB(cubes[i].color.r, cubes[i].color.g, cubes[i].color.b));
        }
    }

}

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam ) {
    switch (uMsg) {
        case WM_DESTROY:
            PostQuitMessage(0);
            return 0;

        case WM_MOUSEMOVE:
            {
                mouseX = LOWORD(lParam);
                mouseY = HIWORD(lParam);

                // se deja eln TRUE el borrar el fondo para oblifgar al PAINT a hacer un repintado y actualice
                static DWORD lastUpdate = 0;
                DWORD currentTime = GetTickCount();
                if (currentTime - lastUpdate > 16) {
                    InvalidateRect(hwnd, NULL, TRUE);
                    lastUpdate = currentTime;
                }
                     
            }
            return 0;

        case WM_PAINT:
            {
                PAINTSTRUCT ps;
                HDC hdc = BeginPaint(hwnd, &ps);

                //Obtener dimensiones de la ventana para poder sacar nuestro width y height cuando hagamos render_2d_test tambien
                RECT rect;
                GetClientRect(hwnd, &rect);

                int ScreenWidth = rect.right - rect.left;
                int ScreenHeight = rect.bottom - rect.top;


                //Limpiar pantalla con fondo negro
                FillRect(hdc, &ps.rcPaint, (HBRUSH)GetStockObject(BLACK_BRUSH));

                // AQUI LLAMAMOS A LA FUNCION RENDER_2D_TEST
                render_3d_test(hdc, ScreenWidth, ScreenHeight, mouseX, mouseY);
                render_2d_test(hdc, ScreenWidth, ScreenHeight, mouseX, mouseY);

                EndPaint(hwnd, &ps);
            }
            return 0;
        case WM_CREATE:
            SetTimer(hwnd, 1, 16, NULL); // Se setea a 60fps
            return 0;

        case WM_TIMER:
            InvalidateRect(hwnd, NULL, FALSE);
            return 0;

        case WM_CLOSE:
            DestroyWindow(hwnd);
            return 0;

        default:
            return DefWindowProc(hwnd, uMsg, wParam, lParam);
    }

    return 0;
}





ATOM RegisterMyClass(WindowParams* params) {

    WNDCLASSEX wc = {0};
    wc.cbSize = sizeof(wc);
    wc.lpfnWndProc = WindowProc;
    wc.lpszClassName = "MiVentana";
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wc.hCursor = LoadCursor(NULL, IDC_CROSS);

    RegisterClassEx(&wc);

    mainW = CreateWindowExW(
        params->dwExStyle,
        L"MiVentana",
        params->lpWindowName,
        params->dwStyle,
        params->x,params->y,
        params->width,params->height,
        params->parent,
        NULL,
        NULL,
        NULL 
    );

    return (mainW ? TRUE : FALSE);

}


int main() {

    mouseX = 400;
    mouseY = 300;

    WindowParams params = {0};
    params.lpWindowName = L"Mi Ventana";
    params.dwExStyle = WS_EX_WINDOWEDGE;
    params.dwStyle = WS_OVERLAPPEDWINDOW;
    params.x = CW_USEDEFAULT;
    params.y = CW_USEDEFAULT;
    params.width = CW_USEDEFAULT;
    params.height = CW_USEDEFAULT;
    params.parent = NULL;

    RegisterMyClass(&params);

    ShowWindow(mainW, SW_SHOWNORMAL | SW_RESTORE);
    UpdateWindow(mainW);

    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return(0);
}
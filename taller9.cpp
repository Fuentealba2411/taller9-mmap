// Taller 9 - Memoria vs Disco
// Compara el rendimiento de procesar un archivo desde disco
// versus mapeado en memoria usando mmap

#include <iostream>
#include <fstream>
#include <chrono>
#include <cstdlib>
#include <cstring>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

// Nombre del archivo de datos
#define ARCHIVO_DATOS "datos.bin"

// Cantidad de enteros a generar (10 millones)
#define CANTIDAD 10000000

// Genera el archivo binario con numeros aleatorios
void generar_archivo() {
    std::cout << "Generando archivo con " << CANTIDAD << " enteros..." << std::endl;

    std::ofstream archivo(ARCHIVO_DATOS, std::ios::binary);
    if (!archivo.is_open()) {
        std::cerr << "Error al crear el archivo de datos" << std::endl;
        exit(1);
    }

    // Escribe enteros aleatorios en el archivo
    for (int i = 0; i < CANTIDAD; i++) {
        int num = rand() % 1000;
        archivo.write(reinterpret_cast<char*>(&num), sizeof(int));
    }

    archivo.close();
    std::cout << "Archivo generado correctamente." << std::endl;
}

// Lee el archivo desde disco y calcula la suma
long long leer_desde_disco() {
    int fd = open(ARCHIVO_DATOS, O_RDONLY);
    if (fd == -1) {
        std::cerr << "Error al abrir el archivo (disco)" << std::endl;
        exit(1);
    }

    // Obtiene el tamanio del archivo
    struct stat info;
    fstat(fd, &info);
    size_t tam = info.st_size;
    int cantidad = tam / sizeof(int);

    // Reserva buffer en heap y lee todo el archivo
    int* buffer = new int[cantidad];
    read(fd, buffer, tam);
    close(fd);

    // Calcula la suma recorriendo el buffer
    long long suma = 0;
    for (int i = 0; i < cantidad; i++) {
        suma += buffer[i];
    }

    delete[] buffer;
    return suma;
}

// Mapea el archivo en memoria y calcula la suma
long long leer_desde_memoria() {
    int fd = open(ARCHIVO_DATOS, O_RDONLY);
    if (fd == -1) {
        std::cerr << "Error al abrir el archivo (mmap)" << std::endl;
        exit(1);
    }

    // Obtiene el tamanio del archivo
    struct stat info;
    fstat(fd, &info);
    size_t tam = info.st_size;
    int cantidad = tam / sizeof(int);

    // Mapea el archivo directamente en memoria
    void* mapeo = mmap(NULL, tam, PROT_READ, MAP_PRIVATE, fd, 0);
    if (mapeo == MAP_FAILED) {
        std::cerr << "Error al mapear el archivo en memoria" << std::endl;
        close(fd);
        exit(1);
    }
    close(fd);

    // Interpreta el puntero como arreglo de enteros
    int* datos = static_cast<int*>(mapeo);

    // Calcula la suma recorriendo el mapeo
    long long suma = 0;
    for (int i = 0; i < cantidad; i++) {
        suma += datos[i];
    }

    // Libera el mapeo de memoria
    munmap(mapeo, tam);
    return suma;
}

int main() {
    // Genera el archivo si no existe
    generar_archivo();

    std::cout << "\n--- Comparacion: Disco vs Memoria ---\n" << std::endl;

    // ---- Prueba desde disco ----
    auto inicio_disco = std::chrono::high_resolution_clock::now();
    long long suma_disco = leer_desde_disco();
    auto fin_disco = std::chrono::high_resolution_clock::now();

    double tiempo_disco = std::chrono::duration<double, std::milli>(fin_disco - inicio_disco).count();

    std::cout << "Suma desde disco:    " << suma_disco << std::endl;
    std::cout << "Tiempo desde disco:  " << tiempo_disco << " ms" << std::endl;

    std::cout << std::endl;

    // ---- Prueba desde memoria (mmap) ----
    auto inicio_mem = std::chrono::high_resolution_clock::now();
    long long suma_mem = leer_desde_memoria();
    auto fin_mem = std::chrono::high_resolution_clock::now();

    double tiempo_mem = std::chrono::duration<double, std::milli>(fin_mem - inicio_mem).count();

    std::cout << "Suma desde memoria:  " << suma_mem << std::endl;
    std::cout << "Tiempo desde memoria:" << tiempo_mem << " ms" << std::endl;

    std::cout << std::endl;

    // ---- Resultado ----
    double mejora = tiempo_disco / tiempo_mem;
    std::cout << "Mejora con mmap: " << mejora << "x mas rapido" << std::endl;

    return 0;
}

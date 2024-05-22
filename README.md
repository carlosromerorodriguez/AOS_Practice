# FileSystem Reader

Este proyecto permite leer y mostrar la información de los sistemas de archivos FAT16 y EXT2. Esta específicamente diseñado para analizar imágenes de dichos sistemas de archivos contenidas en la carpeta `tests/`.

## Características

- Soporte para sistemas de archivos FAT16 y EXT2.
- Muestra información detallada de los metadatos del sistema de archivos.

## Estructura del Proyecto

El código fuente se organiza en la carpeta `src/`, incluyendo los siguientes componentes principales:

- `main.c`: Punto de entrada del programa.
- `common/info.c`: Funciones comunes para mostrar información.
- `ext2/ext2_reader.c`: Funciones para procesar el sistema de archivos EXT2.
- `fat16/fat16_reader.c`: Funciones para procesar el sistema de archivos FAT16.

## Compilación

El proyecto incluye un Makefile para facilitar la compilación. Para compilar el proyecto, ejecuta el siguiente comando en la terminal dentro de la carpeta `src/`:

```bash
make
```

## Ejecución
Una vez compilado el proyecto, se debe ejecutar el programa con el comando deseado desde la carpeta raíz del proyecto:
- `--info`: Para mostrar la información general del fichero.
- `--tree`: Para mostrar los directorios y subdirectorios del fichero.
- `--cat`: Para mostrar el contenido de un fichero concreto de dentro de dicho fichero específicado.

Ejemplo con el fichero libfat:

```bash
./fsutils --info tests/libfat
```
```bash
./fsutils --tree tests/libfat
```
```bash
./fsutils --cat tests/libfat conio.h
```

## Nota
Los archivos .o generados se eliminan automáticamente al ejecutar el comando make.
Si a pesar de todo, se quieren eliminar, se debe ejecutar el siguiente comando dentro de la carpeta `src/`:

```bash
make clean
```
# Makefile para Doncekong - VERSIÓN MEJORADA
PROJECT_NAME = game
RAYLIB_PATH = C:/raylib/raylib

# Compilador
CC = gcc

# Flags
CFLAGS = -Wall -std=c99 -I"$(RAYLIB_PATH)/src" -DPLATFORM_DESKTOP -O2
LDFLAGS = -L"$(RAYLIB_PATH)/src"
LDLIBS = -lraylib -lopengl32 -lgdi32 -lwinmm

# Archivos fuente
SRC = main.c game.c graficos.c controles.c conexion.c  mapa.c
OBJ = $(SRC:.c=.o)

# Target principal
all: $(PROJECT_NAME)

$(PROJECT_NAME): $(OBJ)
	$(CC) -o $@ $(OBJ) $(LDFLAGS) $(LDLIBS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# Limpieza (Windows)
clean:
	@if exist $(PROJECT_NAME).exe del $(PROJECT_NAME).exe
	@if exist *.o del *.o
	@echo "Limpieza completada"

# Recompilar desde cero
rebuild: clean all

.PHONY: all clean rebuild
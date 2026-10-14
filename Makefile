CC = gcc
CFLAGS = -Wall -Wextra -I. -Isrc
LDFLAGS = -lSDL2

# Diretórios
SRC_DIR = src
OBJ_DIR = src

# Fontes e Objetos
SRCS = $(wildcard $(SRC_DIR)/*.c)
OBJS = $(SRCS:.c=.o)
TARGET = space_invaders

# Regra principal
all: $(TARGET)

# Compila o executável final
$(TARGET): $(OBJS)
	$(CC) $(OBJS) -o $(TARGET) $(LDFLAGS)

# Compila os arquivos .c em .o
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# Limpa os arquivos de compilação
clean:
	rm -f $(SRC_DIR)/*.o $(TARGET)

.PHONY: all clean
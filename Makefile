CC = gcc
CFLAGS = -Wall -Wextra -I. -Isrc

ifeq ($(OS),Windows_NT)
    LDFLAGS = -lmingw32 -lSDL2main -lSDL2 -lm
    TARGET = space_invaders.exe
else
    LDFLAGS = -lSDL2 -lm
    TARGET = space_invaders
endif

# Diretórios
SRC_DIR = src

# Fontes e Objetos
SRCS = $(wildcard $(SRC_DIR)/*.c)
OBJS = $(SRCS:.c=.o)

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

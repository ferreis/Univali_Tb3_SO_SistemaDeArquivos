# Nome do compilador que vamos usar
CC = gcc

# Flags do compilador:
# -g -> adiciona informações de debug (para usar com gdb)
# -Wall -> ativa todos os avisos (warning all), muito útil!
# -Wextra -> ativa mais alguns avisos
CFLAGS = -g -Wall -Wextra

# Nome do programa executável que queremos criar
TARGET = fs

# Lista de todos os arquivos objeto (.o) que nosso programa precisa
# O make vai criar um .o para cada .c
SOURCES = main_fs.c filesystem.c
OBJECTS = $(SOURCES:.c=.o)

# A regra principal: o que fazer quando você digita "make"
# Ela depende de todos os arquivos objeto.
all: $(TARGET)

# Regra para criar o programa executável (o TARGET)
# Esta é a linha que corrige o seu erro!
# Ela junta main_fs.o E filesystem.o para criar o "fs"
$(TARGET): $(OBJECTS)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJECTS)

# Regra genérica para criar arquivos .o a partir de arquivos .c
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# Regra para limpar os arquivos gerados (executável e objetos)
# Muito útil! Se algo der errado, você digita "make clean" e começa de novo.
clean:
	rm -f $(TARGET) $(OBJECTS)

.PHONY: all clean
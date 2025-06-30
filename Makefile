# Nome do compilador que vamos usar
CC      = gcc

# Flags do compilador:
# -g      -> adiciona informação de debug (usada pelo gdb)
# -Wall   -> ativa todos os avisos (warnings)
# -Wextra -> ativa avisos adicionais
CFLAGS  = -g -Wall -Wextra

# Nome do executável resultante
TARGET  = fs

# Lista de arquivos-fonte (.c)
SOURCES = main_fs.c filesystem.c

# Converte a lista de .c em lista de .o
OBJECTS = $(SOURCES:.c=.o)

# Regra padrão: quando você digita `make`, roda a meta 'all'
all: $(TARGET)

# Como gerar o executável a partir dos objetos
# Junta todos os .o em um único binário chamado $(TARGET)
$(TARGET): $(OBJECTS)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJECTS)

# Regra genérica: como compilar cada .c em .o
# $< é o nome do arquivo-fonte, $@ é o alvo (.o)
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# Limpar tudo: remove executável e objetos
clean:
	rm -f $(TARGET) $(OBJECTS)

# As metas que não são arquivos
.PHONY: all clean

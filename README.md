# Sistema de Arquivos Simples com Árvore B

Este projeto é uma implementação de um sistema de arquivos simplificado, desenvolvido como parte da avaliação da disciplina de Sistemas Operacionais da UNIVALI. O objetivo é simular as operações básicas de um sistema de arquivos, como criar arquivos e diretórios, navegar e listar conteúdo, usando uma **Árvore B** como estrutura de dados principal.

## Sobre o Projeto

O sistema de arquivos virtual usa uma Árvore B para organizar os arquivos e diretórios em cada pasta. Cada diretório tem sua própria Árvore B, que armazena os nós (arquivos ou subdiretórios) que estão dentro dele.

A escolha da Árvore B é por causa da sua eficiência para buscar, inserir e remover dados, o que é ótimo para sistemas de arquivos e bancos de dados.

### Funcionalidades Implementadas

O sistema suporta os seguintes comandos, simulando uma interface de linha de comando:

  * **`ls`**: Lista o conteúdo do diretório atual.
  * **`ls -l`**: Lista o conteúdo com mais detalhes, mostrando a data da última modificação.
  * **`cd <diretório>`**: Navega para outro diretório.
      * Suporta `..` para voltar para o diretório pai.
      * Suporta `.` para o diretório atual.
  * **`mkdir <diretório>`**: Cria uma nova pasta.
  * **`rmdir <diretório>`**: Remove uma pasta, mas só se ela estiver vazia.
  * **`touch <arquivo.txt>`**: Cria um novo arquivo de texto vazio.
  * **`touch <arquivo.txt> "conteúdo"`**: Cria um arquivo de texto com conteúdo.
  * **`rm <arquivo.txt>`**: Remove um arquivo de texto.
  * **`stat <item>`**: Mostra os metadados de um arquivo ou diretório (data de criação, modificação e último acesso).
  * **`save <imagem.img>`**: Salva a estrutura do sistema de arquivos em um arquivo de texto para visualizar depois.
  * **`exit`**: Sai do programa.
  * **`help`**: Mostra a lista de comandos disponíveis.

### Estrutura do Código

O projeto é escrito em C, com os seguintes arquivos principais:

  * `filesystem.h`: Contém as definições das estruturas de dados (`File`, `Directory`, `TreeNode`, `BTree`, `BTreeNode`) e os protótipos de todas as funções. É o "esqueleto" do sistema.
  * `filesystem.c`: Contém a implementação de todas as funções declaradas em `filesystem.h`, incluindo as operações da Árvore B e as funções de manipulação de arquivos e diretórios.
  * `main_fs.c`: O programa principal, onde fica o loop de comandos do terminal (`ls`, `cd`, `mkdir`, etc.) e a lógica para interpretar o que o usuário digita.

A Árvore B tem uma ordem definida como `BTREE_ORDER 3`.

## Como Compilar e Rodar o Projeto

### Pré-requisitos

Você precisa ter um compilador C instalado, como o GCC (GNU Compiler Collection).

### Compilação

Para compilar o projeto, use o seguinte comando no terminal. Ele vai juntar os arquivos `.c` e criar um executável chamado `my_fs`:

```bash
gcc -o fs main_fs.c filesystem.c
```

**Observação:** O arquivo `.gitignore` já está configurado para ignorar o executável `my_fs`, os arquivos objeto (`*.o`) e as imagens do sistema de arquivos (`*.img`, `fs.img`), o que é ótimo para manter o repositório organizado.

### Execução

Depois de compilar, você pode rodar o sistema de arquivos com este comando:

```bash
./fs
```

O terminal vai mostrar o `prompt` (ex: `fs:/$`), e você pode começar a usar os comandos.

## Exemplo de Uso

1.  **Crie alguns diretórios e arquivos:**

    ```
    fs:/$ mkdir documentos
    fs:/$ mkdir fotos
    fs:/$ touch lista.txt "lista de compras"
    fs:/$ ls
    ```

    (Deve mostrar `documentos/ fotos/ lista.txt`)

2.  **Entre em um diretório e crie mais coisas:**

    ```
    fs:/$ cd documentos
    fs:/documentos$ mkdir univali
    fs:/documentos$ touch trabalho.txt "trabalho de SO"
    fs:/documentos$ ls -l
    ```

    (Você verá a lista detalhada com os metadados de tempo)

3.  **Volte e remova algo:**

    ```
    fs:/documentos$ cd ..
    fs:/$ ls
    ```

    (Deve mostrar `documentos/ fotos/ lista.txt`)

    ```
    fs:/$ rmdir fotos
    fs:/$ ls
    ```

    (A pasta `fotos` será removida se estiver vazia)

4.  **Salve a estrutura do sistema:**

    ```
    fs:/$ save fs.img
    ```

    (Isso cria um arquivo `fs.img` com a estrutura de pastas e arquivos)

5.  **Veja os metadados de um item:**

    ```
    fs:/$ stat documentos
    ```

    (Vai mostrar as datas de criação, modificação e acesso)

6.  **Saia do programa:**

    ```
    fs:/$ exit
    ```

## Considerações Finais

O sistema de arquivos virtual foi projetado para demonstrar como uma estrutura de dados, como a Árvore B, pode ser usada para organizar dados de forma hierárquica e eficiente. É um projeto prático que ajuda a entender os conceitos de sistemas operacionais na prática.

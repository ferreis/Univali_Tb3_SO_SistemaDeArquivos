#ifndef FILESYSTEM_H
#define FILESYSTEM_H

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <time.h>

#define BTREE_ORDER 3 // Ordem da Árvore B

// Enum para os tipos de nós
typedef enum { FILE_TYPE, DIRECTORY_TYPE } NodeType;

// Estrutura para um arquivo
typedef struct File {
    char* name;
    char* content;
    size_t size;
} File;

// Declaração antecipada da estrutura Directory
struct Directory;

// Nó que pode ser arquivo ou diretório
typedef struct TreeNode {
    char* name;
    NodeType type;
    union {
        File* file;
        struct Directory* directory;
    } data;
    time_t creation_time;
    time_t modification_time;
    time_t last_access_time;
} TreeNode;

// Nó da Árvore B
typedef struct BTreeNode {
    bool leaf;
    int num_keys;
    TreeNode* keys[2 * BTREE_ORDER - 1];
    struct BTreeNode* children[2 * BTREE_ORDER];
} BTreeNode;

// Estrutura da Árvore B
typedef struct BTree {
    BTreeNode* root;
} BTree;

// Estrutura de um diretório, que contém uma Árvore B
typedef struct Directory {
    char* name; // Nome do diretório
    struct Directory* parent; // Ponteiro para o diretório pai
    BTree* tree; // Árvore B com os filhos
} Directory;

// --- Funções da Árvore B ---
BTree* btree_create();
void btree_destroy(BTree* tree);
void btree_insert(BTree* tree, TreeNode* node);
void btree_delete(BTree* tree, const char* name);
TreeNode* btree_search(BTree* tree, const char* name);
void btree_traverse(BTreeNode* node, bool long_format); 

// --- Funções de Arquivos e Diretórios ---
TreeNode* delete_txt_file(const char* name, const char* content);
TreeNode* create_directory(const char* name, Directory* parent);
void delete_txt_file(TreeNode* node); 
void delete_directory_recursive(Directory* dir);
void free_tree_node(TreeNode* node); 

// --- Funções de Navegação e Comandos ---
Directory* get_root_directory();
void list_directory_contents(Directory* dir, bool long_format);
void change_directory(Directory** current_dir, const char* path);
char* get_current_path(Directory* dir); 

// --- Funções de Manipulação de Imagem do Sistema de Arquivos ---
void update_parent_modification_time(Directory* dir);
void show_metadata(Directory* dir, const char* name);


#endif // FILESYSTEM_H
#include "filesystem.h"

/* ============================================================================= */
/* --- PROTÓTIPOS DAS FUNÇÕES AUXILIARES (INTERNAS DA ÁRVORE B) --- */
/* ============================================================================= */

static BTreeNode *btree_create_node(bool leaf);
static void btree_destroy_node(BTreeNode *node);
static TreeNode *btree_search_in_node(BTreeNode *node, const char *name);
static void btree_insert_non_full(BTreeNode *node, TreeNode *item);
static void btree_split_child(BTreeNode *parent, int index, BTreeNode *child);
static void btree_delete_from_node(BTreeNode *node, const char *name);
static int btree_find_key(BTreeNode *node, const char *name);
static void btree_merge(BTreeNode *node, int idx);
static void btree_fill(BTreeNode *node, int idx);
static void btree_borrow_from_prev(BTreeNode *node, int idx);
static void btree_borrow_from_next(BTreeNode *node, int idx);
static TreeNode *btree_get_predecessor(BTreeNode *node, int idx);
static TreeNode *btree_get_successor(BTreeNode *node, int idx);


/* ============================================================================= */
/* --- FUNÇÕES PRINCIPAIS
/* ============================================================================= */

// --- Funções de Arquivos e Diretórios ---

TreeNode *create_file(const char *name, const char *content)
{
    TreeNode *node = (TreeNode *)malloc(sizeof(TreeNode)); 
    node->name = strdup(name);
    node->type = FILE_TYPE;
    node->data.file = (File *)malloc(sizeof(File));
    node->data.file->name = strdup(name);
    node->data.file->content = strdup(content);
    node->data.file->size = strlen(content);

    time_t now = time(NULL);
    node->creation_time = now;
    node->modification_time = now;
    node->last_access_time = now;
    return node;
}

TreeNode *create_directory(const char *name, Directory *parent)
{
    TreeNode *node = (TreeNode *)malloc(sizeof(TreeNode));
    node->name = strdup(name);
    node->type = DIRECTORY_TYPE;
    node->data.directory = (Directory *)malloc(sizeof(Directory));
    node->data.directory->name = strdup(name);
    node->data.directory->parent = parent;
    node->data.directory->tree = btree_create();

    time_t now = time(NULL);
    node->creation_time = now;
    node->modification_time = now;
    node->last_access_time = now;
    return node;
}

void delete_file(TreeNode *node)
{
    if (node && node->type == FILE_TYPE)
    {
        free(node->data.file->name);
        free(node->data.file->content);
        free(node->data.file);
        free(node->name);
        free(node);
    }
}

void delete_directory_recursive(Directory *dir)
{
    if (dir)
    {
        if(dir->tree) {
            btree_destroy(dir->tree);
        }
        free(dir->name);
        free(dir);
    }
}

void free_tree_node(TreeNode *node)
{
    if (node)
    {
        if (node->type == FILE_TYPE)
        {
            delete_file(node);
        }
        else if (node->type == DIRECTORY_TYPE)
        {
            free(node->name);
            free(node);
        }
    }
}

// --- Funções de Navegação e Comandos ---

Directory *get_root_directory()
{
    Directory *root = (Directory *)malloc(sizeof(Directory));
    root->name = strdup("/");
    root->parent = NULL;
    root->tree = btree_create();
    return root;
}

void list_directory_contents(Directory *dir, bool long_format)
{
    if (dir && dir->tree && dir->tree->root && dir->tree->root->num_keys > 0)
    {
        printf("Conteúdo de %s:\n", dir->name);
        btree_traverse(dir->tree->root, long_format);
        if (!long_format)
            printf("\n");
    }
    else
    {
        printf("Diretório %s está vazio.\n", dir->name);
    }
    if (dir->parent != NULL)
    {
        TreeNode *self_node = btree_search(dir->parent->tree, dir->name);
        if (self_node)
            self_node->last_access_time = time(NULL);
    }
}

void change_directory(Directory **current_dir, const char *path)
{
    if (strcmp(path, ".") == 0) return;

    if (strcmp(path, "..") == 0)
    {
        if ((*current_dir)->parent != NULL)
        {
            *current_dir = (*current_dir)->parent;
        }
        return;
    }

    TreeNode *target_node = btree_search((*current_dir)->tree, path);
    if (target_node && target_node->type == DIRECTORY_TYPE)
    {
        target_node->last_access_time = time(NULL);
        *current_dir = target_node->data.directory;
    }
    else
    {
        printf("cd: diretório não encontrado: %s\n", path);
    }
}

char *get_current_path(Directory *dir)
{
    if (dir == NULL || dir->parent == NULL)
    {
        return strdup("/");
    }

    char *parent_path = get_current_path(dir->parent);
    if (strcmp(parent_path, "/") == 0)
    {
        char *path = (char *)malloc(strlen(dir->name) + 2);
        sprintf(path, "/%s", dir->name);
        free(parent_path);
        return path;
    }

    char *path = (char *)malloc(strlen(parent_path) + strlen(dir->name) + 2);
    sprintf(path, "%s/%s", parent_path, dir->name);
    free(parent_path);
    return path;
}

void update_parent_modification_time(Directory *dir)
{
    if (dir && dir->parent)
    {
        TreeNode *dir_node_in_parent = btree_search(dir->parent->tree, dir->name);
        if (dir_node_in_parent) {
             dir_node_in_parent->modification_time = time(NULL);
        }
    }
}

void show_metadata(Directory *dir, const char *name)
{
    TreeNode *node = btree_search(dir->tree, name);

    if (!node)
    {
        printf("stat: não foi possível encontrar o arquivo ou diretório '%s'\n", name);
        return;
    }

    char created_buf[20], modified_buf[20], accessed_buf[20];
    strftime(created_buf, sizeof(created_buf), "%d-%m-%Y %H:%M:%S", localtime(&node->creation_time));
    strftime(modified_buf, sizeof(modified_buf), "%d-%m-%Y %H:%M:%S", localtime(&node->modification_time));
    strftime(accessed_buf, sizeof(accessed_buf), "%d-%m-%Y %H:%M:%S", localtime(&node->last_access_time));

    printf("  Arquivo: %s\n", node->name);
    printf("     Tipo: %s\n", (node->type == FILE_TYPE) ? "Arquivo .txt" : "Diretório");
    if (node->type == FILE_TYPE)
    {
        printf("   Tamanho: %ld Bytes\n", node->data.file->size);
        printf("  Conteúdo: %s\n", node->data.file->content);
    }
    printf("    Acesso: %s (Última vez aberto/consultado)\n", accessed_buf);
    printf("Modificado: %s (Última alteração no conteúdo)\n", modified_buf);
    printf("   Criação: %s (Data de criação)\n", created_buf);

    node->last_access_time = time(NULL);
}

/* ============================================================================= */
/* --- FUNÇÕES AUXILIARES (IMPLEMENTAÇÃO INTERNA DA ÁRVORE B) --- */
/* ============================================================================= */

BTree *btree_create()
{
    BTree *tree = (BTree *)malloc(sizeof(BTree));
    tree->root = btree_create_node(true);
    return tree;
}

void btree_destroy(BTree *tree)
{
    if (tree)
    {
        if (tree->root)
        {
            btree_destroy_node(tree->root);
        }
        free(tree);
    }
}

static void btree_destroy_node(BTreeNode *node)
{
    if (node)
    {
        for (int i = 0; i < node->num_keys; i++)
        {
            free_tree_node(node->keys[i]);
        }
        if (!node->leaf)
        {
            for (int i = 0; i <= node->num_keys; i++)
            {
                btree_destroy_node(node->children[i]);
            }
        }
        free(node);
    }
}

static BTreeNode *btree_create_node(bool leaf)
{
    BTreeNode *node = (BTreeNode *)malloc(sizeof(BTreeNode));
    node->leaf = leaf;
    node->num_keys = 0;
    for(int i = 0; i < 2 * BTREE_ORDER; i++) node->children[i] = NULL;
    for(int i = 0; i < 2 * BTREE_ORDER - 1; i++) node->keys[i] = NULL;
    return node;
}

TreeNode *btree_search(BTree *tree, const char *name)
{
    if (!tree || !tree->root)
        return NULL;
    return btree_search_in_node(tree->root, name);
}

static TreeNode *btree_search_in_node(BTreeNode *node, const char *name)
{
    int i = 0;
    while (i < node->num_keys && strcmp(name, node->keys[i]->name) > 0)
    {
        i++;
    }
    if (i < node->num_keys && strcmp(name, node->keys[i]->name) == 0)
    {
        return node->keys[i];
    }
    if (node->leaf)
    {
        return NULL;
    }
    return btree_search_in_node(node->children[i], name);
}

void btree_insert(BTree *tree, TreeNode *item)
{
    BTreeNode *root = tree->root;
    if (root->num_keys == 2 * BTREE_ORDER - 1)
    {
        BTreeNode *new_root = btree_create_node(false);
        tree->root = new_root;
        new_root->children[0] = root;
        btree_split_child(new_root, 0, root);
        btree_insert_non_full(new_root, item);
    }
    else
    {
        btree_insert_non_full(root, item);
    }
}

static void btree_insert_non_full(BTreeNode *node, TreeNode *item)
{
    int i = node->num_keys - 1;
    if (node->leaf)
    {
        while (i >= 0 && strcmp(item->name, node->keys[i]->name) < 0)
        {
            node->keys[i + 1] = node->keys[i];
            i--;
        }
        node->keys[i + 1] = item;
        node->num_keys++;
    }
    else
    {
        while (i >= 0 && strcmp(item->name, node->keys[i]->name) < 0)
        {
            i--;
        }
        i++;
        if (node->children[i]->num_keys == 2 * BTREE_ORDER - 1)
        {
            btree_split_child(node, i, node->children[i]);
            if (strcmp(item->name, node->keys[i]->name) > 0)
            {
                i++;
            }
        }
        btree_insert_non_full(node->children[i], item);
    }
}

static void btree_split_child(BTreeNode *parent, int index, BTreeNode *child)
{
    BTreeNode *new_child = btree_create_node(child->leaf);
    new_child->num_keys = BTREE_ORDER - 1;

    for (int j = 0; j < BTREE_ORDER - 1; j++)
    {
        new_child->keys[j] = child->keys[j + BTREE_ORDER];
    }

    if (!child->leaf)
    {
        for (int j = 0; j < BTREE_ORDER; j++)
        {
            new_child->children[j] = child->children[j + BTREE_ORDER];
        }
    }

    child->num_keys = BTREE_ORDER - 1;

    for (int j = parent->num_keys; j >= index + 1; j--)
    {
        parent->children[j + 1] = parent->children[j];
    }
    parent->children[index + 1] = new_child;

    for (int j = parent->num_keys - 1; j >= index; j--)
    {
        parent->keys[j + 1] = parent->keys[j];
    }
    parent->keys[index] = child->keys[BTREE_ORDER - 1];
    parent->num_keys++;
}

void btree_delete(BTree *tree, const char *name)
{
    if (!tree->root) return;

    btree_delete_from_node(tree->root, name);

    if (tree->root->num_keys == 0 && !tree->root->leaf)
    {
        BTreeNode *old_root = tree->root;
        tree->root = tree->root->children[0];
        free(old_root);
    }
}

static void btree_delete_from_node(BTreeNode *node, const char *name)
{
    int idx = btree_find_key(node, name);

    if (idx < node->num_keys && strcmp(node->keys[idx]->name, name) == 0)
    {
        if (node->leaf)
        {
            free_tree_node(node->keys[idx]);
            for (int i = idx + 1; i < node->num_keys; ++i)
                node->keys[i - 1] = node->keys[i];
            node->num_keys--;
        }
        else
        {
            if (node->children[idx]->num_keys >= BTREE_ORDER)
            {
                TreeNode *pred = btree_get_predecessor(node, idx);
                free_tree_node(node->keys[idx]);
                node->keys[idx] = pred;
                btree_delete_from_node(node->children[idx], pred->name);
            }
            else if (node->children[idx + 1]->num_keys >= BTREE_ORDER)
            {
                TreeNode *succ = btree_get_successor(node, idx);
                free_tree_node(node->keys[idx]);
                node->keys[idx] = succ;
                btree_delete_from_node(node->children[idx + 1], succ->name);
            }
            else
            {
                btree_merge(node, idx);
                btree_delete_from_node(node->children[idx], name);
            }
        }
    }
    else
    {
        if (node->leaf) return;

        bool flag = (idx == node->num_keys);

        if (node->children[idx]->num_keys < BTREE_ORDER)
            btree_fill(node, idx);

        if (flag && idx > node->num_keys)
            btree_delete_from_node(node->children[idx - 1], name);
        else
            btree_delete_from_node(node->children[idx], name);
    }
}

static int btree_find_key(BTreeNode *node, const char *name)
{
    int idx = 0;
    while (idx < node->num_keys && strcmp(node->keys[idx]->name, name) < 0)
        ++idx;
    return idx;
}

static void btree_merge(BTreeNode *node, int idx)
{
    BTreeNode *child = node->children[idx];
    BTreeNode *sibling = node->children[idx + 1];

    child->keys[BTREE_ORDER - 1] = node->keys[idx];

    for (int i = 0; i < sibling->num_keys; ++i)
        child->keys[i + BTREE_ORDER] = sibling->keys[i];

    if (!child->leaf)
    {
        for (int i = 0; i <= sibling->num_keys; ++i)
            child->children[i + BTREE_ORDER] = sibling->children[i];
    }

    for (int i = idx + 1; i < node->num_keys; ++i)
        node->keys[i - 1] = node->keys[i];

    for (int i = idx + 2; i <= node->num_keys; ++i)
        node->children[i - 1] = node->children[i];

    child->num_keys += sibling->num_keys + 1;
    node->num_keys--;

    free(sibling);
}

static void btree_fill(BTreeNode *node, int idx)
{
    if (idx != 0 && node->children[idx - 1]->num_keys >= BTREE_ORDER)
        btree_borrow_from_prev(node, idx);
    else if (idx != node->num_keys && node->children[idx + 1]->num_keys >= BTREE_ORDER)
        btree_borrow_from_next(node, idx);
    else
    {
        if (idx != node->num_keys)
            btree_merge(node, idx);
        else
            btree_merge(node, idx - 1);
    }
}

static void btree_borrow_from_prev(BTreeNode *node, int idx)
{
    BTreeNode *child = node->children[idx];
    BTreeNode *sibling = node->children[idx - 1];

    for (int i = child->num_keys - 1; i >= 0; --i)
        child->keys[i + 1] = child->keys[i];

    if (!child->leaf)
    {
        for (int i = child->num_keys; i >= 0; --i)
            child->children[i + 1] = child->children[i];
    }

    child->keys[0] = node->keys[idx - 1];

    if (!child->leaf)
        child->children[0] = sibling->children[sibling->num_keys];

    node->keys[idx - 1] = sibling->keys[sibling->num_keys - 1];

    child->num_keys += 1;
    sibling->num_keys -= 1;
}

static void btree_borrow_from_next(BTreeNode *node, int idx)
{
    BTreeNode *child = node->children[idx];
    BTreeNode *sibling = node->children[idx + 1];

    child->keys[child->num_keys] = node->keys[idx];

    if (!child->leaf)
        child->children[child->num_keys + 1] = sibling->children[0];

    node->keys[idx] = sibling->keys[0];

    for (int i = 1; i < sibling->num_keys; ++i)
        sibling->keys[i - 1] = sibling->keys[i];

    if (!sibling->leaf)
    {
        for (int i = 1; i <= sibling->num_keys; ++i)
            sibling->children[i - 1] = sibling->children[i];
    }
    child->num_keys += 1;
    sibling->num_keys -= 1;
}

static TreeNode *btree_get_predecessor(BTreeNode *node, int idx)
{
    BTreeNode *cur = node->children[idx];
    while (!cur->leaf)
        cur = cur->children[cur->num_keys];
    return cur->keys[cur->num_keys - 1];
}

static TreeNode *btree_get_successor(BTreeNode *node, int idx)
{
    BTreeNode *cur = node->children[idx + 1];
    while (!cur->leaf)
        cur = cur->children[0];
    return cur->keys[0];
}

void btree_traverse(BTreeNode *node, bool long_format)
{
    if (node != NULL)
    {
        int i;
        for (i = 0; i < node->num_keys; i++)
        {
            if (!node->leaf)
            {
                btree_traverse(node->children[i], long_format);
            }

            if (long_format)
            {
                char time_buf[20];
                strftime(time_buf, sizeof(time_buf), "%d-%m-%Y %H:%M", localtime(&node->keys[i]->modification_time));
                printf("%s  %s%s\n", time_buf, node->keys[i]->name, (node->keys[i]->type == DIRECTORY_TYPE) ? "/" : "");
            }
            else
            {
                printf("%s%s  ", node->keys[i]->name, (node->keys[i]->type == DIRECTORY_TYPE) ? "/" : "");
            }
        }

        if (!node->leaf)
        {
            btree_traverse(node->children[i], long_format);
        }
    }
}
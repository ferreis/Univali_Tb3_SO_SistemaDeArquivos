#include "filesystem.h"
#include <stdio.h>
#include <string.h> 
#include <stdbool.h> 

#define MAX_CMD_LEN 100
#define MAX_ARGS 3

void print_prompt(Directory *current_dir)
{
    char *path = get_current_path(current_dir);
    printf("fs:%s$ ", path);
    free(path);
}

void save_fs_image(Directory *root, const char *filename);
void save_node_to_image_recursive(FILE *fp, BTreeNode *node, int level);

int main()
{
    // A inicialização do diretório raiz pode ser simplificada
    Directory *root = get_root_directory();
    Directory *current_dir = root;

    char cmd_line[MAX_CMD_LEN];
    char *args[MAX_ARGS];
    char *token;

    printf("Sistema de Arquivos Simples com Árvore B. Digite 'help' para ajuda.\n");

    while (1)
    {
        print_prompt(current_dir);
        fgets(cmd_line, MAX_CMD_LEN, stdin);
        cmd_line[strcspn(cmd_line, "\n")] = 0;

        int i = 0;
        token = strtok(cmd_line, " ");
        while (token != NULL && i < MAX_ARGS)
        {
            args[i++] = token;
            token = strtok(NULL, " ");
        }

        if (i == 0)
            continue;

        if (strcmp(args[0], "exit") == 0)
        {
            break;
        }
        else if (strcmp(args[0], "ls") == 0)
        {
            bool long_format = (i > 1 && strcmp(args[1], "-l") == 0);
            list_directory_contents(current_dir, long_format);
        }
        else if (strcmp(args[0], "mkdir") == 0)
        {
            if (i < 2)
            {
                printf("mkdir: faltando operando\n");
            }
            else
            {
                if (btree_search(current_dir->tree, args[1]))
                {
                    printf("mkdir: não é possível criar o diretório '%s': Arquivo ou diretório já existe\n", args[1]);
                }
                else
                {
                    TreeNode *new_dir_node = create_directory(args[1], current_dir);
                    btree_insert(current_dir->tree, new_dir_node);
                    update_parent_modification_time(current_dir);
                }
            }
        }
        else if (strcmp(args[0], "cd") == 0)
        {
            if (i < 2)
            {
                printf("cd: faltando operando\n");
            }
            else
            {
                change_directory(&current_dir, args[1]);
            }
        }
        else if (strcmp(args[0], "touch") == 0)
        {
            if (i < 2)
            {
                printf("touch: faltando operando. Uso: touch <arquivo.txt> [\"conteudo\"]\n");
            }
            else
            {
                if (strstr(args[1], ".txt") == NULL)
                {
                    printf("touch: O nome do arquivo deve terminar com .txt\n");
                }
                else if (btree_search(current_dir->tree, args[1]))
                {
                    printf("touch: não é possível criar o arquivo '%s': Arquivo ou diretório já existe\n", args[1]);
                }
                else
                {
                    char *content = (i > 2) ? args[2] : "";
                    TreeNode *new_file_node = create_file(args[1], content);
                    btree_insert(current_dir->tree, new_file_node);
                    update_parent_modification_time(current_dir);
                }
            }
        }
        else if (strcmp(args[0], "rm") == 0)
        {
            if (i < 2)
            {
                printf("rm: faltando operando. Uso: rm <arquivo.txt>\n");
            }
            else
            {
                 if (strstr(args[1], ".txt") == NULL) {
                    printf("rm: O alvo da remoção deve ser um arquivo .txt\n");
                } else {
                    TreeNode *node_to_delete = btree_search(current_dir->tree, args[1]);
                    if (!node_to_delete)
                    {
                        printf("rm: não foi possível remover '%s': Arquivo não encontrado\n", args[1]);
                    }
                    else if (node_to_delete->type == DIRECTORY_TYPE)
                    {
                        printf("rm: não é possível remover '%s': É um diretório\n", args[1]);
                    }
                    else
                    {
                        btree_delete(current_dir->tree, args[1]);
                        update_parent_modification_time(current_dir);
                        printf("Arquivo '%s' removido.\n", args[1]);
                    }
                }
            }
        }
        else if (strcmp(args[0], "rmdir") == 0)
        {
            if (i < 2)
            {
                printf("rmdir: faltando operando\n");
            }
            else
            {
                TreeNode *node_to_delete = btree_search(current_dir->tree, args[1]);
                if (!node_to_delete)
                {
                    printf("rmdir: não foi possível remover '%s': Arquivo ou diretório não encontrado\n", args[1]);
                }
                else if (node_to_delete->type == FILE_TYPE)
                {
                    printf("rmdir: não foi possível remover '%s': Não é um diretório\n", args[1]);
                }
                else if (node_to_delete->data.directory->tree->root->num_keys > 0)
                {
                    printf("rmdir: não foi possível remover '%s': Diretório não está vazio\n", args[1]);
                }
                else
                {
                    // A função btree_delete cuida de liberar o TreeNode.
                    // A delete_directory_recursive libera a estrutura Directory.
                    delete_directory_recursive(node_to_delete->data.directory);
                    btree_delete(current_dir->tree, args[1]);
                    update_parent_modification_time(current_dir);
                }
            }
        }
        else if (strcmp(args[0], "stat") == 0)
        {
            if (i < 2)
            {
                printf("stat: faltando operando\n");
            }
            else
            {
                show_metadata(current_dir, args[1]);
            }
        }
        else if (strcmp(args[0], "save") == 0)
        {
            if (i < 2)
            {
                printf("save: especifique o nome do arquivo (ex: save fs.img)\n");
            }
            else
            {
                save_fs_image(current_dir, args[1]);
                printf("Sistema de arquivos salvo em %s\n", args[1]);
            }
        }
        else if (strcmp(args[0], "help") == 0)
        {
            printf("Comandos disponíveis:\n");
            printf("  ls              - Lista o conteúdo do diretório atual\n");
            printf("  ls -l           - Lista com detalhes (metadados de tempo)\n");
            printf("  cd <dir>        - Muda para o diretório <dir>\n");
            printf("  mkdir <dir>     - Cria um novo diretório chamado <dir>\n");
            printf("  rmdir <dir>     - Remove o diretório vazio <dir>\n");
            printf("  touch <arq.txt> - Cria um arquivo de texto vazio\n");
            printf("  touch <arq.txt> \"conteudo\" - Cria um arquivo de texto com conteúdo\n");
            printf("  rm <arq.txt>      - Remove o arquivo de texto\n");
            printf("  stat <item>     - Exibe todos os metadados de um arquivo ou diretório\n");
            printf("  save <img_file> - Salva uma imagem do FS no arquivo\n");
            printf("  exit            - Sai do programa\n");
        }
        else
        {
            printf("Comando não encontrado: %s\n", args[0]);
        }
    }

    // A limpeza da memória é feita aqui, depois que o loop termina.
    delete_directory_recursive(root);
    printf("Saindo...\n");

    return 0;
}

void save_node_to_image_recursive(FILE *fp, BTreeNode *node, int level)
{
    if (node)
    {
        for (int i = 0; i < node->num_keys; ++i)
        {
            for (int j = 0; j < level; ++j)
                fprintf(fp, "  "); 

            fprintf(fp, "|-- %s%s\n", node->keys[i]->name, node->keys[i]->type == DIRECTORY_TYPE ? "/" : "");

            if (node->keys[i]->type == DIRECTORY_TYPE)
            {
                if (node->keys[i]->data.directory->tree) {
                    save_node_to_image_recursive(fp, node->keys[i]->data.directory->tree->root, level + 1);
                }
            }
        }
    }
}


void save_fs_image(Directory *root, const char *filename)
{
    FILE *fp = fopen(filename, "w");
    if (!fp)
    {
        perror("Erro ao abrir imagem para escrita");
        return;
    }
    fprintf(fp, "%s\n", root->name);
    if(root->tree) {
        save_node_to_image_recursive(fp, root->tree->root, 0);
    }
    fclose(fp);
}
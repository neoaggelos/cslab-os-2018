#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <sys/types.h>
#include <sys/wait.h>

#include "proc-common.h"
#include "tree.h"

#define SLEEP_PROC_SEC  10
#define SLEEP_TREE_SEC  3

void fork_procs(struct tree_node* node)
{
    change_pname(node->name);

    if (node->nr_children) {
        printf("%s: Creating %d children...\n", node->name, node->nr_children);
        int i;
        for (i = 0; i < node->nr_children; i++) {
            int pid = fork();
            if (pid < 0) {
                perror(node->name);
                exit(1);
            }
            else if (pid == 0) {
                fork_procs(&node->children[i]);
                break;
            }
        }

        printf("%s: Waiting for %d children...\n", node->name, node->nr_children);
        for (i = 0; i < node->nr_children; i++) {
            int status, pid;
            pid = wait(&status);
            explain_wait_status(pid, status);
        }
    } else {
        printf("%s: Sleeping for %d seconds...\n", node->name, SLEEP_PROC_SEC);
        sleep(SLEEP_PROC_SEC);
    }

    printf("%s: Exiting...\n", node->name);
    exit(9);
}

/*
    main process creates process tree, waits and hopes for the best
 */
int main(int argc, char** argv)
{
    pid_t pid;
    int status;
    struct tree_node* tree;

    if (argc != 2) {
        printf("Usage: %s <tree_file>\n", argv[0]);
        exit(1);
    }

    /* Load tree */
    tree = get_tree_from_file(argv[1]);

    /* Fork root of process tree */
    pid = fork();
    if (pid < 0) {
        perror("main: fork");
        exit(1);
    }
    if (pid == 0) {
        /* Child */
        fork_procs(tree);
        exit(1);
    }

    /* wait and hope that the tree gets created in the meantime */
    sleep(SLEEP_TREE_SEC);
    
    /* Print process tree from file, to compare */
    puts("Process tree:");
    print_tree(tree);

    /* Print the process tree root at pid */
    puts("Process tree with pstree:");
    show_pstree(pid);

    /* Wait for the root of the process tree to terminate */
    pid = wait(&status);
    explain_wait_status(pid, status);

    return 0;
}

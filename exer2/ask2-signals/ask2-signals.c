#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>

#include "tree.h"
#include "proc-common.h"

void fork_procs(struct tree_node *node)
{
    int *pid, i;
    
    /* start and set process name */
    printf("PID = %ld, name %s, starting...\n",
            (long)getpid(),  node->name);
    change_pname(node->name);

    /* create children */
    if (node->nr_children) {
        /* save pids, to ensure that we wake the children in the correct order */
        pid = malloc(sizeof(pid_t) * node->nr_children);
        if (pid == NULL) {
            perror(node->name);
            exit(1);
        }

        printf("%s: Creating %d children...\n", node->name, node->nr_children);
        for (i = 0; i < node->nr_children; i++) {
            pid[i] = fork();

            if (pid[i] < 0) {
                perror(node->name);
                exit(1);
            }
            else if (pid[i] == 0) {
                /* child */
                fork_procs(&node->children[i]);

                /* ensure it never returns */
                assert(0);
            }
        }

        /* wait for all children to prepare their own children and raise(SIGSTOP); */
        wait_for_ready_children(node->nr_children);
    }

    /* suspend */
    raise(SIGSTOP);

    /* ... received signal to continue */
    printf("PID = %ld, name = %s is awake\n",
        (long)getpid(), node->name);

    /* wake children up in correct order */
    if (node->nr_children) {
        for (i = 0; i < node->nr_children; i++) {
            int status;

            /* send signal */
            kill(pid[i], SIGCONT);

            /* wait for it to exit */
            waitpid(pid[i], &status, 0);
            explain_wait_status(pid[i], status);
        }
    }

    /* Dont exit. I have a perfectly strong proof for why
     * we shouldnt exit yet, but it does not fit in the
     * margins of this small comment...
     */
    exit(0);
}


/**
 *  creates root process, waits for it to complete
 */
int main(int argc, char *argv[])
{
    pid_t pid;
    int status;
    struct tree_node *root;

    if (argc < 2){
        fprintf(stderr, "Usage: %s <tree_file>\n", argv[0]);
        exit(1);
    }

    /* Read tree into memory */
    root = get_tree_from_file(argv[1]);

    /* Fork root of process tree */
    pid = fork();
    if (pid < 0) {
        perror("main: fork");
        exit(1);
    }
    if (pid == 0) {
        /* Child */
        fork_procs(root);
        exit(1);
    }

    
    /* wait until process tree is complete */
    wait_for_ready_children(1);

    /* Print the process tree root at pid */
    show_pstree(pid);

    /* Wake root process */
    kill(pid, SIGCONT);

    /* Wait for the root of the process tree to terminate */
    wait(&status);
    explain_wait_status(pid, status);

    return 0;
}

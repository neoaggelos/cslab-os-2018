#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>

#include "tree.h"
#include "proc-common.h"

void fork_procs(struct tree_node *node, int parent_pipe)
{
    int *pid, i;

    /*
     * NOTE: pipe[0] is the read end, pipe[1] is the write end
     * we pass pipe[1] as parent_pipe in child processes
     * we then read from pipe[0] and write to parent_pipe
     */
    int pipe_fd[2];

    /* the result we send to parent_pipe */
    int result;

    /* start and set process name */
    printf("PID = %ld, name %s, starting...\n",
            (long)getpid(),  node->name);
    change_pname(node->name);

    /* create children */
    if (node->nr_children) {
        /* better safe than sorry */
        assert(node->nr_children == 2);

        /* open pipe to talk with children */
        if (pipe(pipe_fd) < 0) {
            perror("pipe");
            exit(1);
        }

        /* save pids, to ensure that we wake the children in the correct order */
        /* (not really necessary for expression evaluation but w/e) */
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
                fork_procs(&node->children[i], pipe_fd[1]);

                /* ensure it never returns */
                assert(0);
            }
        }

        /* parent: close writing end */
        close(pipe_fd[1]);

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
        int x;
        for (i = 0; i < node->nr_children; i++) {
            int status;

            /* send signal */
            kill(pid[i], SIGCONT);

            /* wait for it to exit */
            waitpid(pid[i], &status, 0);
            explain_wait_status(pid[i], status);
        }

        /* children finished, read, compute and return */
        if (read(pipe_fd[0], &result, sizeof(int)) != sizeof(int)) {
            perror("1st read error");
            exit(1);
        }
        if (read(pipe_fd[0], &x, sizeof(int)) != sizeof(int)) {
            perror("2nd read error");
            exit(1);
        }

        /* calculate result */
        if (node->name[0] == '*') {
            result *= x;
        } else if (node->name[0] == '+') {
            result += x;
        } else {
            printf("%s error: bad operator\n", node->name);
            exit(1);
        }

        /* cleanup? */
        close(pipe_fd[0]);
    } else {
        result = atoi(node->name);
    }

    /* write to parent */
    if (write(parent_pipe, &result, sizeof(int)) != sizeof(result)) {
        perror("write error");
        exit(1);
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
    int fd[2];
    int result;

    if (argc < 2){
        fprintf(stderr, "Usage: %s <tree_file>\n", argv[0]);
        exit(1);
    }

    if (pipe(fd) < 0) {
        perror("main: pipe:");
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
        fork_procs(root, fd[1]);
        exit(1);
    }

    /* parent: close writing fd */
    close(fd[1]);

    /* wait until process tree is complete */
    wait_for_ready_children(1);

    /* Print the process tree root at pid */
    show_pstree(pid);

    print_tree(root);

    /* Wake root process */
    kill(pid, SIGCONT);

    /* Wait for the root of the process tree to terminate */
    wait(&status);
    explain_wait_status(pid, status);

    /* read result */
    if (read(fd[0], &result, sizeof(int)) != sizeof(int)) {
        perror("main: read:");
        exit(1);
    }

    printf("\n\n\n\nResult is: %d\n", result);
    close(fd[0]);
    close(fd[1]);

    return 0;
}

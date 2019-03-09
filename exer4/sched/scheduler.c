#include <errno.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <signal.h>
#include <string.h>
#include <assert.h>

#include <sys/wait.h>
#include <sys/types.h>

#include "proc-common.h"

/* Compile-time parameters. */
#define SCHED_TQ_SEC 2                /* time quantum */
#define TASK_NAME_SZ 60               /* maximum size for a task's name */

/* List of processes */
#include "plist.h"
struct process_list *procs = NULL;

#ifdef WCONTINUED
# define HANDLE_SIGCONT
#endif

#ifdef HANDLE_SIGCONT
# define WAIT_FLAGS ( WUNTRACED | WNOHANG | WCONTINUED )
#else
# define WAIT_FLAGS ( WUNTRACED | WNOHANG )
#endif

void start_next_process(void)
{
	if (kill(procs->pid, SIGCONT) < 0) {
		perror("kill-SIGCONT");
		exit(-1);
	}
	
	/* restart timer */
	if (alarm(SCHED_TQ_SEC) < 0) {
		perror("alarm");
		exit(-1);
	}
}

/*
 * SIGALRM handler
 */
static void
sigalrm_handler(int signum)
{
	assert(signum == SIGALRM);

	/* pause current process (it is at the head of procs list) */
	if (kill(procs->pid, SIGSTOP) < 0) {
		perror("kill-SIGSTOP");
		exit(-1);
	}

	/* restart timer */
	if (alarm(SCHED_TQ_SEC) < 0) {
		perror("alarm");
		exit(-1);
	}
}

/* 
 * SIGCHLD handler
 */
static void
sigchld_handler(int signum)
{
	assert(signum == SIGCHLD);
	
	for (;;) {
		int status;
		pid_t p = waitpid(-1, &status, WAIT_FLAGS);
		if (p < 0) {
			perror("waitpid");
			exit(-1);
		}
		if (p == 0)
			break;

		/* explain_wait_status(p, status); */

#ifdef HANDLE_SIGCONT
		if(WIFCONTINUED(status)) {
			/* a child continued */
			/* stop it if it is not the current process */
			if (procs->pid != p) {
				if (kill(p, SIGSTOP) < 0) {
					perror("kill-SIGSTOP");
					exit(-1);
				}
			}
		} else
#endif
		if (WIFEXITED(status) || WIFSIGNALED(status)) {
			/* child died */
			pid_t curpid = procs->pid;

			printf("%d was slain in battle\n", p);
			delete_from_list(&procs, p);
			if (procs == NULL) {
				puts("All processes died, exiting...");
				exit(0);
			}

			/* current process died, choose next */
			if (curpid == p)
				start_next_process();
			
		} else if (WIFSTOPPED(status)) {
			/* child was stopped by SIGSTOP */
			/* choose next process */
			
			//assert(procs->pid == p);
			if (p == procs->pid) {
				procs = procs->next;
				/* print_list(procs); */
				
				start_next_process();
			}
		}
	}
}

/* Install two signal handlers.
 * One for SIGCHLD, one for SIGALRM.
 * Make sure both signals are masked when one of them is running.
 */
static void
install_signal_handlers(void)
{
	sigset_t sigset;
	struct sigaction sa;

	sa.sa_handler = sigchld_handler;
	sa.sa_flags = SA_RESTART;
	sigemptyset(&sigset);
	sigaddset(&sigset, SIGCHLD);
	sigaddset(&sigset, SIGALRM);
	sa.sa_mask = sigset;
	if (sigaction(SIGCHLD, &sa, NULL) < 0) {
		perror("sigaction: sigchld");
		exit(1);
	}

	sa.sa_handler = sigalrm_handler;
	if (sigaction(SIGALRM, &sa, NULL) < 0) {
		perror("sigaction: sigalrm");
		exit(1);
	}

	/*
	 * Ignore SIGPIPE, so that write()s to pipes
	 * with no reader do not result in us being killed,
	 * and write() returns EPIPE instead.
	 */
	if (signal(SIGPIPE, SIG_IGN) < 0) {
		perror("signal: sigpipe");
		exit(1);
	}
}

int main(int argc, char *argv[])
{
	int i;
	int nproc = argc-1;

	if (nproc == 0) {
		fprintf(stderr, "Scheduler: No tasks. Exiting...\n");
		exit(-1);
	}

	for (i = 1; i < argc; i++) {
		pid_t pid = fork();
		if (pid < 0) {
			/* FIXME: kill kids? */
			perror("fork");
			exit(-1);
		} else if (pid == 0) {
			/* child, execute and say bye */
			char* new_argv[] = {argv[i], NULL};
			char* new_env[] = {NULL};
			
			/* dont start yet */
			raise(SIGSTOP);
			execve(argv[i], new_argv, new_env);

			perror("execve");
			exit(-1);
		} else {
			/* parent, append child to process list */
			append_to_list(&procs, pid, argv[i]);
		}
	}

	/* Wait for all children to be stopped properly before exec()ing. */
	wait_for_ready_children(nproc);

	/* Install SIGALRM and SIGCHLD handlers. */
	install_signal_handlers();

	/* wake first process */
	start_next_process();

	/* loop forever  until we exit from inside a signal handler. */
	while (pause())
		;

	/* Unreachable */
	fprintf(stderr, "Internal error: Reached unreachable point\n");
	return 1;
}

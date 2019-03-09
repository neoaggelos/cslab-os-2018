// changes are noted using the following comment
//NEW CODE

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
#include "request.h"

/* Compile-time parameters. */
#define SCHED_TQ_SEC 2                /* time quantum */
#define TASK_NAME_SZ 60               /* maximum size for a task's name */
#define SHELL_EXECUTABLE_NAME "shell" /* executable for shell */

//NEW CODE
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

/* Print a list of all tasks currently being scheduled.  */
static void
sched_print_tasks(void)
{
	//NEW CODE
	print_list_with_pid(procs, procs->pid);
}

/* Send SIGKILL to a task determined by the value of its
 * scheduler-specific id.
 */
static int
sched_kill_task_by_id(int id)
{
	//NEW CODE
	assert(procs != NULL);
		
	pid_t pid = get_pid_from_id(procs, id);
	
	if (!pid)
		return -1;

	if (kill(pid, SIGKILL)) {
		perror("kill-SIGKILL");
		exit(-1);
	}

	return 0;
}

/* Create a new task.  */
static void
sched_create_task(char *executable)
{
	//NEW CODE
	pid_t p = fork();
	if (p < 0) {
		/* FIXME: kill kids? */
		perror("fork");
	} else if (p == 0) {
		/* child, execute and say bye */
		char* new_argv[] = {executable, NULL};
		char* new_env[] = {NULL};

		/* dont start yet */
		raise(SIGSTOP);
		execve(executable, new_argv, new_env);

		perror("execve");
		exit(-1);
	} else {
		/* parent, append child to process list */
		append_to_list(&procs, p, executable);
	}
}

/* Process requests by the shell.  */
static int
process_request(struct request_struct *rq)
{
	switch (rq->request_no) {
		case REQ_PRINT_TASKS:
			sched_print_tasks();
			return 0;

		case REQ_KILL_TASK:
			return sched_kill_task_by_id(rq->task_arg);

		case REQ_EXEC_TASK:
			sched_create_task(rq->exec_task_arg);
			return 0;

		default:
			return -ENOSYS;
	}
}

/* 
 * SIGALRM handler
 */
static void
sigalrm_handler(int signum)
{
	//NEW CODE
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
	//NEW CODE
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

		//explain_wait_status(p, status);

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

			printf("Scheduler: %d was slain in battle\n", p);
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
			
			if (procs->pid == p) {
				procs = procs->next;
				start_next_process();
			}
		}
	}
}

/* Disable delivery of SIGALRM and SIGCHLD. */
static void
signals_disable(void)
{
	sigset_t sigset;

	sigemptyset(&sigset);
	sigaddset(&sigset, SIGALRM);
	sigaddset(&sigset, SIGCHLD);
	if (sigprocmask(SIG_BLOCK, &sigset, NULL) < 0) {
		perror("signals_disable: sigprocmask");
		exit(1);
	}
}

/* Enable delivery of SIGALRM and SIGCHLD.  */
static void
signals_enable(void)
{
	sigset_t sigset;

	sigemptyset(&sigset);
	sigaddset(&sigset, SIGALRM);
	sigaddset(&sigset, SIGCHLD);
	if (sigprocmask(SIG_UNBLOCK, &sigset, NULL) < 0) {
		perror("signals_enable: sigprocmask");
		exit(1);
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

static void
do_shell(char *executable, int wfd, int rfd)
{
	char arg1[10], arg2[10];
	char *newargv[] = { executable, NULL, NULL, NULL };
	char *newenviron[] = { NULL };

	sprintf(arg1, "%05d", wfd);
	sprintf(arg2, "%05d", rfd);
	newargv[1] = arg1;
	newargv[2] = arg2;

	raise(SIGSTOP);
	execve(executable, newargv, newenviron);

	/* execve() only returns on error */
	perror("scheduler: child: execve");
	exit(1);
}

/* Create a new shell task.
 *
 * The shell gets special treatment:
 * two pipes are created for communication and passed
 * as command-line arguments to the executable.
 */
static pid_t
sched_create_shell(char *executable, int *request_fd, int *return_fd)
{
	pid_t p;
	int pfds_rq[2], pfds_ret[2];

	if (pipe(pfds_rq) < 0 || pipe(pfds_ret) < 0) {
		perror("pipe");
		exit(1);
	}

	p = fork();
	if (p < 0) {
		perror("scheduler: fork");
		exit(1);
	}

	if (p == 0) {
		/* Child */
		close(pfds_rq[0]);
		close(pfds_ret[1]);
		do_shell(executable, pfds_rq[1], pfds_ret[0]);
		assert(0);
	}
	/* Parent */
	close(pfds_rq[1]);
	close(pfds_ret[0]);
	*request_fd = pfds_rq[0];
	*return_fd = pfds_ret[1];

	//NEW CODE
	return p;
}

static void
shell_request_loop(int request_fd, int return_fd)
{
	int ret;
	struct request_struct rq;

	/*
	 * Keep receiving requests from the shell.
	 */
	for (;;) {
		if (read(request_fd, &rq, sizeof(rq)) != sizeof(rq)) {
			perror("scheduler: read from shell");
			fprintf(stderr, "Scheduler: giving up on shell request processing.\n");
			break;
		}

		signals_disable();
		ret = process_request(&rq);
		signals_enable();

		if (write(return_fd, &ret, sizeof(ret)) != sizeof(ret)) {
			perror("scheduler: write to shell");
			fprintf(stderr, "Scheduler: giving up on shell request processing.\n");
			break;
		}
	}
}

int main(int argc, char *argv[])
{
	//NEW CODE
	int i;
	int nproc = argc; // (argc-1) plus the shell  #quick#mafs

	/* Two file descriptors for communication with the shell */
	static int request_fd, return_fd;

	/* Create the shell. */
	pid_t shell_pid = sched_create_shell(SHELL_EXECUTABLE_NAME, &request_fd, &return_fd);

	//NEW CODE	
	/* add shell to process list */
	append_to_list(&procs, shell_pid, SHELL_EXECUTABLE_NAME);
	if (kill(shell_pid, SIGSTOP) < 0) {
		perror("kill-SIGSTOP");
		exit(-1);
	}

	//NEW CODE
	for (i = 1; i < argc; i++) {
		sched_create_task(argv[i]);
	}

	/* Wait for all children to raise SIGSTOP before exec()ing. */
	wait_for_ready_children(nproc);

	/* Install SIGALRM and SIGCHLD handlers. */
	install_signal_handlers();

	//NEW CODE
	/* wake first process */
	start_next_process();

	shell_request_loop(request_fd, return_fd);

	/* Now that the shell is gone, just loop forever
	 * until we exit from inside a signal handler.
	 */
	while (pause())
		;

	/* Unreachable */
	fprintf(stderr, "Internal error: Reached unreachable point\n");
	return 1;
}

#include <sys/types.h>

struct process_list {
  int id;
  pid_t pid;
  char name[60];

  struct process_list *next;
  struct process_list *prev;
};

void delete_from_list(struct process_list **head, pid_t pid);
void append_to_list(struct process_list **head, pid_t pid, char name[60]);
void print_list(struct process_list *p);
#include <sys/types.h>

struct process_list {
  int id;
  pid_t pid;
  char name[60];

  struct process_list *next;
  struct process_list *prev;
};

void delete_from_list(struct process_list **head, pid_t pid);
void delete_from_list_with_id(struct process_list ** head, int id);
void append_to_list(struct process_list **head, pid_t pid, char name[60]);
void print_list(struct process_list *p);

//NEW CODE
pid_t get_pid_from_id(struct process_list* list, int id);
void print_list_with_pid(struct process_list *list, pid_t current);

//EDITED
struct process_list* find_process_from_id(struct process_list *list, int id);
void append_to_list_with_id(struct process_list **head, int id, pid_t pid, char name[60]);
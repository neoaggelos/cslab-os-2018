#include "plist.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

static int nextID = 0;

void*
Malloc(size_t sz)
{
    void *p = malloc(sz);
    if (!p) {
        perror("malloc");
        exit(-1);
    }

    return p;
}

void
delete_from_list(struct process_list ** head, pid_t pid)
{
    if (*head == NULL)
        return;

    struct process_list *node;

    if ((*head)->pid == pid) {
        if ((*head)->next != *head) {
            node = (*head)->next;
            (*head)->prev->next = (*head)->next;
            (*head)->next->prev = (*head)->prev;

            free(*head);
            *head = node;
        } else {
            free(*head);
            *head = NULL;
        }

        return ;
    }

    node = *head;
    while((node = node->next) != *head) {
        if (node->pid == pid) {
            node->prev->next = node->next;
            node->next->prev = node->prev;

            free(node);
            return ;
        }
    }
}

void
delete_from_list_with_id(struct process_list ** head, int id)
{
    if (*head == NULL)
        return;

    struct process_list *node;

    if ((*head)->id == id) {
        if ((*head)->next != *head) {
            node = (*head)->next;
            (*head)->prev->next = (*head)->next;
            (*head)->next->prev = (*head)->prev;

            free(*head);
            *head = node;
        } else {
            free(*head);
            *head = NULL;
        }

        return ;
    }

    node = *head;
    while((node = node->next) != *head) {
        if (node->id == id) {
            node->prev->next = node->next;
            node->next->prev = node->prev;

            free(node);
            return ;
        }
    }
}

void
append_to_list(struct process_list **head, pid_t pid, char name[60])
{
    append_to_list_with_id(head, nextID++, pid, name);
}

void
print_list(struct process_list *list)
{
    if (list == NULL) {
        return;
    }

    struct process_list *node = list;
    do {
        printf("ID: %d\tPID: %d\tName: %s\n", node->id, node->pid, node->name);
        node = node->next;
    } while(node != list);
}

//NEW CODE
void
print_list_with_pid(struct process_list *list, pid_t current)
{
    if (list == NULL) {
        return;
    }

    struct process_list *node = list;
    do {
        printf("ID: %d\tPID: %d\tName: %s %s\n", node->id, node->pid, node->name, node->pid == current ? "[*]" : "");
        node = node->next;
    } while(node != list);
}

pid_t
get_pid_from_id(struct process_list* list, int id)
{
    if (list == NULL)
        return 0;

    struct process_list *p = list;
    do {
        if (p->id == id)
            return p->pid;
    } while( (p = p->next) != list );

    return 0;
}

//EDITED
struct process_list*
find_process_from_id(struct process_list *list, int id)
{
    if (list == NULL)
        return NULL;

    struct process_list *p = list;
    do {
        if (p->id == id)
            return p;
    } while( (p = p->next) != list );

    return NULL;
}

//EDITED
void
append_to_list_with_id(struct process_list **head, int id, pid_t pid, char name[60])
{
    struct process_list *new_node;
    if (*head == NULL) {
        *head = Malloc(sizeof(struct process_list));
        (*head)->id = id;
        (*head)->pid = pid;
        strncpy((*head)->name, name, 59);

        (*head)->next = *head;
        (*head)->prev = *head;

        return ;
    }

    new_node = Malloc(sizeof(struct process_list));
    new_node->id = id;
    new_node->pid = pid;
    strncpy(new_node->name, name, 59);

    new_node->next = *head;
    new_node->prev = (*head)->prev;
    (*head)->prev->next = new_node;
    (*head)->prev = new_node;
}
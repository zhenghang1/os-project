#include "data_structure.h"
#include <stdio.h>
#include <stdlib.h>

/****************************** stack begin *********************************/

void move_to_top(stack_node *head, stack_node *tail, int id)
{
    stack_node *tmp = head->next;
    while (tmp != tail)
    {
        // the node with id already exists
        if (tmp->id == id)
        {
            tmp->prior->next = tmp->next;
            tmp->next->prior = tmp->prior;
            head->next->prior = tmp;
            tmp->next = head->next;
            head->next = tmp;
            tmp->prior = head;
            return;
        }
        tmp = tmp->next;
    }

    // the node with id doesn't exists, create a new node
    tmp = (stack_node *)malloc(sizeof(stack_node *));
    tmp->id = id;
    head->next->prior = tmp;
    tmp->next = head->next;
    head->next = tmp;
    tmp->prior = head;
    return;
}

int get_buttom_id(stack_node *tail)
{
    return tail->prior->id;
}

void clean_stack(stack_node *head)
{
    while (head)
    {
        stack_node *tmp = head;
        head = head->next;
        free(tmp);
    }
}

/****************************** stack end *********************************/

/****************************** list begin ********************************/

void insert_node(list_node *head, int id)
{
    list_node *tmp = (list_node *)malloc(sizeof(list_node *));
    tmp->id = id;
    tmp->next = head->next;
    head->next = tmp;
}

int remove_node(list_node *head)
{
    if (head->next == NULL)
        return -1;

    int id = head->next->id;
    list_node *tmp = head->next;
    head->next = tmp->next;
    free(tmp);

    return id;
}

void clean_list(list_node *head)
{
    while (head)
    {
        list_node *tmp = head;
        head = head->next;
        free(tmp);
    }
}

/****************************** list end *********************************/
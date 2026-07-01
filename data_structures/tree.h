#ifndef TREE_H_
#define TREE_H_
#include <stdint.h>

/*
Don't delete any fucking node manually.

Use create_tree_node to create the head and any particular child.

You can use the return pointer of find_tree_val to start deleting from there
if you want, altho don't fucking free() manually or change the internals.
*/

struct tree_node;

//if parent is NULL, the node is the head
//returns NULL on failure
struct tree_node* create_tree_node(struct tree_node* parent, uint32_t val);
//Deletes a tree from its head to its leaves.
//Does not work with subnodes, you can only delete a whole tree at a time and not some particular nodes.
//Also sets the head to NULL on success.
void delete_tree(struct tree_node** head);
void print_tree(struct tree_node* head, const char* format);
//Depth first search
//returns the node pointer if the value is found in that node. returns NULL otherwise.
struct tree_node* find_tree_val(struct tree_node* head, uint32_t val);

//Getters
struct tree_node* get_tree_node_child(struct tree_node* node);
struct tree_node* get_tree_node_brother(struct tree_node* node);
uint32_t get_tree_val(struct tree_node* node);
//Setters
void set_tree_val(struct tree_node* node, uint32_t val);

#ifdef TREE_IMPL
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct tree_node_impl
{
    struct tree_node_impl* child;
    struct tree_node_impl* brother;
    uint32_t val;
};

struct tree_node* create_tree_node(struct tree_node* parent, uint32_t val)
{
    struct tree_node_impl* node = calloc(1, sizeof(struct tree_node_impl));
    if (!node) return NULL;

    node->val = val;

    if (parent)
    {
        //put ourselves as the first child of parent, to avoid O(children)
        node->brother = ((struct tree_node_impl*)parent)->child; //the parent's first child is our brother now
        ((struct tree_node_impl*)parent)->child = node; //now we are the first child
    }

    return (struct tree_node*)node;
}

void delete_tree_in(struct tree_node_impl* head)
{
    if (!head) return;
    delete_tree_in(head->brother);
    delete_tree_in(head->child);

    free(head);
}

void delete_tree(struct tree_node** head)
{
    if (!head || !(*head)) return;

    delete_tree_in(((struct tree_node_impl*)(*head))->child);

    free(*head);
    *head = NULL;
}

void print_tree_in(struct tree_node_impl* node, size_t depth, const char* format)
{
    if (!node) return;

    for (size_t i = 0; i < depth; i++)
    {
        printf(". ");
    }

    printf(format, node->val);
    putchar('\n');
    print_tree_in(node->brother, depth, format);
    print_tree_in(node->child, depth + 1, format);
}

void print_tree(struct tree_node* head, const char* format)
{
    if (!head) {printf("print_tree ERROR: head is NULL\n"); return;}

    print_tree_in((struct tree_node_impl*)head, 0, format);
}

struct tree_node_impl* find_tree_val_in(struct tree_node_impl* head, uint32_t val)
{
    if (!head) return NULL;
    if (head->val == val) return head;

    struct tree_node_impl* found = find_tree_val_in(head->child, val);
    if (found) return found;
    found = find_tree_val_in(head->brother, val);
    return found;
}

struct tree_node* find_tree_val(struct tree_node* head, uint32_t val)
{
    if (!head) {printf("find_in_tree ERROR: head is NULL\n"); return NULL;}
    if (((struct tree_node_impl*)head)->val == val) return head;

    return (struct tree_node*)find_tree_val_in((struct tree_node_impl*)head, val);
}

//Getters
struct tree_node* get_tree_node_child(struct tree_node* node)
{
    if (!node)
    {
        printf("get_tree_node_child ERROR: node is NULL\n"); return NULL;
    }

    return (struct tree_node*)((struct tree_node_impl*)node)->child;
}

struct tree_node* get_tree_node_brother(struct tree_node* node)
{
    if (!node)
    {
        printf("get_tree_node_brother ERROR: node is NULL\n"); return NULL;
    }

    return (struct tree_node*)((struct tree_node_impl*)node)->brother;
}

uint32_t get_tree_val(struct tree_node* node)
{
    if (!node)
    {
        printf("get_tree_val ERROR: node is NULL\n"); return 0;
    }

    return ((struct tree_node_impl*)node)->val;
}

void set_tree_val(struct tree_node* node, uint32_t val)
{
    if (!node)
    {
        printf("set_tree_val ERROR: node is NULL\n"); return;
    }

    ((struct tree_node_impl*)node)->val = val;
}

#endif //IMPL

#endif //Include guard
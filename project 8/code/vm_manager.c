#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "data_structure.h"

tlb_item TLB[TLB_ENTRY_NUM];           // TLB
page page_table[PAGE_TABLE_ENTRY_NUM]; // page_table
frame memory[FRAME_NUM];               // memory

FILE *fp_addr;  // addresses.txt
FILE *fp_out;   // out.txt
FILE *fp_store; // BACKING_STORE.bin

stack_node *TLB_stack_head, *TLB_stack_tail;     // TLB stack
stack_node *frame_stack_head, *frame_stack_tail; // frame stack

list_node *free_frame_list; // free frame list
list_node *free_TLB_list;   // free TLB list

int count = 0;      // total addresses number
int tlb_hit = 0;    // TLB hit times
int page_fault = 0; // page fault times

void init(int argc, char *argv[]);
void clean();
int ger_frame_id(int page_id);
int search_in_TLB(int page_id);
int search_in_page_table(int page_id);
int search_unused_frame();
int search_unused_TLB();

int main(int argc, char *argv[])
{
    // initialization
    init(argc, argv);

    int addr, page_id, frame_id, offset;

    while (~fscanf(fp_addr, "%d", &addr))
    {
        count++;
        addr &= 0x0000ffff;
        offset = addr & 0x000000ff;
        page_id = (addr >> 8) & 0x000000ff;

        frame_id = ger_frame_id(page_id);
        int data = memory[frame_id].data[offset];
        int phy_addr = frame_id * FRAME_SIZE + offset;

        fprintf(fp_out, "Virtual address: %d Physical address: %d Value: %d\n", addr, phy_addr, data);
    }

    printf("[Statistics]\n");
    printf("    Frame number: %d\n", FRAME_NUM);
    printf("    TLB hit rate: %.4f %%\n", 100.0 * tlb_hit / count);
    printf("    Page fault rate: %.4f %%\n", 100.0 * page_fault / count);

    clean();
    return 0;
}

void init(int argc, char *argv[])
{
    if (argc <= 1 || argc >= 3)
    {
        printf("Error: invalid arguments!\n");
        exit(1);
    }

    for (int i = 0; i < TLB_ENTRY_NUM; i++)
    {
        TLB[i].valid = 0;
    }

    for (int i = 0; i < PAGE_TABLE_ENTRY_NUM; i++)
    {
        page_table[i].valid = 0;
    }

    fp_addr = fopen(argv[1], "r");
    if (fp_addr == NULL)
    {
        printf("Error: open file %s failed!\n", argv[1]);
        exit(1);
    }
    fp_out = fopen("out.txt", "w");
    if (fp_out == NULL)
    {
        printf("Error: create file out.txt failed!\n");
        exit(1);
    }
    fp_store = fopen("BACKING_STORE.bin", "rb");
    if (fp_store == NULL)
    {
        printf("Error: open file BACKING_STORE.bin failed!\n");
        exit(1);
    }

    TLB_stack_head = (stack_node *)malloc(sizeof(stack_node *));
    TLB_stack_tail = (stack_node *)malloc(sizeof(stack_node *));
    TLB_stack_head->prior = NULL;
    TLB_stack_head->next = TLB_stack_tail;
    TLB_stack_tail->prior = TLB_stack_head;
    TLB_stack_tail->next = NULL;

    frame_stack_head = (stack_node *)malloc(sizeof(stack_node *));
    frame_stack_tail = (stack_node *)malloc(sizeof(stack_node *));
    frame_stack_head->prior = NULL;
    frame_stack_head->next = frame_stack_tail;
    frame_stack_tail->prior = frame_stack_head;
    frame_stack_tail->next = NULL;

    free_frame_list = (list_node *)malloc(sizeof(list_node *));
    free_frame_list->next = NULL;
    for (int i = FRAME_NUM - 1; i >= 0; i--)
        insert_node(free_frame_list, i);

    free_TLB_list = (list_node *)malloc(sizeof(list_node *));
    free_TLB_list->next = NULL;
    for (int i = TLB_ENTRY_NUM - 1; i >= 0; i--)
        insert_node(free_TLB_list, i);
}

void clean()
{
    fclose(fp_addr);
    fclose(fp_out);
    fclose(fp_store);

    clean_stack(TLB_stack_head);
    clean_stack(frame_stack_head);
    clean_list(free_TLB_list);
    clean_list(free_frame_list);
}

int ger_frame_id(int page_id)
{
    int frame_id;
    frame_id = search_in_TLB(page_id);
    if (frame_id != -1)
    {
        return frame_id;
    }

    frame_id = search_in_page_table(page_id);
    return frame_id;
}

int search_in_TLB(int page_id)
{
    for (int i = 0; i < TLB_ENTRY_NUM; i++)
    {
        if (!TLB[i].valid)
            continue;

        if (TLB[i].page_id == page_id)
        {
            tlb_hit++;
            move_to_top(TLB_stack_head, TLB_stack_tail, i);
            move_to_top(frame_stack_head, frame_stack_tail, TLB[i].frame_id);
            return TLB[i].frame_id;
        }
    }
    return -1;
}

int search_in_page_table(int page_id)
{
    int frame_id;
    if (page_table[page_id].valid == 1)
    {
        frame_id = page_table[page_id].frame_id;
    }
    // page fault
    else
    {
        page_fault++;
        frame_id = search_unused_frame();

        // page replacement
        if (frame_id == -1)
        {
            int frame_to_be_replace = get_buttom_id(frame_stack_tail);
            for (int i = 0; i < PAGE_TABLE_ENTRY_NUM; i++)
            {
                if (page_table[i].frame_id == frame_to_be_replace)
                {
                    page_table[i].valid = 0;
                }
            }
            frame_id = frame_to_be_replace;
        }

        fseek(fp_store, page_id * PAGE_SIZE, SEEK_SET);
        fread(memory[frame_id].data, sizeof(char), FRAME_SIZE, fp_store);

        page_table[page_id].frame_id = frame_id;
        page_table[page_id].valid = 1;
    }

    move_to_top(frame_stack_head, frame_stack_tail, frame_id);

    // update TLB
    int TLB_id = search_unused_TLB();
    if (TLB_id == -1)
    {
        TLB_id = get_buttom_id(TLB_stack_tail);
    }
    else
    {
        TLB[TLB_id].valid = 1;
    }

    TLB[TLB_id].page_id = page_id;
    TLB[TLB_id].frame_id = frame_id;
    move_to_top(TLB_stack_head, TLB_stack_tail, TLB_id);

    return frame_id;
}

// return index of an unused frame, return -1 if no such frame
int search_unused_frame()
{
    int id = remove_node(free_frame_list);
    return id;
}

// return index of an unused TLB entry, return -1 if no such TLB entry
int search_unused_TLB()
{
    int id = remove_node(free_TLB_list);
    return id;
}
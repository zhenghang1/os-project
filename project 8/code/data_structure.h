#define PAGE_TABLE_ENTRY_NUM 256
#define PAGE_SIZE 256
#define TLB_ENTRY_NUM 32
#define FRAME_SIZE 256
#define FRAME_NUM 256

typedef struct TLB_ITEM
{
    int valid; // valid bit
    int frame_id;
    int page_id;
} tlb_item;

typedef struct PAGE
{
    int valid; // valid bit
    int frame_id;
} page;

typedef struct FRAME
{
    char data[FRAME_SIZE];
} frame;

/****************************** stack begin *********************************/

typedef struct STACK_NODE
{
    int id;
    struct STACK_NODE *prior;
    struct STACK_NODE *next;
} stack_node;

//move the node with id to the top of the stack (means it is the latest id been visited)
void move_to_top(stack_node *head, stack_node *tail, int id);

//get the bottom node's id (means it's the last recently visited id) 
int get_buttom_id(stack_node *tail);

//release the memory
void clean_stack(stack_node *head);

/****************************** stack end *********************************/

/****************************** list begin ********************************/

typedef struct LIST_NODE
{
    int id;
    struct LIST_NODE *next;
} list_node;

//insert a new free node with id
void insert_node(list_node *head, int id);

//remove a free node (means it is selected to be used)
int remove_node(list_node *head);

//release the memory
void clean_list(list_node *head);

/****************************** list end *********************************/
//#include <stdlib.h>
#include "buffer.h"

// circular queue implemented with array
buffer_item buf[BUFFER_SIZE + 1];
int front, rear;

// insert an item to the buffer
int insert_item(buffer_item item)
{
	if ((rear + 1) % (BUFFER_SIZE + 1) == front)
	{
		return -1;
	}
	rear = (rear + 1) % (BUFFER_SIZE + 1);
	buf[rear] = item;
	return 0;
}

// remove an item from the buffer
int remove_item(buffer_item *item)
{
	if (front == rear)
	{
		return -1;
	}
	front = (front + 1) % (BUFFER_SIZE + 1);
	*item = buf[front];
	return 0;
}

void buffer_init()
{
	front = 0;
	rear = 0;
}
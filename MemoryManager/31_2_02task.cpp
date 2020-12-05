#include <stdio.h>
#include <iostream>
#include <string>
#include <vector>
#include <algorithm>
#include "os_mem.h"

using namespace std;

vector<int> stack; 
int isCreated = 0;
int freeSize = 0;
int fullSize = 0;

int my_create(int size, int num_pages)
{
    if(size > 0 && !isCreated)
    {
        fullSize = size;
        freeSize = size;
        isCreated = 1;
        return 1;
    }
    return 0;
}

int my_destroy()
{
    if(!isCreated)
        return 0;
    else
    {
        stack.clear();//удаляем вектор
        fullSize = 0;
        freeSize = 0;
        isCreated = 0;
        return 1;
    }
}

mem_handle_t my_alloc(int block_size)
{
    int temp;
    if(isCreated && freeSize >= block_size)
    {
        temp = fullSize - freeSize;
        freeSize -= block_size;
        stack.push_back(block_size);
        return mem_handle_t(temp, block_size);
    }
    return mem_handle_t(0, 0);
}

int my_free(mem_handle_t h)
{
    if(isCreated && stack.size() > 0)
    {
        freeSize += stack.back();
        stack.pop_back();
        return 1;
    }
    return 0;
}

int my_get_max_block_size()
{
    if(isCreated)
        return freeSize;
    return 0;
}

int my_get_free_space()
{
    if(isCreated)
        return freeSize;
    return 0;
}

void my_print_blocks()
{
    int temp = 0;
    for(int i = 0; i < stack.size(); i++)
    {
        cout << temp << " " << stack[i] << endl;
        temp += stack[i];
    }
}

void setup_memory_manager(memory_manager_t* mm)
{
    mm->create = my_create;
    mm->destroy = my_destroy;
    mm->alloc = my_alloc;
    mm->free = my_free;
    mm->get_max_block_size = my_get_max_block_size;
    mm->get_free_space = my_get_free_space;
    mm->print_blocks = my_print_blocks;
}
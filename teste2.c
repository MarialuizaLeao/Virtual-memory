#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <ctype.h>

typedef struct t_page{
    unsigned id;
    struct t_page *next;
    struct t_page *prev;
    bool ref;
    bool changed;
}t_page;

typedef struct{
    t_page *head;
    t_page *tail;
    t_page *current;
    int size;
}t_list;

int numFrames, numMemAccess = 0, numPageFaults = 0, diskAccess = 0;
t_list *list;

void FIFO(t_page *page);
void secondChance(t_page *page);
void LRU(t_page *page);
void LRUAlreadyInList(t_page *page, t_page *prev, t_page *next);
void randomAlg(t_page *page);
t_page* searchPage(t_page *page);
void addNewPage(t_page *page, char alg[]);
void checkPage(t_page *page);

void printList(){
    t_page *aux = list->head;
    for(int i = 0; i < list->size; i++){
        printf("%x     %x       %x     %d\n", aux->prev->id, aux->id, aux->next->id, aux->ref);
        aux = aux->next;
    }
    printf("\n ------------------------------------\n");
}
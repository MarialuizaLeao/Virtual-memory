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

void printList(){
    t_page *aux = list->head;
    for(int i = 0; i < list->size; i++){
        printf("%x     %x       %x     %d\n", aux->prev->id, aux->id, aux->next->id, aux->ref);
        aux = aux->next;
    }
    printf("\n ------------------------------------\n");
}


int main(int argc, char* argv[]){
    FILE *file;
    char *alg = argv[1];
    file = fopen(argv[2], "r");
    int pageSize = atoi(argv[3]), memSize = atoi(argv[4]);
    numFrames = memSize/pageSize;
    list = malloc(sizeof(t_list));
    list->head = NULL;
    list->tail = NULL;
    list->size = 0;
    unsigned addr;
    char rw;
    while(fscanf(file, "%x %c", &addr, &rw) != -1){
        numMemAccess++;
        t_page *page = malloc(sizeof(t_page));
        page->id = addr;
        page->ref = true;
        page->changed = false;
        page->next = page;
        page->prev = page;
        if(tolower(rw) == 'w'){
            t_page *aux = searchPage(page);
            page->changed = true;
            if(aux != NULL){ // if page is already in memory and we're using LRU
                    if(strcmp(alg, "lru") == 0){
                        LRUAlreadyInList(page, aux->prev, aux->next); // update the LRU list
                    }
                    if(strcmp(alg, "2a") == 0){
                        list->current = aux->prev;
                        printf("current: %x\n", list->current->id);
                    }
                }
            else{
                addNewPage(page, alg); // add page to memory
                numPageFaults++;
            }
        }
        else{
            if(tolower(rw) == 'r'){
                t_page *aux = searchPage(page);
                if(aux != NULL){ // if page is already in memory and we're using LRU
                    if(strcmp(alg, "lru") == 0){
                        LRUAlreadyInList(page, aux->prev, aux->next); // update the LRU list
                    }
                    if(strcmp(alg, "2a") == 0){
                        list->current = aux->prev;
                        printf("current: %x\n", list->current->id);
                    }
                }
                else{
                    addNewPage(page, alg); // add page to memory
                    numPageFaults++;
                }
            }
            else{
                printf("Error: invalid read/write\n");
                return 1;
            }
        }
        printList();
    }
    fclose(file);
    printf("\nExecutando o simulador...\n");
	printf("Tamanho da memoria: %iKB\n", memSize);
	printf("Tamanho das paginas: %iKB\n", pageSize);
	printf("Tecnica de reposicao: %s\n", alg);
	printf("Numero de paginas: %i\n", numFrames);
	printf("Numero de acessos à memória: %i\n", numMemAccess);
	printf("Page fault: %i\n", numPageFaults);
	printf("Numero de acessos ao disco: %i\n", diskAccess);

}

void LRUAlreadyInList(t_page *page, t_page *prev, t_page *next){
    if(page->id != list->head->id){ // if the page is not the head of the list
        if(page->id != list->tail->id){ // if the page is not the tail of the list
            prev->next = next;
            next->prev = prev;
        }
        else{ // if the page is the tail of the list
            list->tail = prev; // new tail is the previous page's prev
        }
        t_page* aux = list->head;
        page->next = aux; // add page to the head of the list
        list->head = page;
        list->tail->next = list->head;
        list->head->prev = list->tail;
    }
}

void LRU(t_page *page){
    if(list->size == numFrames){ // if there is no space in memory
        t_page *aux1 = list->tail, *aux2 = list->head;
        if(aux1->changed){ // if the page was changed
            diskAccess++;
        }
        aux2->prev = page;
        page->next = aux2; // add page to the head of the list
        list->head = page;
        list->head = page;
        list->head->prev = list->tail;
        list->tail->next = list->head;
    }
    else{ // if there is space in memory
        if(list->size == 0){
            list->head = page;
            list->tail = page;
            list->size++;
        }
        else{
            t_page* aux = list->head;
            page->next = aux; // add page to the head of the list
            list->head = page;
            list->head->prev = list->tail;
            list->tail->next = list->head; 
            list->size++;
        }
    }
}

void addToList(t_page *page){
    if(list->size == 0){ // if the list is empty
        list->head = page;
        list->tail = page;
        list->current = list->tail;
    }
    else if(list->size == 1){ // if the list has only one element
        list->head = page;
        list->head->next = list->tail;
        list->head->prev = list->tail;
        list->tail->next = list->head;
        list->tail->prev = list->head;
        list->current = list->tail;
    }
    else{
        t_page* aux = list->head;
        aux->prev = page;
        page->next = aux;
        list->head = page;
        list->head->prev = list->tail;
        list->tail->next = list->head; 
        list->current = list->tail;
    }
}

void removeElement(t_page *page){
    if(page->id == list->head->id){ // if the page is the head of the list
        list->head = page->next;
        list->head->prev = list->tail;
        list->tail->next = list->head;
    }
    else if(page->id == list->tail->id){ // if the page is the tail of the list
        list->tail = page->prev;
        list->tail->next = list->head;
        list->head->prev = list->tail;
    }
    else{ // if the page is in the middle of the list
        t_page *aux1 = page->prev, *aux2 = page->next;
        aux1->next = aux2;
        aux2->prev = aux1;
    }
}

void secondChance(t_page *page){
    if(list->size == numFrames){ // if there is no space in memory
        t_page *aux1 = list->current;
        bool found = false;
        if(aux1->changed){ // if the page was changed
            diskAccess++;
        }
        while(!found){
            if(aux1->ref == false){
                found = true;
                aux1->id = page->id;
                aux1->ref = true;
                list->current = aux1->prev;
            }
            else{
                aux1->ref = false;
                aux1 = aux1->prev;
            }
        }
    }
    else{ // if there is space in memory
        addToList(page);
        list->size++;
    }
}

void FIFO(t_page *page){
    if(list->size == numFrames){ // if there is no space in memory
        t_page *aux1 = list->tail, *aux2 = list->head;
        if(aux1->changed){ // if the page was changed
            diskAccess++;
        }
        removeElement(aux1);
        addToList(page);
        
    }
    else{ // if there is space in memory
        addToList(page);
        list->size++;
    }
}

void randomAlg(t_page *page){
    int index = rand() % list->size;
    t_page *aux = list->head;
    for(int i = 0; i < index; i++){
        aux = aux->next;
    }
    strcpy(aux->id, page->id);
}

t_page* searchPage(t_page *page){
    if(list->size == 0){
        return NULL;
    }
    t_page *aux = list->head;
    int count = 0;
    while(!(aux->id == page->id) && count < list->size){
        aux = aux->next;
        count++;
    }
    if(aux->id == page->id){
        aux->ref = true;
        return aux;
    }
    else{
        return NULL;
    }
}

void addNewPage(t_page *page, char alg[]){
    diskAccess++;
    if(strcmp(alg, "lru") == 0){
        LRU(page);
    }
    else if(strcmp(alg, "fifo") == 0) {
        FIFO(page);
    }
    else if(strcmp(alg, "2a") == 0){
        secondChance(page);
    }
    else if(strcmp(alg, "random") == 0){
        randomAlg(page);
    }
    else{
        printf("Error: invalid algorithm\n");
    }
}
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
        printf("%x \n", aux->id);
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
        printList();
        if(tolower(rw) == 'w'){
            t_page *aux = searchPage(page);
            page->changed = true;
            if(aux != NULL){ // if page is already in memory and we're using LRU
                    if(alg == "lru"){
                        LRUAlreadyInList(page, aux->prev, aux->next); // update the LRU list
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
                    if(alg == "lru"){
                        LRUAlreadyInList(page, aux->prev, aux->next); // update the LRU list
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
    if(prev->id != list->head->id){ // if the page is not the head of the list
        if(next->id != list->tail->id){ // if the page is not the tail of the list
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

void secondChance(t_page *page){
    if(list->size == numFrames){ // if there is no space in memory
        t_page *aux1 = list->tail, *aux2 = list->head;
        bool found = false;
        if(aux1->changed){ // if the page was changed
            diskAccess++;
        }
        while(!found){
            if(aux1->ref == 0){
                found = true;
            }
            else{
                aux1->ref = 0;
                aux1 = aux1->prev;
            }
        }
        aux1->next->prev = aux1->prev; // remove aux1 from memory
        aux1->prev->next = aux1->next;
        page->next = aux2; // add page to the head of the list
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

void FIFO(t_page *page){
    if(list->size == numFrames){ // if there is no space in memory
        t_page *aux1 = list->tail, *aux2 = list->head;
        if(aux1->changed){ // if the page was changed
            diskAccess++;
        }
        aux1->next = NULL; // remove the last page from memory
        list->tail = aux1;
        page->next = aux2; // add page to the head of the list
        list->head = page;
        free(aux1);
        free(aux2);
        
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
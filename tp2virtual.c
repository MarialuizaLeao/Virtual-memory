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

typedef struct t_list{
    t_page *head;
    t_page *tail;
    t_page *current;
    int size;
}t_list;

int numFrames, numMemAccess = 0, numPageFaults = 0, diskAccess = 0;
t_page *SecondChancePage;
t_list *list;

void FIFO(t_page *page);
void secondChance(t_page *page);
void LRU(t_page *page);
void LRUAlreadyInList(t_page *page, t_page *prev, t_page *next);
void randomAlg(t_page *page);

t_page* searchPage(t_page *page);
void addNewPage(t_page *page, char alg[]);
void updatePage(t_page *page, char alg[]);
int findS(int PageSize);

void removeElement();
void addElement(t_page *page);

void printList(){
    t_page *aux = list->head;
    for(int i = 0; i < list->size; i++){
        printf("%x     %x       %x     %d\n", aux->prev->id, aux->id, aux->next->id, aux->ref);
        aux = aux->next;
    }
    printf("\n ------------------------------------\n");
}

int findS(int PageSize){
    int s = 0;
    int temp = PageSize * 1024;
    while(temp > 1){
        temp = temp >> 1;
        s++;
    }
    return s;
}

int main(int argc, char* argv[]){
    FILE *file;
    char *alg = argv[1];
    file = fopen(argv[2], "r");
    int pageSize = atoi(argv[3]), memSize = atoi(argv[4]);
    numFrames = memSize/pageSize;
    int s = findS(pageSize);

    SecondChancePage = malloc(sizeof(t_page));
    list = malloc(sizeof(t_list));
    list->head = NULL;
    list->tail = NULL;
    list->size = 0;
    SecondChancePage = list->head;
    unsigned addr;
    char rw;
    while(fscanf(file, "%x %c", &addr, &rw) != -1){
        numMemAccess++;
        t_page *page = malloc(sizeof(t_page));
        page->id = addr >> s;
        page->ref = true;
        page->changed = false;
        page->next = page;
        page->prev = page;

        t_page *aux = searchPage(page);
        if(tolower(rw) == 'w'){
            page->changed = true;
        }
        else if(tolower(rw) == 'r'){
            if(aux != NULL && aux->changed){
                page->changed = true;
            }
        }
        else{
            printf("Error: invalid read/write\n");
            return 1;
        }
        if(aux != NULL){ // if page is already in memory
            page->prev = aux->prev;
            page->next = aux->next;
            updatePage(page, alg);
        }
        else{ // if page is not in memory
            addNewPage(page, alg); // add page to memory
            numPageFaults++;
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
	printf("Escritas no disco: %i\n", diskAccess);
}

void addElement(t_page *page){
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

void removeElement(){
    list->tail = list->tail->prev;
    list->tail->next = list->head;
    list->head->prev = list->tail;
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
        addElement(page);
    }
}

void LRU(t_page *page){
    if(list->size == numFrames){ // if there is no space in memory
        if(list->tail->changed){ // if the page was changed
            diskAccess++;
        }
        removeElement();
        addElement(page);
    }
    else{ // if there is space in memory
        addElement(page);
        list->size++;
    }
}

void secondChance(t_page *page){
    if(list->size == numFrames){ // if there is no space in memory
        t_page *aux = SecondChancePage;
        bool found = false;
        while(!found){
            if(aux->ref == false){
                if(aux->changed){ // if the page was changed
                    diskAccess++;
                }
                found = true;
                aux->id = page->id;
                aux->ref = true;
                aux->changed = page->changed;
                list->current = aux->prev;
                SecondChancePage = aux->next;
            }
            else{
                aux->ref = false;
                aux = aux->prev;
            }
            SecondChancePage = aux;
        }
    }
    else{ // if there is space in memory
        addElement(page);
        if(SecondChancePage == NULL) {
            SecondChancePage = list->head;
        }
        list->size++;
    }
}

void FIFO(t_page *page){
    if(list->size == numFrames){ // if there is no space in memory
        if(list->tail->changed){ // if the page was changed
            diskAccess++;
        }
        removeElement();
        addElement(page);
        
    }
    else{ // if there is space in memory
        addElement(page);
        list->size++;
    }
}

void randomAlg(t_page *page){
    if(list->size == numFrames){ // if there is no space in memory
        int index = rand() % list->size;
        t_page *aux = list->head;
        for(int i = 0; i < index; i++){
            aux = aux->next;
        }
        if(aux->changed){ // if the page was changed
            diskAccess++;
        }
        aux->id = page->id;
        aux->ref = true;
        aux->changed = page->changed;
    }
    else{ // if there is space in memory
        addElement(page);
        list->size++;
    }
}

// Returns the page if it is in memory, NULL otherwise
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

void updatePage(t_page *page, char alg[]){
    if(strcmp(alg, "lru") == 0){ // if we're using LRU
        LRUAlreadyInList(page, page->prev, page->next); // update the LRU list
    }
    if(strcmp(alg, "2a") == 0){ // if we're using 2a
        list->current = page->prev; // update the current page
    }
}
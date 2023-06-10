#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>
#include <stdbool.h>

typedef struct{
    char id[9];
    t_page *next;
}t_page;

typedef struct{
    t_page *head;
    t_page *tail;
    int size;
}t_list;

t_list *list;

void FIFO(t_page *page, int numFrames);
void secondChance(t_page *page, int numFrames);
void LRU(t_page *page, int numFrames);
void random(t_page *page, int numFrames);
void addPage(t_page *page);
bool searchPage(char id[], char alg[]);
void removePage(t_page *page, char alg[], int numFrames);

int main(int argc, char* argv[]){
    FILE *file;
    char alg[] = argv[1];
    char fileName[] = argv[2];
    int pageSize = atoi(argv[3]), memSize = atoi(argv[4]), numFrames = memSize/pageSize;

    list->head = NULL;
    list->tail = NULL;
    list->size = 0;

    file = fopen(fileName, "r");
    unsigned addr;
    char rw;
    while(fscanf(file, "%x %c", &addr, &rw) != -1){
        char id[9];
        strncpy(id, addr, 8);
        id[8] = '\0';
        t_page *page = malloc(sizeof(t_page));
        strcpy(page->id, id);
        if(tolower(rw) == 'w'){

            if(list->size < numFrames){
                if(searchPage(id, alg) == false){
                    addPage(page);
                }
            }
            else{
                removePage(page, alg, numFrames);
            }
        }
        else{
            if(tolower(rw) == 'r'){
                if(!searchPage(id, alg)){
                    if(list->size < numFrames){
                        addPage(page);
                    }
                    else{
                        removePage(page, alg, numFrames);
                    }
                }
            }
            else{
                printf("Error: invalid read/write\n");
                return 1;
            }
        }
    }
    fclose(file);
}

void LRU(t_page *page, int numFrames){
    addPage(page);
    if(list->size == numFrames){
        t_page *aux = list->head;
        list->head = aux->next;
        free(aux);
    }
}

void secondChance(t_page *page, int numFrames){

}

void FIFO(t_page *page, int numFrames){
    addPage(page);
    if(list->size == numFrames){
        t_page *aux = list->head;
        list->head = aux->next;
        free(aux);
    }
}

void random(t_page *page, int numFrames){
    int index = rand() % list->size;
    t_page *aux = list->head;
    for(int i = 0; i < index; i++){
        aux = aux->next;
    }
    strcpy(aux->id, page->id);
}

void addPage(t_page *page){
    t_page *aux = list->head;
    if(aux == NULL){
        list->head = page;
        list->tail = list->head;
        list->size++;
    }
    else{
        list->tail->next = page;
        list->tail = page;
        list->size++;
    }
}

bool searchPage(char id[], char alg[]){
    t_page *aux = list->head, *prev = NULL;
    while(aux != NULL){
        if(strcmp(aux->id, id) == 0){
            if(strcmp(alg, "lru") == 0){
                if(prev != NULL){
                    if(aux->next != NULL){
                        prev->next = aux->next;
                    }
                }
                else{
                    list->head = list->head->next;
                }
                list->tail->next = aux;
                list->tail = aux;
                aux->next = NULL;
            }
            return true;
        }
        prev = aux;
        aux = aux->next;
    }
    return false;
}

void removePage(t_page *page, char alg[], int numFrames){
    if(strcmp(alg, "lru") == 0){
		LRU(page, numFrames);
	}
	else{
        if(strcmp(alg, "2a") == 0){
            secondChance(page, numFrames);
        }
        else{
            if(strcmp(alg, "fifo") == 0){
                FIFO(page, numFrames);
            }
            else{
                if(strcmp(alg, "random") == 0){
                    random(page, numFrames);
                }
            }
        }
    }
}
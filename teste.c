#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <ctype.h>
/*essas variaveis globais compoem as estatisticas
pedidas no enunciado do trabalho*/
int totalUsed = 0;
int faults = 0;

int AcessosMem = 0; //linhas no arquivo
int AcessosDisc = 0; //gravar no disc a que vai ser sub e pegar do disc a q vaiu sub

typedef struct Page_t {
	unsigned addr;
	struct Page_t *next;	
    struct Page_t *prev;
    int ref;
    int w;
}Page_t;

Page_t *first, *last, *current, *secondChancePage;

void AddNew(unsigned addr, int modificada){
    Page_t *latest = (Page_t*)malloc(sizeof(Page_t));
    latest->addr = addr;
    latest->next = latest;
    latest->prev = latest;
    latest->ref = 1;
    if(modificada == 1){
        latest->w = 1;
    }
    else{
        latest->w = 0;
    }

    if(totalUsed == 0){
        first = latest;
        last = latest;
        current = first;
    }
    else{
        if(totalUsed == 1){
            last = latest;
            last->next = first;
            last->prev = first;
            first->next = last;
            first->prev = last;
            current = first;
        }
        else{
            Page_t *aux = last;
            aux->next = latest;
            latest->prev = aux;
            last = latest;
            last->next = first;
            first->prev = last;
            current = first;
        }
        
    }
    totalUsed++;
}
void FIFO(unsigned addr, int modificada){
    //criar a pagina que vai subtituir a selecionada
    Page_t *new = (Page_t*)malloc(sizeof(Page_t));
    new->addr = addr;
    new->next = NULL;
    new->prev = NULL;
    if(modificada==1){
        new->w = 1;
    }
    //retiramos a primeira pagina da fila
    if(first->w == 1){
        AcessosDisc++;
        first->w = 0;
    }
    first = first->next;
    //colocamos a nova pagina no final
    last->next = new;
    new->prev = last;
    new->next = first;
    last = new;
    first->prev = last;

}

void lruListModifier(unsigned addr, int modificada){
    //pag_addr = pagina do endereco lido que ja estava na lista
    Page_t *aux = first;
    //procura a pagina na lista e tira ela de la, arrumando os ponteiros
    //de que ta antes dela e de quem ta depois
    
    for(int i = 0; i < totalUsed; i++)
    {
        if(aux->addr == addr){
            if(modificada == 1){
                aux->w = 1;
            }
            break;
        }
        aux = aux->next;
    }

    if(aux->addr != last->addr){
        if(aux->addr == first->addr){
            first = first->next;
        }
        else{
            //coloca ela no final da fila
            aux->prev->next = aux->next;
            aux->next->prev = aux->prev;
            
        }
        aux->prev = last;
        aux->next = first;
        last->next = aux;
        last = aux;
        first->prev = last;
    }
}

void LRU(unsigned addr, int modificada){
    //cria pagina com o novo end
    Page_t *new = (Page_t*)malloc(sizeof(Page_t));
    new->addr = addr;
    if(modificada == 1){
        new->w = 1;
    }
    //retira a pagina mais antiga
    if(first->w == 1){
        AcessosDisc++;
        first->w = 0;
    }
    first = first->next;

    //coloca a pagina nova no topo da fila
    last->next = new;
    new->prev = last;
    new->next = first;
    last = new;

    first->prev = last;

}

void secondChance(unsigned addr, int modificada){
    Page_t *new = secondChancePage;
    int found = 1;
   
    while(found){
        //se o que esta sendo olhado pelo ponteiro for 1 da a segunda
        //chance e troca para 0 
        if(new->ref == 1){
            new->ref = 0;
        }
        //se for 0, tira ele da fila e coloca o outro no lugar.
        //na proxima vez que trocar de pagina, comeca nesse novo (current)
        else{
            if(new->w == 1){
                AcessosDisc++;
                new->w = 0;
            }

            new->addr = addr;
            new->ref = 1;
            if(modificada == 1){
                new->w = 1;
            }
            secondChancePage = new->next;
            found = 0;
            
            break;
        }
        new = new->next;

        secondChancePage = new;
    }
}

void randomAlg(unsigned addr, int totalPages, int modificada){
    int pag_del = rand() % totalPages;
    Page_t *new = first;
    int i = 0;
    while(i < pag_del){
        new = new->next;
        i++;
    }
    if(new->w == 1){
        AcessosDisc++;
        new->w = 0;
    }
    new->addr = addr;
    if(modificada == 1){
        new->w = 1;
    }
}

void writeAddress(unsigned addr, char *sub, int totalPages, int modificada){
    if(strcmp(sub, "lru") == 0){
        LRU(addr, modificada);
    }
    else if(strcmp(sub, "fifo") == 0){
        FIFO(addr, modificada);
    }
    else if(strcmp(sub, "2a") == 0){
        secondChance(addr, modificada);
    }
    else if(strcmp(sub, "random") == 0){
        randomAlg(addr, totalPages, modificada);
    }
}

Page_t* AlreadyWritten(unsigned address){
    Page_t *aux = first;
    for(int i = 0; i < totalUsed; i++)
    {
        if(aux->addr == address){
            aux->ref = 1;
            current = aux->next;
            return aux;
        }
        aux = aux->next;
    }
    return NULL;
}

void printList(){
    Page_t *aux = first;
    for(int i = 0; i < totalUsed; i++){
        printf("%X      %x       %x     %d\n", aux->prev->addr, aux->addr, aux->next->addr, aux->ref);
        aux = aux->next;
    }
    printf("\n ------------------------------------\n");
}

int findS(int pag){
    int s = 0;
    int temp = pag * 1024;
    while(temp > 1){
        temp = temp >> 1;
        s++;
    }
    return s;
}

int main(int argc, char* argv[]){
    FILE *file; 
    unsigned addr;
    char rw;
    char *sub = argv[1];
    file = fopen(argv[2], "r");
    int pag = atoi(argv[3]);
    int mem = atoi(argv[4]); 
    // char *sub = "lru";
    // file = fopen("teste2a.log", "r");
    // int pag = 4;
    // int mem = 12;
    int s = findS(pag);
    int totalPages = mem/pag;
    first = (Page_t*)malloc(sizeof(Page_t));
    last = (Page_t*)malloc(sizeof(Page_t));
    secondChancePage = (Page_t*)malloc(sizeof(Page_t));
    first = NULL;
    last = NULL;
    secondChancePage = first;
    while(fscanf(file,"%x %c",&addr,&rw) != -1){
        int modificada = 0;
        //printf("%x",addr);
        AcessosMem++;
        addr = addr >> s;
        Page_t* pag_addr = AlreadyWritten(addr);
        if(rw == 'W' || rw == 'w'){
            modificada = 1;
            if(pag_addr == NULL){
                if(totalUsed < totalPages){
                    AddNew(addr, modificada);
                    if(secondChancePage == NULL) {
                        secondChancePage = first;
                    }
                }
                else{
			        writeAddress(addr, sub, totalPages, modificada);
                }
                faults++;
            }
            /*caso a pagina a ser escrita ja tiver na lista de paginas
            e o algoritmo usado for LRU, sera necessário colocar a pagina no topo
            da pilha(modificacao na lista anterior) já que ela foi novamente
            referenciada
            */
            else if(strcmp(sub, "lru") == 0){
                lruListModifier(addr, modificada);
            }
		}
        else if(rw == 'R' || rw == 'r'){
            if(pag_addr == NULL){
                if(totalUsed < totalPages){
                    AddNew(addr, modificada);
                    if(secondChancePage == NULL) {
                        secondChancePage = first;
                    }
                }else{
                    writeAddress(addr, sub, totalPages, modificada);
                }
                faults++;
            }
            else if(strcmp(sub, "lru") == 0){
                lruListModifier(addr, modificada);
            }
        }
        //printList();
    }
    fclose(file);
    printf("\nExecutando o simulador...\n");
    printf("Arquivo de entrada: %s\n", argv[2]);
    printf("Tamanho da memoria: %iKB\n", mem);
    printf("Tamanho das paginas: %iKB\n", pag);
    printf("Tecnica de reposicao: %s\n", sub);
    printf("Numero de paginas: %i\n", totalPages);
    printf("Numero de acessos à memória: %i\n", AcessosMem);
    printf("Paginas lidas: %i\n", faults);
    printf("Paginas escritas: %i\n", AcessosDisc);
}
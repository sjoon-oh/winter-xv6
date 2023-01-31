#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <ranlib.h>
#include <time.h>

#define BUCKET_SIZE 10
#define CMD_SIZE 200000

struct node {
    int key;
    int value;
    struct node* next;
};

struct bucket {
    struct node* head;
    int count;
    pthread_mutex_t lock;
};

struct cmd {
    int cmdid;
    int key;
    int value;
    int done;
    pthread_mutex_t cmdlock;
};

struct bucket* hashTable = NULL;
struct cmd* cmdarr;

struct node* createNode(int key, int value) {
    struct node* newNode;
    newNode = (struct node *) malloc(sizeof(struct node));
    newNode->key = key;
    newNode->value = value;
    newNode->next = NULL;

    return newNode;
}

int hashFunction(int key){
    return key%BUCKET_SIZE;
}

void add(int key, int value){
    int hashIndex = hashFunction(key);
    struct node* newNode = createNode(key, value);

    pthread_mutex_lock(&hashTable[hashIndex].lock);
    if(hashTable[hashIndex].count == 0){
        hashTable[hashIndex].head = newNode;
        hashTable[hashIndex].count = 1;
    }else{
        newNode->next = hashTable[hashIndex].head;
        hashTable[hashIndex].head = newNode;
        hashTable[hashIndex].count++;
    }
    pthread_mutex_unlock(&hashTable[hashIndex].lock);
}

void remove_key(int key){
    int hashIndex = hashFunction(key);
    struct node* iter;
    struct node* before;
    int flag = 0;

    pthread_mutex_lock(&hashTable[hashIndex].lock);
    iter = hashTable[hashIndex].head;
    while(iter != NULL){
        if(iter->key == key){
            if(iter == hashTable[hashIndex].head){
                hashTable[hashIndex].head = iter->next;
            }else{
                before->next = iter->next;
            }
            hashTable[hashIndex].count--;
            free(iter);
            flag = 1;
        }
        before = iter;
        iter = iter->next;
    }
    pthread_mutex_lock(&hashTable[hashIndex].lock);
    /*if (flag) {
        printf("%d is deleted\n", key);
    } else {
        printf("%d is not searched\n", key);
    }*/
}

void search(int key){
    struct node* iter;
    int flag = 0;
    int hashIndex = hashFunction(key);
    pthread_mutex_lock(&hashTable[hashIndex].lock);
    iter = hashTable[hashIndex].head;

    while(iter != NULL){
        if(iter->key == key){
            flag = 1;
            break;
        }
        iter = iter->next;
    }
    /*if (flag){
        printf("value of %d key : %d\n", iter->key, iter->value);
    }else{
        printf("No key\n");
    }*/
    pthread_mutex_unlock(&hashTable[hashIndex].lock);
}

void display(void){
    struct node* iter;
    int i;
    printf("\n======== display start ========\n");
    for (i = 0; i < BUCKET_SIZE; i++){
        iter = hashTable[i].head;
        printf("Bucket[%d] : ", i);
        while (iter != NULL) {
            printf("(key : %d, val : %d)  -> ", iter->key, iter->value);
            iter = iter->next;
        }
        printf("\n");
    }
    printf("======== display end ========\n\n");
}
void *threadworker(void *ptr);

int main(int argc, char *argv[]) {
    pthread_t *arr;
    int *thread_ids;
    int tot_thread;
    int i;
    clock_t start = clock();
    clock_t end;

    // Argument parsing and Initializing global variables
    if (argc == 1){
        printf("No number of thread options\n");
        return 1;
    }else if (argc == 2){
        tot_thread = atoi(argv[1]);
        printf("Number of thread : %d\n", tot_thread);
        arr = (pthread_t *)malloc(sizeof(pthread_t)*tot_thread);
        thread_ids = (int *) malloc(sizeof(int)*tot_thread);
        hashTable = (struct bucket *) malloc(sizeof(struct bucket)*BUCKET_SIZE);
        for(i = 0; i < BUCKET_SIZE; i++){
            pthread_mutex_init(&hashTable[i].lock, NULL);
        }
    }else {
        printf("Too many argument\n");
        return 1;
    }
    srand(time(NULL));

    // Make Commands
    cmdarr = (struct cmd *) malloc(sizeof(struct cmd)*CMD_SIZE);
    for (i = 0; i < CMD_SIZE; i++){
        if (i % 2 == 0){
            cmdarr[i].cmdid = 0;
        }else{
            cmdarr[i].cmdid = 1;
        }
        cmdarr[i].value = rand();
        cmdarr[i].key = rand();
        cmdarr[i].done = 0;
        pthread_mutex_init(&cmdarr[i].cmdlock, NULL);
    }

    // Thread working
    for (i = 0; i < tot_thread; i++){
        thread_ids[i] = pthread_create(&arr[i], NULL, threadworker, NULL);
    }

    // Wait Thread
    for(i = 0; i < tot_thread; i++){
        pthread_join(arr[i], NULL);
    }

    end = clock();
    //display();

    free(cmdarr);
    free(hashTable);
    free(thread_ids);
    free(arr);

    printf("execution time: %lf\n", (double)(end-start)/CLOCKS_PER_SEC);

    return 0;
}

void *threadworker(void *ptr){
    int i;
    int random;
    int key, value;

    for(i = 0; i < CMD_SIZE; i++) {
        pthread_mutex_lock(&cmdarr[i].cmdlock);
        if (cmdarr[i].done) {
            pthread_mutex_unlock(&cmdarr[i].cmdlock);
            continue;
        }

        random = cmdarr[i].cmdid;
        key = cmdarr[i].key;
        value = cmdarr[i].value;
        cmdarr[i].done = 1;
        pthread_mutex_unlock(&cmdarr[i].cmdlock);

        if (random == 0) { // add action
            add(key, value);
        } else if (random == 1) { // remove action
            //remove_key(key);
            //}else{ //serach action
            search(key);
        }
    }
}

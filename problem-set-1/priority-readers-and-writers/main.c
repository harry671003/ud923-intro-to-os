#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include <time.h>
#include <stdlib.h>

#define MAX_READER_COUNT 5
#define MAX_WRITER_COUNT 5

int buffer;
int reader_count;
int writer_count;

void *reader(void *arg);
void *writer(void *arg);
int get_random_num();

pthread_mutex_t mutex;
pthread_cond_t write_cond;
pthread_cond_t read_cond;

int main(int argc, char** argv)
{
    srand(time(NULL));
    pthread_mutex_init(&mutex, NULL);

    // Initialize stuff.
    reader_count = 0;
    writer_count = 0;
    buffer = 0;

    pthread_t reader_threads[MAX_READER_COUNT];
    pthread_t writer_threads[MAX_WRITER_COUNT];
    
    // Create writer threads
    printf("[+] Creating writers: \n");
    for(int i = 0; i < MAX_READER_COUNT; i++)
    {
        pthread_create(
            &writer_threads[i],
            NULL,
            writer,
            NULL);
    }

    // Create reader threads
    printf("[+] Creating readers: \n");
    for(int i = 0; i < MAX_READER_COUNT; i++)
    {
        pthread_create(
            &reader_threads[i],
            NULL,
            reader,
            NULL);
    }

    // Join writer threads
    for(int i = 0; i < MAX_WRITER_COUNT; i++)
    {
        pthread_join(writer_threads[i], NULL);
    }

    // Join reader threads
    for(int i = 0; i < MAX_READER_COUNT; i++)
    {
        pthread_join(reader_threads[i], NULL);
    }

    return 0;
}

void *reader(void *arg) 
{
    sleep(get_random_num());

    pthread_mutex_lock(&mutex); // Lock

    while(writer_count > 0)
        pthread_cond_wait(&read_cond, &mutex);

    reader_count++;

    pthread_mutex_unlock(&mutex); // UnLock
    
    printf("READ => value %d | activeCount = %d\n", buffer, reader_count - 1);

    pthread_mutex_lock(&mutex); // Lock

    reader_count--;
    if(reader_count == 0)
        pthread_cond_broadcast(&write_cond);

    pthread_mutex_unlock(&mutex); // UnLock

    return NULL;
}

void *writer(void *arg) 
{
    sleep(get_random_num());
    
    int value = get_random_num();
    int writer_count;

    pthread_mutex_lock(&mutex); // Lock

    while(writer_count > 0 && reader_count > 0)
        pthread_cond_wait(&write_cond, &mutex);

    writer_count++;
    buffer = value;

    printf("WRITE => value = %d | activeCount = %d\n", value, writer_count - 1);

    writer_count--;

    // Tell other writing is done.
    pthread_cond_broadcast(&read_cond);
    pthread_cond_broadcast(&write_cond);

    pthread_mutex_unlock(&mutex); // UnLock


    return NULL;
}

int get_random_num()
{
    return (rand() % 5) + 1;
}

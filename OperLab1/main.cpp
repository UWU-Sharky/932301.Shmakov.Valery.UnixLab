#include <iostream>
#include <pthread.h>
#include <unistd.h>

pthread_cond_t cond1 = PTHREAD_COND_INITIALIZER; // условная переменная в которой хранится
pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER; // мьютекс, который реализует вход в защищенный фрагмент кода
int ready = 0;

void* pthread_provider(void* arg)
{
    while(true)
    {
    sleep(1);
    pthread_mutex_lock(&lock);
    if (ready == 1)
    {
        pthread_mutex_unlock(&lock);
        return 0;
    }
    ready = 1;
    printf("Передал\n");
    pthread_cond_signal(&cond1);
    pthread_mutex_unlock(&lock);
    }
    return NULL;
}

void* pthread_consumer(void* arg)
{
    while(true)
    {
    pthread_mutex_lock(&lock);
    while (ready == 0)
    {
        pthread_cond_wait(&cond1, &lock);
        printf("Проснулся\n");
    }
    ready = 0;
    printf("Получил\n");
    pthread_mutex_unlock(&lock);
    }
    return NULL;
}

int main()
{
    std::setlocale(0,"");
    pthread_t provider, consumer;
    pthread_create(&provider, NULL, pthread_provider, NULL);
    pthread_create(&consumer, NULL, pthread_consumer, NULL);
    pthread_join(provider,NULL);
    pthread_join(consumer,NULL);
    return 0;
}
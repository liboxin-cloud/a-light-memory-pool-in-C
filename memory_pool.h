#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include <stdbool.h>

#define MAX_BLOCK_SIZE 1024

typedef struct memory_block{
    void* __mem_start;
    size_t __size;
    size_t __used;
}memory_block_t;

typedef struct memory_pool {
    memory_block_t* mp_block;
    pthread_mutex_t mutex;
}memory_pool_t;

static void* mp_allocate(memory_pool_t* mp, size_t m_size);
static memory_pool_t* init_memory_pool();
static void destroy_memory_pool(memory_pool_t* mp);
static void free_memory(memory_pool_t* mp, const void* ptr);
static void free_all_memory(memory_pool_t* mp);
static bool is_mp_ptr(memory_pool_t* mp, void* ptr);

static memory_pool_t* init_memory_pool() {
    memory_pool_t* mp = (memory_pool_t*)malloc(sizeof(memory_pool_t));

    if (NULL == mp) {
        perror("mp_pool malloc failed\n");
        return NULL;
    }

    mp->mp_block = (memory_block_t*)malloc(sizeof(memory_block_t));

    if (NULL == mp->mp_block) {
        perror("mp_block malloc failed\n");
        free(mp);
        return NULL;
    }

    mp->mp_block->__size = MAX_BLOCK_SIZE;
    mp->mp_block->__mem_start = malloc(MAX_BLOCK_SIZE);
    mp->mp_block->__used = 0;

    if (0 != pthread_mutex_init(&mp->mutex, NULL)) {
        perror("mutex init failed\n");
        free(mp->mp_block->__mem_start);
        free(mp->mp_block);
        free(mp);
        return NULL;
    }
    return mp;

}
static void* mp_allocate(memory_pool_t* mp, size_t m_size) {
    if (NULL == mp) {
        printf("memory_pool is NULL\n");
        return NULL;
    }

    if (m_size <= 0 || m_size > MAX_BLOCK_SIZE) {
        printf("invaild memory allocate size\n");
        return NULL;
    }

    if (mp->mp_block->__size - mp->mp_block->__used < m_size) {
        printf("the memory pool is full\n");
        return NULL;
    }

    pthread_mutex_lock(&mp->mutex);

    void* current_pos = (char*)mp->mp_block->__mem_start + mp->mp_block->__used;
    mp->mp_block->__used += m_size;

    pthread_mutex_unlock(&mp->mutex);
    return current_pos;
}

static void destroy_memory_pool(memory_pool_t* mp) {
    if (NULL == mp) {
        printf("the memory pool is NULL\n");
        return;
    }

    pthread_mutex_lock(&mp->mutex);

    if (NULL == mp->mp_block) {
        return;
    }

    free(mp->mp_block->__mem_start);
    free(mp->mp_block);

    pthread_mutex_unlock(&mp->mutex);

    pthread_mutex_destroy(&mp->mutex);
    free(mp);
}


static void free_memory(memory_pool_t* mp, const void* ptr) {
    if (NULL == mp) {
        printf("the memory pool is NULL\n");
        return;
    }

    pthread_mutex_lock(&mp->mutex);

    if (is_mp_ptr(mp, (void*)ptr)) {
        mp->mp_block->__used = ((const char*)ptr - (const char*)mp->mp_block->__mem_start);
    } else {
        printf("your ptr is not in the memory pool, cannot be freed\n");
        pthread_mutex_unlock(&mp->mutex);
        return;
    }

    printf("free addr %p memory success\n", ptr);
    pthread_mutex_unlock(&mp->mutex);

}

static void free_all_memory(memory_pool_t* mp) {
    if (NULL == mp) {
        printf("the memory pool is NULL\n");
        return;
    }

    pthread_mutex_lock(&mp->mutex);

    if (NULL == mp->mp_block) {
        return;
    }

    free(mp->mp_block->__mem_start);
    free(mp->mp_block);

    mp->mp_block = (memory_block_t*)malloc(sizeof(memory_block_t));

    if (NULL == mp->mp_block) {
        perror("memory allocate failed\n");
        return;
    }
    mp->mp_block->__mem_start = malloc(MAX_BLOCK_SIZE);
    mp->mp_block->__used = 0;
    mp->mp_block->__size = MAX_BLOCK_SIZE;

    printf("free all allocated memory success\n");

    pthread_mutex_unlock(&mp->mutex);
}

static bool is_mp_ptr(memory_pool_t* mp, void* ptr) {
    if (NULL == mp) {
        printf("the memory pool is NULL\n");
        return false;
    }

    if (NULL == ptr) {
        printf("your ptr is NULL\n");
        return false;
    }

    return (((size_t)((const char*)ptr - (const char*)mp->mp_block->__mem_start) < mp->mp_block->__size) &&
    (((const char*)ptr - (const char*)mp->mp_block->__mem_start) >= 0));
}
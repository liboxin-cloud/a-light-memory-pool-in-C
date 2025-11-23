Simple Thread-Safe Memory Pool
A lightweight, thread-safe memory pool implementation in C, optimized for efficient small object allocation (≤ 1024B) and simplified memory management. It reduces the overhead of frequent malloc/free system calls for small objects and ensures thread safety via POSIX mutexes.
Table of Contents

    Features
    Technical Specifications
    File Structure
    API Reference
    Build & Run
    Usage Examples
    Key Notes & Limitations
    Error Handling

Features

    Thread Safety: Protected critical sections (allocation/deallocation) with POSIX mutexes (pthread_mutex), safe for multi-threaded environments.
    Small Object Optimization: Pre-allocates a fixed-size memory block (1024B) for small objects, avoiding frequent malloc/free overhead.
    Invalid Size Rejection: Rejects invalid allocation sizes (≤ 0 or > 1024B) to prevent misuse.
    Flexible Deallocation:
        Reverse-order freeing of individual pool objects.
        Full pool reset (free_all_memory) to release all pool memory and reuse the pool.
    Robust Validation: Checks for null pointers, invalid pointers, and memory allocation failures to avoid crashes.

Technical Specifications
Parameter	Value	Description
Maximum Pool Object Size	1024B (configurable)	Modify MAX_BLOCK_SIZE macro to adjust.
Allocation Range	1B ~ 1024B	Only valid for pool-managed allocation.
Thread Safety	Yes (POSIX mutex)	Requires pthread library.
Underlying Allocator	malloc/free	Used for pool initialization and reset.
Error Reporting	perror() + console messages	For allocation failures and invalid operations.
File Structure
plaintext

memory_pool/
├── memory_pool.h   # Struct definitions and function declarations
├── test.c          # Memory pool implementation + test suite
└── Makefile        # Build configuration (compile/run/clean)

API Reference
1. Initialization & Destruction
memory_pool_t* init_memory_pool()

    Description: Initializes the memory pool (allocates pool metadata, memory block, and mutex).
    Return Value: Pointer to memory_pool_t on success; NULL on failure (error message printed via perror).
    Usage: Must be called before any other memory pool operations.

void destroy_memory_pool(memory_pool_t* mp)

    Description: Destroys the memory pool and frees all resources (pool memory, metadata, mutex).
    Parameters:
        mp: Pointer to the initialized memory pool (from init_memory_pool).
    Usage: Call when the pool is no longer needed to avoid memory leaks.

2. Memory Allocation
void* mp_allocate(memory_pool_t* mp, size_t m_size)

    Description: Allocates memory from the pool (for small objects) or rejects invalid requests.
    Parameters:
        mp: Pointer to the initialized memory pool.
        m_size: Size of memory to allocate (must be 1B ~ 1024B).
    Return Value: Pointer to allocated memory on success; NULL on failure.
    Failure Scenarios:
        mp is NULL (prints "memory_pool is NULL").
        m_size ≤ 0 or > 1024B (prints "invaild memory allocate size").
        Pool is full (prints "the memory pool is full").

3. Memory Deallocation
void free_memory(memory_pool_t* mp, const void* ptr)

    Description: Frees a pool-allocated object or rejects non-pool pointers.
    Parameters:
        mp: Pointer to the memory pool.
        ptr: Pointer to the memory to free (must be a valid pool-allocated pointer).
    Critical Note: Pool objects must be freed in reverse allocation order (truncation-based deallocation).
    Failure Scenarios:
        mp is NULL (prints "the memory pool is NULL").
        ptr is NULL (prints "your ptr is NULL").
        ptr is not a pool-managed pointer (prints "your ptr is not in the memory pool, cannot be freed").

void free_all_memory(memory_pool_t* mp)

    Description: Resets the memory pool by freeing the old memory block and allocating a new empty block.
    Parameters:
        mp: Pointer to the memory pool.
    Use Case: Efficiently free all pool-allocated objects at once (avoids individual reverse-order frees).

4. Utility
bool is_mp_ptr(memory_pool_t* mp, void* ptr)

    Description: Checks if a pointer belongs to the memory pool.
    Parameters:
        mp: Pointer to the memory pool.
        ptr: Pointer to validate.
    Return Value: true if ptr is a valid pool-managed pointer; false otherwise (prints diagnostic messages for null/invalid inputs).

Build & Run
Prerequisites

    POSIX-compliant system (Linux/macOS)
    gcc compiler
    pthread library (usually pre-installed)

Build Commands
bash

# Clone or navigate to the project directory
cd memory_pool

# Compile the project
make

# Run the test suite
make run

# Clean compiled files
make clean

# Recompile (clean + build)
make rebuild

Example Test Output
plaintext

=============================================
Memory Pool Functional Test Suite
=============================================

[Test 1] Initialize Memory Pool
Test 1 Passed: Memory pool initialized successfully

[Test 2] Valid Memory Allocation (≤1024B)
invaild memory allocate size
the memory pool is full
  Small object 1 (200B) allocated at 0x55f7d2a6c2a0
  Small object 2 (300B) allocated at 0x55f7d2a6c368
  Oversize object (1500B) allocation failed (invalid size, expected)
  Small object 3 (600B) allocation failed (pool full, expected)
Test 2 Completed

[Test 3] Invalid Allocation Sizes (≤0 or >1024B)
invaild memory allocate size
invaild memory allocate size
invaild memory allocate size
  All invalid size allocations failed (expected)
Test 3 Passed

...
All Tests Executed Successfully

Usage Examples
Basic Single-Threaded Usage

#include "memory_pool.h"
#include <stdio.h>

int main() {
    // Initialize memory pool
    memory_pool_t* mp = init_memory_pool();
    if (!mp) return 1;

    // Allocate small objects
    int* obj1 = (int*)mp_allocate(mp, sizeof(int));
    float* obj2 = (float*)mp_allocate(mp, sizeof(float));

    if (obj1) *obj1 = 42;
    if (obj2) *obj2 = 3.14f;

    printf("obj1: %d, obj2: %.2f\n", *obj1, *obj2);

    // Free objects (reverse order!)
    free_memory(mp, obj2);
    free_memory(mp, obj1);

    // Reset pool (free all remaining objects)
    free_all_memory(mp);

    // Destroy pool
    destroy_memory_pool(mp);
    return 0;
}

Multi-Threaded Usage

#include "memory_pool.h"
#include <pthread.h>
#include <stdio.h>

#define NUM_THREADS 4
#define NUM_OBJS_PER_THREAD 10

memory_pool_t* mp; // Shared memory pool

void* thread_func(void* arg) {
    int thread_id = *(int*)arg;
    printf("Thread %d: Starting\n", thread_id);

    // Allocate and use objects
    void* objs[NUM_OBJS_PER_THREAD];
    for (int i = 0; i < NUM_OBJS_PER_THREAD; i++) {
        objs[i] = mp_allocate(mp, 64); // Allocate 64B objects
        if (objs[i]) {
            printf("Thread %d: Allocated obj %d at %p\n", thread_id, i, objs[i]);
        }
    }

    // Free objects (reverse order!)
    for (int i = NUM_OBJS_PER_THREAD - 1; i >= 0; i--) {
        if (objs[i]) {
            free_memory(mp, objs[i]);
            printf("Thread %d: Freed obj %d\n", thread_id, i);
        }
    }

    printf("Thread %d: Exiting\n", thread_id);
    return NULL;
}

int main() {
    mp = init_memory_pool();
    if (!mp) return 1;

    pthread_t threads[NUM_THREADS];
    int thread_ids[NUM_THREADS];

    // Create threads
    for (int i = 0; i < NUM_THREADS; i++) {
        thread_ids[i] = i;
        pthread_create(&threads[i], NULL, thread_func, &thread_ids[i]);
    }

    // Join threads
    for (int i = 0; i < NUM_THREADS; i++) {
        pthread_join(threads[i], NULL);
    }

    destroy_memory_pool(mp);
    return 0;
}

Key Notes & Limitations

    Reverse-Order Free Requirement: Pool objects must be freed in the reverse order of allocation. This is due to the truncation-based deallocation logic (frees by adjusting the used offset of the pool).
    Small Object Focus: Only objects ≤ 1024B are allocated from the pool. Larger objects are rejected (modify MAX_BLOCK_SIZE to adjust this limit).
    Thread Safety: The pool is thread-safe for concurrent allocation/deallocation, but user code must ensure proper pointer management (no double-frees).
    Pool Reset: free_all_memory frees all pool memory and creates a new empty block. Old pool pointers become invalid after a reset.

Error Handling
The memory pool provides clear error messages for common failure scenarios:

    Memory allocation failures (pool/block/mutex initialization) via perror().
    Invalid allocation sizes (≤ 0 or > 1024B) via console message.
    Null pointers (pool/object pointers) via console message.
    Non-pool pointers passed to free_memory via console message.
    All mutex locks are properly unlocked on error paths to avoid deadlocks.


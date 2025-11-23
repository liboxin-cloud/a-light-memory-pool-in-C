#include "memory_pool.h"
#include <stdio.h>
#include <stdlib.h>

int main() {
    printf("=============================================\n");
    printf("Memory Pool Functional Test Suite\n");
    printf("=============================================\n\n");

    // 1. Test memory pool initialization
    printf("[Test 1] Initialize Memory Pool\n");
    memory_pool_t* mp = init_memory_pool();
    if (mp == NULL) {
        printf("Test 1 Failed: Memory pool initialization failed\n");
        return EXIT_FAILURE;
    }
    printf("Test 1 Passed: Memory pool initialized successfully\n\n");

    // 2. Test valid memory allocation (small/large objects + pool full scenario)
    printf("[Test 2] Valid Memory Allocation\n");
    // Small objects (<= MAX_BLOCK_SIZE)
    void* small_obj1 = mp_allocate(mp, 200);  // 200B (within pool limit)
    void* small_obj2 = mp_allocate(mp, 300);  // 300B (within pool limit)
    // Large object (> MAX_BLOCK_SIZE)
    void* large_obj = mp_allocate(mp, 1500);  // 1500B (exceeds pool limit, uses malloc directly)
    // Pool full test (200+300+600=1100 > MAX_BLOCK_SIZE=1024)
    void* small_obj3 = mp_allocate(mp, 600);  // Should fail

    // Verify allocation results
    if (small_obj1 != NULL) printf("  Small object 1 (200B) allocated at %p\n", small_obj1);
    else printf("  Small object 1 allocation failed\n");

    if (small_obj2 != NULL) printf("  Small object 2 (300B) allocated at %p\n", small_obj2);
    else printf("  Small object 2 allocation failed\n");

    if (large_obj != NULL) printf("  Large object (1500B) allocated at %p\n", large_obj);
    else printf("  Large object allocation failed\n");

    if (small_obj3 == NULL) printf("  Small object 3 (600B) allocation failed (pool full, expected)\n");
    else printf("  Small object 3 allocation should have failed\n");
    printf("Test 2 Completed\n\n");

    // 3. Test invalid allocation sizes (0 and negative values)
    printf("[Test 3] Invalid Allocation Sizes\n");
    void* invalid_obj1 = mp_allocate(mp, 0);   // Size = 0 (invalid)
    void* invalid_obj2 = mp_allocate(mp, -40); // Negative size (invalid)

    if (invalid_obj1 == NULL && invalid_obj2 == NULL) {
        printf("  All invalid size allocations failed (expected)\n");
        printf("Test 3 Passed\n\n");
    } else {
        printf("  Invalid size allocation did not fail as expected\n");
        printf("Test 3 Failed\n\n");
    }

    // 4. Test memory deallocation (reverse order for pool objects + external object + invalid pointer)
    printf("[Test 4] Memory Deallocation\n");
    // Note: Pool objects must be freed in reverse allocation order (truncation-based free)
    printf("  Freeing small object 2 (reverse order)...\n");
    free_memory(mp, small_obj2);

    printf("  Freeing small object 1...\n");
    free_memory(mp, small_obj1);

    printf("  Freeing large object (external)...\n");
    free_memory(mp, large_obj);

    printf("  Freeing invalid pointer (0x123456)...\n");
    free_memory(mp, (void*)0x123456);  // Pointer not in pool

    printf("Test 4 Completed\n\n");

    // 5. Test free_all_memory (reset pool for reuse)
    printf("[Test 5] Free All Memory (Pool Reset)\n");
    free_all_memory(mp);
    printf("Test 5 Completed: Pool memory freed and reset\n\n");

    // 6. Test re-allocation after pool reset
    printf("[Test 6] Re-allocation After Pool Reset\n");
    void* new_small_obj1 = mp_allocate(mp, 500);  // New 500B object
    void* new_small_obj2 = mp_allocate(mp, 400);  // New 400B object
    void* new_small_obj3 = mp_allocate(mp, 200);  // Should fail (500+400+200=1100 > 1024)

    if (new_small_obj1 != NULL) printf("  New small object 1 (500B) allocated at %p\n", new_small_obj1);
    else printf("  New small object 1 allocation failed\n");

    if (new_small_obj2 != NULL) printf("  New small object 2 (400B) allocated at %p\n", new_small_obj2);
    else printf("  New small object 2 allocation failed\n");

    if (new_small_obj3 == NULL) printf("  New small object 3 (200B) allocation failed (pool full, expected)\n");
    else printf("  New small object 3 allocation should have failed\n");
    printf("Test 6 Completed\n\n");

    // 7. Test memory pool destruction
    printf("[Test 7] Destroy Memory Pool\n");
    destroy_memory_pool(mp);
    printf("Test 7 Completed: Memory pool destroyed successfully\n\n");

    printf("=============================================\n");
    printf("All Tests Executed\n");
    printf("Critical Note: Pool-managed objects require reverse-order deallocation\n");
    printf("=============================================\n");

    return EXIT_SUCCESS;
}

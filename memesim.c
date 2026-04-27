#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#include <meme.h>

#define MEGABYTE (1024UL * 1024UL)
#define SMALL_BLOCK_COUNT 100
#define BIG_BLOCK_COUNT 3
#define MAX_MAPS 4096
#define MAP_LINE_SIZE 512

typedef struct {
    uintptr_t start;
    uintptr_t end;
    char perms[5];
    char path[256];
} map_entry;

static char* skip_space(char* str) {
    while (*str == ' ' || *str == '\t') {
        str++;
    }

    return str;
}

static bool parse_map_line(const char* line, map_entry* entry) {
    unsigned long start = 0;
    unsigned long end = 0;
    unsigned long offset = 0;
    unsigned long inode = 0;
    char perms[5] = {0};
    char dev[16] = {0};
    char path[256] = {0};

    int fields = sscanf(
        line,
        "%lx-%lx %4s %lx %15s %lu %255[^\n]",
        &start,
        &end,
        perms,
        &offset,
        dev,
        &inode,
        path
    );

    if (fields < 6) {
        return false;
    }

    entry->start = (uintptr_t)start;
    entry->end = (uintptr_t)end;
    snprintf(entry->perms, sizeof(entry->perms), "%s", perms);

    char* trimmed_path = skip_space(path);
    snprintf(entry->path, sizeof(entry->path), "%s", trimmed_path);

    return true;
}

static size_t read_maps(map_entry* maps, size_t capacity) {
    FILE* file = fopen("/proc/self/maps", "r");
    if (file == NULL) {
        perror("fopen /proc/self/maps");
        return 0;
    }

    char line[MAP_LINE_SIZE];
    size_t count = 0;

    while (count < capacity && fgets(line, sizeof(line), file) != NULL) {
        if (parse_map_line(line, &maps[count])) {
            count++;
        }
    }

    fclose(file);
    return count;
}

static const map_entry* find_map_containing(
    const map_entry* maps,
    size_t count,
    const void* ptr
) {
    uintptr_t address = (uintptr_t)ptr;

    for (size_t i = 0; i < count; i++) {
        if (maps[i].start <= address && address < maps[i].end) {
            return &maps[i];
        }
    }

    return NULL;
}

static bool is_anonymous_mapping(const map_entry* entry) {
    return entry->path[0] == '\0' || strncmp(entry->path, "[anon", 5) == 0;
}

static void print_statm(const char* label) {
    FILE* file = fopen("/proc/self/statm", "r");
    if (file == NULL) {
        perror("fopen /proc/self/statm");
        return;
    }

    char line[MAP_LINE_SIZE];

    printf("\n=== /proc/self/statm: %s ===\n", label);
    if (fgets(line, sizeof(line), file) != NULL) {
        printf("%s", line);
    }

    fclose(file);
}

static void print_maps(const char* label) {
    FILE* file = fopen("/proc/self/maps", "r");
    if (file == NULL) {
        perror("fopen /proc/self/maps");
        return;
    }

    char line[MAP_LINE_SIZE];

    printf("\n=== /proc/self/maps: %s ===\n", label);
    while (fgets(line, sizeof(line), file) != NULL) {
        printf("%s", line);
    }

    fclose(file);
}

static void print_mapping_for_block(size_t idx, const void* ptr, size_t requested_size) {
    map_entry maps[MAX_MAPS];
    size_t map_count = read_maps(maps, MAX_MAPS);
    const map_entry* entry = find_map_containing(maps, map_count, ptr);

    if (entry == NULL) {
        printf(
            "big block[%zu]: ptr=%p request=%zu bytes -> mapping not found\n",
            idx,
            ptr,
            requested_size
        );
        return;
    }

    const char* path = entry->path[0] == '\0' ? "(anonymous/no pathname)" : entry->path;
    printf(
        "big block[%zu]: ptr=%p request=%zu bytes -> %p-%p, size=%zu bytes, perms=%s, %s%s\n",
        idx,
        ptr,
        requested_size,
        (void*)entry->start,
        (void*)entry->end,
        (size_t)(entry->end - entry->start),
        entry->perms,
        path,
        is_anonymous_mapping(entry) ? " [allocator anonymous mapping candidate]" : ""
    );
}

static bool address_is_mapped(const void* ptr) {
    map_entry maps[MAX_MAPS];
    size_t map_count = read_maps(maps, MAX_MAPS);

    return find_map_containing(maps, map_count, ptr) != NULL;
}

int main(void) {
    void* small_blocks[SMALL_BLOCK_COUNT] = {0};
    void* big_blocks[BIG_BLOCK_COUNT] = {0};
    size_t big_sizes[BIG_BLOCK_COUNT] = {
        2 * MEGABYTE,
        3 * MEGABYTE,
        4 * MEGABYTE
    };

    print_statm("before allocator work");
    print_maps("before allocator work");

    for (size_t i = 0; i < SMALL_BLOCK_COUNT; i++) {
        size_t size = 4096 + i * 17;
        small_blocks[i] = ms_malloc(size);

        if (small_blocks[i] != NULL) {
            memset(small_blocks[i], (int)('A' + (i % 26)), size);
        }
    }

    print_statm("after allocating 100 small blocks");

    for (size_t i = 0; i < SMALL_BLOCK_COUNT; i += 2) {
        ms_free(small_blocks[i]);
        small_blocks[i] = NULL;
    }

    print_statm("after freeing half of the small blocks");

    for (size_t i = 0; i < BIG_BLOCK_COUNT; i++) {
        big_blocks[i] = ms_malloc(big_sizes[i]);

        if (big_blocks[i] != NULL) {
            char* bytes = (char*)big_blocks[i];
            bytes[0] = (char)('0' + i);
            bytes[big_sizes[i] - 1] = (char)('0' + i);
        }
    }

    print_statm("after allocating several large blocks");
    print_maps("after allocating several large blocks");

    printf("\n=== anonymous mappings containing large allocator blocks ===\n");
    for (size_t i = 0; i < BIG_BLOCK_COUNT; i++) {
        print_mapping_for_block(i, big_blocks[i], big_sizes[i]);
    }

    if (big_blocks[0] != NULL) {
        void* freed_big_block = big_blocks[0];

        ms_free(big_blocks[0]);
        big_blocks[0] = NULL;

        print_statm("after freeing one large block");
        printf("\n=== munmap check for freed large block ===\n");
        printf(
            "freed big block address %p is %s in /proc/self/maps\n",
            freed_big_block,
            address_is_mapped(freed_big_block) ? "still present" : "gone"
        );
    }

    for (size_t i = 1; i < BIG_BLOCK_COUNT; i++) {
        ms_free(big_blocks[i]);
    }

    for (size_t i = 1; i < SMALL_BLOCK_COUNT; i += 2) {
        ms_free(small_blocks[i]);
    }

    print_statm("after freeing remaining blocks");

    return 0;
}

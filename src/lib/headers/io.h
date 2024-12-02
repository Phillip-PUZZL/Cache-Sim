//
// Created by Phillip Driscoll on 9/18/24.
//

#ifndef CACHE_SIM_IO_H
#define CACHE_SIM_IO_H
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <math.h>
#include <string.h>
#include <stdbool.h>

//Defining cache read and write values
#define CACHE_READ 0
#define CACHE_WRITE 1

//Defining the start address and size of main memory to print
#define MAIN_MEMORY_START_PRINT 0x003f7f00
#define MAIN_MEMORY_PRINT_SIZE 1024

//Defining word size based on bits
#ifdef M16
#define WORD_SIZE 2
#elif M64
#define WORD_SIZE 8
#else
#define WORD_SIZE 4
#endif

//Defining integer size based on bits
#ifdef _WIN32
#ifdef M16
#define INT_TYPE unsigned __int16
#elif M64
#define INT_TYPE unsigned __int64
#else
#define INT_TYPE unsigned __int32
#endif
#elif __linux__
#ifdef M16
#define INT_TYPE u_int16_t
#elif M64
#define INT_TYPE u_int64_t
#else
#define INT_TYPE u_int32_t
#endif
#elif __APPLE__
#ifdef M16
#define INT_TYPE uint16_t
#elif M64
#define INT_TYPE uint64_t
#else
#define INT_TYPE uint32_t
#endif
#endif

//Setting the size of main memory, its block size, and calculating total number of blocks
//and the total number of words per block
static const int MM_SIZE = 16777216;
static const int MM_BLOCK_SIZE = 32;
static const int TOTAL_MM_BLOCKS = MM_SIZE / MM_BLOCK_SIZE;
static const int MM_WORDS_PER_BLOCK = MM_BLOCK_SIZE / WORD_SIZE;

//Data structure to house a block of main memory
struct main_mem_block {
    INT_TYPE address;

    INT_TYPE* words;
};

//Data structure to house a line of cache memory
struct cache_mem_block {
    bool loaded;
    INT_TYPE tag;
    INT_TYPE set;
    bool valid;
    bool dirty;
    int last_pc;

    INT_TYPE* words;
};

//Data structure which contains all info for the cache itself
struct cache {
    int size;
    int line_size;
    int associativity;
    int total_lines;
    int words_per_line;
    int total_sets;
    int pc;

    struct cache_mem_block* lines;
};

//Data structure to house all the simulation statistics
struct cache_stats {
    long total_actions;
    long total_reads;
    long total_writes;

    long total_misses;
    long read_misses;
    long write_misses;

    long total_evictions;
    long dirty_evictions;
    long total_loads;
};

//Data structure to house the tag, set, and word information for an address
struct address_info {
    INT_TYPE tag;
    INT_TYPE set;
    INT_TYPE word;
};

struct cache zero_cache();
struct cache_stats zero_stats();
struct cache init_cache_mem(struct cache cache_mem);
struct main_mem_block* init_main_mem();
void free_io(struct cache cache_mem, struct main_mem_block* main_mem);

void print_cache_and_memory(struct cache cache_mem, struct cache_stats stats,
        struct main_mem_block* main_mem);
void write_cache_and_memory(char* output, struct cache cache_mem, struct cache_stats stats,
                            struct main_mem_block* main_mem);

struct address_info info_from_address(INT_TYPE addr, int total_cache_sets, int cache_block_size);
INT_TYPE address_from_info(INT_TYPE tag, INT_TYPE set, INT_TYPE word,
                           int total_cache_sets, int cache_block_size);

bool addr_in_cache(struct cache cache_mem, struct address_info info);
bool set_has_empty(struct cache cache_mem, INT_TYPE set);

INT_TYPE get_lru_cm_line(struct cache cache_mem, INT_TYPE set);

bool evict_line(struct cache* cache_mem, struct main_mem_block* main_mem, INT_TYPE line, bool keep_in_cache);
int load_line(struct cache* cache_mem, struct main_mem_block* main_mem, INT_TYPE addr);

int write_back(struct cache* cache_mem, struct address_info info, INT_TYPE new_val);

int write_to_cache(struct cache* cache_mem, struct cache_stats* stats,
        struct main_mem_block* main_mem, INT_TYPE addr, INT_TYPE new_val);
int read_from_cache(struct cache* cache_mem, struct cache_stats* stats,
        struct main_mem_block* main_mem, INT_TYPE addr);
void write_cache_to_memory(struct cache* cache_mem, struct main_mem_block* main_mem);

#endif //CACHE_SIM_IO_H

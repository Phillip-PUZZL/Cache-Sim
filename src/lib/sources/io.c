#include "../headers/io.h"
//
// Created by Phillip Driscoll on 9/18/24.
//

//Function for setting the values of a cache struct to zero
struct cache zero_cache() {
    struct cache primer;

    primer.associativity = 0;
    primer.total_lines = 0;
    primer.words_per_line = 0;
    primer.line_size = 0;
    primer.size = 0;
    primer.total_sets = 0;
    primer.pc = 0;

    return primer;
}

//Function for setting the values of a statistics struct to zero
struct cache_stats zero_stats() {
    struct cache_stats primer;

    primer.total_evictions = 0;
    primer.total_loads = 0;
    primer.write_misses = 0;
    primer.read_misses = 0;
    primer.total_misses = 0;
    primer.total_actions = 0;
    primer.total_writes = 0;
    primer.total_reads = 0;

    return primer;
}

//Function for initializing cache memory struct
struct cache init_cache_mem(struct cache cache_mem) {
    struct cache curr_cache = cache_mem;

    //Allocate memory
    curr_cache.lines = calloc(cache_mem.total_lines, sizeof(struct cache_mem_block));
    int current_set = -1;
    //Initialize cache memory
    for(int i = 0; i < cache_mem.total_lines; i++) {
        //Check associativity and if it is time to increase set number by 1
        if(i % cache_mem.associativity == 0) {
            current_set++;
        }

        //Set loaded and last used program counter variables
        curr_cache.lines[i].loaded = 0;
        curr_cache.lines[i].last_pc = 0;
        curr_cache.lines[i].set = current_set;
        //Allocate memory for line words
        curr_cache.lines[i].words = calloc(cache_mem.words_per_line, WORD_SIZE);
        //Initialize line words to 0
        for (int j = 0; j < cache_mem.words_per_line; j++) {
            curr_cache.lines[i].words[j] = 0;
        }
    }

    //Configure total number of sets in cache
    curr_cache.total_sets = current_set + 1;

    return curr_cache;
}

//Function for initializing main memory
struct main_mem_block* init_main_mem() {
    struct main_mem_block* main_mem;

    //Allocate memory for blocks
    main_mem = calloc(TOTAL_MM_BLOCKS, sizeof(struct main_mem_block));
    //Initialize main memory
    for(int i = 0; i < TOTAL_MM_BLOCKS; i++) {
        //Set block address and allocate memory for words in the block
        main_mem[i].address = i * MM_WORDS_PER_BLOCK;
        main_mem[i].words = calloc(MM_WORDS_PER_BLOCK, WORD_SIZE);
        //Set the value for each word in the block to its address
        for (int j = 0; j < MM_WORDS_PER_BLOCK; j++) {
            main_mem[i].words[j] = main_mem[i].address + j;
        }
    }

    return main_mem;
}

//Function to free the memory allocated to the cache and main memory structs
void free_io(struct cache cache_mem, struct main_mem_block* main_mem) {
    //Free all words in all blocks
    for(int i = 0; i < TOTAL_MM_BLOCKS; i++) {
        free(main_mem[i].words);
    }
    //Free the array of memory blocks
    free(main_mem);

    //Free all words in all lines
    for(int i = 0; i < cache_mem.total_lines; i++) {
        free(cache_mem.lines[i].words);
    }
    //Free the array of cache lines
    free(cache_mem.lines);
}

//Function to print the cache and specified amount of memory to screen
void print_cache_and_memory(struct cache cache_mem, struct cache_stats stats,
        struct main_mem_block* main_mem) {
    // Use this code to format and print your output
    printf("STATISTICS\n");
    printf("Misses:\n");
    printf("Total: %ld Data Reads: %ld Data Writes: %ld\n", stats.total_misses, stats.read_misses, stats.write_misses);
    printf("Miss rate:\n");
    printf("Total: %.6f Data Reads: %.6f Data Writes: %.6f\n", ((float) stats.total_misses / (float) stats.total_actions),
           ((float) stats.read_misses / (float) stats.total_reads), ((float) stats.write_misses / (float) stats.total_writes));
    printf("Number of Dirty Blocks Evicted from the Cache: %ld\n\n", stats.total_evictions);
    printf("CACHE CONTENTS\n");
    printf("%-6s %-5s %-10s %-6s", "Set", "Valid", "Tag", "Dirty");

    for(int i = 0; i < cache_mem.line_size / WORD_SIZE; i++) {
        printf("Word%-7d", i);
    }
    printf("\n");

    for(int i = 0; i < cache_mem.total_lines; i++) {
        struct cache_mem_block block = cache_mem.lines[i];
        printf("0x%04X %-5d 0x%08X %-5d", block.set, block.valid, block.tag, block.dirty);

        int cutoff = cache_mem.line_size / WORD_SIZE;

        for(int j = 0; j < cutoff; j++) {
            printf(" 0x%08X", block.words[j]);
        }
        printf("\n");
    }
    printf("\n");

    printf("MAIN MEMORY:\n");
    printf("%-11s", "Address");

    for(int i = 0; i < MM_BLOCK_SIZE / WORD_SIZE; i++) {
        printf("Word%-7d", i);
    }
    printf("\n");

    int start_block = (int) floor((double) MAIN_MEMORY_START_PRINT / (double) MM_WORDS_PER_BLOCK);
    int end_block = (int) floor((double) (MAIN_MEMORY_START_PRINT + MAIN_MEMORY_PRINT_SIZE) / (double) MM_WORDS_PER_BLOCK);
    for(int i = start_block; i < end_block; i++) {
        struct main_mem_block block = main_mem[i];
        printf("0x%08X", block.address);

        //Determining how many words to write in case the print size results in a less than perfect
        //block being printed
        int cutoff = MM_BLOCK_SIZE / WORD_SIZE;

        if(i == start_block) {
            cutoff = cutoff + (MAIN_MEMORY_START_PRINT % MM_WORDS_PER_BLOCK);
        } else if(i == end_block) {
            cutoff = cutoff - ((MAIN_MEMORY_START_PRINT + MAIN_MEMORY_PRINT_SIZE) % MM_WORDS_PER_BLOCK);
        }

        for(int j = 0; j < cutoff; j++) {
            printf(" 0x%08X", block.words[j]);
        }
        printf("\n");
    }
}

//Function to write the cache and memory outputs to a file
void write_cache_and_memory(char* output, struct cache cache_mem, struct cache_stats stats,
                            struct main_mem_block* main_mem) {
    FILE* output_file = fopen(output, "w");

    if(!output_file) {
        printf("Error: Output file could not be created / opened!\n");
        fclose(output_file);
        return;
    }

    // Use this code to format and print your output
    fprintf(output_file, "STATISTICS\n");
    fprintf(output_file, "Misses:\n");
    fprintf(output_file, "Total: %ld Data Reads: %ld Data Writes: %ld\n", stats.total_misses, stats.read_misses, stats.write_misses);
    fprintf(output_file, "Miss rate:\n");
    fprintf(output_file, "Total: %.6f Data Reads: %.6f Data Writes: %.6f\n", ((float) stats.total_misses / (float) stats.total_actions),
           ((float) stats.read_misses / (float) stats.total_reads), ((float) stats.write_misses / (float) stats.total_writes));
    fprintf(output_file, "Number of Dirty Blocks Evicted from the Cache: %ld\n\n", stats.total_evictions);
    fprintf(output_file, "CACHE CONTENTS\n");
    fprintf(output_file, "%-6s %-5s %-10s %-6s", "Set", "Valid", "Tag", "Dirty");

    for(int i = 0; i < cache_mem.line_size / WORD_SIZE; i++) {
        fprintf(output_file, "Word%-7d", i);
    }
    fprintf(output_file, "\n");

    for(int i = 0; i < cache_mem.total_lines; i++) {
        struct cache_mem_block block = cache_mem.lines[i];
        fprintf(output_file, "0x%04X %-5d 0x%08X %-5d", block.set, block.valid, block.tag, block.dirty);

        int cutoff = cache_mem.line_size / WORD_SIZE;

        for(int j = 0; j < cutoff; j++) {
            fprintf(output_file, " 0x%08X", block.words[j]);
        }
        fprintf(output_file, "\n");
    }
    fprintf(output_file, "\n");

    fprintf(output_file, "MAIN MEMORY:\n");
    fprintf(output_file, "%-11s", "Address");

    for(int i = 0; i < MM_BLOCK_SIZE / WORD_SIZE; i++) {
        fprintf(output_file, "Word%-7d", i);
    }
    fprintf(output_file, "\n");

    int start_block = (int) floor((double) MAIN_MEMORY_START_PRINT / (double) MM_WORDS_PER_BLOCK);
    int end_block = (int) floor((double) (MAIN_MEMORY_START_PRINT + MAIN_MEMORY_PRINT_SIZE) / (double) MM_WORDS_PER_BLOCK);
    for(int i = start_block; i < end_block; i++) {
        struct main_mem_block block = main_mem[i];
        fprintf(output_file, "0x%08X", block.address);

        int cutoff = MM_BLOCK_SIZE / WORD_SIZE;

        if(i == start_block) {
            cutoff = cutoff + (MAIN_MEMORY_START_PRINT % MM_WORDS_PER_BLOCK);
        } else if(i == end_block) {
            cutoff = cutoff - ((MAIN_MEMORY_START_PRINT + MAIN_MEMORY_PRINT_SIZE) % MM_WORDS_PER_BLOCK);
        }

        for(int j = 0; j < cutoff; j++) {
            fprintf(output_file, " 0x%08X", block.words[j]);
        }
        fprintf(output_file, "\n");
    }

    fclose(output_file);
}

//Getting tag, set, and word information from the address
struct address_info info_from_address(INT_TYPE addr, int total_cache_sets, int cache_block_size) {
    struct address_info addr_info;

    //Set = d where 2^d = num of sets
    int set = (int) log2(total_cache_sets);
    //Word = w where 2^w = num of words per cache line
    int word = (int) log2((int) (cache_block_size / WORD_SIZE));
    //tag and set = s where s = address length - word
    int tag_and_set = (int) WORD_SIZE * 8 - word;
    //Tag = s - d
    int tag = tag_and_set - set;

    //Bitshift right w + d (length of set and word)
    addr_info.tag = (INT_TYPE) (addr >> ((WORD_SIZE * 8) - tag));
    //Bitshift left length of tag to cutoff tag, then bitshift right length of tag and word to cutoff word
    //leaving set behind
    addr_info.set = (INT_TYPE) ((addr << tag) >> (tag + word));

    //If word length = 0 (direct mapping) then the bitshifts don't work and we need to manually set it to 0
    if(word != 0) {
        //Bitshift to the left the length of tag and set to cut them off then bitshift to the right the same
        //amount to leave just the word value
        addr_info.word = (INT_TYPE) ((addr << tag_and_set) >> tag_and_set);
    } else {
        addr_info.word = 0;
    }

    return addr_info;
}

//Get memory address from the tag, set, and word information
INT_TYPE address_from_info(INT_TYPE tag, INT_TYPE set, INT_TYPE word,
                           int total_cache_sets, int cache_block_size) {
    //Set value of return to tag since they are the leftmost bits
    INT_TYPE addr = tag;

    //Get initial s and w values
    int s = (int) log2(total_cache_sets);
    int w = (int) log2((int) (cache_block_size / WORD_SIZE));

    //Shift left the length of the tag + set
    addr <<= s;
    //Bitwise or on the rightmost set-length bits
    addr |= set;
    //Shift left the length of the word
    addr <<= w;
    //Bitwise or on the rightmost word-length bits
    addr |= word;

    return addr;
}

//Function to find a line that is loaded in cache memory
INT_TYPE get_loaded_cm_line(struct cache cache_mem, struct address_info info) {
    INT_TYPE line = 0;

    //Get the start location of the search based on the set of the address
    INT_TYPE cm_set_start = cache_mem.associativity * info.set;
    //Loop over just that set
    for(INT_TYPE i = 0; i < cache_mem.associativity; i++) {
        //Determine if the tags line up for a loaded cache line
        if(cache_mem.lines[cm_set_start + i].tag == info.tag) {
            //Set line number to the one loaded
            line = cm_set_start + i;
            break;
        }
    }

    return line;
}

//Function to return the cache line number for an empty cache line in a set
INT_TYPE get_available_cm_line(struct cache cache_mem, INT_TYPE set) {
    INT_TYPE line = 0;

    //Set the search start location
    INT_TYPE cm_set_start = cache_mem.associativity * set;
    //Loop over set
    for(INT_TYPE i = 0; i < cache_mem.associativity; i++) {
        //Check if the line is loaded
        if(cache_mem.lines[cm_set_start + i].loaded == 0) {
            //If line is not loaded, return it as an available line in the set
            line = cm_set_start + i;
            break;
        }
    }

    return line;
}

//Function to get the least recently used cache line of a set
INT_TYPE get_lru_cm_line(struct cache cache_mem, INT_TYPE set) {
    INT_TYPE line = 0;

    //Set the search start params and initialize the current lowest program counter to -1
    INT_TYPE cm_set_start = cache_mem.associativity * set;
    //Initialize the lowest program counter and its associated line to the first line in the set
    INT_TYPE lowest_pc = cache_mem.lines[cm_set_start].last_pc;
    line = cm_set_start;
    //Loop over the set
    for(INT_TYPE i = 1; i < cache_mem.associativity; i++) {
        //Check if the line has a lower program counter than the previous lowest
        if(cache_mem.lines[cm_set_start + i].last_pc < lowest_pc) {
            //If lower than the current lowest program counter,
            //set the lowest pc to this one and current line to this one
            line = cm_set_start + i;
            lowest_pc = cache_mem.lines[cm_set_start + i].last_pc;
        }
    }

    return line;
}

//Function to verify if the address is currently loaded in the cache
bool addr_in_cache(struct cache cache_mem, struct address_info info) {
    //Set the start index
    INT_TYPE cm_set_start = cache_mem.associativity * info.set;
    //Loop through the set associated with that address
    for(INT_TYPE i = 0; i < cache_mem.associativity; i++) {
        //Check if tags match up
        if(cache_mem.lines[cm_set_start + i].tag == info.tag) {
            //If tags match, address is currently loaded
            return 1;
        }
    }

    return 0;
}

//Function to check if a set has an empty cache line available
bool set_has_empty(struct cache cache_mem, INT_TYPE set) {
    //Set start index
    INT_TYPE cm_set_start = cache_mem.associativity * set;
    //Loop through set
    for(INT_TYPE i = 0; i < cache_mem.associativity; i++) {
        //Check if each line in set is loaded
        if(cache_mem.lines[cm_set_start + i].loaded == 0) {
            //If it finds a single unloaded line, it can return as having an empty line
            return 1;
        }
    }

    return 0;
}

//Function to evict line from cache back to the memory
bool evict_line(struct cache* cache_mem, struct main_mem_block* main_mem, INT_TYPE line, bool keep_in_cache) {
    //Get the main memory address associated with the first block in the cache line
    INT_TYPE addr = address_from_info(cache_mem->lines[line].tag, cache_mem->lines[line].set, 0, cache_mem->total_sets, cache_mem->line_size);
    int code = 0;

    //Get the offset from the beginning of the memory block which the first block of the cache line
    //belongs to. Example: cache line address is associated with the 4th out of 8 words in the memory block
    //then we must set the offset to 4 since not the entire memory block is loaded into the cache
    INT_TYPE mm_block_offset = addr % MM_WORDS_PER_BLOCK;
    //Get the index of the memory block in the main_mem_block* struct
    INT_TYPE mm_block = (addr - mm_block_offset) / MM_WORDS_PER_BLOCK;

    //Calculate the total number of memory blocks which the cache line holds data for
    //Example: Cache lines hold 16 words and main memory blocks hold only 4. Total number
    //of memory blocks to write to, in this case, is ceil(16 / 4)
    int mem_blocks_to_write = (int) ceil((double) cache_mem->words_per_line / (double) MM_WORDS_PER_BLOCK);
    //Index of the cache line
    int cm_line_offset = 0;
    //Loop through the total number of memory blocks to write to
    for(int i = 0; i < mem_blocks_to_write; i++) {
        //Loop through the words in the memory block until the end of the memory block is reached
        //or until the end of the cache line is reached
        for(INT_TYPE j = mm_block_offset; j < MM_WORDS_PER_BLOCK && cm_line_offset < cache_mem->words_per_line; j++) {
            //Write word from cache line index to memory block index
            main_mem[mm_block].words[j] = cache_mem->lines[line].words[cm_line_offset];

            //Check if the line is being kept in the cache as well, if not then set that word in the cache
            //back to 0
            if(!keep_in_cache) {
                cache_mem->lines[line].words[cm_line_offset] = 0;
            }

            //Increment the current cache line word
            cm_line_offset++;
        }
        //Main memory block offset is automatically 0 after the first line since finishing
        //one main memory block means resetting back to the 0 index of the next memory block
        mm_block_offset = 0;
    }

    //If the cache line is not being kept in the cache, reset all variables associated with that line
    if(!keep_in_cache) {
        cache_mem->lines[line].last_pc = -1;
        cache_mem->lines[line].tag = 0;
        cache_mem->lines[line].loaded = 0;
        cache_mem->lines[line].dirty = 0;
        cache_mem->lines[line].valid = 0;
    }

    return code;
}

//Function to load a cache line from main memory
int load_line(struct cache* cache_mem, struct main_mem_block* main_mem, INT_TYPE addr) {
    //Get tag, set, word information from the address
    struct address_info info = info_from_address(addr, cache_mem->total_sets, cache_mem->line_size);
    int code = 0;

    //Check if the cache has an empty line for the associated set
    if(!set_has_empty(*cache_mem, info.set)) {
        //If no empty line is available, evict the least recently used line from the cache first
        bool evict_status = evict_line(cache_mem, main_mem, get_lru_cm_line(*cache_mem, info.set), 0);

        //If cache fails to evict, return from this function with an error
        if(evict_status != 0) {
            return 3;
        }

        code = 1;
    }

    //Calculate the main memory block offset (see explanation in evict_line function if confused about its purpose)
    INT_TYPE mm_block_offset = address_from_info(info.tag, info.set, 0, cache_mem->total_sets, cache_mem->line_size) % MM_WORDS_PER_BLOCK;
    //Calculate the main memory index of the main_mem_block struct
    INT_TYPE mm_block = (addr - mm_block_offset) / MM_WORDS_PER_BLOCK;

    //Get the cache line which we are loading the data into
    INT_TYPE cm_line = get_available_cm_line(*cache_mem, info.set);

    //Get total number of memory blocks which the cache line spans (see evict_line explanation)
    int mem_blocks_to_load = (int) ceil((double) cache_mem->words_per_line / (double) MM_WORDS_PER_BLOCK);
    //Offset for the word in the cache line
    int cm_line_offset = 0;
    //Iterate over memory blocks
    for(int i = 0; i < mem_blocks_to_load; i++) {
        //Iterate over cache lines until the end of a memory block or cache line is reached
        for(INT_TYPE j = mm_block_offset; j < MM_WORDS_PER_BLOCK && cm_line_offset < cache_mem->words_per_line; j++) {
            //Load memory into cache
            cache_mem->lines[cm_line].words[cm_line_offset] = main_mem[mm_block].words[j];

            //Increase cache word offset
            cm_line_offset++;
        }
        mm_block_offset = 0;
    }

    //Setting metadata info for the line
    cache_mem->lines[cm_line].tag = info.tag;
    cache_mem->lines[cm_line].loaded = 1;
    cache_mem->lines[cm_line].valid = 1;

    return code;
}

//Function to write a new value into a cache line word
int write_back(struct cache* cache_mem, struct address_info info, INT_TYPE new_val) {
    //Get the loaded cache line
    INT_TYPE cm_line = get_loaded_cm_line(*cache_mem, info);

    //Set the new value for the cache line word using the word offset
    cache_mem->lines[cm_line].words[info.word] = new_val;
    //Mark the line as dirty
    cache_mem->lines[cm_line].dirty = 1;

    return 0;
}

//Function to write data into the cache
int write_to_cache(struct cache* cache_mem, struct cache_stats* stats,
                    struct main_mem_block* main_mem, INT_TYPE addr, INT_TYPE new_val) {
    //Get tag, set, word info for address
    struct address_info info = info_from_address(addr, cache_mem->total_sets, cache_mem->line_size);
    //Verify whether the address is already loaded into the cache
    bool in_cache = addr_in_cache(*cache_mem, info);

    int status;
    if(!in_cache) {
        //If the address is not in the cache, register a write miss
        stats->total_misses++;
        stats->write_misses++;

        //Load the line
        status = load_line(cache_mem, main_mem, addr);
        //Verify the line was loaded and whether an eviction was necessary to load the address
        if(status == 0) {
            //Load with no eviction
            stats->total_loads++;
        } else if(status == 1) {
            //Load with an eviction
            stats->total_loads++;
            stats->total_evictions++;
        } else {
            return status - 1;
        }
    }

    //Write the new data to the address
    status = write_back(cache_mem, info, new_val);
    if(status != 0) {
        return status;
    }

    //Increment program counter and set the last program counter of the line to this pc
    cache_mem->pc++;
    cache_mem->lines[get_loaded_cm_line(*cache_mem, info)].last_pc = cache_mem->pc;

    //Increase total number of actions and total number of writes
    stats->total_actions++;
    stats->total_writes++;

    return 0;
}

//Function to register a cache read
int read_from_cache(struct cache* cache_mem, struct cache_stats* stats,
                     struct main_mem_block* main_mem, INT_TYPE addr) {
    //Get tag, set, word information
    struct address_info info = info_from_address(addr, cache_mem->total_sets, cache_mem->line_size);
    //Check if the address is in the cache
    bool in_cache = addr_in_cache(*cache_mem, info);

    if(!in_cache) {
        //Address is not in the cache, register a cache miss
        stats->total_misses++;
        stats->read_misses++;

        //Load the cache line
        int status = load_line(cache_mem, main_mem, addr);
        //Check line status
        if(status == 0) {
            //Load w/ no eviction
            stats->total_loads++;
        } else if(status == 1) {
            //Load w/ eviction
            stats->total_loads++;
            stats->total_evictions++;
        } else {
            return status - 1;
        }
    }

    //Increment program counter and set last program counter of the line to the current pc
    cache_mem->pc++;
    cache_mem->lines[get_loaded_cm_line(*cache_mem, info)].last_pc = cache_mem->pc;

    //Increase total number of actions and total number of reads
    stats->total_actions++;
    stats->total_reads++;

    return 0;
}

//Function to write the results of the cache simulator to a file
void write_cache_to_memory(struct cache* cache_mem, struct main_mem_block* main_mem) {
    //Increment through all cache lines
    for(int i = 0; i < cache_mem->total_lines; i++) {
        //Check if a cache line is loaded
        if(cache_mem->lines[i].loaded) {
            //Evict all loaded cache lines to memory, but also keep them in the cache and do not register it
            //as an eviction for the statistics
            evict_line(cache_mem, main_mem, i, 1);
        }
    }
}
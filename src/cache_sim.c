#include <stdio.h>
#include <string.h>

//OS Specific Libraries
#ifdef _WIN32
// Windows-specific code
#include <limits.h>
#elif __linux__
// Linux-specific code
#include <sys/syslimits.h>
#include <unistd.h>
#include <sys/time.h>
#elif __APPLE__
// MacOS-specific code
#include <sys/syslimits.h>
#include <unistd.h>
#include <sys/time.h>
#endif

#include "lib/headers/io.h"
//
// Created by Phillip Driscoll on 9/18/24.
//

//Function to calculate time difference for benchmarking
float time_difference_msec(struct timeval t0, struct timeval t1) {
    return (float) (t1.tv_sec - t0.tv_sec) * 1000.0f + (float) (t1.tv_usec - t0.tv_usec) / 1000.0f;
}

//Function to read input file and process traces
int process_trace(char* input_file, struct cache* cache_mem, struct cache_stats* stats,
                    struct main_mem_block* main_mem) {

    //Verify validity of file
    if(access(input_file, F_OK) == 0) {
        // File exists
        printf("Input file found!\n");

        //Open the file
        FILE* trace_file = fopen(input_file, "r");
        char* line = NULL;
        size_t len = 0;
        ssize_t read;
        int line_num = 1;
        bool failure = 0;

        //If trace_file still NULL, failed to open and need to exit
        if(!trace_file) {
            printf("Error: Input file could not be read!\n");
            fclose(trace_file);
            return 3;
        }

        printf("Running cache simulation...\n");

        struct timeval t0;
        struct timeval t1;
        float elapsed;

        //Start benchmark
        gettimeofday(&t0, 0);

        //Get each line from file
        while((read = getline(&line, &len, trace_file)) != -1) {
            int read_write = -1;
            //Longs so that we can verify that a 32-bit address was actually pulled from the file
            unsigned long addr = -1;
            unsigned long val = -1;

            //Verify that the line only contains hex characters, spaces, and new lines
            for(int i = 0; i < read; i++) {
                if(!isxdigit(line[i]) && line[i] != ' ' && line[i] != '\n') {
                    printf("Error: Malformed input file: Unrecognizable instruction on line %d position %d\n", line_num, i + 1);
                    failure = 1;
                    break;
                }
            }

            if(failure) {
                break;
            }

            //Read instruction type
            sscanf(line, "%x", &read_write);

            if(read_write == CACHE_READ) {
                //Line is a read, get the address
                sscanf(line, "%*x %lx", &addr);

                //Verify address was retrieved
                if(addr == -1) {
                    printf("Error: Malformed address: Could not read address on line %d\n", line_num);
                    failure = 1;
                    break;
                }

                //Read from the cache
                read_from_cache(cache_mem, stats, main_mem, addr);
            } else if(read_write == CACHE_WRITE) {
                //Line is a write, get the address and new value
                sscanf(line, "%*x %lx %lx", &addr, &val);

                //Verify the address and value retrieved from the line
                if(addr == -1 || val == -1) {
                    printf("Error: Malformed address or value: Could not read address or value on line %d\n", line_num);
                    failure = 1;
                    break;
                }

                //Write to the cache
                write_to_cache(cache_mem, stats, main_mem, addr, val);
            } else {
                //Not a read or write instruction
                printf("Error: Unrecognized instruction: Invalid instruction on line %d\n", line_num);
                failure = 1;
                break;
            }

            line_num++;
        }

        //Verify that no failure occurred
        if(!failure) {
            //If not a failure, write the contents of the cache to the memory
            write_cache_to_memory(cache_mem, main_mem);
        }

        //Stop benchmark
        gettimeofday(&t1, 0);
        //Calculate time difference
        elapsed = time_difference_msec(t0, t1);

        //Close file descriptor
        fclose(trace_file);
        //Free memory allocated to the line
        if(line) {
            free(line);
        }

        if(failure) {
            return 4;
        }

        printf("Finished cache simulation!\nProcessed %ld instructions in %f ms\n\n", stats->total_actions, elapsed);
    } else {
        // File doesn't exist
        printf("Error: Input file could not be found!\n");
        return 2;
    }

    return 0;
}

//Application entry point
int main(int argc, char *argv[]) {
    //Defining variables for file input, output, and the current working directory
    char input[PATH_MAX] = {0};
    char output[PATH_MAX] = {0};
    char cwd[PATH_MAX];

    //Verifying input flags
    if(argc < 9) {
        // If there are 2 arguments and the second on is the -h flag, print usage
        // information
        if (argc == 2 && strcmp(argv[1], "-h") == 0) {
            printf("Usage information:\nYou must use the -c, -b, -a, and -i flags\n\n");
            printf("-c <capacity> with <capacity> in KB: 4, 8, 16, 32, or 64\n"
                   "-b <blocksize> with <blocksize> in bytes: 4, 8, 16, 32, 64, 128, 256, or 512\n"
                   "-a <associativity> where <associativity> is integer size of set: 1, 2, 4, 8, or 16\n"
                   "-i <input_file> where <input_file> is the name and / or path of your memory trace file\n"
                   "[-o] <output_file> where <output_file> is the name and / or path of your output file \n\n");
            printf("Example: ./cache_sim -c 8 -b 16 -a 4 -i mem.trace -o mem_trace.txt\n");
            return 0;
        }
        // If there are too few arguments, have the user check the -h
        // flag to see usage information
        printf("Improper command line usage. Use the -h flag to see usage "
               "instructions.\n");
        return 1;
    }

    //Zero the cache
    struct cache cache_memory = zero_cache();

    //Loop over all program parameters
    //First param is program name so we can skip
    for(int i = 1; i < argc; i++) {
        if(strcmp(argv[i], "-c") == 0) {
            //If capacity flag
            i++;
            //Verify validity of the flag
            for(int j = 0; j < (int)strlen(argv[i]); j++) {
                if (!isdigit(argv[i][j]) || strtol(argv[i], NULL, 10) < 0) {
                    printf("capacity must be a positive integer\n");
                    return 1;
                }
            }
            int test = (int) strtol(argv[i], NULL, 10);
            if(!((test != 0 && test != 2) && ((test & (test - 1)) == 0)) || test > 64) {
                printf("capacity must be 4, 8, 16, 32, or 64");
                return 1;
            }
            //Flag contains a valid value, set capacity. Since measured in KB, multiply by 1024
            cache_memory.size = test * 1024;
        } else if(strcmp(argv[i], "-b") == 0) {
            //If block size flag
            i++;
            //Verify validity of flag
            for(int j = 0; j < (int)strlen(argv[i]); j++) {
                if (!isdigit(argv[i][j]) || strtol(argv[i], NULL, 10) < 0) {
                    printf("block size must be a positive integer\n");
                    return 1;
                }
            }
            int test = (int) strtol(argv[i], NULL, 10);
            if(!((test != 0 && test != 2) && ((test & (test - 1)) == 0)) || test > 512) {
                printf("block size must be 4, 8, 16, 32, 64, 128, 256, or 512");
                return 1;
            }
            //Flag is valid and we can set it
            cache_memory.line_size = test;
        } else if(strcmp(argv[i], "-a") == 0) {
            //If associativity flag
            i++;
            //Verify validity of flag
            for(int j = 0; j < (int)strlen(argv[i]); j++) {
                if (!isdigit(argv[i][j]) || strtol(argv[i], NULL, 10) < 0) {
                    printf("associativity must be a positive integer\n");
                    return 1;
                }
            }
            int test = (int) strtol(argv[i], NULL, 10);
            if((!((test != 0) && ((test & (test - 1)) == 0)) || test > 16) && test != 1) {
                printf("associativity must be 1, 2, 4, 8, or 16");
                return 1;
            }
            //Flag is valid, set it
            cache_memory.associativity = test;
        } else if(strcmp(argv[i], "-i") == 0) {
            //If input flag
            i++;

            //Copy value into our input char[]
            strcpy(input, argv[i]);
        } else if(strcmp(argv[i], "-o") == 0) {
            //If output flag
            i++;

            //Copy value into our output char[]
            strcpy(output, argv[i]);
        }
    }

    //Verify that the file input is not empty
    if(input[0] == '\0') {
        printf("File input is empty!\n");
        printf("Improper command line usage. Use the -h flag to see usage "
               "instructions.\n");
        return 1;
    }
    //Verify that proper flags are set
    if(cache_memory.size == 0 || cache_memory.line_size == 0 || cache_memory.associativity == 0) {
        printf("Improper command line usage. Use the -h flag to see usage "
               "instructions.\n");
        return 1;
    }

    //Calculate total number of cache lines and words per line based on params
    cache_memory.total_lines = cache_memory.size / cache_memory.line_size;
    cache_memory.words_per_line = cache_memory.line_size / WORD_SIZE;

    //Print information on the main memory and cache setups
    printf("WORD SIZE: %d\n\n", WORD_SIZE);
    printf("MAIN MEMORY CONFIGURATION:\nSIZE: %d\nBLOCK SIZE: %d\nWORDS PER BLOCK: %d\n"
           "TOTAL BLOCKS: %d\n\n", MM_SIZE, MM_BLOCK_SIZE, MM_WORDS_PER_BLOCK, TOTAL_MM_BLOCKS);
    printf("CACHE MEMORY CONFIGURATION:\nSIZE: %d\nBLOCK SIZE: %d\nWORDS PER BLOCK: %d\n"
           "TOTAL BLOCKS: %d\nASSOCIATIVITY: %d\n\n", cache_memory.size, cache_memory.line_size, cache_memory.words_per_line, cache_memory.total_lines, cache_memory.associativity);

    //Zero out stats
    struct cache_stats stats = zero_stats();

    //Initialize main memory and cache memory to allocate memory to pointers
    struct main_mem_block* main_memory = init_main_mem();
    cache_memory = init_cache_mem(cache_memory);

    //Print info on input file, output file, and the current directory
    printf("INPUT: %s\n", input);
    if(output[0] == '\0') {
        printf("File output is empty!\n");
    } else {
        printf("OUTPUT: %s\n", output);
    }
    if (getcwd(cwd, sizeof(cwd)) != NULL) {
        printf("CURR DIR: %s\n\n", cwd);
    }

    //Trace the input file
    int status = process_trace(input, &cache_memory, &stats, main_memory);

    //Verify the status from the trace
    if(status == 0) {
        //If no output file was specified, just print to the screen
        if(output[0] == '\0') {
            printf("No output file: Just printing to screen\n\n");
        } else {
            //Output file specified, write output to file
            write_cache_and_memory(output, cache_memory, stats, main_memory);
        }
        //Print to the terminal
        print_cache_and_memory(cache_memory, stats, main_memory);
    } else {
        //Trace failed, free memory and exit
        free_io(cache_memory, main_memory);
        return status;
    }

    //Free memory and exit
    free_io(cache_memory, main_memory);
    return 0;
}
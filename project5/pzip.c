#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <sys/mman.h>
#include <sys/sysinfo.h>
#include <stdbool.h>
#include <string.h>

typedef struct {
    __int32_t n;
    char c;
} __attribute__((packed)) data_t;

typedef struct {
    char * fm;
    size_t start_index; // inclusive
    size_t end_index; // exclusive
} pthread_arg_t;

typedef struct {
    data_t * data;
    size_t last_used_index;
} pthread_ret_t;

size_t get_file_length(char *file_name) { //simply finds the end of file and gives the index of that mark.
    FILE *file;
    long len;
    // try to open file
    if ((file = fopen(file_name, "r"))){
        fseek(file, 0L, SEEK_END);
        len = ftell(file);
    } else {
        len = 0;
    }
    fclose(file);
    return len;
}

void *p_compress(void *pthread_args) {  // goes file symbol by symbol and counts them, then returns the chars and amounts
    pthread_arg_t *args = (pthread_arg_t*) pthread_args;

    pthread_ret_t *ret = malloc(sizeof *ret);
    ret->data = malloc(sizeof(data_t) * (args->end_index - args->start_index));

    // begin zipping
    char prev_char = args->fm[args->start_index];
    __int32_t length = 0;
    size_t file_index = args->start_index;
    size_t data_index = 0;

    while (file_index < args->end_index) {
        // check if the character if the same as previous
        if ((args->fm[file_index] == prev_char)) {
            // add 1 to length if it is
            length++;
        } else {
            // write to data if it is not
            ret->data[data_index].n = length;
            ret->data[data_index].c = prev_char;
            data_index++;
            length = 1;
        }
        // set previous character
        prev_char = args->fm[file_index];
        file_index++;
    }

    ret->data[data_index].n = length;
    ret->data[data_index].c = prev_char;
    ret->last_used_index = data_index;

    pthread_exit(ret);
}

void *write_chunk(void *args) {
    pthread_ret_t *pret = (pthread_ret_t *) args;
    fwrite(pret->data, sizeof(data_t), pret->last_used_index + 1, stdout);
    
    pthread_exit(NULL);
}


void compress_file(char *fm, size_t file_length, size_t num_of_threads /*, char lastLetter */) {

    // init arrays for passing to and retrieving from threads
    pthread_t pThreads [num_of_threads];
    pthread_arg_t p_args [num_of_threads];
    pthread_ret_t *p_rets [num_of_threads];
    
    // divide file evenly for threads
    size_t thread_len [num_of_threads];

    for (size_t i = 0; i < num_of_threads; i++) thread_len[i] = file_length / num_of_threads;
    for (size_t i = 1; i < file_length % num_of_threads; i++) thread_len[(num_of_threads - i) % num_of_threads]++;

    // move a part of the length from first thread to last
    size_t change = thread_len[0] * 8 / 10;
    thread_len[0] -= change;
    thread_len[num_of_threads - 1] += change;

    // set args
    p_args[0].fm = fm;
    p_args[0].start_index = 0;
    p_args[0].end_index = thread_len[0];

    for (size_t i = 1; i < num_of_threads; i++) {
        p_args[i].fm = fm;
        p_args[i].start_index = p_args[i - 1].end_index;
        p_args[i].end_index = p_args[i].start_index + thread_len[i];
    }

    // create threads
    for (size_t i = 0; i < num_of_threads; i++) {
        pthread_create(&pThreads[i], NULL, p_compress, &p_args[i]);
    }

    // join the first thread and get data
    // other threads will be joined in the loop
    pthread_join(pThreads[0], (void**) &p_rets[0]);

    pthread_t write_pthread;

    // merge threads and write data
    for (size_t thread = 0; thread < num_of_threads; thread++) {
        //printf("%d %c\n", p_rets[thread]->data->n, p_rets[thread]->data->c);

        if (thread + 1 < num_of_threads ) {
            // if this is the last chunk of data from this thread
            // make sure the next thread has finished
            pthread_join(pThreads[thread + 1], (void**) &p_rets[thread + 1]);
            

            if (p_rets[thread]->data[p_rets[thread]->last_used_index].c == p_rets[thread + 1]->data[0].c) {
                // if next chunk would be the same char, add the repetitions to the next one

                p_rets[thread + 1]->data[0].n += p_rets[thread]->data[p_rets[thread]->last_used_index].n;
                p_rets[thread]->data[p_rets[thread]->last_used_index].n = 1;
                p_rets[thread]->last_used_index -= 1;
            }

        }

        // write the data
        if (thread > 0) {
            pthread_join(write_pthread, (void**) NULL);
        }
        pthread_create(&write_pthread, NULL, write_chunk, p_rets[thread]);
    }
    pthread_join(write_pthread, (void**) NULL);
    
    // free memory
    for (size_t i = 0; i < num_of_threads; i++) {
        free(p_rets[i]->data);
        p_rets[i]->data = NULL;
        free(p_rets[i]);
        p_rets[i] = NULL;
    }

} 


int main(int argc, char *argv[]) {

    if (argc < 2) {
        printf("pzip: file1 [file2 ...]\n");
        exit(1);
    }

    FILE *fp;
    char * fm = NULL;
    size_t currentSize = 3;
    fm = malloc(sizeof(char) * currentSize);
    size_t file_length = 0;

    // This is not a smart solution but atleast on my machine this is close to the optimal number of threads.
    // From my testing I can say that writing data is the slowest part of this program.
    // Increasing the number of threads makes it so you can start writing earlier 
    // since each data chunk is smaller and thus takes less time to compute.
    // But too many threads will slow down the program a lot (maybe the writing thread gets pushed back??)
    // I am too lazy to re-write this program to have a permanent writing thread 
    // that would get paused when no data is available for writing
    size_t num_of_threads = get_nprocs();

    for (size_t i = 1; i < argc; i++) {
        if ((fp = fopen(argv[i], "r"))){ //opens a file then makes a memory map with mmap, then adds it to overal memory map   
            file_length += get_file_length(argv[i]); 
            currentSize += file_length;
            char * tempfm = mmap(NULL, file_length, PROT_READ, MAP_PRIVATE, fileno(fp), 0);
            fm = realloc(fm, sizeof(char)*currentSize);
            strcat(fm, tempfm);
            fclose(fp);
            fp = NULL;
            tempfm = NULL;
            
        } else {
            // if opening fails, exit
            printf("failed opening file %s\n", argv[i]);
            exit(1);
        }
    }
    file_length += 1;             // fixes files that doesn't have empty line at end of the file 
    compress_file(fm, file_length, num_of_threads); // compresses the complete memory map
    free(fm);
    fm = NULL;
    exit(0);
}

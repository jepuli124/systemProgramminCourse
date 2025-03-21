#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>
#include <sys/file.h>
#include <time.h>
#include <sys/wait.h>
#include <sys/sysinfo.h>
#include <pthread.h>
#include <sys/mman.h>
#include <math.h>

// this has to packed so it is exactly 5 bytes
typedef struct {
    __int32_t n;
    char c;
} __attribute__((packed)) data_t;

typedef struct {
    char * fm;
    size_t start_index; // inclusive

} pthread_arg_t;

void decompress_file(pthread_arg_t *p_args, char ** returnValue) {
    data_t buffer;

    char * text = malloc(sizeof(char)*2); //allocating start memory
    text[0] = '\0';
    // read data into buffer by byte at time
    buffer.n = (p_args->fm[p_args->start_index+3] << 24) | (p_args->fm[p_args->start_index+2] << 16) | (p_args->fm[p_args->start_index+1] << 8) | (p_args->fm[p_args->start_index]);
    buffer.c = p_args->fm[p_args->start_index+4];

    
    // print the given character the given number of times
    for (size_t i = 0; i < buffer.n; i++) {
        text[strlen(text)+1] = '\0'; // adding end of text mark to prevent random symbols from new memory to spawn 
        if ((text = realloc(text, sizeof(char)*(strlen(text)+5))) == NULL) { //allocating correct amount of memory
            perror("realloc didn't succeed \n");
            } 
        text[strlen(text)] = buffer.c;
    }
    
    //printf("inside decompress file:\n %s\n", text);
    text[strlen(text)+1] = '\0'; //adding end mark
    *returnValue = text;
    text = NULL;
    return;
}

void * runThread(void *p_args){ //the function that the thread runs, makes stuff easy to handle

    char * returnValues;
    decompress_file((pthread_arg_t *) p_args, &returnValues);

    return returnValues;
}

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

int main(int argc, char *argv[]) {
    // if atleast 1 file is not given, exit
    if (argc < 2) {
        printf("my-unzip: file1 [file2 ...]\n");
        exit(1);
    }

    FILE *fp;

    for (size_t i = 1; i < argc; i++) {
        if ((fp = fopen(argv[i], "r"))){
            size_t file_length = get_file_length(argv[i]); 
            //printf("%li", file_length);
            
            char * tempfm = mmap(NULL, file_length, PROT_READ, MAP_PRIVATE, fileno(fp), 0);
            if (tempfm == MAP_FAILED) {
                perror("mmap failed");
                exit(1);
            }
            
            // prevents missing characters at end of file (hopefully)
            //file_length += 1; 

            // a way to prevent too many threads
            size_t threadLimit = get_nprocs();

            // the loop for threads.
            for (size_t counter = 0; counter < file_length; counter += threadLimit * 5)
            {
                // each thread handles single characater 
                size_t num_of_threads = 0; 
                if (counter + (threadLimit * 5) > file_length) {
                    num_of_threads = (size_t) floor((file_length - counter)/5);
                } else {
                    num_of_threads = threadLimit;
                }
                void ** returnValues = malloc(sizeof(char *) * (num_of_threads));
                
                
                pthread_arg_t * p_args = malloc(sizeof(pthread_arg_t) * (num_of_threads));

                // set args
                
                p_args[0].fm = tempfm;
                p_args[0].start_index = counter;


                for (size_t i = 1; i < num_of_threads; i++) {
                    p_args[i].fm = tempfm;
                    p_args[i].start_index = counter + (i * 5);

                }
                
                pthread_t * thread_id = malloc(sizeof(pthread_t) * (num_of_threads));    // Allocating a slot for each thread. https://www.geeksforgeeks.org/multithreading-in-c/

                for (size_t i = 0; i < num_of_threads; i++) { //creating a thread for each file.
                    pthread_create(&thread_id[i], NULL, runThread, &p_args[i]);
                    
                }

                for (size_t i = 0; i < num_of_threads; i++) { //waiting that all threads complete
                    pthread_join(thread_id[i], &returnValues[i]);
                }
                
                for (size_t i = 0; i < num_of_threads; i++) { //printing out values
                    printf("%s", (char *) returnValues[i]);
                    
                }
                for (size_t i = 0; i < num_of_threads; i++) { // free memory
                    // free(p_args[i].fm);
                    // p_args[i].fm = NULL;
                    
                    free(returnValues[i]);
                }
                free(thread_id); // free some values
                free(p_args);
                free(returnValues);
                p_args = NULL;
                returnValues = NULL;
                
            }

            fclose(fp); // free some more values
            fp = NULL;
            tempfm = NULL;
            
        } else {
            // if opening fails, exit 
            printf("failed opening file %s\n", argv[i]);
            exit(1);
            
        }
    }
    exit(0);
}

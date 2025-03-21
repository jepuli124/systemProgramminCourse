
#define  _GNU_SOURCE
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>

void reverse_file(FILE *file_out, FILE *file_in, int argc) { // reverses file
    char *string = NULL;
    size_t n = 0;

    if (argc == 1 && getline(&string, &n, file_in) > 1) { // recursivly calls it self and takes a line to output causing lines appearing in reverse order.  
        reverse_file(file_out, file_in, argc);
        fprintf(file_out, "%s", string);
    }
    else if (argc > 1 && getline(&string, &n, file_in) != -1) {
        reverse_file(file_out, file_in, argc);
        fprintf(file_out, "%s", string);
    }
    free(string);
    return;
}

int same_file(int fd1, int fd2) { // https://stackoverflow.com/questions/12502552/can-i-check-if-two-file-or-file-descriptor-numbers-refer-to-the-same-file 
    struct stat stat1, stat2;
    if(fstat(fd1, &stat1) < 0) return -1; //Gets file info and puts it in stat buffer 
    if(fstat(fd2, &stat2) < 0) return -1; 
    return (stat1.st_dev == stat2.st_dev) && (stat1.st_ino == stat2.st_ino); // chechks if the files information is equal meaning that they are the same file
}


int main(int argc, char *argv[]) {
    if (3 < argc) {  // if there is less than 3 arguments, gives error
        fprintf(stderr, "usage: reverse <input> <output>\n");
        exit(1);
    }

    

    if ((argc == 3) && (0 == strcmp(argv[1], argv[2]))) { // if file paths are same, gives error
        fprintf(stderr, "error: input and output file must differ\n");
        exit(1);
    }

    //basic file handling session: open a file and check if it opened.
    FILE *file_out;
    file_out = stdout;
    if (argc > 2) {
        if (!(file_out = fopen(argv[2], "w"))) {
            fprintf(stderr, "error: cannot open file '%s'\n", argv[2]);
            exit(1);
        }
    }

    FILE *file_in;
    file_in = stdin;
    if (argc > 1) {
        if (!(file_in = fopen(argv[1], "r"))) {
            fprintf(stderr, "error: cannot open file '%s'\n", argv[1]);
            exit(1);
        }
    }
    if (same_file(fileno(file_in), fileno(file_out))) { // checks if the files are hard linked, if so gives error
        fprintf(stderr, "error: input and output file must differ\n");
        exit(1);
    }


    reverse_file(file_out, file_in, argc); // reverses given file
    
    if (argc > 1) { // closes file if file pointer is given
        fclose(file_in);
    }
    if (argc > 2) { 
        fclose(file_out);
    }

    exit(0); // exits
}

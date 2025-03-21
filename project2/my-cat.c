#include <stdlib.h>
#include <stdio.h>

void print_file(FILE *file_out, FILE *file_in) {
    char *line = NULL;
    size_t n = 0;

    // get line
    while (getline(&line, &n, file_in) > 0) { // gets and prints the file line by line 
        // print line
        fprintf(file_out, "%s", line);
        free(line); // frees the memory from the line pointer
        line = NULL;
    }
    free(line);
    line = NULL;
    return;
}

int main(int argc, char *argv[]) {
    
    // if atleast 1 input file is not given, exit
    if (argc < 2) {
        exit(0);
    }

    FILE *file_out = stdout;

    // for each input file
    for (size_t i = 1; i < argc; i++) {
        FILE *file_in;
        // try to open file
        if (!(file_in = fopen(argv[i], "r"))) {
            // if fails, exit
            fprintf(stdout, "my-cat: cannot open file\n");
            exit(1);
        }
        // print lines
        print_file(file_out, file_in);
        fclose(file_in);
    }
    
    exit(0);
}

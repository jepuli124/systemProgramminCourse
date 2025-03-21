
#include <stdlib.h>
#include <stdio.h>


int search_line_for_term(char *line, char *search_term){
    size_t i = 0, j = 0;

    // check if line has ended
    while (line[i] != '\n' && line[i] != '\0'){

        // if search terms end has been reached, return true
        if (search_term[j] == '\0') {
            return 1;
        }
        
        // compare possible matching position to search term
        if (line[i + j] == search_term[j]) {
            j++;
        } else {
            j = 0;
            i++;
        }
    }
    return 0;
}

 
void print_file(FILE *file_out, FILE *file_in, char *search_term) {
    char *line = NULL;
    size_t n = 0;

    // get line
    while (getline(&line, &n, file_in) > 0) {
        // check if search term is in line
        if (search_line_for_term(line, search_term)) {
            fprintf(file_out, "%s", line);
        }
        free(line);
        line = NULL;
    }
    free(line);
    line = NULL;

    return;
}

int main(int argc, char *argv[]) {
    
    // if atleast search term is not given, exit
    if (argc < 2) {
        fprintf(stdout, "my-grep: searchterm [file ...]\n");
        exit(1);
    }

    FILE *file_out = stdout;

    // if input file is not given, use stdin
    if (argc < 3) {
        // print matching lines
        print_file(file_out, stdin, argv[1]);
    }

    // for each input file
    for (size_t i = 2; i < argc; i++) {
        FILE *file_in;
        // try to open file
        if (!(file_in = fopen(argv[i], "r"))) {
            // if fails, exit
            fprintf(stdout, "my-grep: cannot open file\n");
            exit(1);
        }
        // print matching lines
        print_file(file_out, file_in, argv[1]);
        fclose(file_in);
    }
    
    exit(0);
}

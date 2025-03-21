
#define  _GNU_SOURCE
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

void reverse_file(FILE *file_out, FILE *file_in, int argc) {
    char *string = NULL;
    size_t n = 0;

    if (argc == 1 && getline(&string, &n, file_in) > 1) {
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

int main(int argc, char *argv[]) {
    if (3 < argc) {
        fprintf(stderr, "usage: reverse <input> <output>\n");
        exit(1);
    }

    if ((argc == 3) && (0 == strcmp(argv[1], argv[2]))) {
        fprintf(stderr, "Input and output file must differ\n");
        exit(1);
    }

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


    reverse_file(file_out, file_in, argc);
    
    if (argc > 1) {
        fclose(file_in);
    }
    if (argc > 2) {
        fclose(file_out);
    }

    exit(0);
}

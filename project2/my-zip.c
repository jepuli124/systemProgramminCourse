#include <stdlib.h>
#include <stdio.h>

void compress_file(FILE *file) {
    // previous character is set to terminator so it cannot be matched
    char prev_char = '\0';
    __int32_t length = 1;

    // while not at end-of-file
    while (!feof(file)) {
        // read character
        char c = fgetc(file);
        // check if the character if the same as previous
        // or the previous character is the default
        if ((c == prev_char || prev_char == '\0')) {
            // add 1 to length if it is
            length++;
        } else {
            // write if it is not
            fwrite(&length, sizeof(__int32_t), 1, stdout);
            fwrite(&prev_char, sizeof(char), 1, stdout);
            length = 1;
        }
        // set previous character
        prev_char = c;
    }
}

int main(int argc, char *argv[]) { 
    // if atleast 1 file is not given, exit
    if (argc < 2) {
        printf("my-zip: file1 [file2 ...]\n");
        exit(1);
    }

    for (size_t i = 1; i < argc; i++) {
        FILE *file;
        // try to open file
        if ((file = fopen(argv[i], "r"))){
            compress_file(file);
        } else {
            // if opening fails, exit
            fclose(file);
            file = NULL;
            exit(1);
        }
        fclose(file);
        file = NULL;
    }

    exit(0);
}

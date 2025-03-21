#include <stdlib.h>
#include <stdio.h>

void compress_file(FILE *file, char *prev_char, __int32_t *length) {


    // while not at end-of-file
    while (!feof(file)) {
        // read character
        char c = fgetc(file);

        // break if this is the last character (terminator)
        if (feof(file)) {
            break;
        }

        // check if the character if the same as previous
        // or the previous character is the default
        if ((c == *prev_char || *prev_char == '\0')) {
            // add 1 to length if it is
            (*length)++;
        } else {
            // write if it is not
            fwrite(length, sizeof(__int32_t), 1, stdout);
            fwrite(prev_char, sizeof(char), 1, stdout);
            *length = 1;
        }

        // set previous character
        *prev_char = c;
    }
}

int main(int argc, char *argv[]) { 
    // if atleast 1 file is not given, exit
    if (argc < 2) {
        printf("my-zip: file1 [file2 ...]\n");
        exit(1);
    }

    // previous character is set to terminator so it cannot be matched
    // these persist between function calls since files should be melded together
    char prev_char = '\0';
    __int32_t length = 0;

    for (size_t i = 1; i < argc; i++) {
        FILE *file;
        // try to open file
        if ((file = fopen(argv[i], "r"))){
            compress_file(file, &prev_char, &length);
        } else {
            // if opening fails, exit
            fclose(file);
            file = NULL;
            exit(1);
        }
        fclose(file);
        file = NULL;


        if (i + 1 >= argc) {
            // write the characters if this was the last file
            fwrite(&length, sizeof(__int32_t), 1, stdout);
            fwrite(&prev_char, sizeof(char), 1, stdout);
        }
    }

    exit(0);
}

#include <stdlib.h>
#include <stdio.h>

// this has to packed so it is exactly 5 bytes
typedef struct {
    __int32_t n;
    char c;
} __attribute__((packed)) data_t;

void decompress_file(FILE *file) {
    data_t buffer;
    // read data into buffer
    while((fread(&buffer, sizeof(data_t), 1, file))){ 
        // print the given character the given number of times
        for (size_t i = 0; i < buffer.n; i++) {
            printf("%c", buffer.c);
        }
    }
}

int main(int argc, char *argv[]) {
    // if atleast 1 file is not given, exit
    if (argc < 2) {
        printf("my-unzip: file1 [file2 ...]\n");
        exit(1);
    }
    
    for (size_t i = 1; i < argc; i++) {
        FILE *file;
        // try to open file
        if ((file = fopen(argv[i], "r"))){
            decompress_file(file);
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

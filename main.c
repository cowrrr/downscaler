#define STB_IMAGE_IMPLEMENTATION
#include "stb/stb_image.h"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb/stb_image_write.h"

#include <stdio.h>

#include <sys/stat.h>

int width, height, channels;
long int FileSize(const char* input);
void WriteImage(int compr, unsigned char *input);

int main(int argc, char *argv[]) {
    if (argc < 3) {
        fprintf(stderr, "Usage: %s <input_file> <target_size_in_bytes>\n", argv[0]);
        return 1;
    }

    char *endptr;
    long int target_size = strtol(argv[2], &endptr, 10);
    int best_low = 0;
    int best_high = 100;
    int middle;
    double tenprtarget = (target_size * 0.95);

    if (*endptr != '\0' || target_size <= 0) {
        fprintf(stderr, "Invalid target size. Please provide a positive number of bytes.\n");
        return 1;
    }

    char *input_file = argv[1];
    

    char output_file[11] = "output.jpg";

    unsigned char *input = stbi_load(input_file, &width, &height, &channels, 0);
    if (!input) {
        fprintf(stderr, "failed to load image\n");
        return 1;
    }

    if (!target_size) {
        fprintf(stderr, "no target size");
        return 1;
    }
    printf("123\n");
    long int current_size = FileSize(input_file);
    printf("%ld\n", current_size);

    while (current_size >= tenprtarget) {
        printf("trying to find size\n");
        middle = (best_low + best_high) / 2;
        WriteImage(middle, input);
        current_size = FileSize(output_file);
        if (current_size < target_size) {
            best_low = middle;
        };
        if (current_size > target_size) {
            best_high = middle;
        };
    }

    stbi_image_free(input);
    printf("done");
    return 0;
}

long int FileSize(const char* input) {
    struct stat st;
    if (stat(input, &st) != 0) {
        perror("Failed to get input file size");
        return 1;
    };
    long int current_size = st.st_size;
    return current_size;
}

void WriteImage(int compr, unsigned char *input) {
    if (!stbi_write_jpg("output.jpg", width, height, channels, input, compr)) {
        fprintf(stderr, "Error writing output with quality %d\n", compr);
    };
}
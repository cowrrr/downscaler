#include <stdlib.h>
#define STB_IMAGE_IMPLEMENTATION
#include "stb/stb_image.h"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb/stb_image_write.h"

#include <stdio.h>

#include <sys/stat.h>
#include <unistd.h>

struct stat st;
int width, height, channels;
long int FileSize(const char* input);
void WriteImage(int compr, unsigned char *input);

int main(int argc, char *argv[]) {
    if (argc < 5) {
        fprintf(stderr, "Usage: %s <input_file> <target_size_in_bytes> <max_attempts> <range in %%>\n", argv[0]);
        return 1;
    }

    char output_file[11] = "output.jpg";

    if (stat(output_file, &st) == 0) {
        if (unlink(output_file) == 0) {
            printf("Deleted old file\n");
        } else {
            perror("Error deleting old image");
            return 1;
        }
    }

    char *endptr;
    long int target_size = strtol(argv[2], &endptr, 10);
    float range = strtol(argv[4], &endptr, 10);
    int best_low = 0;
    int best_high = 100;
    int middle = 100;
    double tenprtarget = (target_size * (1 - (range / 100)));
    int attempts = 1;
    int max_attempts = strtol(argv[3], &endptr, 10);

    if (*endptr != '\0' || target_size <= 0) {
        fprintf(stderr, "Invalid target size. Please provide a positive number of bytes.\n");
        return 1;
    }

    char *input_file = argv[1];
    
    unsigned char *input = stbi_load(input_file, &width, &height, &channels, 0);
    if (!input) {
        fprintf(stderr, "failed to load image\n");
        return 1;
    }

    if (!target_size) {
        fprintf(stderr, "no target size");
        return 1;
    }
    long int current_size = FileSize(input_file);
    printf("%ld\n", current_size);

    while (current_size >= tenprtarget && attempts <= max_attempts && middle >= 1) {
        middle = (best_low + best_high) / 2;
        WriteImage(middle, input);
        current_size = FileSize(output_file);
        if (current_size < target_size) {
            best_low = middle;
        };
        if (current_size > target_size) {
            best_high = middle;
        };
        attempts++;
        printf("%ld\n", current_size);
    }

    stbi_image_free(input);
    if (attempts >= max_attempts) {
        printf("unable to find exact match in %d attempts\n", max_attempts);
    }
    else if (middle < 1) {
        printf("exact compression level falls below 1\n");
    }
    else {
        printf("done in %d attempts with compression level %d/100\n", attempts, middle);
    }
    return 0;
}

long int FileSize(const char* input) {
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
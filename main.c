#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#define STB_IMAGE_IMPLEMENTATION
#include "stb/stb_image.h"
#include <math.h>
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb/stb_image_write.h"

#include <stdio.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>

struct stat st;
int width, height, channels;
long int FileSize(const char* input);
void WriteImage(int compr, unsigned char *input);
char output_file[11] = "temp.jpg";
char *output_name = NULL;
char *input_file = NULL;
int verbose;
int replace = 1;
double target_size;
char *endptr;
int max_attempts = 10;
float range = 10;

int main(int argc, char *argv[]) {
    if (argc < 3) {
        fprintf(stderr, "Usage: %s <input_file> <target_size_in_megabytes> <max_attempts: opt> <range in %%: opt> <replace: opt>\n", argv[0]);
        return 1;
    }

    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-v") == 0 || strcmp(argv[i], "-verbose") == 0) {
            verbose = 1;
        }
        else if (strcmp(argv[i], "-i") == 0 || strcmp(argv[i], "-input") == 0) {
            if (i + 1 < argc) {
                input_file = argv[i + 1];
                i++;
            } else {
                fprintf(stderr, "Error: -i or --input requires a filename.\n");
                return 1;
            }
        } else if (strcmp(argv[i], "-o") == 0 || strcmp(argv[i], "-output") == 0) {
            if (i + 1 < argc) {
                output_name = argv[i + 1];
                replace = 0;
                i++;
            } else {
                fprintf(stderr, "Error: -o or --output requires a filename.\n");
                return 1;
            }
        } else if (strcmp(argv[i], "-t") == 0 || strcmp(argv[i], "-target") == 0) {
            if (i + 1 < argc) {
                target_size = strtol(argv[i + 1], &endptr, 10);
                target_size = target_size * 1024 * 1024;
            } else {
               fprintf(stderr, "Error: -t or -target requires a size.\n");
               return 1; 
            }
        } else if (strcmp(argv[i], "-ma") == 0) {
            if (i + 1 < argc) {
                max_attempts = strtol(argv[i + 1], &endptr, 10);
            } else {
                fprintf(stderr, "Error: -ma requires a value\n");
            }
        } else if (strcmp(argv[i], "-r") == 0) {
            if (i + i < argc) {
                range = strtol(argv[i + 1], &endptr, 10);
            } else {
                fprintf(stderr, "Error: -r requires a value\n");
            }
        }

    }


    printf("%f\n", target_size);
    int best_low = 0;
    int best_high = 100;
    int middle = 100;
    double tenprtarget = (target_size * (1 - (range / 100)));
    int attempts = 1;

    if (*endptr != '\0' || target_size <= 0) {
        fprintf(stderr, "Invalid target size. Please provide a positive number of megabytes\n");
        return 1;
    }

    
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
    // move temp file to original file
    if (replace == 1) {
        rename(output_file, input_file);
    }   else {
        rename(output_file, "output.jpg");
    }
    
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
    if (!stbi_write_jpg(output_file, width, height, channels, input, compr)) {
        fprintf(stderr, "Error writing output with quality %d\n", compr);
    };
}
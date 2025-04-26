#define STB_IMAGE_IMPLEMENTATION
#include "stb/stb_image.h"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb/stb_image_write.h"

#include <stdio.h>

#include <sys/stat.h>

int main(int argc, char *argv[]) {
    int width, height, channels;
    long int target_size = atoi(argv[2]);
    char *input_file = argv[1];
    int current_comp_level = 50;

    struct stat st;

    unsigned char *input = stbi_load(input_file, &width, &height, &channels, 0);
    if (!input) {
        fprintf(stderr, "failed to load image\n");
        return 1;
    }

    stat(input_file, &st);
    long int current_size = st.st_size;
    printf("%ld\n", current_size);

    while (current_size > target_size && current_comp_level >= 1) {
        stbi_write_jpg("output.jpg", width, height, channels, input, current_comp_level);
        stat("output.jpg", &st);
        current_size = st.st_size;
        if (current_size > target_size) {
            current_comp_level = (current_comp_level / 2);
        }
        else {
            while (current_size < target_size && current_comp_level >= 1) {
                current_comp_level++;
                stbi_write_jpg("output.jpg", width, height, channels, input, current_comp_level);
                stat("output.jpg", &st);
                current_size = st.st_size;
                if (current_size > target_size) {
                    current_comp_level--;
                    stbi_write_jpg("output.jpg", width, height, channels, input, current_comp_level);
                    goto ending;
                }
            }
        }
    }

    ending:
        printf("%d\n", current_comp_level);
        printf("image resized successfully\n");

        stbi_image_free(input);
        return 0;
}
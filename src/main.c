#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"
#include "ini.h"
#include <stdio.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>
#include <errno.h>

#if defined(_WIN32)
    #define PLATFORM_NAME "windows"
    #define DEFAULT_CONFIG_LOCATION "C:\Program Files\Downscaler\share\downscaler\config.ini"
#elif defined(_WIN64)
    #define PLATFORM_NAME "windows"
    #define DEFAULT_CONFIG_LOCATION "C:\Program Files\Downscaler\share\downscaler\config.ini"
#elif defined(__linux__)
    #define PLATFORM_NAME "linux"
    #define DEFAULT_CONFIG_LOCATION "/usr/local/share/downscaler/config.ini"
#elif defined(__APPLE__) && defined(__MACH__)
    #define PLATFORM_NAME "osx"
    #define DEFAULT_CONFIG_LOCATION "/usr/local/share/downscaler/config.ini"
#else
    #define PLATFORM_NAME NULL
#endif

typedef struct
{
    int verbose;
    int target_size;
    int max_attempts;
    int range;
} configuration;

typedef struct
{
    char config_location[256];
    char config_dir[256];
} configpaths;

static int handler(void* Defaults, const char* section, const char* name,
                   const char* value)
{
    configuration* pconfig = (configuration*)Defaults;

    #define MATCH(s, n) strcmp(section, s) == 0 && strcmp(name, n) == 0
    if (MATCH("Defaults", "verbose")) {
        pconfig->verbose = atoi(value);
    } else if (MATCH("Defaults", "target_size")) {
        pconfig->target_size = atoi(value);
    } else if (MATCH("Defaults", "max_attempts")) {
        pconfig->max_attempts = atoi(value);
    } else if (MATCH("Defaults", "range")) {
        pconfig->range = atoi(value);
    } else {
        return 0;  /* unknown section/name, error */
    }
    return 1;
}
struct stat st;
int width, height, channels;
long int FileSize(const char* input);
void WriteImage(int compr, unsigned char *input);
char output_file[11] = "temp.jpg";
char *output_name = NULL;
char *input_file = NULL;
int replace = 1;
char *endptr;

int copy_config_file(const char* src, const char* dest, const char* config_dir);
configpaths set_config();

int main(int argc, char *argv[]) {
    configpaths new_paths = set_config();
    configuration config;

    if (ini_parse(new_paths.config_location, handler, &config) < 0) {
        printf("%s%s\n", "Can't load 'config.ini' from", new_paths.config_location);

        if (copy_config_file(DEFAULT_CONFIG_LOCATION, new_paths.config_location, new_paths.config_dir) != 0) {
            printf("failed to copy default config file\n");
        }
        if (ini_parse(new_paths.config_location, handler, &config) < 0) {
            printf("exhausted all options for loading the config file\n");
        }
    }

    int verbose = config.verbose;
    double target_size = config.target_size;
    int max_attempts = config.max_attempts;
    float range = config.range;
    target_size = target_size * 1024 * 1024;

    if (argc < 2) {
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
                target_size = config.target_size; 
                target_size = target_size * 1024 * 1024;
            }
        } else if (strcmp(argv[i], "-ma") == 0) {
            if (i + 1 < argc) {
                max_attempts = strtol(argv[i + 1], &endptr, 10);
            } else {
                max_attempts = config.max_attempts;
            }
        } else if (strcmp(argv[i], "-r") == 0) {
            if (i + i < argc) {
                range = strtol(argv[i + 1], &endptr, 10);
            } else {
                range = config.range;
            }
        }

    }


    if (verbose == 1) {
        printf("%s%f\n", "target size is ", target_size);
    }
    int best_low = 0;
    int best_high = 100;
    int middle = 100;
    double tenprtarget = (target_size * (1 - (range / 100)));
    int attempts = 1;

    if (target_size <= 0) {
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
        if (verbose == 1) {
            printf("%s%ld\n", "current size is ", current_size);
        }
    }

    stbi_image_free(input);
    // move temp file to original file
    if (replace == 1) {
        rename(output_file, input_file);
    }   else {
        rename(output_file, "output.jpg");
    }

    if (attempts >= max_attempts) {
        if (verbose == 1) {
            printf("unable to find exact match in %d attempts\n", max_attempts);
        }
    }
    else if (middle < 1) {
        if (verbose == 1) {
            printf("exact compression level falls below 1\n");
        }
    }
    else {
        if (verbose == 1) {
            printf("done in %d attempts with compression level %d/100\n", attempts, middle);
        }
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

int copy_config_file(const char* src, const char* dest, const char* config_dir) {
    int src_fd = open(src, O_RDONLY);
    if (src_fd == -1) {
        perror("Failed to open source config file");
        return -1;
    }

    if (mkdir(config_dir, 0755) == 0) {
        printf("Directory '%s' created successfully.\n", "downscaler");
    } else {
        if (errno == EEXIST) {
            printf("Error: Directory '%s' already exists.\n", "downscaler");
        } else {
            perror("Error creating directory"); // Prints a descriptive error message based on errno
        }
        return 1;
    }
    int dest_fd = open(dest, O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (dest_fd == -1) {
        perror("Failed to create destination config file");
        close(src_fd);
        return -1;
    }

    char buffer[8192];
    ssize_t bytes_read, bytes_written;

    while ((bytes_read = read(src_fd, buffer, sizeof(buffer))) > 0) {
        bytes_written = write(dest_fd, buffer, bytes_read);
        if (bytes_written != bytes_read) {
            perror("Failed to write to destination file");
            close(src_fd);
            close(dest_fd);
            return -1;
        }
    }

    if (bytes_read == -1) {
        perror("Failed to read from source file");
    }

    close(src_fd);
    close(dest_fd);
    return (bytes_read == -1) ? -1 : 0;
}

configpaths set_config() {
    const char *user = getenv("USER");

    if (user == NULL) {
        fprintf(stderr, "USER environment variable not found.\n");
    }

    configpaths new_config;
    int chars_written;

    if (strcmp(PLATFORM_NAME, "linux")) {
        chars_written = snprintf(new_config.config_location, sizeof(new_config.config_location), "/home/%s/.config/downscaler/config.ini", user);
    } else if (strcmp(PLATFORM_NAME, "osx")) {
        chars_written = snprintf(new_config.config_location, sizeof(new_config.config_location), "/home/%s/Library/Application Support/downscaler/config.ini", user);
    }

    if (chars_written < 0 || (size_t)chars_written >= sizeof(new_config.config_location)) {
        fprintf(stderr, "Error: Path string too long or snprintf error.\n");
        if (chars_written >= (int)sizeof(new_config.config_location)) {
            fprintf(stderr, "Path was truncated.\n");
        }
    }


    if (strcmp(PLATFORM_NAME, "linux") == 0) {
        chars_written = snprintf(new_config.config_dir, sizeof(new_config.config_dir), "/home/%s/.config/downscaler", user);
    } else if (strcmp(PLATFORM_NAME, "osx") == 0) {
        chars_written = snprintf(new_config.config_dir, sizeof(new_config.config_dir), "/Users/%s/Library/Application Support/downscaler", user);
    }

    if (chars_written < 0 || (size_t)chars_written >= sizeof(new_config.config_dir)) {
        fprintf(stderr, "Error: Path string too long or snprintf error.\n");
        if (chars_written >= (int)sizeof(new_config.config_dir)) {
            fprintf(stderr, "Path was truncated.\n");
        }
    }

    return new_config;
}

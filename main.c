#include <tostring.h>

#define MAX_PATH 256U
#define CONSOLE  false

int main(const int argc, char* argv[]) {
    if (argc == 1) {
        fputs("error - expected at least one argument (file path)\n", stderr);
        return EXIT_FAILURE;
    }

    bitmap image          = { 0 };
    char   path[MAX_PATH] = { 0 }; // use this to replace the .bmp extension with .txt for file outputs
    string repr           = { 0 };
    char*  period         = NULL; // pointer to the . in .bmp in the file names

    for (int i = 1; i < argc; ++i) {
        image = bmpread(argv[i]);
        repr  = to_string(&image, CONSOLE);

        if (!repr.buffer) {
            printf("Error :: failed processing image %s!\n", argv[i]);
            bmpclose(&image);
            continue; // move on to the next image
        }

        if (CONSOLE) {
            puts(repr.buffer);
            puts("\n\n");
        } else {
            strncpy(path, argv[i], MAX_PATH);
            period  = strstr(path, ".bmp");
            *period = 0; // replace the . with a null terminator
            strncat(path, ".txt", 4);

            _write(path, repr.buffer, repr.size); // if anything fails - _write would report them
            // so no need for error handling here
            memset(path, 0, MAX_PATH);
            period = NULL;
        }

        free(repr.buffer);
        bmpclose(&image);
    }

    return EXIT_SUCCESS;
}

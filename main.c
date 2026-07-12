#include <tostring.h>

int main(const int argc, char* argv[]) {
    if (argc == 1) {
        fputs("error - expected at least one argument (file path)\n", stderr);
        return EXIT_FAILURE;
    }

    bitmap      image = { 0 };
    const char* repr  = NULL;

    for (int i = 1; i < argc; ++i) {
        image = bmpread(argv[i]);
        repr  = to_string(&image);

        if (!repr) {
            printf("Error :: failed processing image %s!\n", argv[i]);
            bmpclose(&image);
            continue; // move on to the next image
        }

        puts(repr);
        puts("\n\n");

        free(repr);
        bmpclose(&image);
    }

    return EXIT_SUCCESS;
}

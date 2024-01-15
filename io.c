#include <stdio.h>
#include <stdlib.h>
#include <libgen.h>
#include <unistd.h>

#ifdef USE_CLOCK
#include <time.h>
#else
#include "cycles.h"
#endif

#define FNMAX 255

void transform_image(char *source, char *curve, int light, char *dest) {
    FILE *in, *map, *out;
    char c1, c2;
    int height, width, maxval;
    long size;
    unsigned char *source_image, *dest_image, *lut;

    in = fopen(source, "r");
    map = fopen(curve, "r");
    out = fopen(dest, "w");

    // Error handling for file opening
    if (in == NULL || map == NULL || out == NULL) {
        perror("Error opening file");
        exit(EXIT_FAILURE);
    }

    fscanf(in, "P%c%c %d %d %d", &c1, &c2, &height, &width, &maxval);

    // Check if input file is PGM
    if (c1 != '5' || c2 != '\n') {
        fprintf(stderr, "Error, input file is not PGM\n");
        exit(EXIT_FAILURE);
    }

    // Check if it's an 8-bit grayscale image
    if (maxval > 255) {
        fprintf(stderr, "Input file is not an 8-bit grayscale image.\n");
        exit(EXIT_FAILURE);
    }

    size = width * height;

    // Allocate memory for images and look-up table
    source_image = (unsigned char *)malloc(sizeof(unsigned char) * size);
    dest_image = (unsigned char *)malloc(sizeof(unsigned char) * size);
    lut = (unsigned char *)malloc(sizeof(unsigned char) * 256);

    if (source_image == NULL || dest_image == NULL || lut == NULL) {
        perror("Memory allocation error");
        exit(EXIT_FAILURE);
    }

    // Read look-up table and source image
    fread(lut, 1, 256, map);
    fread(source_image, 1, size, in);

#ifdef USE_CLOCK
    clock_t start = clock();
#else
    start_counter();
#endif

    // Perform image transformation
    transfo(width, height, source_image, dest_image, lut, light);

#ifdef USE_CLOCK
    clock_t stop = clock();
    printf("%ld clock cycles.\n", stop - start);
#else
    double t = get_counter();
    printf("%f clock cycles.\n", t);
#endif

    // Write the transformed image to the output file
    fprintf(out, "P5\n%d %d\n%d\n", width, height, maxval);
    fwrite(dest_image, 1, size, out);

    // Free allocated memory
    free(source_image);
    free(dest_image);
    free(lut);

    // Close files
    fclose(in);
    fclose(map);
    fclose(out);
}

void run_transfo_file(FILE *tf) {
    char source[FNMAX], curve[FNMAX], dest[FNMAX];
    int light;
#ifdef USE_CLOCK
    clock_t total = 0;
#else
    double total = 0;
#endif

    while (fscanf(tf, "%s %s %d %s", source, curve, &light, dest) == 4) {
        printf("%s %s %d %s\n", source, curve, light, dest);
#ifdef USE_CLOCK
        total += transform_image(source, curve, light, dest);
#else
        total += get_counter();
#endif
    }

#ifdef USE_CLOCK
    printf("TOTAL: %f clock cycles.\n", (double)total);
#else
    printf("TOTAL: %f clock cycles.\n", total);
#endif
}

int main(int ac, char *av[]) {
    FILE *tf;
    char *dname;

    if (ac != 2) {
        printf("Usage: %s transfofile\n", av[0]);
        exit(1);
    }

    tf = fopen(av[1], "r");
    if (tf == NULL) {
        perror(av[1]);
        exit(EXIT_FAILURE);
    }

    dname = dirname(av[1]);

    if (chdir(dname) == -1) {
        perror(dname);
        exit(EXIT_FAILURE);
    }

    run_transfo_file(tf);

    fclose(tf);

    return 0;
}


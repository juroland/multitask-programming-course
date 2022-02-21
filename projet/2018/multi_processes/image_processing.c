#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

struct image
{
    int height;
    int width;
    unsigned char **raster;
};

unsigned char **
raster_alloc(int height, int width)
{
    unsigned char **raster = (unsigned char **)calloc(height, sizeof(unsigned char *));
    if (raster == NULL)
        return NULL;

    raster[0] = (unsigned char *)calloc(height * width, sizeof(unsigned char));
    if (raster[0] == NULL)
        return NULL;

    for (int i = 1; i < height; ++i)
        raster[i] = raster[0] + i * width;

    return raster;
}

void raster_free(unsigned char **raster)
{
    if (raster == NULL)
        return;
    free(raster[0]);
    free(raster);
}

void image_free(struct image *self)
{
    raster_free(self->raster);
    free(self);
}

struct image *
image_alloc(int height, int width)
{
    struct image *self = (struct image *)calloc(1, sizeof(struct image));
    if (self == NULL)
        return NULL;

    self->height = height;
    self->width = width;

    self->raster = raster_alloc(height, width);

    if (self->raster == NULL)
        image_free(self);

    return self;
}

struct image *
pgm_parse(FILE *fp)
{
    /* Check the file format */
    {
        int magic_number = 0;
        int match = fscanf(fp, "P%d\n", &magic_number);
        if (match != 1)
        {
            fprintf(stderr, "Parsing failed : magic number expected\n");
            return NULL;
        }
        if (magic_number != 5)
        {
            fprintf(stderr, "Parsing failed : magic number 5 expected, "
                            "got %d instead\n",
                    magic_number);
            return NULL;
        }
    }

    int width, height;
    {
        int match = fscanf(fp, "%d %d\n", &width, &height);
        if (match != 2)
        {
            fprintf(stderr, "Parsing failed : width and height expected\n");
            return NULL;
        }
    }

    /* Check maximum gray value - only the single byte version is implemented */
    {
        int maxval = 0;
        int match = fscanf(fp, "%d\n", &maxval);
        if (match != 1)
        {
            fprintf(stderr, "Parsing failed : maximum gray value expected\n");
            return NULL;
        }

        if (maxval > 255)
        {
            fprintf(stderr,
                    "Parsing failed : maximum gray value < 255 is expected, "
                    "got %d instead\n",
                    maxval);
            return NULL;
        }
    }

    struct image *img = image_alloc(height, width);
    if (img == NULL)
    {
        fprintf(stderr, "Unable to allocate image struct\n");
        return NULL;
    }

    fread(img->raster[0], sizeof(unsigned char), height * width, fp);

    return img;
}

void pgm_write_header(struct image *img, FILE *fp)
{
    fprintf(fp, "P5\n");
    fprintf(fp, "%d %d\n", img->width, img->height);
    fprintf(fp, "255\n");
}

void pgm_write_raster(struct image *img, FILE *fp)
{
    fwrite(img->raster[0], sizeof(unsigned char), img->height * img->width, fp);
}

#define MAX(A, B) ((A) > (B) ? (A) : (B))
#define MIN(A, B) ((A) < (B) ? (A) : (B))

int sum_over_kernel(struct image *img, int row, int col, double kernel[3][3])
{
    double sum = 0;
    for (int k = 0; k < 3; ++k)
    {
        int i = row + k - 1;
        if (i < 0 || i >= img->height)
            continue;
        for (int l = 0; l < 3; ++l)
        {
            int j = col + l - 1;
            if (j < 0 || j >= img->width)
                continue;
            sum += kernel[k][l] * img->raster[i][j];
        }
    }
    return sum;
}

typedef double kernel_t[3][3];

void convolve(struct image *img, kernel_t *kernels[2], struct image *out)
{
    for (int row = 0; row < img->height; ++row)
    {
        for (int col = 0; col < img->width; ++col)
        {
            double val = sum_over_kernel(img, row, col, *kernels[0]);
            if (kernels[1] != NULL)
            {
                double valy = sum_over_kernel(img, row, col, *kernels[1]);
                val = sqrt(val * val + valy * valy);
            }

            val = MIN(val, 255);
            val = MAX(val, 0);
            out->raster[row][col] = (unsigned char)val;
        }
    }
}

int main(int argc, char *argv[])
{

    if (argc != 3)
    {
        fprintf(stderr, "Usage: %s filename1 filename2\n", argv[0]);
        return EXIT_FAILURE;
    }

    double edge_detect[3][3] = {{0, 1, 0},
                                {1, -4, 1},
                                {0, 1, 0}};

    double edge_detect2[3][3] = {{-1, -1, -1},
                                 {-1, 8, -1},
                                 {-1, -1, -1}};

    double edge_detect_x[3][3] = {{-1, 0, 1},
                                  {-2, 0, 2},
                                  {-1, 0, 1}};

    double edge_detect_y[3][3] = {{1, 2, 1},
                                  {0, 0, 0},
                                  {-1, -2, -1}};

    double sharpen[3][3] = {{0, -1, 0},
                            {-1, 5, -1},
                            {0, -1, 0}};

    double box_blur[3][3] = {{1. / 9, 1. / 9, 1. / 9},
                             {1. / 9, 1. / 9, 1. / 9},
                             {1. / 9, 1. / 9, 1. / 9}};

    double gaussian_blur[3][3] = {{1. / 16, 2. / 16, 1. / 16},
                                  {2. / 16, 4. / 16, 2. / 16},
                                  {1. / 16, 2. / 16, 1. / 16}};

    double identity[3][3] = {{0, 0, 0},
                             {0, 1, 0},
                             {0, 0, 0}};

    // kernel_t *kernels[] = {&identity, NULL};
    // kernel_t *kernels[] = {&box_blur, NULL};
    kernel_t *kernels[] = {&edge_detect2, NULL};
    // kernel_t *kernels[] = {&sharpen, NULL};
    // kernel_t *kernels[] = {&edge_detect_x, &edge_detect_y};

    FILE *fp = fopen(argv[1], "r");
    if (fp == NULL)
    {
        fprintf(stderr, "Unable to open input file : %s\n", argv[1]);
        return EXIT_FAILURE;
    }

    struct image *img = pgm_parse(fp);
    fclose(fp);

    struct image *out = image_alloc(img->height, img->width);
    convolve(img, kernels, out);

    fp = fopen(argv[2], "w");
    if (fp == NULL)
    {
        fprintf(stderr, "Unable to open output file : %s\n", argv[2]);
        return EXIT_FAILURE;
    }

    pgm_write_header(out, fp);
    pgm_write_raster(out, fp);
    fclose(fp);

    image_free(img);
    image_free(out);

    return 0;
}

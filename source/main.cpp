#include <test.h>

#include <stdio.h>

using namespace ImgLib;

#define W 5
#define H 5

void testConvolution()
{
    float matrix[] = {
        0.01, 0.11, 0.01,
        0.11, 0.72, 0.11,
        0.01, 0.11, 0.01
    };

    float R[W][H] = {
        {   0,   0,   0,   0,   0 },
        {  10,  20,  30,  40,  50 },
        { 100, 101, 102, 103, 104 },
        {  12,  55,  33, 122, 144 },
        { 104, 107, 109, 203, 204 }
    };

    float G[W][H] = {
        {   0,  10,  20,  30,  40 },
        {   1,  11,  22,  33,  44 },
        {   2, 101, 102, 103, 104 },
        {   3, 255,  23, 222, 244 },
        {   4,   7, 109, 203,   4 }
    };

    float B[W][H] = {
        {   0,   5,  15,  25,  55 },
        {   3,   0,  33,  22, 222 },
        { 111, 122,   0, 133, 144 },
        {   2,  22, 222,   0, 111 },
        { 104,  12,  13, 203,   0 }
    };

    Image test;
    test.create(W, H, FLOAT, RGB);
    for (size_t x = 0; x < W; ++x)
    for (size_t y = 0; y < H; ++y) {
        FloatRgb value = { .r = R[x][y], .g = G[x][y], .b = B[x][y] };
        test.setPixel(x, y, value);
    }

    test.convolve_3x3(matrix);

    for (size_t x = 0; x < W; ++x)
    for (size_t y = 0; y < H; ++y) {
        FloatRgb value;
        test.getPixel(x, y, &value);
        printf("%9.6f %9.6f %9.6f\n", value.r, value.g, value.b);
    }
}

void testScaling()
{
    float image[5][10] = {
        { 0.0, 0.1, 0.2, 0.3, 0.4, 0.5, 0.6, 0.7, 0.8, 0.9},
        { 0.1, 0.2, 0.3, 0.4, 0.5, 0.6, 0.7, 0.8, 0.9, 1.0},
        { 0.2, 0.3, 0.4, 0.5, 0.6, 0.7, 0.8, 0.9, 1.0, 0.9},
        { 0.3, 0.4, 0.5, 0.6, 0.7, 0.8, 0.9, 1.0, 0.9, 0.8},
        { 0.4, 0.5, 0.6, 0.7, 0.8, 0.9, 1.0, 0.9, 0.8, 0.7}
    };

    Image test;
    test.create(5, 10, FLOAT, GRAY);
    for (size_t x = 0; x <  5; ++x)
    for (size_t y = 0; y < 10; ++y) {
        test.setPixel(x, y, image[x][y]);
    }

    test.scale(0.5);

    printf("W = %lu\n", test.getWidth());
    printf("H = %lu\n", test.getHeight());
    for (size_t x = 0; x < test.getWidth(); ++x) {
        for (size_t y = 0; y < test.getHeight(); ++y) {
            FloatGrayscale value;
            test.getPixel(x, y, &value);
            printf("%9.6f", value);
        }
        printf("\n");
    }
}

int main()
{
    testConvolution();
    printf("\n");
    testScaling();
    return 0;
}

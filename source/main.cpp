#include <test.h>

using namespace ImgLib;

#define W 5
#define H 5

int main()
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
    for (int x=0; x<W; ++x)
    for (int y=0; y<H; ++y) {
        FloatRgb value = { .r = R[x][y], .g = G[x][y], .b = B[x][y] };
        test.setPixel(x, y, value);
    }

    test.convolve_3x3(matrix);

    for (int x=0; x<W; ++x)
    for (int y=0; y<H; ++y) {
        FloatRgb value;
        test.getPixel(x, y, &value);
        printf("%9.6f %9.6f %9.6f\n", value.r, value.g, value.b);
    }

    return 0;
}

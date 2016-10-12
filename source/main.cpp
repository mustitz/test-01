#include <test.h>

using namespace ImgLib;

int main()
{
    float matrix[] = {
        0.01, 0.11, 0.01,
        0.11, 0.72, 0.11,
        0.01, 0.11, 0.01
    };

    Image test;
    test.Create(10, 10, FLOAT, RGB);
    test.Convolve_3x3(matrix);
    return 0;
}

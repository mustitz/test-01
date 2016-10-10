#ifndef MU__TEST_LIB__HEADER__
#define MU__TEST_LIB__HEADER__

#include <stdlib.h>

#include <stdexcept>
#include <string>
#include <vector>

namespace ImgLib {

    struct FloatRgb
    {
        float r;
        float g;
        float b;
    };

    struct IntRgb
    {
        int r;
        int g;
        int b;
    };

    typedef float FloatGrayscale;
    typedef int IntGrayscale;

    class Exception: public std::runtime_error 
    {
        public:
            explicit Exception(const std::string & what):
                std::runtime_error(what)
            {}
    };

    /* My style to make Image class very easy to use. Client should know nothing */
    /* about internals and template magic inside                                 */

    class Image
    {
        public:
            enum PixelFormat { FLOAT, INT };
            enum PixelType { RGB, GRAY };

            Image();
            ~Image();

            void Create(size_t width, size_t height, PixelFormat pixelFormat, PixelType pixelType);

            size_t getWidth() const { return width; }
            size_t getHeight() const { return height; }

            void cloneTo(Image * image) const;
            
            void getPixel(size_t x, size_t y, FloatRgb * rgb) const;            
            void getPixel(size_t x, size_t y, IntRgb * rgb) const;            
            void getPixel(size_t x, size_t y, FloatGrayscale * value) const;            
            void getPixel(size_t x, size_t y, IntGrayscale * value) const;            

            void setPixel(size_t x, size_t y, const FloatRgb * rgb);            
            void setPixel(size_t x, size_t y, const IntRgb * rgb);            
            void setPixel(size_t x, size_t y, const FloatGrayscale * value);            
            void setPixel(size_t x, size_t y, const IntGrayscale * value);            

            void Convolve_3x3(const float * data);
            void Scale(float factor);

        private:
            size_t width;
            size_t height;

            PixelFormat pixelFormat;
            PixelType pixelType;

            void * storage;
    };

}

#endif

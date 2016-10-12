#ifndef MU__TEST_LIB__HEADER__
#define MU__TEST_LIB__HEADER__

#include <stdlib.h>

#include <vector>

namespace ImgLib {

    /* Here is list of the possible pixel representation structures.
     * According to conditions only four possible pixel representation 
     * are supported. But we know that also CMYK, HSV and many other  
     * representation are possible. So we plan our architeckture to 
     * support as much color model as possible
     */

    /* But I know only two possible ranges: integers from 0 to 255 
     * (or 360) and float from 0.0 to 1.0. Anyway I guess that we
     * have no combinatorical explosion to enumerate all possible
     * combinations of struct types and color models.
     */

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




    class BasePixelBuf;

    /* My style to make Image class very easy to use. Client should know nothing
     * about internals and template magic inside. For user most simple way to set 
     * pixel format in class arguments and just to call functions without any
     * magic.
     */

    class Image
    {
        public:
            enum PixelFormat { FLOAT, INT };
            enum PixelType { RGB, GRAY };

            Image();  // Create an empty image
            ~Image(); 

            // Allocate data and assign empty values to all pixels */
            void Create(size_t width, size_t height, PixelFormat pixelFormat, PixelType pixelType);


            /* Here is getters block */

            size_t getWidth() const { return width; }
            size_t getHeight() const { return height; }

            PixelFormat getPixelFormat() const { return pixelFormat; }
            PixelType getPixelType() const { return pixelType; }

            void * getStorage() const { return storage; }
            
            void cloneTo(Image * image) const;

            void getPixel(size_t x, size_t y, FloatRgb * rgb) const;            
            void getPixel(size_t x, size_t y, IntRgb * rgb) const;            
            void getPixel(size_t x, size_t y, FloatGrayscale * value) const;            
            void getPixel(size_t x, size_t y, IntGrayscale * value) const;            

            void setPixel(size_t x, size_t y, const FloatRgb * rgb);            
            void setPixel(size_t x, size_t y, const IntRgb * rgb);            
            void setPixel(size_t x, size_t y, const FloatGrayscale * value);            
            void setPixel(size_t x, size_t y, const IntGrayscale * value);            

            /* To be simple just convolution 3x3 will be supported now. In future we
             * can easyly ann generic convolution */
            void Convolve_3x3(const float * data);

            /* Also method for scaling */
            void Scale(float factor);

        private:
            size_t width;
            size_t height;

            PixelFormat pixelFormat;
            PixelType pixelType;

            BasePixelBuf * storage;
    };

    /* So, our header is quite simple */
}

#endif

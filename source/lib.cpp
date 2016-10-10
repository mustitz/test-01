#include "test.h"

#define DO_ACTION(pixelFormat, pixelType, PixelClass, action, ...)        \
    {                                                                     \
        ActionWrapper<pixelFormat, pixelType, PixelClass> actionWrapper;  \
        actionWrapper.action(this, ## __VA_ARGS__);                       \
    }

#define TRY_ALL_ACTIONS(action, ...)   \
    DO_ACTION( FLOAT, RGB, FloatRgb, action, ## __VA_ARGS__); \
    DO_ACTION(   INT, RGB,   IntRgb, action, ## __VA_ARGS__);



namespace ImgLib {

    // It is difficult for me to say is it possible to represent 
    // such kind of universal pixel format. Because except RGB
    // it is possible to use other color formats like CMYK, HSV.
    // But we assume that the given format is exists.
     
    struct UniversalPixel
    {
        void load(const FloatRgb & pixel);
    };

    void pixelAdd(FloatRgb * a, const FloatRgb & b);
    void pixelScale(FloatRgb * p, float factor);

    void pixelAdd(IntRgb * a, const IntRgb & b);
    void pixelScale(IntRgb * p, float factor);


    struct ConvolutionElement
    {
        int deltaX;
        int deltaY;
        float factor;
    };

    template <class ImgData>
        void ConvolutionAlgorithm(ImgData * destination, ImgData * source, const std::vector<ConvolutionElement> & convolutionVector)
    {
        for (size_t x = 0; x < source->getWidth(); ++x)
        for (size_t y = 0; y < source->getHeight(); ++y) {

            auto cit = convolutionVector.cbegin();
            auto pixel = source->getPixel(x + cit->deltaX, y + cit->deltaY);
            pixelScale(&pixel, cit->factor);
            ++cit;

            for (; cit != convolutionVector.cend(); ++cit) {
                auto nextPixel = source->getPixel(x + cit->deltaX, y + cit->deltaY);
                pixelScale(&nextPixel, cit->factor);
                pixelAdd(&pixel, nextPixel);
            }
        }
    }

    template <class Pixel> 
        class PixelVector: private std::vector<Pixel>
    {
        private:
            typedef std::vector<Pixel> Base;

        public:
            Pixel getPixel(size_t x, size_t y);
            size_t getWidth() const;
            size_t getHeight() const;
    };
        


    template <Image::PixelFormat pixelFormat, Image::PixelType pixelType, class Pixel>
        class ActionWrapper
    {
        public:
            bool check(const Image * me) const
            {
                if (me->getPixelFormat() != pixelFormat) return false;
                if (me->getPixelType() != pixelType) return false;
                return true;
            }

            PixelVector<Pixel> * getStorage(const Image * me)
            {
                return static_cast<PixelVector<Pixel> *>(me->getStorage());
            }

            void convolve(Image * me, const std::vector<ConvolutionElement> & convolutionVector)
            {
                if (!check(me)) {
                    /* Unsuported, nothing to do */
                    return;
                }

                Image tmp;
                me->cloneTo(&tmp);

                auto source = getStorage(&tmp);
                auto destination = getStorage(me);
                ConvolutionAlgorithm(destination, source, convolutionVector);
            }
    };

    void Image::Convolve_3x3(const float * data)
    {
        /* Create convolution vector */

        std::vector<ConvolutionElement> convolutionVector;

        for (int deltaX = -1; deltaX <= 1; ++deltaX)
        for (int deltaY = -1; deltaY <= 1; ++deltaY) {
            ConvolutionElement element;
            element.deltaX = deltaX;
            element.deltaY = deltaY;
            element.factor = *data++;
            convolutionVector.push_back(element);
        }

        TRY_ALL_ACTIONS(convolve, convolutionVector);
    }
}

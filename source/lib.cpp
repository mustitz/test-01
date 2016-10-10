#include "test.h"

/* Out main goal to avoid copy+paste for algorhthm implementtion.
 * So any algorithm like convolution or scaling is a template function.
 * Template function is better due to optimization reason. Virtual 
 * functions may give us an overhead.
 */ 

/* So here is macros to try a pixel format call. If it is matched then
 * corresponding template function will be called. If not nothing will
 * be happen.
 */

#define TRY_ACTION(pixelFormat, pixelType, PixelClass, action, ...)        \
    {                                                                     \
        ActionWrapper<pixelFormat, pixelType, PixelClass> actionWrapper;  \
        actionWrapper.action(this, ## __VA_ARGS__);                       \
    }

/* And here we try to enumerate all possible pixel formats to select best one.
 * As I noted in the header I assume that it is not very huge amount of
 * possible pixel formats so it might be not very hard to enumerate them
 * one time here.
 */

#define TRY_ALL_ACTIONS(action, ...)   \
    TRY_ACTION( FLOAT, RGB, FloatRgb, action, ## __VA_ARGS__); \
    TRY_ACTION(   INT, RGB,   IntRgb, action, ## __VA_ARGS__);



namespace ImgLib {

    /* It is difficult for me to say is it possible to represent 
     * such kind of universal pixel format. Because except RGB
     * it is possible to use other color formats like CMYK, HSV.
     * But we assume that the given format is exists.
     * Class UniversalPixel would be used to converttion from
     * one format to another throught universal pixel format.
     */
     
    struct UniversalPixel
    {
        void load(const FloatRgb & pixel);
        void save(FloatRgb * pixel);
    };




    /* Each pixel representation is a structure. Here we define 
     * operations for each such representation. For convolution
     * only pixel addition and scaling is used. So here they are.
     */

    void pixelAdd(FloatRgb * a, const FloatRgb & b);
    void pixelScale(FloatRgb * p, float factor);

    void pixelAdd(IntRgb * a, const IntRgb & b);
    void pixelScale(IntRgb * p, float factor);



    /* May be it is not very good during convolution to run two inner
     * cycles in matrix. So I will replace 3x3 matrix with linear
     * vector with 9 elements
     */

    struct ConvolutionElement
    {
        int deltaX;
        int deltaY;
        float factor;
    };



    /* Here is template for convolution algorithm which may be run for
     * any possible pixel storage
     */
    
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



    /* Pixel vector is used to save data for specific pixel representation */

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
        


    /* This is action wrapper on template algorithms like convolution */

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



    /* Example call for convolution with 3x3 matrix */

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

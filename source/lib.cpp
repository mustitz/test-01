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

#define TRY_ACTION(pixelFormat, pixelType, PixelClass, action, ...)       \
    {                                                                     \
        ActionWrapper<pixelFormat, pixelType, PixelClass> actionWrapper;  \
        if (actionWrapper.check(this)) {                                  \
            actionWrapper.action(this, ## __VA_ARGS__);                   \
            break;                                                        \
        }                                                                 \
    }



/* And here we try to enumerate all possible pixel formats to select best one.
 * As I noted in the header I assume that it is not very huge amount of
 * possible pixel formats so it might be not very hard to enumerate them
 * one time here.
 */

#define TRY_ALL_ACTIONS(action, ...)   \
    do { \
        TRY_ACTION( FLOAT, RGB, FloatRgb, action, ## __VA_ARGS__); \
        TRY_ACTION(   INT, RGB,   IntRgb, action, ## __VA_ARGS__); \
    } while (0)



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



    /* Pixel vector is used to save data for specific pixel representation */

    template <class Pixel>
        class PixelBuf: public BasePixelBuf
    {
        public:
            template <PixelFormat, PixelType, class Pixel2>
                friend class ActionWrapper;

            virtual void create(size_t width, size_t height) override;

            Pixel getPixel(size_t x, size_t y)
            {
                return data[x + y * width];
            }

        private:
            std::vector<Pixel> data;

    };




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



    /* This is action wrapper on template algorithms like convolution */

    template <PixelFormat pixelFormat, PixelType pixelType, class Pixel>
        class ActionWrapper
    {
        public:
            bool check(const Image * me) const
            {
                if (me->getPixelFormat() != pixelFormat) return false;
                if (me->getPixelType() != pixelType) return false;
                return true;
            }

            PixelBuf<Pixel> * getStorage(const Image * me)
            {
                return static_cast<PixelBuf<Pixel> *>(me->buf);
            }

            void create(Image * me, size_t width, size_t height)
            {
                me->buf = new PixelBuf<Pixel>();
                me->buf->create(width, height);
            }

            void cloneTo(const Image * me, Image * copy)
            {
                auto source = getStorage(me);
                auto destination = getStorage(me);

                destination->data = source->data;
                destination->width = source->width;
                destination->height = source->height;

                copy->pixelFormat = me->pixelFormat;
                copy->pixelType = me->pixelType;
            }

            void convolve(Image * me, const std::vector<ConvolutionElement> & convolutionVector)
            {
                Image tmp;
                me->cloneTo(&tmp);

                auto source = getStorage(&tmp);
                auto destination = getStorage(me);
                ConvolutionAlgorithm(destination, source, convolutionVector);
            }
    };



    Image::Image():
        pixelFormat(EMPTY_PIXEL_FORMAT),
        pixelType(EMPTY_PIXEL_TYPE),
        buf(nullptr)
    {
    }

    Image::~Image()
    {
        Reset();
    }

    void Image::Create(size_t width, size_t height, PixelFormat pixelFormat, PixelType pixelType)
    {
        Reset();

        TRY_ALL_ACTIONS(create, width, height);

        if (buf != nullptr) {
            this->pixelFormat = pixelFormat;
            this->pixelType = pixelType;
        }
    }

    void Image::Reset()
    {
        if (buf != nullptr) {
            delete buf;
        }

        new (this) Image();
    }

    void Image::cloneTo(Image * image) const
    {
        image->Reset();

        TRY_ALL_ACTIONS(cloneTo, image);
    }



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

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
        TRY_ACTION( FLOAT,  RGB,       FloatRgb, action, ## __VA_ARGS__); \
        TRY_ACTION(   INT,  RGB,         IntRgb, action, ## __VA_ARGS__); \
        TRY_ACTION( FLOAT, GRAY, FloatGrayscale, action, ## __VA_ARGS__); \
        TRY_ACTION(   INT, GRAY,   IntGrayscale, action, ## __VA_ARGS__); \
    } while (0)



namespace ImgLib {

    /* Each pixel representation is a structure. Here we define
     * operations for each such representation. For convolution
     * only pixel addition and scaling is used. So here they are.
     */

    static inline void pixelSetDefault(FloatRgb * a)
    {
        a->r = a->g = a->b = 0.0;
    }

    static inline void pixelAdd(FloatRgb * a, const FloatRgb & b)
    {
        a->r += b.r;
        a->g += b.g;
        a->b += b.b;
    }

    static inline void pixelScale(FloatRgb * p, float factor)
    {
        p->r *= factor;
        p->g *= factor;
        p->b *= factor;
    }


    static inline void pixelSetDefault(IntRgb * a)
    {
        a->r = a->g = a->b = 0;
    }

    static inline void pixelAdd(IntRgb * a, const IntRgb & b)
    {
        a->r += b.r;
        a->g += b.g;
        a->b += b.b;
    }

    static inline void pixelScale(IntRgb * p, float factor)
    {
        p->r *= factor;
        p->g *= factor;
        p->b *= factor;
    }


    static inline void pixelSetDefault(FloatGrayscale * a)
    {
        *a = 0.0;
    }

    static inline void pixelAdd(FloatGrayscale * a, const FloatGrayscale & b)
    {
        *a += b;
    }

    static inline void pixelScale(FloatGrayscale * p, float factor)
    {
        *p *= factor;
    }


    static inline void pixelSetDefault(IntGrayscale * a)
    {
        *a = 0;
    }

    static inline void pixelAdd(IntGrayscale * a, const IntGrayscale & b)
    {
        *a += b;
    }

    static inline void pixelScale(IntGrayscale * p, float factor)
    {
        *p *= factor;
    }



    /* Pixel vector is used to save data for specific pixel representation */

    template <PixelFormat pixelFormat, PixelType pixelType, class Pixel>
        class PixelBuf: public BasePixelBuf
    {
        public:
            template <PixelFormat, PixelType, class Pixel2>
                friend class ActionWrapper;

            virtual ~PixelBuf() override  = default;

            virtual void create(size_t width, size_t height) override
            {
                Pixel def;
                pixelSetDefault(&def);

                std::vector<Pixel> newData(width * height, def);
                data = std::move(newData);

                this->width = width;
                this->height = height;
            }

            Pixel getPixel(size_t x, size_t y) const
            {
                if (x >= width || y >= height) {
                    Pixel def;
                    pixelSetDefault(&def);
                    return def;
                }

                return data[x + y * width];
            }

            bool setPixel(size_t x, size_t y, const Pixel & pixel)
            {
                if (x > width || y > height) {
                    return false;
                }

                data[x + y * width] = pixel;
                return true;
            }

            static void safeSetPixel(const Image * image, BasePixelBuf * buf, size_t x, size_t y, const Pixel & pixel)
            {
                if (image->getPixelType() != pixelType) throw Exception("Arg type mismatch");
                if (image->getPixelFormat() != pixelFormat) throw Exception("Arg type mismatch");
                auto * me = static_cast<PixelBuf<pixelFormat, pixelType, Pixel> *> (buf);
                if (!me->setPixel(x, y, pixel)) {
                    throw Exception("Coordinates are out of bitmap");
                }
            }

            static void safeGetPixel(const Image * image, const BasePixelBuf * buf, size_t x, size_t y, Pixel * pixel)
            {
                if (image->getPixelType() != pixelType) throw Exception("Arg type mismatch");
                if (image->getPixelFormat() != pixelFormat) throw Exception("Arg type mismatch");
                auto * me = static_cast<const PixelBuf<pixelFormat, pixelType, Pixel> *> (buf);
                *pixel = me->getPixel(x, y);
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
        void ConvolutionAlgorithm(ImgData * destination, const ImgData * source, const std::vector<ConvolutionElement> & convolutionVector)
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

            destination->setPixel(x, y, pixel);
        }
    }



    /* Scaling.
     * I dit not dig very much into a lot of scaling technics... Here is simplest one:
     */

    template <class ImgData>
        void ScaleAlgorithm(ImgData * destination, const ImgData * source, double scaleFactor)
    {
        for (size_t y = 0; y < destination->getHeight(); ++y)
        for (size_t x = 0; x < destination->getWidth(); ++x) {

            size_t sourceX = x / scaleFactor;
            size_t sourceY = y / scaleFactor;

            auto pixel = source->getPixel(sourceX, sourceY);
            destination->setPixel(x, y, pixel);
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

            PixelBuf<pixelFormat, pixelType, Pixel> * getBuf(const Image * me)
            {
                return static_cast<PixelBuf<pixelFormat, pixelType, Pixel> *>(me->buf);
            }

            void create(Image * me, size_t width, size_t height)
            {
                me->buf = new PixelBuf<pixelFormat, pixelType, Pixel>();
                me->buf->create(width, height);
            }

            void cloneTo(const Image * me, Image * copy)
            {
                auto source = getBuf(me);
                auto destination = new PixelBuf<pixelFormat, pixelType, Pixel>();;

                destination->data = source->data;
                destination->width = source->width;
                destination->height = source->height;

                copy->pixelFormat = me->pixelFormat;
                copy->pixelType = me->pixelType;
                copy->buf = destination;
            }

            void convolve(Image * me, const std::vector<ConvolutionElement> & convolutionVector)
            {
                Image tmp;
                me->cloneTo(&tmp);

                auto source = getBuf(&tmp);
                auto destination = getBuf(me);
                ConvolutionAlgorithm(destination, source, convolutionVector);
            }

            void scale(Image * me, float factor)
            {
                size_t newWidth = factor * me->getWidth();
                size_t newHeight = factor * me->getHeight();

                Image tmp;
                me->cloneTo(&tmp);

                me->create(newWidth, newHeight, me->pixelFormat, me->pixelType);
                auto source = getBuf(&tmp);
                auto destination = getBuf(me);
                ScaleAlgorithm(destination, source, factor);
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
        if (buf != nullptr) {
            delete buf;
        }
    }

    void Image::create(size_t width, size_t height, PixelFormat pixelFormat, PixelType pixelType)
    {
        reset();

        this->pixelFormat = pixelFormat;
        this->pixelType = pixelType;

        TRY_ALL_ACTIONS(create, width, height);

        if (buf == nullptr) {
            throw Exception("Image::create(...) failed.");
        }
    }

    void Image::reset()
    {
        if (buf != nullptr) {
            delete buf;
        }

        new (this) Image();
    }

    void Image::cloneTo(Image * image) const
    {
        image->reset();

        TRY_ALL_ACTIONS(cloneTo, image);
    }



    void Image::setPixel(size_t x, size_t y, const FloatRgb & pixel)
    {
        PixelBuf<FLOAT, RGB, FloatRgb>::safeSetPixel(this, buf, x, y, pixel);
    }

    void Image::getPixel(size_t x, size_t y, FloatRgb * pixel) const
    {
        PixelBuf<FLOAT, RGB, FloatRgb>::safeGetPixel(this, buf, x, y, pixel);
    }

    void Image::setPixel(size_t x, size_t y, const IntRgb & pixel)
    {
        PixelBuf<INT, RGB, IntRgb>::safeSetPixel(this, buf, x, y, pixel);
    }

    void Image::getPixel(size_t x, size_t y, IntRgb * pixel) const
    {
        PixelBuf<INT, RGB, IntRgb>::safeGetPixel(this, buf, x, y, pixel);
    }

    void Image::setPixel(size_t x, size_t y, const FloatGrayscale & pixel)
    {
        PixelBuf<FLOAT, GRAY, FloatGrayscale>::safeSetPixel(this, buf, x, y, pixel);
    }

    void Image::getPixel(size_t x, size_t y, FloatGrayscale * pixel) const
    {
        PixelBuf<FLOAT, GRAY, FloatGrayscale>::safeGetPixel(this, buf, x, y, pixel);
    }

    void Image::setPixel(size_t x, size_t y, const IntGrayscale & pixel)
    {
        PixelBuf<INT, GRAY, IntGrayscale>::safeSetPixel(this, buf, x, y, pixel);
    }

    void Image::getPixel(size_t x, size_t y, IntGrayscale * pixel) const
    {
        PixelBuf<INT, GRAY, IntGrayscale>::safeGetPixel(this, buf, x, y, pixel);
    }



    /* Example call for convolution with 3x3 matrix */

    void Image::convolve_3x3(const float * data)
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



    void Image::scale(float factor)
    {
        TRY_ALL_ACTIONS(scale, factor);
    }
}

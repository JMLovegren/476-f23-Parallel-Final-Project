#include <FreeImage.h>
#include <FreeImagePlus.h>
#include <vector>
#include <algorithm>
#include <cassert>
#include <iostream>

using uchar = unsigned char;


void
filterOil(fipImage& image, const int height, const int width, const int bytesPerPixel, const int radius);

void
prepOil(fipImage& image, const int height, const int width, const int bytesPerPixel, const int radius);


int
main (int argc, char** argv)
{
    fipImage image(FIT_BITMAP);
    image.load("FlatIrons.png");

    int height = image.getHeight();
    int bytesPerPixel = image.getBitsPerPixel() / 8;
    int width = image.getScanWidth();
    FREE_IMAGE_TYPE type = image.getImageType();
    assert (type == FIT_BITMAP);

    std::cout << "Height: " << height << std::endl;
    std::cout << "Width:  " << width << std::endl;


    int radius = 5;
    filterOil(image, height, width, bytesPerPixel, radius );
}

void
filterOil(fipImage& image, const int height, const int width, const int bytesPerPixel, const int radius)
{
    std::cout << "Got to filterOil\n";
    prepOil(image, height, width, bytesPerPixel, radius);
}


void
prepOil(fipImage& image, const int height, const int width, const int bytesPerPixel, const int radius)
{
    fipImage imageCopy = (FIT_BITMAP);
    imageCopy.load("FlatIrons.png");
    const uchar RED = 2;
    const uchar GREEN = 1;
    const uchar BLUE = 0;
    for (int line = 0; line < height; ++line)
    {
        BYTE* byteArray = image.getScanLine(line);
        for (int col = 0; col < width; col += bytesPerPixel)
        {
            int calc = col / bytesPerPixel;
            int xMin = std::max((calc - radius), 0);
            int xMax = std::min((calc + radius), width - bytesPerPixel);
            int yMin = std::max((line - radius), 0);
            int yMax = std::min((line + radius), height - bytesPerPixel);
            uchar maxRed = 0;
            uchar maxGreen = 0;
            uchar maxBlue = 0;
            for (int checkRow = yMin; checkRow <= yMax; ++checkRow)
            {
                for (int checkCol = xMin; checkCol <= xMax; ++checkCol)
                {
                    BYTE* thisCheck = imageCopy.getScanLine(checkRow);
                    int checkPixel = checkCol * bytesPerPixel;
                    maxRed = std::max(maxRed, thisCheck[checkPixel + RED]);
                    maxGreen = std::max(maxGreen, thisCheck[checkPixel + GREEN]);
                    maxBlue = std::max(maxBlue, thisCheck[checkPixel + BLUE]);
                }
            }
            byteArray[col + RED] = maxRed;
            byteArray[col + GREEN] = maxGreen;
            byteArray[col + BLUE] = maxBlue;
        }
    }
    image.save("FlatIronsOil.png");
}
#include <FreeImage.h>
#include <FreeImagePlus.h>
#include <vector>
#include <algorithm>
#include <cassert>
#include <iostream>

using uchar = unsigned char;


void
prepOil(fipImage& image, const int height, const int width, const int bytesPerPixel, const int radius, const int intensity);


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

    int radius = 5;
    int intensity = 20;
    prepOil(image, height, width, bytesPerPixel, radius, intensity);
}


void
prepOil(fipImage& image, const int height, const int width, const int bytesPerPixel, const int radius, const int intensity)
{
    fipImage imageCopy = (FIT_BITMAP);
    imageCopy.load("FlatIrons.png");
    const uchar RED = 2;
    const uchar GREEN = 1;
    const uchar BLUE = 0;
    const int checkRange = (radius * 2) + 1;
    for (int line = 0; line < height; ++line)
    {
        BYTE* byteArray = image.getScanLine(line);
        for (int col = 0; col < width; col += bytesPerPixel)
        {
            int calc = col / bytesPerPixel;
            int xMin = std::max((calc - radius), 0);
            int xMax = std::min((calc + radius), width - bytesPerPixel * 2);
            int yMin = std::max((line - radius), 0);
            int yMax = std::min((line + radius), height - bytesPerPixel * 2);
            uchar maxRed = 0;
            uchar maxGreen = 0;
            uchar maxBlue = 0;
            std::vector<int> avgRed(intensity + 1);
            std::vector<int> avgGreen(intensity + 1);
            std::vector<int> avgBlue(intensity + 1);
            std::vector<int> intensityCount(intensity + 1);
            int xCount = 0;
            for (int count = 0; count < checkRange; ++count)
            {
                int y = line - radius + count;
                if (y >= yMin && y <= yMax)
                {
                    for (int x = 0; x <= xCount; ++x)
                    {
                        int curX = calc - xCount + x;
                        if (curX >= xMin && curX <= xMax)
                        {
                            BYTE* thisLine = imageCopy.getScanLine(y);
                            int xStart = curX * bytesPerPixel;
                            int red = thisLine[xStart + RED];
                            int green = thisLine[xStart + GREEN];
                            int blue = thisLine[xStart + BLUE];
                            
                            int curIntensity = (((red + green + blue) / 3) * intensity) / 255;

                            avgRed[curIntensity] += red;
                            avgGreen[curIntensity] += green;
                            avgBlue[curIntensity] += blue;
                            intensityCount[curIntensity]++;
                        }
                    }
                }
                if (count <= radius)
                {
                    xCount++;
                }
                else
                {
                    xCount--;
                }
            }
            int maxVal = intensityCount[0];
            int maxIndex = 0;
            for (int check = 1; check < intensity; ++check)
            {
                if (intensityCount[check] > maxVal)
                {
                    maxVal = intensityCount[check];
                    maxIndex = check;
                }
            }
            byteArray[col + RED] = avgRed[maxIndex] / maxVal;
            byteArray[col + GREEN] = avgGreen[maxIndex] / maxVal;
            byteArray[col + BLUE] = avgBlue[maxIndex] / maxVal;
        }
    }
    image.save("FlatIronsOil.png");
}
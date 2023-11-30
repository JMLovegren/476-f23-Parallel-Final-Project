#include <Magick++.h>
#include <iostream>
#include <string>
#include <variant>
#include <vector>
#include <algorithm>

using uchar = unsigned char;

void prepOil(Magick::Image& image, const int radius, const int intensity);

int
main(int argc, char** argv)
{
    Magick::InitializeMagick(*argv);

    Magick::Image image;
    image.read("FlatIrons.png");

    int radius = 3;
    int intensity = 30;

    prepOil(image, radius, intensity);

    image.write("FlatIronsOil.png");

    return 0;
}

void
prepOil(Magick::Image& image, const int radius, const int intensity)
{
    int height = image.rows();
    int width = image.columns();
    Magick::Image copyImage = image;
    for (int y = 0; y < height; ++y)
    {
        for (int x = 0; x < width; ++x)
        {
            std::vector<double> avgRed(intensity + 1);
            std::vector<double> avgGreen(intensity + 1);
            std::vector<double> avgBlue(intensity + 1);
            std::vector<int> intensityCount(intensity + 1);
            for (int subY = 0 - radius; subY <= radius; ++subY)
            {
                for (int subX = 0 - radius; subX <= radius; ++subX)
                {
                    if (x + subX >= 0 && x + subX < width && y + subY >= 0  && y + subY < height)
                    {
                        Magick::ColorRGB pix = copyImage.pixelColor((x + subX), (y + subY));
                        int curIntensity = (((pix.red() + pix.green() + pix.blue()) / 3) * intensity);

                        avgRed[curIntensity] += pix.red();
                        avgGreen[curIntensity] += pix.green();
                        avgBlue[curIntensity] += pix.blue();
                        intensityCount[curIntensity]++;
                    }
                }
            }
            double maxVal = intensityCount[0];
            int maxIndex = 0;
            for (int check = 1; check < intensity; ++check)
            {
                if (intensityCount[check] > maxVal)
                {
                    maxVal = intensityCount[check];
                    maxIndex = check;
                }
            }
            Magick::ColorRGB pixel = image.pixelColor(x, y);
            pixel.red(avgRed[maxIndex] / maxVal);
            pixel.green(avgGreen[maxIndex] / maxVal);
            pixel.blue(avgBlue[maxIndex] / maxVal);
            image.pixelColor(x, y, pixel);
        }
    }
}
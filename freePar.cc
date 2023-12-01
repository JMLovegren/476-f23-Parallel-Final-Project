/*
  Filename   : main.cc
  Author     : Timothy Moser, Jacob Lovegren
  Course     : CSCI 476
  Date       : 11/30/2023
  Assignment : Final Project - Oil Painting
  Description: Our own version of the oil painting algorithm
*/

#include <FreeImage.h>
#include <FreeImagePlus.h>
#include <vector>
#include <algorithm>
#include <cassert>
#include <iostream>

#include "BS_thread_pool.hpp"
#include "Timer.hpp"

using uchar = unsigned char;


void
applyOil(fipImage& image, const int height, const int width, const int bytesPerPixel, const int radius, const int intensity);

void
parOil(fipImage& image, const int height, const int width, const int bytesPerPixel, const int radius, const int intensity);

void
parOilProcess(fipImage& image, int first, int second, int height, int width, int bytesPerPixel, int radius, int intensity);

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

    int radius = 15;
    int intensity = 20;

    Timer serClock;
    serClock.start();
    applyOil(image, height, width, bytesPerPixel, radius, intensity);
    image.save("FlatIronsOil.png");
    serClock.stop();

    fipImage parImage(FIT_BITMAP);
    parImage.load("FlatIrons.png");

    Timer parClock;
    parClock.start();
    parOil(parImage, height, width, bytesPerPixel, radius, intensity);
    image.save("FlatIronsParOil.png");
    parClock.stop();

    std::cout << "Parallel Time:  " << std::format("{:.8f}", parClock.getElapsedMs()) << " ms\n";
    std::cout << "Serial Time  :  " << std::format("{:.8f}", serClock.getElapsedMs()) << " ms\n";
    std::cout << "Speed Up     :  " << serClock.getElapsedMs() / parClock.getElapsedMs() << std::endl;
}


void
applyOil(fipImage& image, const int height, const int width, const int bytesPerPixel, const int radius, const int intensity)
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
    
}


void
parOil(fipImage& image, const int height, const int width, const int bytesPerPixel, const int radius, const int intensity)
{
    uint concur = std::thread::hardware_concurrency();
    BS::thread_pool pool (concur);
    for (uint thread = 0; thread < concur; ++thread)
    {
        uint first = (thread * height) / concur;
        uint second = ((thread + 1) * height) / concur;

        pool.push_task (parOilProcess, std::ref(image), first, second, height, width, bytesPerPixel, radius, intensity);
    }
    pool.wait_for_tasks();
}

void
parOilProcess(fipImage& image, int first, int second, int height, int width, int bytesPerPixel, int radius, int intensity)
{
    fipImage imageCopy = (FIT_BITMAP);
    imageCopy.load("FlatIrons.png");
    const uchar RED = 2;
    const uchar GREEN = 1;
    const uchar BLUE = 0;
    const int checkRange = (radius * 2) + 1;
    for (int line = first; line < second; ++line)
    {
        BYTE* byteArray = image.getScanLine(line);
        for (int col = 0; col < width; col += bytesPerPixel)
        {
            int calc = col / bytesPerPixel;
            int xMin = std::max((calc - radius), 0);
            int xMax = std::min((calc + radius), width - bytesPerPixel * 2);
            int yMin = std::max((line - radius), 0);
            int yMax = std::min((line + radius), height - bytesPerPixel * 2);
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
}
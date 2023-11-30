/*
  Filename   : main.cc
  Author     : Jacob Lovegren, Timothy Moser
  Course     : CSCI 476
  Date       : 11/30/2023
  Assignment : Final Project - Deepfried Memes
  Description: Make deep fried memes out of gifs in
*/

#include <filesystem>
#include <iostream>
#include <string>
#include <format>
#include <fmt/ranges.h>
#include <omp.h>

#include <Magick++.h> 
#include "Timer.hpp"
#include "BS_thread_pool.hpp"

/******************************************************************************/

using namespace Magick; 

/******************************************************************************/

std::tuple<std::filesystem::path, int, int>
getInput ();

size_t
findNumFrames (std::string& infileString);

std::vector<Image> 
makeImageObjects (std::string& infileString, size_t numFrames);

void
serialProcess (std::vector<Image>& seriImages, int oilNum);

void
parallelSpatialProcess (std::vector<Image>& paraImages, int oilNum, 
                        int numThreads);

void
parallelProcess (std::vector<Image>& paraImages, int oilNum, int numThreads);

void
openMP (std::vector<Image>& ompImages, int oilNum);

void
convertToGIF(std::vector<Image>& images, const std::string& outputGifPath);

/******************************************************************************/


int main (int argc, char **argv) 
{ 
  auto [infile, numThreads, oilNum] = getInput ();
  std::string infileString = infile.string ();

  InitializeMagick(*argv);

  size_t numFrames = findNumFrames (infileString);

  std::vector<Image> seriImages = makeImageObjects (infileString, numFrames);
  std::vector<Image> paraSpacImages = seriImages;
  std::vector<Image> paraImages = seriImages;
  std::vector<Image> ompImages = seriImages;
  Timer timer;

  // Serial
  timer.start ();
  serialProcess (seriImages, oilNum);
  timer.stop ();
  double serProcTime = timer.getElapsedMs ();
  timer.start ();
  std::string outputGifPath = infile.stem().string() +  
                                std::to_string (oilNum) + "SeriMod.gif";
  convertToGIF(seriImages, outputGifPath);
  timer.stop();
  double serZipTime = timer.getElapsedMs ();

  // Parallel with locality
  timer.start ();
  parallelSpatialProcess (paraSpacImages, oilNum, numThreads);
  timer.stop ();
  double paraSpacProcTime = timer.getElapsedMs ();
  outputGifPath = infile.stem().string() + 
                    std::to_string (oilNum) + "ParaSpacMod.gif";
  convertToGIF (paraSpacImages, outputGifPath);

  // Parallel without locality
  timer.start ();
  parallelProcess (paraImages, oilNum, numThreads);
  timer.stop ();
  double paraProcTime = timer.getElapsedMs ();
  outputGifPath = infile.stem().string() + 
                    std::to_string (oilNum) + "ParaMod.gif";
  convertToGIF (paraImages, outputGifPath);

  // OpenMP
  timer.start();
  openMP(ompImages, oilNum);
  timer.stop();
  double ompProcTime = timer.getElapsedMs();
  outputGifPath = infile.stem().string() + 
                    std::to_string (oilNum) + "OmpMod.gif";
  convertToGIF(ompImages, outputGifPath);

  // output
  fmt::print ("Frames: {}\n\n", numFrames);

  fmt::print ("Zip Time:                   {:.2f}\n", serZipTime);

  fmt::print ("Serial time:                {:.2f} ms\n", serProcTime);
  fmt::print ("Parallel time w/ Locality:  {:.2f} ms\n", paraSpacProcTime);
  fmt::print ("Parallel time w/o Locality: {:.2f} ms\n", paraProcTime);
  fmt::print ("OpenMP time:                {:.2f} ms\n", ompProcTime);

  fmt::print ("Parallel Local Speedup:     {:.2f}\n", serProcTime / paraSpacProcTime);
  fmt::print ("Parallel no Local Speedup:  {:.2f}\n", serProcTime / paraProcTime);
  fmt::print ("Open MP Speedup:            {:.2f}\n", serProcTime / ompProcTime);

  return 0; 
} 

/******************************************************************************/

std::tuple<std::filesystem::path, int, int>
getInput ()
{
  std::cout << "Image ==> ";
  std::filesystem::path infile;
  std::cin >> infile;

  std::cout << "Threads ==> ";
  int numThreads;
  std::cin >> numThreads;

  std::cout << "Oil Effect Radius ==> ";
  int oilNum;
  std::cin >> oilNum;

  std::cout << "\n";
  return {infile, numThreads, oilNum};
}

/******************************************************************************/

size_t
findNumFrames (std::string& infileString)
{
  Image temp;

  std::string tempIn = infileString + "[-1]";

  temp.ping(tempIn);
  return temp.scene() + 1;
}

/******************************************************************************/

std::vector<Image> 
makeImageObjects (std::string& infileString, size_t numFrames)
{
  std::vector<Image> images;

  for (size_t i = 0; i < numFrames; ++i) 
  {
    images.emplace_back(infileString + "[ " + std::to_string(i) +  "]");
  }

  return images;
}

/******************************************************************************/

void
serialProcess (std::vector<Image>& seriImages, int oilNum)
{
  for (size_t i = 0; i < seriImages.size (); ++i)
  {
    seriImages[i].oilPaint ();
    seriImages[i].blur (5.0, 0.5);
    seriImages[i].modulate(100, 500, 100);
    seriImages[i].contrast (true);
    seriImages[i].contrast (true);
    seriImages[i].addNoise(Magick::UniformNoise);
    seriImages[i].swirl(60);
  }
}

/******************************************************************************/

void
parallelSpatialProcess (std::vector<Image>& paraImages, int oilNum, int numThreads)
{
  // Constructs a thread pool with only p threads.
  BS::thread_pool pool (numThreads);

  for (int threadCtr = 0; threadCtr < numThreads; ++threadCtr)
  {
  // partition which range this thread is responsible for of vector size
    auto lowRange = threadCtr * paraImages.size () / numThreads;
    auto highRange = (threadCtr + 1) * paraImages.size () / numThreads;
    pool.push_task ([=, &paraImages] 
      {
        for (auto i = lowRange; i < highRange; ++i)
        {
          paraImages[i].oilPaint ();
          paraImages[i].blur (5.0, 0.5);
          paraImages[i].modulate(100, 500, 100);
          paraImages[i].contrast (true);
          paraImages[i].contrast (true);
          paraImages[i].addNoise(Magick::UniformNoise);
          paraImages[i].swirl(60);
        }
      }
    );
  }
  pool.wait_for_tasks ();
}

/******************************************************************************/

void
parallelProcess (std::vector<Image>& paraImages, int oilNum, int numThreads)
{
  // Constructs a thread pool with only p threads.
  BS::thread_pool pool (numThreads);

  for (size_t i = 0; i < paraImages.size (); ++i)
  {
    pool.push_task ([=, &paraImages] 
      {

        paraImages[i].oilPaint ();
        paraImages[i].blur (5.0, 0.5);
        paraImages[i].modulate(100, 500, 100);
        paraImages[i].contrast (true);
        paraImages[i].contrast (true);
        paraImages[i].addNoise(Magick::UniformNoise);
        paraImages[i].swirl(60);

      }
    );
  }
  pool.wait_for_tasks ();
}

/******************************************************************************/

void
openMP (std::vector<Image>& ompImages, int oilNum)
{
#pragma omp parallel for
  for (size_t i = 0; i < ompImages.size (); ++i)
  {
    ompImages[i].oilPaint ();
    ompImages[i].blur (5.0, 0.5);
    ompImages[i].modulate(100, 500, 100);
    ompImages[i].contrast (true);
    ompImages[i].contrast (true);
    ompImages[i].addNoise(Magick::UniformNoise);
    ompImages[i].swirl(60);

  }
}

/******************************************************************************/

void convertToGIF(std::vector<Image> & images, const std::string& outputGifPath) 
{
  try 
  {
      InitializeMagick("");

      // Set Animation Settings
      for (auto& image : images) {
          // Set the delay between frames (in 1/100th of a second)
          image.animationDelay(10);  // Adjust as needed
      }
      // Write GIF
      writeImages(images.begin(), images.end(), outputGifPath);
  } 
  catch (Exception& e) 
  {
      std::cerr << "Caught exception: " << e.what() << std::endl;
  }
}

/******************************************************************************/

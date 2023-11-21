// #include <algorithm>
// #include <cassert>
// #include <condition_variable>
// #include <cstdlib>
#include <filesystem>
#include <iostream>
// #include <mutex>
// #include <queue>
#include <string>
// #include <thread>

#include <Magick++.h> 


using namespace Magick; 

/******************************************************************************/


std::filesystem::path 
getFileName ();


void convertToGIF(std::vector<Image>& images, const std::string& outputGifPath);

/******************************************************************************/



int main(int argc,char **argv) 
{ 
  std::filesystem::path infile = getFileName();
  std::string infileString = infile.string ();
  std::vector<Image> images;

  InitializeMagick(*argv);

  try
  { 
    Image temp;
    size_t total_frames = 0;

    std::string tempIn = infileString + "[-1]";

    temp.ping(tempIn);
    total_frames = temp.scene() + 1;

    for (size_t i = 0; i < total_frames; ++i) 
    {
      images.emplace_back(infileString + "[ " + std::to_string(i) +  "]");
    }

    for (size_t i = 0; i < total_frames; ++i)
    {
      images[i].oilPaint(5);
      images[i].flop();
    }
  } 
  catch( Exception &error_ ) 
  { 
    std::cout << "Caught exception: " << error_.what() << std::endl; 
    return 1; 
  } 

  std::string outputGifPath = infile.stem().string() + "Mod.gif";

  convertToGIF(images, outputGifPath);
  return 0; 
} 

/******************************************************************************/

std::filesystem::path 
getFileName ()
{
  std::filesystem::path infile;
  std::cout << "Image ==> ";
  std::cin >> infile;
  std::cout << "\n";
  return infile;
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
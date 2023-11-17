
// #include <algorithm>
// #include <cassert>
// #include <condition_variable>
// #include <cstdlib>
#include <filesystem>
#include <iostream>
// #include <mutex>
// #include <queue>
// #include <string>
// #include <thread>

#include <Magick++.h> 


using namespace std; 
using namespace Magick; 

/******************************************************************************/


//std::filesystem::path 
//getFileName ();

/******************************************************************************/



int main(int argc,char **argv) 
{ 
  InitializeMagick(*argv);

  // Construct the image object. Seperating image construction from the 
  // the read operation ensures that a failure to read the image file 
  // doesn't render the image object useless. 
  Image image;
  try { 
    // Read a file into image object 
    image.read("Animhorse.gif");

    // Crop the image to specified size (width, height, xOffset, yOffset)
    image.crop( Geometry(100,100, 100, 100) );

    // Write the image to a file 
    image.write("AnimhorseMod.gif"); 
  } 
  catch( Exception &error_ ) 
    { 
      cout << "Caught exception: " << error_.what() << endl; 
      return 1; 
    } 
  return 0; 
} 

/******************************************************************************/

/*
std::filesystem::path 
getFileName ()
{
  std::filesystem::path infile;
  std::cout << "Image ==> ";
  std::cin >> infile;
  std::cout << "\n";
  return infile;
}*/

// g++ -I/usr/include/ImageMagick-7 main.cc -Wcpp -DMAGICKCORE_HDRI_ENABLE=0 -DMAGICKCORE_QUANTUM_DEPTH=16

// use this
// g++ `Magick++-config --cxxflags --cppflags` -O2 -Wall -o main main.cc `Magick++-config --ldflags --libs`

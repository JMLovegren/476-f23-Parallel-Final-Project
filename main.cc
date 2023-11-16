
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
// #include <ImageMagick-7/Magick++.h>

/******************************************************************************/


//std::filesystem::path 
//getFileName ();

/******************************************************************************/

int main (int argc, char ** argv)
{
    InitializeMagick(*argv);

    //std::filesystem::path gifInfile = getFileName();


    // Construct the image object. Separating image construction from the 
    // the read operation ensures that a failure to read the image file 
    // doesn't render the image object useless. 
    Image image;
    try 
    { 
        // Read a file into image object 
        image.read("Animhorse.gif");

        // Crop the image to specified size (width, height, xOffset, yOffset)
        image.crop( Geometry(100,100, 100, 100) );

        // Write the image to a file 
        image.write( "x.gif" ); 
    } 
    catch( Exception &error_ ) 
    { 
        std::cout << "Caught exception: " << error_.what() << std::endl; 
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

// g++ -I/usr/include/ImageMagick-7 main.cc
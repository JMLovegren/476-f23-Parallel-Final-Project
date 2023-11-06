/*
 * Filename   : imageFilters.cc
 * Author     : Tim Moser
 * Course     : Parallel Programming
 * Assignment : Assignment 5 - Put that in your Pipe and Smoke It
 * Description: Takes an image, converts it to greyscale and
 *                  then performs a horizontal flip.
 *
 *              There is a define TIMED that will allow
 *                  timing and serial process.
 */
/************************************************************/
// System includes

#include <FreeImage.h>
#include <FreeImagePlus.h>
#include <algorithm>
#include <cassert>
#include <condition_variable>
#include <cstdlib>
#include <filesystem>
#include <iostream>
#include <mutex>
#include <queue>
#include <string>
#include <thread>

/************************************************************/
// Definition Macro

// #define TIMED

/************************************************************/
// Local includes

#ifdef TIMED
#include "Timer.hpp"
#endif

/************************************************************/
// Using declarations

using uint = unsigned int;
using fs = std::filesystem::path;

/************************************************************/
// Function prototypes/global vars/type definitions

template<typename T>
struct threadTools
{
private:
  std::queue<T> data;
  std::mutex m;
  std::condition_variable cv;

public:
  void
  enqueue (BYTE* row)
  {
    {
      std::lock_guard<std::mutex> lock (m);
      data.push (row);
    }
    cv.notify_one ();
  }
  T
  dequeue ()
  {
    std::unique_lock<std::mutex> lock (m);
    cv.wait (lock, [&] () { return !data.empty (); });
    T temp = data.front ();
    data.pop ();
    return temp;
  }
};

#ifdef TIMED
double
processSerial (fipImage& image, const uint height, const uint width,
               const uint bytesPerPixel, fs input);

void
serialFilter (fipImage& image, const uint height, const uint width,
              const uint bytesPerPixel, fs input);

double
processParallel (fipImage& image, const uint height, const uint width,
                 const uint bytesPerPixel, fs input);

double
processParTwo (fipImage& image, const uint height, const uint width,
               const uint bytesPerPixel, fs input);

void
filterGrey2 (fipImage& image, const uint height, const uint width,
             const uint bytesPerPixel, threadTools<BYTE*>& q);

void
filterHorizontal2 (fipImage& image, const uint height, const uint width,
                   const uint bytesPerPixel, threadTools<BYTE*>& q, fs input);
#endif

void
filterGreyscale (fipImage& image, const uint height, const uint width,
                 const uint bytesPerPixel, threadTools<BYTE*>& q);

void
filterHorizontalReflect (fipImage& image, const uint height, const uint width,
                         const uint bytesPerPixel, threadTools<BYTE*>& q,
                         fs input);

/************************************************************/
int
main (int argc, char* argv[])
{
  fs input;
  std::cout << "Image ==> ";
  std::cin >> input;
  std::cout << std::endl;

  fipImage image (FIT_BITMAP);
  image.load (input.c_str ());
  uint height = image.getHeight ();
  uint bytesPerPixel = image.getBitsPerPixel () / 8;
  uint width = image.getScanWidth ();
  FREE_IMAGE_TYPE type = image.getImageType ();
  assert (type == FIT_BITMAP);

#ifdef TIMED
  fipImage imageSer (FIT_BITMAP);
  imageSer.load (input.c_str ());
  fipImage imagePar (FIT_BITMAP);
  imagePar.load (input.c_str ());
  double serTime =
    processSerial (imageSer, height, width, bytesPerPixel, input);
  double parTime = processParallel (image, height, width, bytesPerPixel, input);
  double parTwo = processParTwo (imagePar, height, width, bytesPerPixel, input);

#else
  threadTools<BYTE*> q;
  {
    std::jthread t1 (
      [&image, height, width, bytesPerPixel, &q] ()
      { filterGreyscale (image, height, width, bytesPerPixel, q); });
    std::jthread t2 (
      [&image, height, width, bytesPerPixel, &q, input] () {
        filterHorizontalReflect (image, height, width, bytesPerPixel, q, input);
      });
  }
#endif

  std::string newName =
    input.filename ().stem ().string ().append ("Mod").append (
      input.extension ());
  std::cout << "Output:   " << newName << std::endl;

#ifdef TIMED
  std::cout << std::endl;
  std::cout << "Serial Time:      " << serTime << " ms" << std::endl;
  std::cout << "Parallel Time:    " << parTime << " ms" << std::endl;
  std::cout << "Par_Two Time:     " << parTwo << " ms" << std::endl;
  std::cout << "Par speed up:     " << serTime / parTime << std::endl;
  std::cout << "Par_Two speed up: " << serTime / parTwo << std::endl;
#endif
}

#ifdef TIMED
/************************************************************/
// Sets up clock and starts serial process.
double
processSerial (fipImage& image, const uint height, const uint width,
               const uint bytesPerPixel, fs input)
{
  Timer clock;
  clock.start ();
  serialFilter (image, height, width, bytesPerPixel, input);
  clock.stop ();
  return clock.getElapsedMs ();
}

/************************************************************/
// Runs the entire process in serial.
void
serialFilter (fipImage& image, const uint height, const uint width,
              const uint bytesPerPixel, fs input)
{
  using uchar = unsigned char;
  const uchar BLUE = 0;
  const uchar GREEN = 1;
  const uchar RED = 2;
  for (uint line = 0; line < height; ++line)
  {
    BYTE* byteArray = image.getScanLine (line);
    for (uint byte = 0; byte < width; byte += bytesPerPixel)
    {
      uchar grey = std::clamp ((0.2126 * byteArray[byte + RED] +
                                0.7152 * byteArray[byte + GREEN] +
                                0.0722 * byteArray[byte + BLUE]),
                               0.0, 255.0);
      byteArray[byte + RED] = grey;
      byteArray[byte + GREEN] = grey;
      byteArray[byte + BLUE] = grey;
    }
  }
  for (uint line = 0; line < height; ++line)
  {
    BYTE* byteArray = image.getScanLine (line);
    for (uint pixel = 0; pixel < (width / 2); pixel += bytesPerPixel)
    {
      std::swap (byteArray[pixel + RED],
                 byteArray[width - pixel - bytesPerPixel + RED]);
      std::swap (byteArray[pixel + GREEN],
                 byteArray[width - pixel - bytesPerPixel + GREEN]);
      std::swap (byteArray[pixel + BLUE],
                 byteArray[width - pixel - bytesPerPixel + BLUE]);
    }
  }
  std::string newName =
    input.filename ().stem ().string ().append ("Serial").append (
      input.extension ());
  image.save (newName.c_str ());
}

/************************************************************/
// Runs the jthread process and times.
double
processParallel (fipImage& image, const uint height, const uint width,
                 const uint bytesPerPixel, fs input)
{
  Timer clock;
  clock.start ();
  threadTools<BYTE*> q;
  {
    std::jthread t1 (
      [&image, height, width, bytesPerPixel, &q] ()
      { filterGreyscale (image, height, width, bytesPerPixel, q); });
    std::jthread t2 (
      [&image, height, width, bytesPerPixel, &q, input] () {
        filterHorizontalReflect (image, height, width, bytesPerPixel, q, input);
      });
  }
  clock.stop ();
  return clock.getElapsedMs ();
}

/************************************************************/
// Starts up parallel and a timer.
double
processParTwo (fipImage& image, const uint height, const uint width,
               const uint bytesPerPixel, fs input)
{
  Timer clock;
  clock.start ();
  threadTools<BYTE*> q;
  {
    std::jthread t1 ([&image, height, width, bytesPerPixel, &q] ()
                    { filterGrey2 (image, height, width, bytesPerPixel, q); });
    std::jthread t2 (
      [&image, height, width, bytesPerPixel, &q, input] ()
      { filterHorizontal2 (image, height, width, bytesPerPixel, q, input); });
  }
  clock.stop ();
  return clock.getElapsedMs ();
}

/************************************************************/
// Same filter grey concept, but different math.
void
filterGrey2 (fipImage& image, const uint height, const uint width,
             const uint bytesPerPixel, threadTools<BYTE*>& q)
{
  using uchar = unsigned char;
  // const uchar BLUE = 0;
  const uchar GREEN = 1;
  const uchar RED = 2;
  for (uint line = 0; line < height; ++line)
  {
    BYTE* byteArray = image.getScanLine (line);
    for (uint byte = 0; byte < width; byte += bytesPerPixel)
    {
      uchar grey =
        (byteArray[byte + RED] + byteArray[byte + GREEN] + byteArray[byte]) / 3;
      byteArray[byte + RED] = grey;
      byteArray[byte + GREEN] = grey;
      byteArray[byte] = grey; // BLUE
    }
    q.enqueue (byteArray);
  }
}

/************************************************************/
// Same with different math.
void
filterHorizontal2 (fipImage& image, const uint height, const uint width,
                   const uint bytesPerPixel, threadTools<BYTE*>& q, fs input)
{
  using uchar = unsigned char;
  // const uchar BLUE = 0;
  const uchar GREEN = 1;
  const uchar RED = 2;
  uint part = width - bytesPerPixel;
  uint half = width / 2;
  for (uint line = 0; line < height; ++line)
  {
    BYTE* byteArray = q.dequeue ();
    for (uint pixel = 0; pixel < half; pixel += bytesPerPixel)
    {
      std::swap (byteArray[pixel + RED], byteArray[part - pixel + RED]);
      std::swap (byteArray[pixel + GREEN], byteArray[part - pixel + GREEN]);
      std::swap (byteArray[pixel], byteArray[part - pixel]); //BLUE
    }
  }
  std::string newName =
    input.filename ().stem ().string ().append ("Par2").append (
      input.extension ());
  image.save (newName.c_str ());
}
#endif

/************************************************************/
// Converts an image line to greyscale, line go into queue.
void
filterGreyscale (fipImage& image, const uint height, const uint width,
                 const uint bytesPerPixel, threadTools<BYTE*>& q)
{
  using uchar = unsigned char;
  const uchar BLUE = 0;
  const uchar GREEN = 1;
  const uchar RED = 2;
  for (uint line = 0; line < height; ++line)
  {
    BYTE* byteArray = image.getScanLine (line);
    for (uint byte = 0; byte < width; byte += bytesPerPixel)
    {
      uchar grey = std::clamp (0.2126 * byteArray[byte + RED] +
                                 0.7152 * byteArray[byte + GREEN] +
                                 0.0722 * byteArray[byte + BLUE],
                               0.0, 255.0);
      byteArray[byte + RED] = grey;
      byteArray[byte + GREEN] = grey;
      byteArray[byte + BLUE] = grey;
    }
    q.enqueue (byteArray);
  }
}

/************************************************************/
// Horizontal flip on image. Save.
void
filterHorizontalReflect (fipImage& image, const uint height, const uint width,
                         const uint bytesPerPixel, threadTools<BYTE*>& q,
                         fs input)
{
  using uchar = unsigned char;
  const uchar BLUE = 0;
  const uchar GREEN = 1;
  const uchar RED = 2;
  for (uint line = 0; line < height; ++line)
  {
    BYTE* byteArray = q.dequeue ();
    for (uint pixel = 0; pixel < (width / 2); pixel += bytesPerPixel)
    {
      std::swap (byteArray[pixel + RED],
                 byteArray[width - pixel - bytesPerPixel + RED]);
      std::swap (byteArray[pixel + GREEN],
                 byteArray[width - pixel - bytesPerPixel + GREEN]);
      std::swap (byteArray[pixel + BLUE],
                 byteArray[width - pixel - bytesPerPixel + BLUE]);
    }
  }
  std::string newName =
    input.filename ().stem ().string ().append ("Mod").append (
      input.extension ());
  image.save (newName.c_str ());
}

/************************************************************/
// RESULTS                                                  //
/************************************************************/
/* Should have at least a minor speed up compared to serial
 * version, but currently runs at about the same. Might be
 * sped up by pulling multiple lines and storing into a local
 * queue each time instead of one per access. This would reduce
 * Mutex/lock switches.
 *
 * Attempted a second parallel version with different operations
 * to see if that would change the performance. Via the VM there
 * is potentially a small improvement, but that could be noise.
 */
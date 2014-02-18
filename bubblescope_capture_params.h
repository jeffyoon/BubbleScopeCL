#ifndef BUBBLESCOPECAPPARMAS_H
#define BUBBLESCOPECAPPARMAS_H

#include <stdio.h>
#include <string>

enum BubbleScopeCaptureMode
{
  MODE_STILLS         = 0,
  MODE_VIDEO          = 1,
  MODE_MJPG           = 2,
  MODE_SHOW_ORIGINAL,
  MODE_SHOW_UNWRAP,
};

/*
 * Stores user options defining capture properties.
 */
struct BubbleScopeParameters
{
  int captureDevice;
  int originalWidth;
  int originalHeight;
  int unwrapWidth;
  float radiusMin;
  float radiusMax;
  float uCentre;
  float vCentre;
  float offsetAngle;
  int mode[5];
  std::string outputFilename[3];
  float fps;
  int showCaptureProps;
};

void setupDefaultParameters(BubbleScopeParameters *);
void printParameters(BubbleScopeParameters *);

#endif

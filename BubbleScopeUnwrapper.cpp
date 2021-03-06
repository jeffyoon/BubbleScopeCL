#include "BubbleScopeUnwrapper.h"

BubbleScopeUnwrapper::BubbleScopeUnwrapper() :
  m_transformation(NULL),
  m_originalWidth(0), m_originalHeight(0),
  m_unwrapWidth(0),
  m_uCentre(0.0), m_vCentre(0.0),
  m_radiusMin(0.0), m_radiusMax(0.0),
  m_offsetAngle(0.0),
  m_unwrapHeight(0), m_outMatSize(0)
{
}

/**
 * \brief Destructor
 *
 * Cleans up memory allocated to transformation matrix
 */
BubbleScopeUnwrapper::~BubbleScopeUnwrapper()
{
  //Deallocate memory used for transformation array
  delete[] this->m_transformation;
}

/**
 * \brief Computes the pixel transformation array.
 *
 * All unwrap parameters must be set beforehand.
 * Any changes to unwrap parameters must be followed by a call to this function
 * before any further calls to unwrap()
 *
 * \return True if transformation is computed successfully
 */
bool BubbleScopeUnwrapper::generateTransformation()
{
  this->m_transformation = new unsigned long[this->m_outMatSize];

  float radius_delta = this->m_radiusMax - this->m_radiusMin;
  float aspect = (float) this->m_originalWidth / (float) this->m_originalHeight;

  unsigned int pixelSpan = this->m_originalWidth * this->m_radiusMax * 2;
  if(pixelSpan > this->m_originalHeight)
    return false;

  unsigned long index = 0;
  unsigned int i, j;
  for (i = this->m_unwrapHeight - 1; i > 0; i--)
  {
    float amplitutde = (radius_delta * (i / (float) this->m_unwrapHeight))
      + this->m_radiusMin;

    for (j = 0; j < this->m_unwrapWidth; ++j)
    {
      float longitudeAngle = (float) (D_PI * (j / (float) this->m_unwrapWidth))
        + this->m_offsetAngle;

      float sinLongAngle = sin(longitudeAngle);
      float cosLongAngle = cos(longitudeAngle);

      float u = aspect * sinLongAngle;
      float v = cosLongAngle;

      u *= amplitutde;
      v *= amplitutde;

      u += this->m_uCentre;
      v += (1.0f - this->m_vCentre);

      if(u > 1.0f)
        u = 1.0f;
      if(v > 1.0f)
        v = 1.0f;

      int xPixel = (int) ((1.0f - v) * this->m_originalWidth);
      int yPixel = (int) ((1.0f - u) * this->m_originalHeight);

      unsigned long oldPixelIndex =
        ((yPixel * this->m_originalWidth) + xPixel) * 3;

      this->m_transformation[index] = oldPixelIndex;
      this->m_transformation[index + 1] = oldPixelIndex + 1;
      this->m_transformation[index + 2] = oldPixelIndex + 2;

      index += 3;
    }
  }

  return true;
}

/**
 * \brief Creates a 360 degree unwrap using the pre-computed array.
 *
 * Must call generateTransformation() before calling this function.
 *
 * \param imageIn Pointer to image to be converted
 *
 * \param imageOut Double pointer to output image
 *
 * \return True if unwrapped successfully, false otherwise
 */
bool BubbleScopeUnwrapper::unwrap(cv::Mat *imageIn, cv::Mat **imageOut)
{
  if(!this->m_transformation)
    return false;

  *imageOut = new cv::Mat(this->m_unwrapHeight, this->m_unwrapWidth, CV_8UC3,
      cv::Scalar::all(0));
  unsigned char *unwrapPixels = (*imageOut)->data;
  unsigned char *originalPixels = imageIn->data;

  unsigned long i;
  for(i = 0; i < this->m_outMatSize; i++)
    unwrapPixels[i] = originalPixels[this->m_transformation[i]];

  return true;
}

/**
 * \brief Sets width of unwrapped image.
 *
 * Used to calculate height of unwrapped image and to allocate memory for
 * transformation array.
 *
 * \param width Desired width of unwrapped image
 *
 * \return True if setting was applied successfully, false otherwise
 */
bool BubbleScopeUnwrapper::unwrapWidth(int width)
{
  if(width <= 0)
    return false;

  this->m_unwrapWidth = width;
  this->m_unwrapHeight = (int) (this->m_unwrapWidth / D_PI);
  this->m_outMatSize = this->m_unwrapWidth * this->m_unwrapHeight * 3;

  return true;
}

/**
 * \brief Sets the width and height of the original captured image.
 *
 * Used to generate pixel transformation values.
 *
 * \param width Width of original image
 *
 * \param height Height of original image
 *
 * \return True if setting was applied successfully, false otherwise
 */
bool BubbleScopeUnwrapper::originalSize(int width, int height)
{
  if((width < 0) || (height < 0))
    return false;

  this->m_originalWidth = width;
  this->m_originalHeight = height;

  return true;
}

/**
 * \brief Sets the centre of the image relative to it's dimensions.
 *
 * \param u U centre
 *
 * \param v V centre
 *
 * \return True if setting was applied successfully, false otherwise
 */
bool BubbleScopeUnwrapper::originalCentre(float u, float v)
{
  if((u < 0.0f) || (u > 1.0f))
    return false;
  if((v < 0.0f) || (v > 1.0f))
    return false;

  this->m_uCentre = u;
  this->m_vCentre = v;

  return true;
}

/**
 * \brief Sets the upper and lower radii defining the section of the original image
 * to unwrap.
 *
 * \param min Minimum radius
 *
 * \param max Maximum radius
 *
 * \return True if setting was applied successfully, false otherwise
 */
bool BubbleScopeUnwrapper::imageRadius(float min, float max)
{
  if((min < 0.0f) || (min > 0.5f))
    return false;
  if((max < 0.0f) || (max > 0.5f))
    return false;
  if(min >= max)
    return false;

  this->m_radiusMin = min;
  this->m_radiusMax = max;

  return true;
}

/**
 * \brief Sets offset angle for unwrapped image.
 *
 * Equivalent of rotating BubbleScope on camera.
 *
 * \param angle Offset angle in degrees
 *
 * \return True if setting was applied successfully, false otherwise
 */
bool BubbleScopeUnwrapper::offsetAngle(float angle)
{
  if((angle < 0.0f) || (angle > 360.0f))
    return false;

  this->m_offsetAngle = angle * DEG_2_RAD;

  return true;
}

/**
 * \brief Gets the computed height of the unwrapped images.
 *
 * \return Calculated height of unwrapped image
 */
unsigned int BubbleScopeUnwrapper::getUnwrapHeight()
{
  return this->m_unwrapHeight;
}

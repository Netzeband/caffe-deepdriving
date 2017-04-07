/**
 * Image.cpp
 *
 *  Created on: Mar 26, 2017
 *      Author: Andre Netzeband
 */

#include "Image.hpp"

#include <glog/logging.h>

#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/highgui/highgui_c.h>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/imgproc/imgproc_c.h>

using namespace caffe;
using namespace std;

CImage::CImage():
    pCurrentWindowName(0),
    ImageHeight(0),
    ImageWidth(0),
    ImageChannels(0),
    pImage(0),
    pOriginalImage(0)
{

}

CImage::~CImage()
{
  destroyWindow();

  if (pOriginalImage)
  {
    cvReleaseImage(&pOriginalImage);
  }

  if (pImage)
  {
    cvReleaseImage(&pImage);
  }
}

void CImage::readFromDatum(caffe::Datum const &rData)
{
  // must always be 3 since the data input excepts 3 channels
  CHECK(rData.channels() == 3);

  int32_t Height   = rData.height();
  int32_t Width    = rData.width();
  int32_t Channels = rData.channels();

  setImage(Height, Width, Channels);

  if (pImage)
  {
    // must always be 3 since the data input excepts 3 channels
    CHECK(ImageChannels == 3);

    string const Bytes = rData.data();
    for (int h = 0; h < ImageHeight; ++h)
    {
      for (int w = 0; w < ImageWidth; ++w)
      {
        pImage->imageData[(h*ImageWidth+w)*3+0]=(uint8_t)Bytes[h*ImageWidth+w];
        pImage->imageData[(h*ImageWidth+w)*3+1]=(uint8_t)Bytes[ImageHeight*ImageWidth+h*ImageWidth+w];
        pImage->imageData[(h*ImageWidth+w)*3+2]=(uint8_t)Bytes[ImageHeight*ImageWidth*2+h*ImageWidth+w];
      }
    }
  }
}

void CImage::readFromMemory(uint8_t * pMemory, int SourceWidth, int SourceHeight)
{
  readFromMemory(pMemory, SourceWidth, SourceHeight, SourceWidth, SourceHeight);
}

void CImage::readFromMemory(uint8_t * pMemory, int SourceWidth, int SourceHeight, int TargetWidth, int TargetHeight)
{
  IplImage * pTempImage = 0;
  bool HasSourceAndTargetSameSize = (SourceWidth == TargetWidth && SourceHeight == TargetHeight);

  setImage(TargetHeight, TargetWidth, 3);

  if (HasSourceAndTargetSameSize)
  {
    pTempImage = pImage;
  }
  else
  {
    if (!pOriginalImage)
    {
      pOriginalImage = cvCreateImage(cvSize(SourceWidth, SourceHeight), IPL_DEPTH_8U, 3);
    }
    else if (SourceWidth != pOriginalImage->width || SourceHeight != pOriginalImage->height)
    {
      cvReleaseImage(&pOriginalImage);
      pOriginalImage = cvCreateImage(cvSize(SourceWidth, SourceHeight), IPL_DEPTH_8U, 3);
    }

    pTempImage = pOriginalImage;
  }

  if (pTempImage)
  {
    for (int h = 0; h < SourceHeight; ++h)
    {
      for (int w = 0; w < SourceWidth; ++w)
      {
        pTempImage->imageData[(h*SourceWidth+w)*3+2]=pMemory[((SourceHeight-h-1)*SourceWidth+w)*3+0];
        pTempImage->imageData[(h*SourceWidth+w)*3+1]=pMemory[((SourceHeight-h-1)*SourceWidth+w)*3+1];
        pTempImage->imageData[(h*SourceWidth+w)*3+0]=pMemory[((SourceHeight-h-1)*SourceWidth+w)*3+2];
      }
    }
  }

  if (!HasSourceAndTargetSameSize)
  {
    cvResize(pOriginalImage, pImage);
  }
}

void CImage::setNoVideo(int TargetWidth, int TargetHeight)
{
  setImage(TargetHeight, TargetWidth, 3);

  if (pImage)
  {
    for (int h = 0; h < TargetHeight; ++h)
    {
      for (int w = 0; w < TargetWidth; ++w)
      {
        uint8_t White = rand() & 0xFF;

        pImage->imageData[(h*TargetWidth+w)*3+2]=White;
        pImage->imageData[(h*TargetWidth+w)*3+1]=White;
        pImage->imageData[(h*TargetWidth+w)*3+0]=White;
      }
    }
  }

  CvFont Font;
  cvInitFont(&Font, CV_FONT_HERSHEY_TRIPLEX, 1, 1, 1, 2, 8);
  cvPutText(pImage, "- No Signal -", cvPoint(TargetWidth/2-125, TargetHeight/2+5), &Font, cvScalar(0, 0, 255));
}

void CImage::show(std::string &rWindowName)
{
  show(rWindowName.c_str());
}

void CImage::show(char const * const pName)
{
  setWindow(pName);

  if (pImage && pCurrentWindowName)
  {
    cvShowImage(pCurrentWindowName, pImage);
  }
}

void CImage::setWindow(char const * const pName)
{
  if (pName)
  {
    if (pCurrentWindowName != pName)
    {
      destroyWindow();

      pCurrentWindowName = pName;

      cvNamedWindow(pCurrentWindowName, 1);
    }
  }
}

void CImage::destroyWindow()
{
  if (pCurrentWindowName)
  {
    cvDestroyWindow(pCurrentWindowName);
    pCurrentWindowName = 0;
  }
}

void CImage::setImage(int32_t Height, int32_t Width, int32_t Channels)
{
  if (!pImage || Height != ImageHeight || Width != ImageWidth || Channels != ImageChannels)
  {
    destroyImage();

    ImageHeight   = Height;
    ImageWidth    = Width;

    CHECK(Channels == 3);
    ImageChannels = 3;
    pImage = cvCreateImage(cvSize(ImageWidth, ImageHeight), IPL_DEPTH_8U, ImageChannels);
  }
}

void CImage::destroyImage()
{
  if (pImage)
  {
    cvReleaseImage(&pImage);
  }
}

IplImage * CImage::getImage()
{
  return pImage;
}




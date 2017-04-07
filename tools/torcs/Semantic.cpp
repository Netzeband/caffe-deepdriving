/**
 * Semantic.cpp
 *
 *  Created on: Mar 25, 2017
 *      Author: Andre Netzeband
 *
 *  Attention: This is a reimplementation of the code the DeepDriving project.
 *  See http://deepdriving.cs.princeton.edu for more details.
 *  Thus much code comes from Chenyi Chen.
 *
 *  Take the original DeepDriving license into account!
 */

#include "Semantic.hpp"

#include <glog/logging.h>
#include <iostream>
#include <algorithm>

#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>

#include <boost/filesystem/operations.hpp>
#include <boost/filesystem/path.hpp>

#define SEMANTIC_WIDTH  320
#define SEMANTIC_WIDTH_BIG  700
#define SEMANTIC_HEIGHT 660
#define SEMANTIC_WINDOW_NAME "Semantic Visualization"
#define BACKGROUND_FILENAME "semantic_background.png"
#define BACKGROUND_BIG_FILENAME "semantic_background_big.png"
#define LANE1_FILENAME "semantic_1lane.png"
#define LANE2_FILENAME "semantic_2lane.png"
#define LANE3_FILENAME "semantic_3lane.png"
#define ERROR_MEASUREMENT_FILENAME "semantic_error_background.png"

using namespace boost::filesystem;

static IplImage * loadImage(path FilePath)
{
  path FullPath = current_path() / FilePath;

  IplImage * pImage = cvLoadImage(FullPath.c_str());
  CHECK(pImage) << "Cannot load image " << FullPath.c_str() << std::endl;
  return pImage;
}

CSemantic::CSemantic()
{
  pSemanticImage = cvCreateImage(cvSize(SEMANTIC_WIDTH, SEMANTIC_HEIGHT), IPL_DEPTH_8U, 3);
  cvNamedWindow(SEMANTIC_WINDOW_NAME, 1);

  pBackground = loadImage(BACKGROUND_FILENAME);
  pLane1      = loadImage(LANE1_FILENAME);
  pLane2      = loadImage(LANE2_FILENAME);
  pLane3      = loadImage(LANE3_FILENAME);
  pErrorMeasurementBackground = loadImage(ERROR_MEASUREMENT_FILENAME);

  MarkingHead = 0;

  pFrameImage = 0;
  pAdditionalData = 0;
  pErrorMeasurement = 0;

  cvInitFont(&Font, CV_FONT_HERSHEY_PLAIN, 1, 1.5, 1, 1, 8);
}

CSemantic::~CSemantic()
{
  cvDestroyWindow(SEMANTIC_WINDOW_NAME);
  cvReleaseImage(&pSemanticImage);

  cvReleaseImage(&pBackground);
  cvReleaseImage(&pLane1);
  cvReleaseImage(&pLane2);
  cvReleaseImage(&pLane3);
  cvReleaseImage(&pErrorMeasurementBackground);
}

void CSemantic::setFrameImage(CImage * pFrame)
{
  pFrameImage = pFrame;

  setupBackground();
}

void CSemantic::setAdditionalData(TorcsData_t * pData)
{
  pAdditionalData = pData;

  setupBackground();
}

void CSemantic::setErrorMeasurement(CErrorMeasurement * pErrorMeasurementObject)
{
  pErrorMeasurement = pErrorMeasurementObject;

  setupBackground();
}

void CSemantic::setupBackground()
{
  if (isBigWindow())
  {
    cvReleaseImage(&pSemanticImage);
    cvReleaseImage(&pBackground);
    pBackground = loadImage(BACKGROUND_BIG_FILENAME);
    pSemanticImage = cvCreateImage(cvSize(SEMANTIC_WIDTH_BIG, SEMANTIC_HEIGHT), IPL_DEPTH_8U, 3);
  }
  else
  {
    cvReleaseImage(&pSemanticImage);
    cvReleaseImage(&pBackground);
    pBackground = loadImage(BACKGROUND_FILENAME);
    pSemanticImage = cvCreateImage(cvSize(SEMANTIC_WIDTH, SEMANTIC_HEIGHT), IPL_DEPTH_8U, 3);
  }
}

bool CSemantic::isBigWindow() const
{
  return (pFrameImage != 0) || (pAdditionalData != 0) || (pErrorMeasurement != 0);
}

void CSemantic::show(Indicators const * pGroundTruth, Indicators const * pGuessed, bool WasUpdated, int Lanes)
{
  Lanes = guessLanes(pGroundTruth, pGuessed, Lanes);
  copyBackground();

  int const LanePosition = getLanePosition(pGroundTruth, pGuessed, SEMANTIC_WIDTH/2);
  addLanes(Lanes, LanePosition);
  addLaneMarkings(Lanes, LanePosition, WasUpdated);
  addHostCar(pGroundTruth, pGuessed, Lanes);
  addObstacles(pGroundTruth, pGuessed, LanePosition);
  drawFrame();
  drawAdditionalData();
  drawErrorMeasurement();
  cvShowImage(SEMANTIC_WINDOW_NAME, pSemanticImage);
}

int CSemantic::guessLanes(Indicators const * pGroundTruth, Indicators const * pGuessed, int Lanes)
{
  if (Lanes >= 1 && Lanes <= 3)
  {
    return Lanes;
  }
  else if (pGuessed)
  {
    return pGuessed->getNumberOfLanes();
  }
  else if (pGroundTruth)
  {
    return pGroundTruth->getNumberOfLanes();
  }

  return 0;
}

void CSemantic::addHostCar(Indicators const * pGroundTruth, Indicators const * pGuessed, int Lanes)
{
  if (pGroundTruth)
  {
    drawHostCar(pGroundTruth, SEMANTIC_WIDTH/2, true);
  }

  if (pGuessed)
  {
    drawHostCar(pGuessed, SEMANTIC_WIDTH/2, false);
  }
}

static void getCarBoxFromAngle( float * pP1x, float * pP1y,
                                float * pP2x, float * pP2y,
                                float * pP3x, float * pP3y,
                                float * pP4x, float * pP4y,
                                float Angle)
{
  Angle = -Angle;
  *pP1x = -14 * cos(Angle) + 28 * sin(Angle);
  *pP1y =  14 * sin(Angle) + 28 * cos(Angle);
  *pP2x =  14 * cos(Angle) + 28 * sin(Angle);
  *pP2y = -14 * sin(Angle) + 28 * cos(Angle);
  *pP3x =  14 * cos(Angle) - 28 * sin(Angle);
  *pP3y = -14 * sin(Angle) - 28 * cos(Angle);
  *pP4x = -14 * cos(Angle) - 28 * sin(Angle);
  *pP4y =  14 * sin(Angle) - 28 * cos(Angle);
}

void CSemantic::drawHostCar(Indicators const * pIndicator, float CarPosition, bool IsGroundTruth)
{
  float P1x, P1y, P2x, P2y, P3x, P3y, P4x, P4y;
  getCarBoxFromAngle(&P1x, &P1y, &P2x, &P2y, &P3x, &P3y, &P4x, &P4y, pIndicator->Angle);

  CvPoint Points[4];

  Points[0].x = P1x + CarPosition;
  Points[0].y = P1y + 600;
  Points[1].x = P2x + CarPosition;
  Points[1].y = P2y + 600;
  Points[2].x = P3x + CarPosition;
  Points[2].y = P3y + 600;
  Points[3].x = P4x + CarPosition;
  Points[3].y = P4y + 600;

  if (IsGroundTruth)
  {
    cvFillConvexPoly(pSemanticImage, Points, 4, cvScalar(0, 0, 255));
  }
  else
  {
    int NumberOfPoints = 4;
    CvPoint * pPoints = Points;
    cvPolyLine(pSemanticImage, &pPoints, &NumberOfPoints, 1, 1, cvScalar(0, 255, 0), 2, CV_AA);
  }
}

void CSemantic::addLanes(int Lanes, int LanePosition)
{
  IplImage * pLane = 0;

  switch(Lanes)
  {
    case 1:
      pLane = pLane1;
      break;

    case 2:
      pLane = pLane2;
      break;

    case 3:
      pLane = pLane3;
      break;

    default:
      break;
  }

  if (pLane && LanePosition > 0)
  {
    int LaneTopLeftCorner = LanePosition - pLane->width / 2;
    cv::Mat(pLane).copyTo(cv::Mat(pSemanticImage, cv::Rect(LaneTopLeftCorner, 0, pLane->width, pLane->height)));
  }
}

void CSemantic::copyBackground()
{
  CHECK(pBackground);
  CHECK(pSemanticImage);

  cvCopy(pBackground, pSemanticImage);

  return;
}

void CSemantic::drawFrame()
{
  if (pFrameImage)
  {
    IplImage * pFrame = pFrameImage->getImage();
    if (pFrame)
    {
      cv::Mat(pFrame).copyTo(cv::Mat(pSemanticImage, cv::Rect(380, 40, pFrame->width, pFrame->height)));
    }
  }
}

void CSemantic::drawAdditionalData()
{
  if (pAdditionalData)
  {
    static char TextBuffer[255];

    // print speed
    cvPutText(pSemanticImage, "Speed", cvPoint(265, 65), &Font, cvScalar(255,255,255));
    snprintf(TextBuffer, sizeof(TextBuffer), "%d km/h", int(pAdditionalData->Speed * 3.6));
    cvPutText(pSemanticImage, TextBuffer, cvPoint(265, 85), &Font, cvScalar(255,255,255));

    // print pause
    if (!pAdditionalData->IsNotPause)
    {
      cvPutText(pSemanticImage, "[Pause]", cvPoint(380, 275), &Font, cvScalar(0, 0, 255));
    }

    // print control state
    if (!pAdditionalData->IsControlling)
    {
      cvPutText(pSemanticImage, "[Disabled]", cvPoint(470, 275), &Font, cvScalar(0, 0, 255));
    }
    else
    {
      cvPutText(pSemanticImage, "[Enabled]", cvPoint(470, 275), &Font, cvScalar(255, 255, 255));
    }

    // print AI state
    if (!pAdditionalData->IsAIControlled)
    {
      cvPutText(pSemanticImage, "[no AI]", cvPoint(580, 275), &Font, cvScalar(0, 0, 255));
    }
    else
    {
      cvPutText(pSemanticImage, "[AI]", cvPoint(580, 275), &Font, cvScalar(255, 255, 255));
    }
  }
}

int CSemantic::getLanePosition(Indicators const * pGroundTruth, Indicators const * pGuessed, int MidOfSemantic)
{
  if (pGuessed)
  {
    return getLanePosition(pGuessed, MidOfSemantic);
  }
  else if (pGroundTruth)
  {
    return getLanePosition(pGroundTruth, MidOfSemantic);
  }

  return MidOfSemantic;
}

int CSemantic::getLanePosition(Indicators const * pIndicator, int MidOfSemantic)
{
  int LanePosition = MidOfSemantic;

  float StreetWidth = pIndicator->getNumberOfLanes() * pIndicator->LaneWidth;
  float CarPosition = 0;

  if (pIndicator->isCarInLane())
  {
    if (pIndicator->isLeftLane())
    {
      CarPosition = StreetWidth / 2 + pIndicator->getLL();
    }
    else
    {
      CarPosition = StreetWidth / 2 + pIndicator->getML();
    }
  }
  else if (pIndicator->isCarOnMarking())
  {
    if (pIndicator->isLeftLane())
    {
      CarPosition = StreetWidth / 2 + pIndicator->getL();
    }
    else
    {
      CarPosition = StreetWidth / 2 + pIndicator->getM();
    }
  }
  else if (pIndicator->isCarOffTheRoad())
  {
    return -100;
  }

  LanePosition += CarPosition * 12;

  return LanePosition;
}

void CSemantic::addLaneMarkings(int Lanes, int LanePosition, bool WasUpdated)
{
  if (Lanes > 1)
  {
    int Pace = int(getSpeed() * 1.2);
    if (Pace > 50)
    {
      Pace = 50;
    }

    if (WasUpdated)
    {
      MarkingHead += Pace;
    }
    if (MarkingHead > 0)
    {
      MarkingHead = MarkingHead - 110;
    }
    else if (MarkingHead < -110)
    {
      MarkingHead = MarkingHead + 110;
    }

    int MarkingStart = MarkingHead;
    int MarkingEnd   = MarkingHead+55;

    while (MarkingStart <= 660)
    {
      if (Lanes >= 3)
      {
        int LeftLanePosition  = LanePosition - 25;
        int RightLanePosition = LanePosition + 25;

        cvLine(pSemanticImage, cvPoint(LeftLanePosition,  MarkingStart), cvPoint(LeftLanePosition,  MarkingEnd), cvScalar(255,255,255), 2);
        cvLine(pSemanticImage, cvPoint(RightLanePosition, MarkingStart), cvPoint(RightLanePosition, MarkingEnd), cvScalar(255,255,255), 2);
      }
      else
      {
        cvLine(pSemanticImage, cvPoint(LanePosition, MarkingStart), cvPoint(LanePosition, MarkingEnd), cvScalar(255,255,255), 2);
      }

      MarkingStart = MarkingStart + 110;
      MarkingEnd   = MarkingEnd   + 110;
    }
  }
}

void CSemantic::addObstacles(Indicators const * pGroundTruth, Indicators const * pGuessed, int LanePosition)
{
  if (pGroundTruth)
  {
    addObstacles(pGroundTruth, LanePosition, true);
  }

  if (pGuessed)
  {
    addObstacles(pGuessed, LanePosition, false);
  }
}

void CSemantic::drawObstacle(int X, int Y, bool Filled)
{
  if (Filled)
  {
    cvRectangle(pSemanticImage, cvPoint(X - 14, 600 - Y - 28), cvPoint(X + 14, 600 - Y + 28), cvScalar(0, 255, 255), -1);
  }
  else
  {
    cvRectangle(pSemanticImage, cvPoint(X - 14, 600 - Y - 28), cvPoint(X + 14, 600 - Y + 28), cvScalar(237, 99, 157), 2);
  }
}

void CSemantic::addObstacles(Indicators const * pIndicator, int LanePosition, bool Filled)
{
  int Lanes = pIndicator->getNumberOfLanes();

  if (pIndicator->isCarInLane())
  {
    if (Lanes == 3 || Lanes == 1)
    {
      if (pIndicator->isLeftLane() && pIndicator->isDistLInLaneValid())
      {
        drawObstacle(LanePosition - 45, pIndicator->getDistLInLane()*12, Filled);
      }

      if (pIndicator->isDistMInLaneValid())
      {
        drawObstacle(LanePosition, pIndicator->getDistMInLane()*12, Filled);
      }

      if (pIndicator->isRightLane() && pIndicator->isDistRInLaneValid())
      {
        drawObstacle(LanePosition + 45, pIndicator->getDistRInLane()*12, Filled);
      }
    }
    else if (Lanes == 2)
    {
      if (pIndicator->isLeftLane())
      {
        if (pIndicator->isDistLInLaneValid())
        {
          drawObstacle(LanePosition - 22, pIndicator->getDistLInLane()*12, Filled);
        }

        if (pIndicator->isDistMInLaneValid())
        {
          drawObstacle(LanePosition + 22, pIndicator->getDistMInLane()*12, Filled);
        }
      }
      else if (pIndicator->isRightLane())
      {
        if (pIndicator->isDistRInLaneValid())
        {
          drawObstacle(LanePosition + 22, pIndicator->getDistRInLane()*12, Filled);
        }

        if (pIndicator->isDistMInLaneValid())
        {
          drawObstacle(LanePosition - 22, pIndicator->getDistMInLane()*12, Filled);
        }
      }

    }
  }
  else if (pIndicator->isCarOnMarking())
  {
    if (Lanes == 2)
    {
      if (pIndicator->isLeftLane() && pIndicator->isDistLOnMarkingValid())
      {
        drawObstacle(LanePosition - 22, pIndicator->getDistLOnMarking()*12, Filled);
      }

      if (pIndicator->isRightLane() && pIndicator->isDistROnMarkingValid())
      {
        drawObstacle(LanePosition + 22, pIndicator->getDistROnMarking()*12, Filled);
      }
    }
    else
    {
      if (pIndicator->isLeftLane() && pIndicator->isDistLOnMarkingValid())
      {
        drawObstacle(LanePosition, pIndicator->getDistLOnMarking()*12, Filled);
      }
      else if (pIndicator->isRightLane() && pIndicator->isDistROnMarkingValid())
      {
        drawObstacle(LanePosition, pIndicator->getDistROnMarking()*12, Filled);
      }
    }
  }
}

float CSemantic::getSpeed() const
{
  if (pAdditionalData)
  {
    return pAdditionalData->Speed;
  }

  return 30;
}

static void drawSingleErrorBar(IplImage * pImage, int X, int Y, int Width, int Height, float Percentage, CvScalar Color)
{
  if ((Width * Percentage) > 0)
  {
    Percentage = std::min(1.0F, Percentage);
    cvRectangle(pImage ,cvPoint(X, Y), cvPoint(X + Width * Percentage, Y + Height), Color, -1);
  }
}

static void drawErrorBars(IplImage * pImage, int X, int Y, float Current, float Mean, float Max)
{
  static int const BarWidth  = 241;
  static int const BarHeight = 5;

  drawSingleErrorBar(pImage, X, Y,             BarWidth, BarHeight, Current/Max, cvScalar(0,   0, 255));
  drawSingleErrorBar(pImage, X, Y + BarHeight, BarWidth, BarHeight, Mean/Max,    cvScalar(255, 0, 0));
}

void CSemantic::drawErrorMeasurement()
{
  if (pErrorMeasurement)
  {
    static int const X = 330;
    static int const Y = 290;

    cv::Mat(pErrorMeasurementBackground).copyTo(cv::Mat(pSemanticImage, cv::Rect(X, Y, pErrorMeasurementBackground->width, pErrorMeasurementBackground->height)));

    static int const BarX = X + 75;
    static int const BarY = Y + 41;
    static int const EntryHeight = 17;

    int i = 0;

    drawErrorBars(pSemanticImage, BarX, BarY + i * EntryHeight,
        pErrorMeasurement->getCurrentSquaredError().DistLL,
        pErrorMeasurement->getMeanSequaredError().DistLL,
        1.0);
    i++;

    drawErrorBars(pSemanticImage, BarX, BarY + i * EntryHeight,
        pErrorMeasurement->getCurrentSquaredError().DistMM,
        pErrorMeasurement->getMeanSequaredError().DistMM,
        1.0);
    i++;

    drawErrorBars(pSemanticImage, BarX, BarY + i * EntryHeight,
        pErrorMeasurement->getCurrentSquaredError().DistRR,
        pErrorMeasurement->getMeanSequaredError().DistRR,
        1.0);
    i++;

    drawErrorBars(pSemanticImage, BarX, BarY + i * EntryHeight,
        pErrorMeasurement->getCurrentSquaredError().LL,
        pErrorMeasurement->getMeanSequaredError().LL,
        1.0);
    i++;

    drawErrorBars(pSemanticImage, BarX, BarY + i * EntryHeight,
        pErrorMeasurement->getCurrentSquaredError().ML,
        pErrorMeasurement->getMeanSequaredError().ML,
        1.0);
    i++;

    drawErrorBars(pSemanticImage, BarX, BarY + i * EntryHeight,
        pErrorMeasurement->getCurrentSquaredError().MR,
        pErrorMeasurement->getMeanSequaredError().MR,
        1.0);
    i++;

    drawErrorBars(pSemanticImage, BarX, BarY + i * EntryHeight,
        pErrorMeasurement->getCurrentSquaredError().RR,
        pErrorMeasurement->getMeanSequaredError().RR,
        1.0);
    i++;
    i++;

    drawErrorBars(pSemanticImage, BarX, BarY + i * EntryHeight,
        pErrorMeasurement->getCurrentSquaredError().DistL,
        pErrorMeasurement->getMeanSequaredError().DistL,
        1.0);
    i++;

    drawErrorBars(pSemanticImage, BarX, BarY + i * EntryHeight,
        pErrorMeasurement->getCurrentSquaredError().DistR,
        pErrorMeasurement->getMeanSequaredError().DistR,
        1.0);
    i++;

    drawErrorBars(pSemanticImage, BarX, BarY + i * EntryHeight,
        pErrorMeasurement->getCurrentSquaredError().L,
        pErrorMeasurement->getMeanSequaredError().L,
        1.0);
    i++;

    drawErrorBars(pSemanticImage, BarX, BarY + i * EntryHeight,
        pErrorMeasurement->getCurrentSquaredError().M,
        pErrorMeasurement->getMeanSequaredError().M,
        1.0);
    i++;

    drawErrorBars(pSemanticImage, BarX, BarY + i * EntryHeight,
        pErrorMeasurement->getCurrentSquaredError().R,
        pErrorMeasurement->getMeanSequaredError().R,
        1.0);
    i++;
    i++;

    drawErrorBars(pSemanticImage, BarX, BarY + i * EntryHeight,
        pErrorMeasurement->getCurrentSquaredError().Angle,
        pErrorMeasurement->getMeanSequaredError().Angle,
        1.0);
    i++;
    i++;

    drawErrorBars(pSemanticImage, BarX, BarY + i * EntryHeight,
        pErrorMeasurement->getCurrentSquaredError().calcLoss()*10,
        pErrorMeasurement->getMeanSequaredError().calcLoss()*10,
        14*4.0*0.5);
    i++;

  }
}

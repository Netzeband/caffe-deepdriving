/**
 * Indicators.cpp
 *
 *  Created on: Mar 25, 2017
 *      Author: Andre Netzeband
 */

#include <cmath>

#include "Indicators.hpp"

float const Indicators::LaneWidth = 4.0;
float const Indicators::MaxLaneWidth = Indicators::LaneWidth * 1.375; // +37,5% tolerance

float const Indicators::MaxRR = 8;
float const Indicators::MaxMR = 5;
float const Indicators::MaxML = -Indicators::MaxMR;
float const Indicators::MaxLL = -Indicators::MaxRR;

float const Indicators::MaxR  = 6.5;
float const Indicators::MaxM  = 3;
float const Indicators::MaxL  = -Indicators::MaxR;

float const Indicators::MaxObstacleDist = 50;

Indicators::Indicators()
{
  Angle                              = 0;
  DistanceToLeftMarking              = 0;
  DistanceToCenterMarking            = 0;
  DistanceToRightMarking             = 0;
  DistanceToLeftObstacle             = 0;
  DistanceToRightObstacle            = 0;
  DistanceToLeftMarkingOfLeftLane    = 0;
  DistanceToLeftMarkingOfCenterLane  = 0;
  DistanceToRightMarkingOfCenterLane = 0;
  DistanceToRightMarkingOfRightLane  = 0;
  DistanceToLeftObstacleInLane       = 0;
  DistanceToCenterObstacleInLane     = 0;
  DistanceToRightObstacleInLane      = 0;
  Fast                               = 0;
}

float Indicators::getLaneWidth() const
{
  return -getML() + getMR();
}

bool Indicators::isLaneWidthValid() const
{
  return getLaneWidth() < MaxLaneWidth;
}

float Indicators::getLL() const
{
  return DistanceToLeftMarkingOfLeftLane;
}

float Indicators::getML() const
{
  return DistanceToLeftMarkingOfCenterLane;
}

float Indicators::getMR() const
{
  return DistanceToRightMarkingOfCenterLane;
}

float Indicators::getRR() const
{
  return DistanceToRightMarkingOfRightLane;
}

bool Indicators::isLLValid() const
{
  return getLL() > MaxLL;
}

bool Indicators::isMLValid() const
{
  return getML() > MaxML;
}

bool Indicators::isMRValid() const
{
  return getMR() < MaxMR;
}

bool Indicators::isRRValid() const
{
  return getRR() < MaxRR;
}

bool Indicators::isDistLInLaneValid() const
{
  return getDistLInLane() < MaxObstacleDist;
}

bool Indicators::isDistMInLaneValid() const
{
  return getDistMInLane() < MaxObstacleDist;
}

bool Indicators::isDistRInLaneValid() const
{
  return getDistRInLane() < MaxObstacleDist;
}

float Indicators::getDistLInLane() const
{
  return DistanceToLeftObstacleInLane;
}

float Indicators::getDistMInLane() const
{
  return DistanceToCenterObstacleInLane;
}

float Indicators::getDistRInLane() const
{
  return DistanceToRightObstacleInLane;
}

float Indicators::getL() const
{
  return DistanceToLeftMarking;
}

float Indicators::getM() const
{
  return DistanceToCenterMarking;
}

float Indicators::getR() const
{
  return DistanceToRightMarking;
}

bool Indicators::isLValid() const
{
  return getL() > MaxL;
}

bool Indicators::isRValid() const
{
  return getR() < MaxR;
}

bool Indicators::isMValid() const
{
  return (getM() > -MaxM) && (getM() < MaxM);
}

float Indicators::getDistLOnMarking() const
{
  return DistanceToLeftObstacle;
}

float Indicators::getDistROnMarking() const
{
  return DistanceToRightObstacle;
}

bool Indicators::isDistLOnMarkingValid() const
{
  return getDistLOnMarking() < MaxObstacleDist;
}

bool Indicators::isDistROnMarkingValid() const
{
  return getDistROnMarking() < MaxObstacleDist;
}

bool Indicators::isCarInLane() const
{
  if (isLaneWidthValid() && !isMValid())
  {
    return true;
  }
  else if (!isLaneWidthValid() && isMValid())
  {
    return false;
  }
  else if (isLaneWidthValid() && isMValid())
  {
    float const DiffToMaxLaneWidth = MaxLaneWidth - getLaneWidth();
    float const DiffToMaxM         = MaxM         - std::abs(getM());

    if (DiffToMaxLaneWidth > DiffToMaxM)
    {
      return true;
    }

    return false;
  }
  else if (!isLaneWidthValid() && !isMValid())
  {
    float const DiffToMaxLaneWidth = getLaneWidth()   - MaxLaneWidth;
    float const DiffToMaxM         = std::abs(getM()) - MaxM;

    if (DiffToMaxLaneWidth < DiffToMaxM)
    {
      return !isCarOffTheRoad();
    }

    return false;
  }

  return false;
}

bool Indicators::isCarOnMarking() const
{
  if (isLaneWidthValid() && !isMValid())
  {
    return false;
  }
  else if (!isLaneWidthValid() && isMValid())
  {
    return true;
  }
  else if (isLaneWidthValid() && isMValid())
  {
    float const DiffToMaxLaneWidth = MaxLaneWidth - getLaneWidth();
    float const DiffToMaxM         = MaxM         - std::abs(getM());

    if (DiffToMaxLaneWidth > DiffToMaxM)
    {
      return false;
    }

      return true;
  }
  else if (!isLaneWidthValid() && !isMValid())
  {
    float const DiffToMaxLaneWidth = getLaneWidth()   - MaxLaneWidth;
    float const DiffToMaxM         = std::abs(getM()) - MaxM;

    if (DiffToMaxLaneWidth < DiffToMaxM)
    {
      return false;
    }

    return !isCarOffTheRoad();
  }

  return false;
}

bool Indicators::isCarOffTheRoad() const
{
  // TODO: when is the car off the road?

  if (!isLaneWidthValid() && !isMValid())
  {
    if (!isMLValid() && !isMRValid() && !isMValid())
    {
      return true;
    }
  }

  return false;
}

bool Indicators::isLeftLane() const
{
  if (!isCarOffTheRoad())
  {
    if (isCarInLane())
    {
      return isLLValid();
    }
    else
    {
      return isLValid();
    }
  }

  return false;
}

bool Indicators::isRightLane() const
{
  if (isCarInLane())
  {
    return isRRValid();
  }
  else
  {
    return isRValid();
  }

  return false;
}

int  Indicators::getNumberOfLanes() const
{
  int NumberOfLanes = 0;

  if (isCarInLane())
  {
    NumberOfLanes = 1;
  }
  else
  {
    NumberOfLanes = 0;
  }

  if (isLeftLane())
  {
    NumberOfLanes++;
  }

  if (isRightLane())
  {
    NumberOfLanes++;
  }

  return NumberOfLanes;
}

void Indicators::print(std::ostream &rStream)
{
  rStream << "Is in lane:    " << isCarInLane()      << std::endl;
  rStream << "Is on marking: " << isCarOnMarking()   << std::endl;
  rStream << "Is off road:   " << isCarOffTheRoad()  << std::endl;
  rStream << "Lanes:         " << getNumberOfLanes() << std::endl;
  rStream << "Is left lane:  " << isLeftLane()       << std::endl;
  rStream << "Is right lane: " << isRightLane()      << std::endl;

  rStream << std::endl << "In Lane Context: " << std::endl;
  rStream << "Dist-L " << DistanceToLeftObstacleInLane   << ", ";
  rStream << "Dist-M " << DistanceToCenterObstacleInLane << ", ";
  rStream << "Dist-R " << DistanceToRightObstacleInLane  << ", ";
  rStream << "LL " << DistanceToLeftMarkingOfLeftLane    << ", ";
  rStream << "ML " << DistanceToLeftMarkingOfCenterLane  << ", ";
  rStream << "MR " << DistanceToRightMarkingOfCenterLane << ", ";
  rStream << "RR " << DistanceToRightMarkingOfRightLane  << std::endl;

  rStream << std::endl << "On Marking Context: " << std::endl;
  rStream << "Dist-L " << DistanceToLeftObstacle         << ", ";
  rStream << "Dist-M " << DistanceToRightObstacle        << ", ";
  rStream << "L " << DistanceToLeftMarking               << ", ";
  rStream << "M " << DistanceToCenterMarking             << ", ";
  rStream << "R " << DistanceToRightMarking              << std::endl;

  rStream << "Angle " << Angle                           << ", ";
  rStream << "Fast " << Fast                             << std::endl;
  rStream << std::endl;
  rStream.flush();
}

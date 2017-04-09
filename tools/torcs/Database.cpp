/**
 * Database.cpp
 *
 *  Created on: Mar 25, 2017
 *      Author: Andre Netzeband
 */

#include "Database.hpp"

#include <glog/logging.h>

#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>

using namespace caffe;

CLabel::CLabel()
{

}

void CLabel::readFromDatum(Datum const &rData)
{
  Indicators.Angle                              = rData.float_data(0);
  Indicators.DistanceToLeftMarking              = rData.float_data(1);
  Indicators.DistanceToCenterMarking            = rData.float_data(2);
  Indicators.DistanceToRightMarking             = rData.float_data(3);
  Indicators.DistanceToLeftObstacle             = rData.float_data(4);
  Indicators.DistanceToRightObstacle            = rData.float_data(5);
  Indicators.DistanceToLeftMarkingOfLeftLane    = rData.float_data(6);
  Indicators.DistanceToLeftMarkingOfCenterLane  = rData.float_data(7);
  Indicators.DistanceToRightMarkingOfCenterLane = rData.float_data(8);
  Indicators.DistanceToRightMarkingOfRightLane  = rData.float_data(9);
  Indicators.DistanceToLeftObstacleInLane       = rData.float_data(10);
  Indicators.DistanceToCenterObstacleInLane     = rData.float_data(11);
  Indicators.DistanceToRightObstacleInLane      = rData.float_data(12);
  Indicators.Fast                               = rData.float_data(13);
}

void CLabel::writeToDatum(caffe::Datum &rData, Indicators_t const &rIndicators)
{
  rData.add_float_data(rIndicators.Angle);
  rData.add_float_data(rIndicators.DistanceToLeftMarking);
  rData.add_float_data(rIndicators.DistanceToCenterMarking);
  rData.add_float_data(rIndicators.DistanceToRightMarking);
  rData.add_float_data(rIndicators.DistanceToLeftObstacle);
  rData.add_float_data(rIndicators.DistanceToRightObstacle);
  rData.add_float_data(rIndicators.DistanceToLeftMarkingOfLeftLane);
  rData.add_float_data(rIndicators.DistanceToLeftMarkingOfCenterLane);
  rData.add_float_data(rIndicators.DistanceToRightMarkingOfCenterLane);
  rData.add_float_data(rIndicators.DistanceToRightMarkingOfRightLane);
  rData.add_float_data(rIndicators.DistanceToLeftObstacleInLane);
  rData.add_float_data(rIndicators.DistanceToCenterObstacleInLane);
  rData.add_float_data(rIndicators.DistanceToRightObstacleInLane);
  rData.add_float_data(rIndicators.Fast);
}



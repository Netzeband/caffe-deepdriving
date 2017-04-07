/**
 * Indicators.hpp
 *
 *  Created on: Mar 25, 2017
 *      Author: Andre Netzeband
 */

#ifndef INDICATORS_HPP_
#define INDICATORS_HPP_

#include <iostream>

typedef struct Indicators
{
  /// @brief The angle of the host car.
  float Angle;

  /// @brief ?
  float Fast;

  // only valid if host car is driving directly on a lane marking
  /// @brief Distance of host car to the left marking.
  float DistanceToLeftMarking;

  /// @brief Distance of the host car to the marking where the car is driving on.
  float DistanceToCenterMarking;

  /// @brief Distance of host car to the right marking.
  float DistanceToRightMarking;

  /// @brief Distance of host car to any obstacle (traffic car) on the left lane.
  float DistanceToLeftObstacle;

  /// @brief Distance of host car to any obstacle (traffic car) on the right lane.
  float DistanceToRightObstacle;

  // only valid if host car is inside a lane
  /// @brief Distance of host car to the left marking of the left lane.
  float DistanceToLeftMarkingOfLeftLane;

  /// @brief Distance of host car to the left marking of the center lane
  ///        (where the car is currently driving in).
  float DistanceToLeftMarkingOfCenterLane;

  /// @brief Distance of host car to the right marking of the center lane
  ///        (where the car is currently driving in).
  float DistanceToRightMarkingOfCenterLane;

  /// @brief Distance of host car to the right marking of the right lane.
  float DistanceToRightMarkingOfRightLane;

  /// @brief Distance of host car to any obstacle (traffic car) on the left lane.
  float DistanceToLeftObstacleInLane;

  /// @brief Distance of host car to any obstacle (traffic car) on the center lane
  ///        (where the car is currently driving in).
  float DistanceToCenterObstacleInLane;

  /// @brief Distance of host car to any obstacle (traffic car) on the right lane.
  float DistanceToRightObstacleInLane;

  /// @brief The constructor.
  Indicators();

  /// @name Constant values
  /// @{

  /// @brief The width of a single lane (in normal cases).
  static float const LaneWidth;

  /// @brief The maximum width of a single lane (to take a tolerance into account).
  static float const MaxLaneWidth;

  /// @brief The maximum valid distance for the left lane marking of left lane.
  static float const MaxLL;

  /// @brief The maximum valid distance for the left lane marking of current lane.
  static float const MaxML;

  /// @brief The maximum valid distance for the right lane marking of current lane.
  static float const MaxMR;

  /// @brief The maximum valid distance for the right lane marking of right lane.
  static float const MaxRR;

  /// @brief The maximum valid distance for the left lane marking (if car is on lane marking).
  static float const MaxL;

  /// @brief The maximum valid distance for the lane marking the car is driving on (if car is on lane marking).
  static float const MaxM;

  /// @brief The maximum valid distance for the right lane marking (if car is on lane marking).
  static float const MaxR;

  /// @brief The maximum valid distance for obstacles.
  static float const MaxObstacleDist;

  /// @}

  /// @name Car is in lane
  /// @{

  /// @return Returns the measured lane width (only valid if car is in lane).
  float getLaneWidth() const;

  /// @return Returns true, if the lane width is valid.
  bool isLaneWidthValid() const;

  /// @return Returns the distance from the car to the left lane marking of the left lane, when car is in lane.
  float getLL() const;

  /// @return Returns the distance from the car to the left lane marking of the current lane, when car is in lane.
  float getML() const;

  /// @return Returns the distance from the car to the right lane marking of the current lane, when car is in lane.
  float getMR() const;

  /// @return Returns the distance from the car to the right lane marking of the right lane, when car is in lane.
  float getRR() const;

  /// @return Returns, if the distance to the left lane marking for the left lane is a valid value. If not, the left lane does not exist.
  bool isLLValid() const;

  /// @return Returns, if the distance to the left lane marking for the current lane is a valid value. If not, the car is no inside any lane.
  bool isMLValid() const;

  /// @return Returns, if the distance to the right lane marking for the current lane is a valid value. If not, the car is no inside any lane.
  bool isMRValid() const;

  /// @return Returns, if the distance to the right lane marking for the right lane is a valid value. If not, the right lane does not exist.
  bool isRRValid() const;

  /// @return Returns true, if the distance to an obstacle in the left lane has a valid value.
  bool isDistLInLaneValid() const;

  /// @return Returns true, if the distance to an obstacle in the current lane has a valid value.
  bool isDistMInLaneValid() const;

  /// @return Returns true, if the distance to an obstacle in the right lane has a valid value.
  bool isDistRInLaneValid() const;

  /// @return Returns the distance to an obstacle in the left lane.
  float getDistLInLane() const;

  /// @return Returns the distance to an obstacle in the current lane.
  float getDistMInLane() const;

  /// @return Returns the distance to an obstacle in the right lane.
  float getDistRInLane() const;

  /// @}

  /// @name Car is on lane marking
  /// @{

  /// @return Returns the distance to the left lane marking.
  float getL() const;

  /// @return Returns the distance to the lane marking, the car is driving on.
  float getM() const;

  /// @return Returns the distance to the right lane marking.
  float getR() const;

  /// @return Returns, if the distance to the left lane marking is valid. If this value is false, there is no lane left to the car.
  bool isLValid() const;

  /// @return Returns, if the distance to the right lane marking is valid. If this value is false, there is no lane right to the car.
  bool isRValid() const;

  /// @return Returns, if the distance to the lane marking where the car is driving on is valid. If this value is false, there the car is not driving on a lane marking.
  bool isMValid() const;

  /// @return Returns the distance of an obstacle in the left lane.
  float getDistLOnMarking() const;

  /// @return Returns the distance of an obstacle in the right lane.
  float getDistROnMarking() const;

  /// @return Returns true, if there is an obstacle in the left lane.
  bool isDistLOnMarkingValid() const;

  /// @return Returns true, if there is an obstacle in the right lane.
  bool isDistROnMarkingValid() const;

  /// @}

  /// @name Car states within lane system
  /// @{

  /// @return Returns true, if the car is driving in a lane.
  bool isCarInLane() const;

  /// @return Returns true, if the car is driving on a lane marking.
  bool isCarOnMarking() const;

  /// @return Returns true, if the car is driving off the road.
  bool isCarOffTheRoad() const;

  /// @return Returns true, if there is a left lane available.
  bool isLeftLane() const;

  /// @return Returns true, if there is a right lane available.
  bool isRightLane() const;

  /// @return Returns the number of lanes.
  int  getNumberOfLanes() const;

  /// @}

  /// @brief Prints the labels on a stream.
  void print(std::ostream &rStream);

} Indicators_t;

#endif /* INDICATORS_HPP_ */

/**
 * DriveController.hpp
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

#ifndef DRIVECONTROLLER_HPP_
#define DRIVECONTROLLER_HPP_

#include "Indicators.hpp"
#include "SharedMemory.hpp"

class CDriveController
{
  public:
    /// @brief Constructor.
    CDriveController();

    /// @brief Destructor.
    ~CDriveController();

    /// @brief Controls the driving commands according to the current indicators.
    void control(Indicators_t &rIndicators, TorcsData_t &rCommands, int Lanes);

  private:
    /// TODO: Merge all functions
    void controlLane1(Indicators_t &rIndicators, TorcsData_t &rCommands);
    void controlLane2(Indicators_t &rIndicators, TorcsData_t &rCommands);
    void controlLane3(Indicators_t &rIndicators, TorcsData_t &rCommands);

    float slow_down;
    float pre_dist_L;
    float pre_dist_R;
    int left_clear;
    int left_timer;
    int right_clear;
    int right_timer;
    int timer_set;
    int lane_change;
    float steer_trend;
    float steering_record[5];
    float coe_steer;
    float center_line;
    float pre_ML;
    float pre_MR;
    int steering_head;
    float desired_speed;
};




#endif /* DRIVECONTROLLER_HPP_ */

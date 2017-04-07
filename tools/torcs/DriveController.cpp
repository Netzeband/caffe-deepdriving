/**
 * DriveController.cpp
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

#include "DriveController.hpp"

#include <math.h>

CDriveController::CDriveController():
  slow_down(0),
  pre_dist_L(60),
  pre_dist_R(60),
  left_clear(0),
  left_timer(0),
  right_clear(0),
  right_timer(0),
  timer_set(60),
  lane_change(0),
  steer_trend(0),
  coe_steer(1.0),
  center_line(0),
  pre_ML(0),
  pre_MR(0),
  steering_head(0),
  desired_speed(0)
{
  steering_record[0] = 0;
  steering_record[1] = 0;
  steering_record[2] = 0;
  steering_record[3] = 0;
  steering_record[4] = 0;
}

CDriveController::~CDriveController()
{

}

void CDriveController::control(Indicators_t &rIndicators, TorcsData_t &rCommands, int Lanes)
{
  if (!(Lanes >= 1 && Lanes <= 3))
  {
    Lanes = rIndicators.getNumberOfLanes();
  }

  //std::cout << "Controlling: " << rCommands.IsControlling << ", Lanes: " << Lanes << std::endl;

  if (rCommands.IsControlling)
  {
    switch(Lanes)
    {
      case 1:
        controlLane1(rIndicators, rCommands);
        break;

      case 2:
        controlLane2(rIndicators, rCommands);
        break;

      case 3:
        controlLane3(rIndicators, rCommands);
        break;
    }
  }
}

void CDriveController::controlLane3(Indicators_t &rIndicators, TorcsData_t &rCommands)
{
  slow_down=100;

  if (pre_dist_L<20 && rIndicators.getDistLInLane()<20) {   // left lane is occupied or not
      left_clear=0;
      left_timer=0;
  } else left_timer++;

  if (pre_dist_R<20 && rIndicators.getDistRInLane()<20) {   // right lane is occupied or not
      right_clear=0;
      right_timer=0;
  } else right_timer++;

  pre_dist_L=rIndicators.getDistLInLane();
  pre_dist_R=rIndicators.getDistRInLane();

  if (left_timer>timer_set) {  // left lane is clear
     left_timer=timer_set;
     left_clear=1;
  }

  if (right_timer>timer_set) {  // right lane is clear
     right_timer=timer_set;
     right_clear=1;
  }


  if (lane_change==0 && rIndicators.getDistMInLane()<15) {  // if current lane is occupied

     steer_trend=steering_record[0]+steering_record[1]+steering_record[2]+steering_record[3]+steering_record[4];  // am I turning or not

     if (rIndicators.getLL()>-8 && left_clear==1 && steer_trend>=0 && steer_trend<0.2) {  // move to left lane
        lane_change=-2;
        coe_steer=6;
        right_clear=0;
        right_timer=0;
        left_clear=0;
        left_timer=30;
        timer_set=60;
     }

     else if (rIndicators.getRR()<8 && right_clear==1 && steer_trend<=0 && steer_trend>-0.2) {  // move to right lane
        lane_change=2;
        coe_steer=6;
        left_clear=0;
        left_timer=0;
        right_clear=0;
        right_timer=30;
        timer_set=60;
     }

     else {
        float v_max=20;
        float c=2.772;
        float d=-0.693;
        slow_down=v_max*(1-exp(-c/v_max*rIndicators.getDistMInLane()-d));  // optimal velocity car-following model
        if (slow_down<0) slow_down=0;
     }
  }

  ///////////////////////////////////////////////// prefer to stay in the central lane
  else if (lane_change==0 && rIndicators.getDistMInLane()>=15) {

     steer_trend=steering_record[0]+steering_record[1]+steering_record[2]+steering_record[3]+steering_record[4];  // am I turning or not

     if (rIndicators.getRR()>8 && left_clear==1 && steer_trend>=0 && steer_trend<0.2) {  // in right lane, move to central lane
        lane_change=-2;
        coe_steer=6;
        left_clear=0;
        left_timer=30;
     }

     else if (rIndicators.getLL()<-8 && right_clear==1 && steer_trend<=0 && steer_trend>-0.2) {  // in left lane, move to central lane
        lane_change=2;
        coe_steer=6;
        right_clear=0;
        right_timer=30;
     }
  }
  ///////////////////////////////////////////////// END prefer to stay in the central lane

  ///////////////////////////////////////////////// implement lane changing or car-following
  if (lane_change==0) {
     if (-rIndicators.getML()+rIndicators.getMR()<5.5) {
        coe_steer=1.5;
        center_line=(rIndicators.getML()+rIndicators.getMR())/2;
        pre_ML=rIndicators.getML();
        pre_MR=rIndicators.getMR();
        if (rIndicators.getM()<1)
           coe_steer=0.4;
     } else {
        if (-pre_ML>pre_MR)
           center_line=(rIndicators.getL()+rIndicators.getM())/2;
        else
           center_line=(rIndicators.getR()+rIndicators.getM())/2;
        coe_steer=0.3;
     }
  }

  else if (lane_change==-2) {
     if (-rIndicators.getML()+rIndicators.getMR()<5.5) {
        center_line=(rIndicators.getLL()+rIndicators.getML())/2;
        if (rIndicators.getL()>-5 && rIndicators.getM()<1.5)
           center_line=(center_line+(rIndicators.getL()+rIndicators.getM())/2)/2;
     } else {
        center_line=(rIndicators.getL()+rIndicators.getM())/2;
        coe_steer=20;
        lane_change=-1;
     }
  }

  else if (lane_change==-1) {
     if (rIndicators.getL()>-5 && rIndicators.getM()<1.5) {
        center_line=(rIndicators.getL()+rIndicators.getM())/2;
        if (-rIndicators.getML()+rIndicators.getMR()<5.5)
           center_line=(center_line+(rIndicators.getML()+rIndicators.getMR())/2)/2;
     } else {
        center_line=(rIndicators.getML()+rIndicators.getMR())/2;
        lane_change=0;
     }
  }

  else if (lane_change==2) {
     if (-rIndicators.getML()+rIndicators.getMR()<5.5) {
        center_line=(rIndicators.getRR()+rIndicators.getMR())/2;
        if (rIndicators.getR()<5 && rIndicators.getM()<1.5)
           center_line=(center_line+(rIndicators.getR()+rIndicators.getM())/2)/2;
     } else {
        center_line=(rIndicators.getR()+rIndicators.getM())/2;
        coe_steer=20;
        lane_change=1;
     }
  }

  else if (lane_change==1) {
     if (rIndicators.getR()<5 && rIndicators.getM()<1.5) {
        center_line=(rIndicators.getR()+rIndicators.getM())/2;
        if (-rIndicators.getML()+rIndicators.getMR()<5.5)
           center_line=(center_line+(rIndicators.getML()+rIndicators.getMR())/2)/2;
     } else {
        center_line=(rIndicators.getML()+rIndicators.getMR())/2;
        lane_change=0;
     }
  }
  ///////////////////////////////////////////////// END implement lane changing or car-following

  static float const road_width = 8.0;
  rCommands.Steering = (rIndicators.Angle - center_line/road_width) / 0.541052/coe_steer;  // steering control, "rCommands.Steering" [-1,1] is the value sent back to TORCS

  if (lane_change==0 && coe_steer>1 && rCommands.Steering>0.1)   // reshape the steering control curve
     rCommands.Steering=rCommands.Steering*(2.5*rCommands.Steering+0.75);

  steering_record[steering_head]=rCommands.Steering;  // update previous steering record
  steering_head++;
  if (steering_head==5) steering_head=0;


  if (rIndicators.Fast==1) desired_speed=20;
  else desired_speed=20-fabs(steering_record[0]+steering_record[1]+steering_record[2]+steering_record[3]+steering_record[4])*4.5;
  if (desired_speed<10) desired_speed=10;

  if (slow_down<desired_speed) desired_speed=slow_down;

  ///////////////////////////// speed control
  if (desired_speed>=rCommands.Speed) {
      rCommands.Accelerating = 0.2*(desired_speed-rCommands.Speed+1);
      if (rCommands.Accelerating>1) rCommands.Accelerating=1.0;
      rCommands.Breaking = 0.0;
  } else {
      rCommands.Breaking = 0.1*(rCommands.Speed-desired_speed);
      if (rCommands.Breaking>1) rCommands.Breaking=1.0;
      rCommands.Accelerating = 0.0;
  }
  ///////////////////////////// END speed control
}

void CDriveController::controlLane2(Indicators_t &rIndicators, TorcsData_t &rCommands)
{
  slow_down=100;

  if (pre_dist_L<20 && rIndicators.getDistLInLane()<20) {   // left lane is occupied or not
      left_clear=0;
      left_timer=0;
  } else left_timer++;

  if (pre_dist_R<20 && rIndicators.getDistRInLane()<20) {   // right lane is occupied or not
      right_clear=0;
      right_timer=0;
  } else right_timer++;

  pre_dist_L=rIndicators.getDistLInLane();
  pre_dist_R=rIndicators.getDistRInLane();

  if (left_timer>timer_set) {   // left lane is clear
     left_timer=timer_set;
     left_clear=1;
  }

  if (right_timer>timer_set) {   // right lane is clear
     right_timer=timer_set;
     right_clear=1;
  }


  if (lane_change==0 && rIndicators.getDistMInLane()<15) {   // if current lane is occupied

     steer_trend=steering_record[0]+steering_record[1]+steering_record[2]+steering_record[3]+steering_record[4];   // am I turning or not

     if (rIndicators.getLL()>-8 && left_clear==1 && steer_trend>=0) {   // move to left lane
        lane_change=-2;
        coe_steer=6;
        right_clear=0;
        right_timer=0;
        left_clear=0;
        left_timer=0;
        timer_set=30;
     }

     else if (rIndicators.getRR()<8 && right_clear==1 && steer_trend<=0) {   // move to right lane
        lane_change=2;
        coe_steer=6;
        left_clear=0;
        left_timer=0;
        right_clear=0;
        right_timer=0;
        timer_set=30;
     }

     else {
        float v_max=20;
        float c=2.772;
        float d=-0.693;
        slow_down=v_max*(1-exp(-c/v_max*rIndicators.getDistMInLane()-d));  // optimal vilcity car-following model
        if (slow_down<0) slow_down=0;
     }
  }

  ///////////////////////////////////////////////// prefer to stay in the right lane
  else if (lane_change==0 && rIndicators.getDistMInLane()>=15) {

     steer_trend=steering_record[0]+steering_record[1]+steering_record[2]+steering_record[3]+steering_record[4];  // am I turning or not

     if (rIndicators.getLL()<-8 && right_clear==1 && steer_trend<=0 && steer_trend>-0.2) {  // in left lane, move to right lane
        lane_change=2;
        coe_steer=6;
        right_clear=0;
        right_timer=20;
     }
  }
  ///////////////////////////////////////////////// END prefer to stay in the right lane

  ///////////////////////////////////////////////// implement lane changing or car-following
  if (lane_change==0) {
     if (-rIndicators.getML()+rIndicators.getMR()<5.5) {
        coe_steer=1.5;
        center_line=(rIndicators.getML()+rIndicators.getMR())/2;
        pre_ML=rIndicators.getML();
        pre_MR=rIndicators.getMR();
        if (rIndicators.getM()<1)
           coe_steer=0.4;
     } else {
        if (-pre_ML>pre_MR)
           center_line=(rIndicators.getL()+rIndicators.getM())/2;
        else
           center_line=(rIndicators.getR()+rIndicators.getM())/2;
        coe_steer=0.3;
     }
  }

  else if (lane_change==-2) {
     if (-rIndicators.getML()+rIndicators.getMR()<5.5) {
        center_line=(rIndicators.getLL()+rIndicators.getML())/2;
        if (rIndicators.getL()>-5 && rIndicators.getM()<1.5)
           center_line=(center_line+(rIndicators.getL()+rIndicators.getM())/2)/2;
     } else {
        center_line=(rIndicators.getL()+rIndicators.getM())/2;
        coe_steer=20;
        lane_change=-1;
     }
  }

  else if (lane_change==-1) {
     if (rIndicators.getL()>-5 && rIndicators.getM()<1.5) {
        center_line=(rIndicators.getL()+rIndicators.getM())/2;
        if (-rIndicators.getML()+rIndicators.getMR()<5.5)
           center_line=(center_line+(rIndicators.getML()+rIndicators.getMR())/2)/2;
     } else {
        center_line=(rIndicators.getML()+rIndicators.getMR())/2;
        lane_change=0;
     }
  }

  else if (lane_change==2) {
     if (-rIndicators.getML()+rIndicators.getMR()<5.5) {
        center_line=(rIndicators.getRR()+rIndicators.getMR())/2;
        if (rIndicators.getR()<5 && rIndicators.getM()<1.5)
           center_line=(center_line+(rIndicators.getR()+rIndicators.getM())/2)/2;
     } else {
        center_line=(rIndicators.getR()+rIndicators.getM())/2;
        coe_steer=20;
        lane_change=1;
     }
  }

  else if (lane_change==1) {
     if (rIndicators.getR()<5 && rIndicators.getM()<1.5) {
        center_line=(rIndicators.getR()+rIndicators.getM())/2;
        if (-rIndicators.getML()+rIndicators.getMR()<5.5)
           center_line=(center_line+(rIndicators.getML()+rIndicators.getMR())/2)/2;
     } else {
        center_line=(rIndicators.getML()+rIndicators.getMR())/2;
        lane_change=0;
     }
  }
  ///////////////////////////////////////////////// END implement lane changing or car-following

  static const float road_width = 8.0;
  rCommands.Steering = (rIndicators.Angle - center_line/road_width) / 0.541052/coe_steer;   // steering control, "rCommands.Steering" [-1,1] is the value sent back to TORCS

  if (lane_change==0 && coe_steer>1 && rCommands.Steering>0.1)   // reshape the steering control curve
     rCommands.Steering=rCommands.Steering*(2.5*rCommands.Steering+0.75);

  steering_record[steering_head]=rCommands.Steering;   // update previous steering record
  steering_head++;
  if (steering_head==5) steering_head=0;


  if (rIndicators.Fast==1) desired_speed=20;
  else desired_speed=20-fabs(steering_record[0]+steering_record[1]+steering_record[2]+steering_record[3]+steering_record[4])*4.5;
  if (desired_speed<10) desired_speed=10;

  if (slow_down<desired_speed) desired_speed=slow_down;

  ///////////////////////////// speed control
  if (desired_speed>=rCommands.Speed) {
      rCommands.Accelerating = 0.2*(desired_speed-rCommands.Speed+1);
      if (rCommands.Accelerating>1) rCommands.Accelerating=1.0;
      rCommands.Breaking = 0.0;
  } else {
      rCommands.Breaking = 0.1*(rCommands.Speed-desired_speed);
      if (rCommands.Breaking>1) rCommands.Breaking=1.0;
      rCommands.Accelerating = 0.0;
  }
  ///////////////////////////// END speed control
}

void CDriveController::controlLane1(Indicators_t &rIndicators, TorcsData_t &rCommands)
{
  slow_down=100;

  if (rIndicators.getDistMInLane()<15) {
        float v_max=20;
        float c=2.772;
        float d=-0.693;
        slow_down=v_max*(1-exp(-c/v_max*rIndicators.getDistMInLane()-d));  // optimal vilcity car-following model
        if (slow_down<0) slow_down=0;
  }

  if (-rIndicators.getML()+rIndicators.getMR()<5.5) {
     coe_steer=1.5;
     center_line=(rIndicators.getML()+rIndicators.getMR())/2;
     pre_ML=rIndicators.getML();
     pre_MR=rIndicators.getMR();
     if (rIndicators.getM()<1)
        coe_steer=0.4;
  } else {
     if (-pre_ML>pre_MR)
        center_line=(rIndicators.getL()+rIndicators.getM())/2;
     else
        center_line=(rIndicators.getR()+rIndicators.getM())/2;
     coe_steer=0.3;
  }

  static float const road_width = 8.0;
  rCommands.Steering = (rIndicators.Angle - center_line/road_width) / 0.541052/coe_steer;   // steering control, "rCommands.Steering" [-1,1] is the value sent back to TORCS

  if (coe_steer>1 && rCommands.Steering>0.1)   // reshape the steering control curve
     rCommands.Steering=rCommands.Steering*(2.5*rCommands.Steering+0.75);

  steering_record[steering_head]=rCommands.Steering;  // update previous steering record
  steering_head++;
  if (steering_head==5) steering_head=0;

  if (rIndicators.Fast==1) desired_speed=20;
  else desired_speed=20-fabs(steering_record[0]+steering_record[1]+steering_record[2]+steering_record[3]+steering_record[4])*4.5;
  if (desired_speed<10) desired_speed=10;

  if (slow_down<desired_speed) desired_speed=slow_down;

  ///////////////////////////// speed control
  if (desired_speed>=rCommands.Speed) {
      rCommands.Accelerating = 0.2*(desired_speed-rCommands.Speed+1);
      if (rCommands.Accelerating>1) rCommands.Accelerating=1.0;
      rCommands.Breaking = 0.0;
  } else {
      rCommands.Breaking = 0.1*(rCommands.Speed-desired_speed);
      if (rCommands.Breaking>1) rCommands.Breaking=1.0;
      rCommands.Accelerating = 0.0;
  }
  ///////////////////////////// END speed control
}


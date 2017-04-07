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

#include "SharedMemory.hpp"

#include <glog/logging.h>

#include <iostream>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/shm.h>
#include <cstring>

#define TORCS_IMAGE_WIDTH   640
#define TORCS_IMAGE_HEIGHT  480
#define RESIZE_IMAGE_WIDTH  280
#define RESIZE_IMAGE_HEIGHT 210


typedef struct
{
    int written;  //a label, if 1: available to read, if 0: available to write
    uint8_t data[TORCS_IMAGE_WIDTH*TORCS_IMAGE_HEIGHT*3];  // image data field
    int control;
    int pause;
    double fast;

    double dist_L;
    double dist_R;

    double toMarking_L;
    double toMarking_M;
    double toMarking_R;

    double dist_LL;
    double dist_MM;
    double dist_RR;

    double toMarking_LL;
    double toMarking_ML;
    double toMarking_MR;
    double toMarking_RR;

    double toMiddle;
    double angle;
    double speed;

    double steerCmd;
    double accelCmd;
    double brakeCmd;
} SharedMemoryLayout_t;

CSharedMemory::CSharedMemory():
    pMemory(0),
    SharedMemoryID(-1),
    IsDataUpdated(false)
{
  attach();
  initMemory();
}

CSharedMemory::~CSharedMemory()
{
  detach();
}

void CSharedMemory::attach()
{
  SharedMemoryID = shmget((key_t)4567, sizeof(SharedMemoryLayout_t), 0666 | IPC_CREAT);

  if(SharedMemoryID == -1)
  {
    std::cerr << "ERROR: Cannot attach to shared memory from TORCS (ID is invalid)!" << std::endl;
  }
  else
  {
    pMemory = shmat(SharedMemoryID, 0, 0);
    if(pMemory == ((void*)-1))
    {
      pMemory = 0;
      std::cerr << "ERROR: Cannot attach to shared memory from TORCS (Memory address is invalid)!" << std::endl;
    }
    else
    {
      std::cout << std::endl << "********** Memory sharing started, attached at " << pMemory << " **********" << std::endl;
    }
  }
}

void CSharedMemory::detach()
{
  if (pMemory)
  {
    if(shmdt(pMemory) == -1)
    {
      std::cerr << "ERROR: Cannot detach from shared memory!" << std::endl;
    }
    else
    {
      if(shmctl(SharedMemoryID, IPC_RMID, 0) == -1)
      {
        std::cerr << "ERROR: Cannot detach from shared memory!" << std::endl;
      }
      else
      {
        std::cout << std::endl << "********** Memory sharing stopped. Good Bye! **********" << std::endl;
      }
    }
  }
}

void CSharedMemory::initMemory()
{
  if (pMemory)
  {
    SharedMemoryLayout_t * const pShared = (SharedMemoryLayout_t *)pMemory;

    memset(pShared->data, 0, TORCS_IMAGE_WIDTH * TORCS_IMAGE_HEIGHT * 3);

    pShared->written = 0;
    pShared->control = 0;
    pShared->pause = 0;
    pShared->fast = 0.0;

    pShared->dist_L = 0.0;
    pShared->dist_R = 0.0;

    pShared->toMarking_L = 0.0;
    pShared->toMarking_M = 0.0;
    pShared->toMarking_R = 0.0;

    pShared->dist_LL = 0.0;
    pShared->dist_MM = 0.0;
    pShared->dist_RR = 0.0;

    pShared->toMarking_LL = 0.0;
    pShared->toMarking_ML = 0.0;
    pShared->toMarking_MR = 0.0;
    pShared->toMarking_RR = 0.0;

    pShared->toMiddle = 0.0;
    pShared->angle = 0.0;
    pShared->speed = 0.0;

    pShared->steerCmd = 0.0;
    pShared->accelCmd = 0.0;
    pShared->brakeCmd = 0.0;

    Indicators.Angle = 0;
    Indicators.Fast  = 0;

    Indicators.DistanceToLeftObstacle  = Indicators.MaxObstacleDist;
    Indicators.DistanceToRightObstacle = Indicators.MaxObstacleDist;

    Indicators.DistanceToLeftMarking   = Indicators.MaxL;
    Indicators.DistanceToCenterMarking = Indicators.MaxM;
    Indicators.DistanceToRightMarking  = Indicators.MaxR;

    Indicators.DistanceToLeftObstacleInLane   = Indicators.MaxObstacleDist;
    Indicators.DistanceToCenterObstacleInLane = Indicators.MaxObstacleDist;
    Indicators.DistanceToRightObstacleInLane  = Indicators.MaxObstacleDist;

    Indicators.DistanceToLeftMarkingOfLeftLane    =  Indicators.MaxLL;
    Indicators.DistanceToLeftMarkingOfCenterLane  = -Indicators.LaneWidth/2;
    Indicators.DistanceToRightMarkingOfCenterLane =  Indicators.LaneWidth/2;
    Indicators.DistanceToRightMarkingOfRightLane  =  Indicators.MaxRR;

    Image.setNoVideo(RESIZE_IMAGE_WIDTH, RESIZE_IMAGE_HEIGHT);

    TorcsData.IsNotPause      = 0;
    TorcsData.IsControlling   = 0;
    TorcsData.Accelerating    = 0;
    TorcsData.Breaking        = 0;
    TorcsData.Steering        = 0;
    TorcsData.Speed           = 0;
    TorcsData.IsAIControlled  = true;
    TorcsData.ShowGroundTruth = true;
  }
}

void CSharedMemory::read()
{
  SharedMemoryLayout_t * const pShared = (SharedMemoryLayout_t *)pMemory;

  if (pShared->written)
  {
    Indicators.Angle = pShared->angle;
    Indicators.Fast  = int(pShared->fast);

    Indicators.DistanceToLeftObstacle  = pShared->dist_L;
    Indicators.DistanceToRightObstacle = pShared->dist_R;

    Indicators.DistanceToLeftMarking   = pShared->toMarking_L;
    Indicators.DistanceToCenterMarking = pShared->toMarking_M;
    Indicators.DistanceToRightMarking  = pShared->toMarking_R;

    Indicators.DistanceToLeftObstacleInLane   = pShared->dist_LL;
    Indicators.DistanceToCenterObstacleInLane = pShared->dist_MM;
    Indicators.DistanceToRightObstacleInLane  = pShared->dist_RR;

    Indicators.DistanceToLeftMarkingOfLeftLane    = pShared->toMarking_LL;
    Indicators.DistanceToLeftMarkingOfCenterLane  = pShared->toMarking_ML;
    Indicators.DistanceToRightMarkingOfCenterLane = pShared->toMarking_MR;
    Indicators.DistanceToRightMarkingOfRightLane  = pShared->toMarking_RR;

    Image.readFromMemory(pShared->data, TORCS_IMAGE_WIDTH, TORCS_IMAGE_HEIGHT, RESIZE_IMAGE_WIDTH, RESIZE_IMAGE_HEIGHT);

    TorcsData.IsNotPause    = pShared->pause;
    TorcsData.IsControlling = pShared->control;
    TorcsData.Accelerating  = pShared->accelCmd;
    TorcsData.Breaking      = pShared->brakeCmd;
    TorcsData.Steering      = pShared->steerCmd;
    TorcsData.Speed         = pShared->speed;

    IsDataUpdated = true;
  }
}

bool CSharedMemory::isDataUpdated()
{
  return IsDataUpdated;
}

void CSharedMemory::write()
{
  SharedMemoryLayout_t * const pShared = (SharedMemoryLayout_t *)pMemory;

  pShared->pause    = TorcsData.IsNotPause ? 1 : 0;
  pShared->control  = TorcsData.IsControlling ? 1 : 0;
  pShared->accelCmd = TorcsData.Accelerating;
  pShared->brakeCmd = TorcsData.Breaking;
  pShared->steerCmd = TorcsData.Steering;

  //printf("Pause %d, Control %d\n", pShared->pause, pShared->control);

  if (IsDataUpdated)
  {
    pShared->written=0;

    IsDataUpdated = false;
  }
}

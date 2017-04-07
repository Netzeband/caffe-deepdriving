/*
 * torcs_visualize_database.cpp
 *
 *  Created on: Mar 24, 2017
 *      Author: Andre Netzeband
 *
 *  This file was initially creates by Chenyi Chen for the DeepDriving project.
 *  See http://deepdriving.cs.princeton.edu for more details.
 *
 *  This implementations uses the latest caffe implementation (1.0.0-rc5) instead of the old
 *  one used by Chenyi Chen in 2014.
 */

////////////////////////////////////////////////
//
//  Drives the host car in TORCS.
//
////////////////////////////////////////////////

#include <glog/logging.h>

#include "caffe/caffe.hpp"
#include "caffe/util/db_leveldb.hpp"

#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/highgui/highgui_c.h>

#include "torcs/Arguments.hpp"
#include "torcs/SharedMemory.hpp"
#include "torcs/Semantic.hpp"
#include "torcs/DriveController.hpp"
#include "torcs/NeuralNet.hpp"

#define ImageWidth  280
#define ImageHeight 210

using namespace caffe;
using std::string;

int run(string ModelPath, string WeightsPath, string MeanPath, int Lanes, int GPUDevice);

int main(int argc, char** argv)
{
  ::google::InitGoogleLogging(argv[0]);

  string const ModelPath = getArgument(argc, argv, "--model");

  if (ModelPath.empty())
  {
    std::cout << "Please define a path to the model description." << std::endl;
    std::cout << "Example: " << std::endl << std::endl;
    std::cout << argv[0] << " --model pre_trained/modelfile.prototxt --weights pre_trained/weightsfile.binaryproto --mean pre_trained/meanfile.binaryproto --lanes 3" << std::endl << std::endl;
    return -1;
  }

  string const WeightsPath = getArgument(argc, argv, "--weights");

  if (WeightsPath.empty())
  {
    std::cout << "Please define a path to the model weights." << std::endl;
    std::cout << "Example: " << std::endl << std::endl;
    std::cout << argv[0] << " --model pre_trained/modelfile.prototxt --weights pre_trained/weightsfile.binaryproto --mean pre_trained/meanfile.binaryproto --lanes 3" << std::endl << std::endl;
    return -1;
  }

  string const MeanPath = getArgument(argc, argv, "--mean");

  if (MeanPath.empty())
  {
    std::cout << "Please define a path to the mean file." << std::endl;
    std::cout << "Example: " << std::endl << std::endl;
    std::cout << argv[0] << " --model pre_trained/modelfile.prototxt --weights pre_trained/weightsfile.binaryproto --mean pre_trained/meanfile.binaryproto --lanes 3" << std::endl << std::endl;
    return -1;
  }

  string const LaneString   = getArgument(argc, argv, "--lanes");

  if (LaneString.empty())
  {
    std::cout << "Please specify the number of lanes (from 1 to 3)." << std::endl;
    std::cout << "Example: " << std::endl << std::endl;
    std::cout << argv[0] << " --data-path pre_trained/TORCS_Training_1F --lanes 3" << std::endl << std::endl;
    return -1;
  }

  int Lanes = atoi(LaneString.c_str());

  if (Lanes < 1 || Lanes > 3)
  {
    std::cout << "Please specify the number of lanes (from 1 to 3)." << std::endl;
    std::cout << "Example: " << std::endl << std::endl;
    std::cout << argv[0] << " --data-path pre_trained/TORCS_Training_1F --lanes 3" << std::endl << std::endl;
    return -1;
  }

  int GPUDevice = -1;
  string const GPUString   = getArgument(argc, argv, "--gpu");

  if (!GPUString.empty())
  {
    GPUDevice = atoi(GPUString.c_str());

    if (GPUDevice < 0)
    {
      GPUDevice = -1;
    }
  }
  else
  {
    std::cout << "WARNING: GPU usage is disabled. Enable it with --gpu <DeviceNumber> or disable it explicitly with --gpu -1." << std::endl;
  }

  return run(ModelPath, WeightsPath, MeanPath, Lanes, GPUDevice);
}

bool processKeys(TorcsData_t &rData);

int run(string ModelPath, string WeightsPath, string MeanPath, int Lanes, int GPUDevice)
{
  CSharedMemory     TorcsMemory;
  CSemantic         Semantic;
  CDriveController  DriveController;
  CNeuralNet        NeuralNet(ModelPath, WeightsPath, MeanPath, GPUDevice);
  CErrorMeasurement ErrorMeas;

  Semantic.setFrameImage(&TorcsMemory.Image);
  Semantic.setAdditionalData(&TorcsMemory.TorcsData);
  Semantic.setErrorMeasurement(&ErrorMeas);
  Semantic.show(0, 0, false);

  Indicators_t * pGroundTruth = &TorcsMemory.Indicators;
  Indicators_t * pEstimatedIndicators = 0;
  Indicators_t EstimatedIndicators;

  bool IsEnd = false;
  while(!IsEnd)
  {
    TorcsMemory.read();

    if (TorcsMemory.TorcsData.ShowGroundTruth)
    {
      pGroundTruth = &TorcsMemory.Indicators;
    }
    else
    {
      pGroundTruth = 0;
    }

    if (TorcsMemory.isDataUpdated())
    {
      pEstimatedIndicators = &EstimatedIndicators;
      NeuralNet.process(pEstimatedIndicators, TorcsMemory.Image);
      ErrorMeas.measure(&TorcsMemory.Indicators, pEstimatedIndicators);

      if (TorcsMemory.TorcsData.IsAIControlled)
      {
        DriveController.control(*pEstimatedIndicators, TorcsMemory.TorcsData, Lanes);
      }
      else
      {
        DriveController.control(TorcsMemory.Indicators, TorcsMemory.TorcsData, Lanes);
      }

      Semantic.show(pGroundTruth, pEstimatedIndicators, true);

      std::cout << std::endl << "Ground-Truth: " << std::endl;
      std::cout << "============= " << std::endl;
      TorcsMemory.Indicators.print(std::cout);
      std::cout << std::endl << "Estimated: " << std::endl;
      std::cout << "============= " << std::endl;
      EstimatedIndicators.print(std::cout);
    }
    else
    {
      Semantic.show(pGroundTruth, pEstimatedIndicators, false);
    }

    IsEnd = processKeys(TorcsMemory.TorcsData);
    TorcsMemory.write();
  }

  ErrorMeas.print(std::cout);
  NeuralNet.printTimeSummery(std::cout);

  return 0;
}

bool processKeys(TorcsData_t &rData)
{
  static const char PauseKey     = 'p';
  static const char ControlKey   = 'c';
  static const char AIKey        = 'a';
  static const char VisualKey    = 'v';
  static const char ForwardKey   = 82;
  static const char BackwardKey  = 84;
  static const char LeftKey      = 81;
  static const char RightKey     = 83;
  static const char EscKey       = 27;
  static const int KeyTime = 1;
  static int KeyCounter = 0;

  char Key = cvWaitKey(KeyTime);

  // Escape Key
  if (Key == EscKey)
  {
    return true;
  }

  KeyCounter++;

  if (Key == PauseKey)
  {
    rData.IsNotPause = !rData.IsNotPause;
  }

  if (Key == ControlKey)
  {
    rData.IsControlling = !rData.IsControlling;
  }

  if (Key == AIKey)
  {
    rData.IsAIControlled = !rData.IsAIControlled;
  }

  if (Key == VisualKey)
  {
    rData.ShowGroundTruth = !rData.ShowGroundTruth;
  }

  if (Key == ForwardKey)
  {
    rData.Accelerating = 1.0;
    rData.Breaking = 0.0;
  }

  if (Key == BackwardKey)
  {
    rData.Accelerating = 0.0;
    rData.Breaking = 0.8;
  }

  if (Key == LeftKey)
  {
    rData.Steering = 0.5;
    KeyCounter = 0;
  }

  if (Key == RightKey)
  {
    rData.Steering = -0.5;
    KeyCounter = 0;
  }

  if (KeyCounter > 20)
  {
    // TODO: manual = 0
  }

  return false;
}



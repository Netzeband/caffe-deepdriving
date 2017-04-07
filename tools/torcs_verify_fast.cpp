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
//  Verifies the network model with a recorded
//  data set and outputs the error.
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
#include "torcs/Semantic.hpp"
#include "torcs/NeuralNet.hpp"
#include "torcs/Database.hpp"

#define ImageWidth  280
#define ImageHeight 210

using namespace caffe;
using std::string;

int verify(string DataPath, string ModelPath, string WeightsPath, string MeanPath, int GPUDevice);

int main(int argc, char** argv)
{
  ::google::InitGoogleLogging(argv[0]);

  string const DataPath = getArgument(argc, argv, "--data");

  if (DataPath.empty())
  {
    std::cout << "Please define a path to the laveldb data." << std::endl;
    std::cout << "Example: " << std::endl << std::endl;
    std::cout << argv[0] << " --data pre_trained/TORCS_Training_1F --model pre_trained/modelfile.prototxt --weights pre_trained/weightsfile.binaryproto --mean pre_trained/meanfile.binaryproto" << std::endl << std::endl;
    return -1;
  }

  string const ModelPath = getArgument(argc, argv, "--model");

  if (ModelPath.empty())
  {
    std::cout << "Please define a path to the model description." << std::endl;
    std::cout << "Example: " << std::endl << std::endl;
    std::cout << argv[0] << " --data pre_trained/TORCS_Training_1F --model pre_trained/modelfile.prototxt --weights pre_trained/weightsfile.binaryproto --mean pre_trained/meanfile.binaryproto" << std::endl << std::endl;
    return -1;
  }

  string const WeightsPath = getArgument(argc, argv, "--weights");

  if (WeightsPath.empty())
  {
    std::cout << "Please define a path to the model weights." << std::endl;
    std::cout << "Example: " << std::endl << std::endl;
    std::cout << argv[0] << " --data pre_trained/TORCS_Training_1F --model pre_trained/modelfile.prototxt --weights pre_trained/weightsfile.binaryproto --mean pre_trained/meanfile.binaryproto" << std::endl << std::endl;
    return -1;
  }

  string const MeanPath = getArgument(argc, argv, "--mean");

  if (MeanPath.empty())
  {
    std::cout << "Please define a path to the mean file." << std::endl;
    std::cout << "Example: " << std::endl << std::endl;
    std::cout << argv[0] << " --data pre_trained/TORCS_Training_1F --model pre_trained/modelfile.prototxt --weights pre_trained/weightsfile.binaryproto --mean pre_trained/meanfile.binaryproto" << std::endl << std::endl;
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

  return verify(DataPath, ModelPath, WeightsPath, MeanPath, GPUDevice);
}

int verify(string DataPath, string ModelPath, string WeightsPath, string MeanPath, int GPUDevice)
{
  int const BatchSize = 64;
  int const NumberUntilOutput = 1000;

  CLabel            Label[BatchSize];
  Indicators_t      EstimatedIndicators[BatchSize];
  CNeuralNet        NeuralNet(ModelPath, WeightsPath, MeanPath, GPUDevice);

  CErrorMeasurement ErrorMeas;

  db::LevelDB Database;
  Database.Open(DataPath, db::READ);

  db::LevelDBCursor * pCursor = Database.NewCursor();
  CHECK(pCursor)          << "Cannot generate cursor from database \'" << DataPath << "\'.";
  CHECK(pCursor->valid()) << "Database \'" << DataPath << "\' is empty.";

  int FrameNumber = 1;
  int OutputCounter = NumberUntilOutput;
  bool IsEnd = false;
  while(!IsEnd)
  {
    OutputCounter -= BatchSize;

    if (OutputCounter <= 0)
    {
      std::cout << "Process Frame: " << FrameNumber << " to " << (FrameNumber+BatchSize) << std::endl;
      std::cout.flush();
    }

    IsEnd = NeuralNet.processBatch(&EstimatedIndicators[0], &Label[0], pCursor, BatchSize);

    ErrorMeas.measureBatch(&EstimatedIndicators[0], &Label[0], BatchSize);

    FrameNumber += BatchSize;

    if (OutputCounter <= 0)
    {
      ErrorMeas.print(std::cout);
      OutputCounter = NumberUntilOutput;
    }
  }

  ErrorMeas.print(std::cout);
  NeuralNet.printTimeSummery(std::cout);

  return 0;
}

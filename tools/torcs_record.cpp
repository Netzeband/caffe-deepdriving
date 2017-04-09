/*
 * torcs_record.cpp
 *
 *  Created on: April 09, 2017
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
//  Records frames from TROCS as training- or test-data
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

int run(string DataPath, int MaxFrames);

int main(int argc, char** argv)
{
  ::google::InitGoogleLogging(argv[0]);

  string const DatabasePath   = getArgument(argc, argv, "--data");

  if (DatabasePath.empty())
  {
    std::cout << "Please define a path to the leveldb database where to store the frames." << std::endl;
    std::cout << "Example: " << std::endl << std::endl;
    std::cout << argv[0] << " --data pre_trained/TORCS_Training_1F" << std::endl << std::endl;
    return -1;
  }

  int Frames = -1;
  string const FramesString = getArgument(argc, argv, "--frames");

  if (!FramesString.empty())
  {
    Frames = atoi(FramesString.c_str());

    if (Frames <= 0)
    {
      Frames = -1;
    }
  }

  return run(DatabasePath, Frames);
}

bool processKeys(TorcsData_t &rData);

int getLastKey(db::LevelDB &rDatabase)
{
  db::LevelDBCursor * pCursor = rDatabase.NewCursor();
  CHECK(pCursor) << "Cannot generate cursor from database.";

  pCursor->SeekToLast();

  if (pCursor->valid())
  {
    int Key = std::atoi(pCursor->key().c_str());

    return Key;
  }

  return 0;
}

void writeToDatabase(int const Frame, db::Transaction *pTransaction, CImage const &rImage, Indicators_t const &rGroundTruth)
{
  string ValueString;
  Datum Data;

  rImage.writeToDatum(Data);
  CLabel::writeToDatum(Data, rGroundTruth);

  static int const MaxKeyLength = 256;
  char KeyString[MaxKeyLength];
  snprintf(KeyString, MaxKeyLength, "%08d", Frame);
  Data.SerializeToString(&ValueString);

  pTransaction->Put(KeyString, ValueString);
}

int run(string DataPath, int const MaxFrames)
{
  CSharedMemory     TorcsMemory;
  CSemantic         Semantic;

  TorcsMemory.TorcsData.IsAIControlled = false;
  TorcsMemory.TorcsData.IsControlling = false;
  Semantic.setFrameImage(&TorcsMemory.Image);
  Semantic.setAdditionalData(&TorcsMemory.TorcsData);
  Semantic.show(0, 0, false);

  Indicators_t * pGroundTruth = &TorcsMemory.Indicators;

  db::LevelDB Database;
  Database.Open(DataPath, db::WRITE);

  int FrameOffset = getLastKey(Database);
  int FrameNumber = 0;
  int FrameLimit = MaxFrames;

  if (FrameOffset == 0)
  {
    std::cout << "Create a new database." << std::endl;
  }
  else
  {
    std::cout << "Add all frames to the end of database with offset " << FrameOffset << std::endl;
  }

  if (FrameLimit > 0)
  {
    std::cout << "Collect exactly " << MaxFrames << " Frames." << std::endl;
  }

  db::Transaction * pTransaction = Database.NewTransaction();

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
      Semantic.show(pGroundTruth, 0, true);

      if (TorcsMemory.TorcsData.IsRecording)
      {
        FrameNumber++;
        writeToDatabase(FrameNumber+FrameOffset, pTransaction, TorcsMemory.Image, TorcsMemory.Indicators);
      }

      std::cout << std::endl << "Current Frame: " << FrameNumber << std::endl;
      std::cout << std::endl << "Ground-Truth: " << std::endl;
      std::cout << "============= " << std::endl;
      TorcsMemory.Indicators.print(std::cout);
    }
    else
    {
      Semantic.show(pGroundTruth, 0, false);
    }

    if (FrameLimit > 0 && FrameNumber >= FrameLimit)
    {
      TorcsMemory.TorcsData.IsRecording = false;
      std::cout << "Reached maximum number of frames. Press 'r' to record another " << MaxFrames << " frames.";
      FrameLimit = MaxFrames + FrameNumber;
    }

    IsEnd = processKeys(TorcsMemory.TorcsData);
    TorcsMemory.write();
  }

  if (FrameNumber > 0)
  {
    pTransaction->Commit();
  }

  std::cout << std::endl << "Wrote " << FrameNumber << " Frames to database." << std::endl;
  std::cout << "Database contains " << (FrameNumber+FrameOffset) << " Frames in sum." << std::endl;

  return 0;
}

bool processKeys(TorcsData_t &rData)
{
  static const char PauseKey     = 'p';
  static const char RecordKey    = 'r';
  static const char EscKey       = 27;
  static const int KeyTime = 1;

  char Key = cvWaitKey(KeyTime);

  // Escape Key
  if (Key == EscKey)
  {
    return true;
  }

  if (Key == PauseKey)
  {
    rData.IsNotPause = !rData.IsNotPause;
  }

  if (Key == RecordKey)
  {
    rData.IsRecording = !rData.IsRecording;
  }

  return false;
}



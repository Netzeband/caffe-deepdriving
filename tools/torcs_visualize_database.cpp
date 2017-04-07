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
//  Read the pre-collected leveldb database, display the image and
//  print the corresponding affordance indicators
//
//  Input keys
//  Esc: exit
//
////////////////////////////////////////////////

#include <glog/logging.h>

#include "caffe/caffe.hpp"
#include "caffe/util/db_leveldb.hpp"

#include "torcs/Arguments.hpp"
#include "torcs/Database.hpp"
#include "torcs/Semantic.hpp"

#include <opencv2/highgui/highgui.hpp>

#define ImageWidth  280
#define ImageHeight 210

using namespace caffe;
using std::string;

bool processKeys();

int main(int argc, char** argv)
{
  ::google::InitGoogleLogging(argv[0]);

  string const DatabasePath = getArgument(argc, argv, "--data-path");

  if (DatabasePath.empty())
  {
    std::cout << "Please define a path to the database." << std::endl;
    std::cout << "Example: " << std::endl << std::endl;
    std::cout << argv[0] << " --data-path pre_trained/TORCS_Training_1F" << std::endl << std::endl;
    return -1;
  }

  db::LevelDB Database;
  Database.Open(DatabasePath, db::READ);

  db::LevelDBCursor * pCursor = Database.NewCursor();
  CHECK(pCursor)          << "Cannot generate cursor from database \'" << DatabasePath << "\'.";
  CHECK(pCursor->valid()) << "Database \'" << DatabasePath << "\' is empty.";

  CLabel Label;
  CImage Image;
  CSemantic Semantic;

  bool IsEnd = false;
  int FrameNumber = 0;

  std::cout << std::endl;
  std::cout << std::endl;
  std::cout << "Start with visualization in PAUSE mode. Possible Keys:" << std::endl;
  std::cout << " p = Pause on/off, n = next frame, + = faster, - = slower, ESC = stop" << std::endl;
  std::cout << std::endl;
  std::cout << std::endl;

  Semantic.setFrameImage(&Image);

  while(pCursor->valid() && !IsEnd)
  {
    FrameNumber++;
    std::cout << "Frame: " << FrameNumber << std::endl;
    std::cout.flush();

    Datum Data;
    Data.ParseFromString(pCursor->value());

    Label.readFromDatum(Data);
    Image.readFromDatum(Data);

    Label.Indicators.print(std::cout);
    Semantic.show(&Label.Indicators, 0, true);

    pCursor->Next();

    IsEnd = processKeys();
  }

  if (!IsEnd)
  {
    std::cout << "Database contains " << FrameNumber << " frames." << std::endl;
  }
  else
  {
    std::cout << "Abort database read after frame " << FrameNumber << "." << std::endl;
  }

  return 0;
}

bool processKeys()
{
  static bool IsPause = true;
  static const char PauseKey = 'p';
  static const char NextKey  = 'n';
  static const char EscKey   = 27;
  static const char PlusKey = '+';
  static const char MinusKey = '-';
  static const int WaitStep = 100;
  static int WaitTime = 500;
  static int KeyTime = 20;


  bool IsNext = false;
  int Time = WaitTime;

  while(!IsNext)
  {
    char Key = cvWaitKey(KeyTime);

    if (IsPause)
    {
      IsNext = false;
    }
    else
    {
      Time -= KeyTime;
      IsNext = Time <= 0;
    }

    // Escape Key
    if (Key == EscKey)
    {
      return true;
    }

    if (Key == PauseKey)
    {
      IsPause = !IsPause;
    }

    if (Key == PlusKey)
    {
      Time -= WaitTime;
      WaitTime = std::max(0, WaitTime - WaitStep);
      Time += WaitTime;
    }

    if (Key == MinusKey)
    {
      Time -= WaitTime;
      WaitTime = std::max(0, WaitTime + WaitStep);
      Time += WaitTime;
    }

    if (Key == NextKey)
    {
      IsNext = true;
    }
  }

  return false;
}



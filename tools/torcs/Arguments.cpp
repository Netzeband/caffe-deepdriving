/**
 * Arguments.cpp
 *
 *  Created on: Mar 25, 2017
 *      Author: Andre Netzeband
 */

#include "Arguments.hpp"

using namespace std;

string getArgument(int NumberOfArguments,  char ** ppArguments, string ArgumentName)
{
  for (int i = 1; i < NumberOfArguments; i++)
  {
    if (ppArguments[i])
    {
      string Argument = ppArguments[i];

      if (Argument == ArgumentName)
      {
        if (i + 1 < NumberOfArguments)
        {
          if (ppArguments[i+1])
          {
            return ppArguments[i+1];
          }
        }
      }
    }
    else
    {
      break;
    }
  }

  return "";
}






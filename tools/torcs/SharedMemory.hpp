/**
 * SharedMemory.hpp
 *
 *  Created on: Mar 26, 2017
 *      Author: Andre Netzeband
 */

#ifndef SHAREDMEMORY_HPP_
#define SHAREDMEMORY_HPP_

#include "Indicators.hpp"
#include "Image.hpp"

typedef struct
{
  bool   IsControlling;
  bool   IsNotPause;
  double Speed;
  double Steering;
  double Accelerating;
  double Breaking;
  bool   IsAIControlled;
  bool   ShowGroundTruth;
} TorcsData_t;

class CSharedMemory
{
  public:
    /// @brief Constructor.
    CSharedMemory();

    /// @brief Destructor.
    ~CSharedMemory();

    /// @brief Reads the shared memory if possible.
    void read();

    /// @return Returns true, if new data was read.
    bool isDataUpdated();

    /// @brief Writes the shared memory.
    void write();

    /// @brief The indicators from the shared memory.
    Indicators_t Indicators;

    /// @brief The other data which comes from torcs.
    TorcsData_t TorcsData;

    /// @brief The image comming from torcs.
    CImage Image;

  private:
    void * pMemory;
    int    SharedMemoryID;
    bool   IsDataUpdated;

    void attach();
    void detach();
    void initMemory();
};

#endif /* SHAREDMEMORY_HPP_ */

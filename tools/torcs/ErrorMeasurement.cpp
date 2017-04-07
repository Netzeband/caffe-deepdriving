/*
 * ErrorMeasurement.cpp
 *
 *  Created on: Mar 30, 2017
 *      Author: netzeband
 */

#include "ErrorMeasurement.hpp"

#include <algorithm>
#include <cmath>

void ErrorSummary_t::reset()
{
  DistLL = 0;
  DistMM = 0;
  DistRR = 0;
  LL     = 0;
  ML     = 0;
  MR     = 0;
  RR     = 0;
  DistL  = 0;
  DistR  = 0;
  L      = 0;
  M      = 0;
  R      = 0;
  Angle  = 0;
  Fast   = 0;
}

float ErrorSummary_t::calcLoss() const
{
  float Loss = 0;

  Loss += DistLL;
  Loss += DistMM;
  Loss += DistRR;
  Loss += LL;
  Loss += ML;
  Loss += MR;
  Loss += RR;
  Loss += DistL;
  Loss += DistR;
  Loss += L;
  Loss += M;
  Loss += R;
  Loss += Angle;
  Loss += Fast;

  return Loss * 0.5;
}

void ErrorSummary_t::print(std::ostream &rStream) const
{
  std::cout << "DistLL Error : " << DistLL << std::endl;
  std::cout << "DistMM Error : " << DistMM << std::endl;
  std::cout << "DistRR Error : " << DistRR << std::endl;
  std::cout << "LL Error     : " << LL << std::endl;
  std::cout << "ML Error     : " << ML << std::endl;
  std::cout << "MR Error     : " << MR << std::endl;
  std::cout << "RR Error     : " << RR << std::endl;
  std::cout << "DistL Error  : " << DistL << std::endl;
  std::cout << "DistR Error  : " << DistR << std::endl;
  std::cout << "L Error      : " << L << std::endl;
  std::cout << "M Error      : " << M << std::endl;
  std::cout << "R Error      : " << R << std::endl;
  std::cout << "Angle Error  : " << Angle << std::endl;
  std::cout << "Fast Error   : " << Fast << std::endl;
  std::cout << "Loss         : " << calcLoss() << std::endl;
}

CErrorMeasurement::CErrorMeasurement():
  NumberOfMeasurements(0)
{
  CurrentSquaredError.reset();
  MeanSquaredError.reset();
  CurrentAbsoluteError.reset();
  MeanAbsoluteError.reset();
  StandardDeviation.reset();
}

CErrorMeasurement::~CErrorMeasurement()
{

}

void CErrorMeasurement::measureBatch(Indicators_t * pEstimatedArray, CLabel * pLabelArray, int BatchSize)
{
  for (int i = 0; i < BatchSize; i++)
  {
    measure(&pLabelArray[i].Indicators, &pEstimatedArray[i]);
  }
}

static float getAbsoluteError(float Y, float Z)
{
  float const Diff = Y - Z;
  return std::abs(Diff);
}

void CErrorMeasurement::measure(Indicators_t * pGroundTruth, Indicators_t * pEstimated)
{
  CurrentAbsoluteError.DistLL = getAbsoluteError(
      pGroundTruth->getDistLInLane(),
      pEstimated->getDistLInLane()) / 95;

  CurrentAbsoluteError.DistMM = getAbsoluteError(
      pGroundTruth->getDistMInLane(),
      pEstimated->getDistMInLane()) / 95;

  CurrentAbsoluteError.DistRR = getAbsoluteError(
      pGroundTruth->getDistRInLane(),
      pEstimated->getDistRInLane()) / 95;

  CurrentAbsoluteError.LL = getAbsoluteError(
      pGroundTruth->getLL(),
      pEstimated->getLL()) * 0.14545;

  CurrentAbsoluteError.ML = getAbsoluteError(
      pGroundTruth->getML(),
      pEstimated->getML()) * 0.16;

  CurrentAbsoluteError.MR = getAbsoluteError(
      pGroundTruth->getMR(),
      pEstimated->getMR()) * 0.16;

  CurrentAbsoluteError.RR = getAbsoluteError(
      pGroundTruth->getRR(),
      pEstimated->getRR()) * 0.14545;

  CurrentAbsoluteError.DistL = getAbsoluteError(
      pGroundTruth->getDistLOnMarking(),
      pEstimated->getDistLOnMarking()) / 95;

  CurrentAbsoluteError.DistR = getAbsoluteError(
      pGroundTruth->getDistROnMarking(),
      pEstimated->getDistROnMarking()) / 95;

  CurrentAbsoluteError.L = getAbsoluteError(
      pGroundTruth->getL(),
      pEstimated->getL()) * 0.17778;

  CurrentAbsoluteError.M = getAbsoluteError(
      pGroundTruth->getM(),
      pEstimated->getM()) * 0.14545;

  CurrentAbsoluteError.R = getAbsoluteError(
      pGroundTruth->getR(),
      pEstimated->getR()) * 0.17778;

  CurrentAbsoluteError.Angle = std::min(1.0, std::max(0.0, getAbsoluteError(
      pGroundTruth->Angle,
      pEstimated->Angle) * 1.1));

  CurrentAbsoluteError.Fast = getAbsoluteError(
      pGroundTruth->Fast,
      pEstimated->Fast) * 0.6;


  CurrentSquaredError.DistLL = CurrentAbsoluteError.DistLL * CurrentAbsoluteError.DistLL;
  CurrentSquaredError.DistMM = CurrentAbsoluteError.DistMM * CurrentAbsoluteError.DistMM;
  CurrentSquaredError.DistRR = CurrentAbsoluteError.DistRR * CurrentAbsoluteError.DistRR;
  CurrentSquaredError.LL     = CurrentAbsoluteError.LL * CurrentAbsoluteError.LL;
  CurrentSquaredError.ML     = CurrentAbsoluteError.ML * CurrentAbsoluteError.ML;
  CurrentSquaredError.MR     = CurrentAbsoluteError.MR * CurrentAbsoluteError.MR;
  CurrentSquaredError.RR     = CurrentAbsoluteError.RR * CurrentAbsoluteError.RR;
  CurrentSquaredError.DistL  = CurrentAbsoluteError.DistL * CurrentAbsoluteError.DistL;
  CurrentSquaredError.DistR  = CurrentAbsoluteError.DistR * CurrentAbsoluteError.DistR;
  CurrentSquaredError.L      = CurrentAbsoluteError.L * CurrentAbsoluteError.L;
  CurrentSquaredError.M      = CurrentAbsoluteError.M * CurrentAbsoluteError.M;
  CurrentSquaredError.R      = CurrentAbsoluteError.R * CurrentAbsoluteError.R;
  CurrentSquaredError.Angle  = CurrentAbsoluteError.Angle * CurrentAbsoluteError.Angle;
  CurrentSquaredError.Fast   = CurrentAbsoluteError.Fast * CurrentAbsoluteError.Fast;

  calculateMeanValues();
}

void CErrorMeasurement::calculateMeanValues()
{
  MeanSquaredError.DistLL  = (MeanSquaredError.DistLL * NumberOfMeasurements + CurrentSquaredError.DistLL)/(NumberOfMeasurements+1);
  MeanSquaredError.DistMM  = (MeanSquaredError.DistMM * NumberOfMeasurements + CurrentSquaredError.DistMM)/(NumberOfMeasurements+1);
  MeanSquaredError.DistRR  = (MeanSquaredError.DistRR * NumberOfMeasurements + CurrentSquaredError.DistRR)/(NumberOfMeasurements+1);
  MeanSquaredError.LL      = (MeanSquaredError.LL     * NumberOfMeasurements + CurrentSquaredError.LL    )/(NumberOfMeasurements+1);
  MeanSquaredError.ML      = (MeanSquaredError.ML     * NumberOfMeasurements + CurrentSquaredError.ML    )/(NumberOfMeasurements+1);
  MeanSquaredError.MR      = (MeanSquaredError.MR     * NumberOfMeasurements + CurrentSquaredError.MR    )/(NumberOfMeasurements+1);
  MeanSquaredError.RR      = (MeanSquaredError.RR     * NumberOfMeasurements + CurrentSquaredError.RR    )/(NumberOfMeasurements+1);
  MeanSquaredError.DistL   = (MeanSquaredError.DistL  * NumberOfMeasurements + CurrentSquaredError.DistL )/(NumberOfMeasurements+1);
  MeanSquaredError.DistR   = (MeanSquaredError.DistL  * NumberOfMeasurements + CurrentSquaredError.DistL )/(NumberOfMeasurements+1);
  MeanSquaredError.L       = (MeanSquaredError.L      * NumberOfMeasurements + CurrentSquaredError.L     )/(NumberOfMeasurements+1);
  MeanSquaredError.M       = (MeanSquaredError.M      * NumberOfMeasurements + CurrentSquaredError.M     )/(NumberOfMeasurements+1);
  MeanSquaredError.R       = (MeanSquaredError.R      * NumberOfMeasurements + CurrentSquaredError.R     )/(NumberOfMeasurements+1);
  MeanSquaredError.Angle   = (MeanSquaredError.Angle  * NumberOfMeasurements + CurrentSquaredError.Angle )/(NumberOfMeasurements+1);
  MeanSquaredError.Fast    = (MeanSquaredError.Fast   * NumberOfMeasurements + CurrentSquaredError.Fast  )/(NumberOfMeasurements+1);

  MeanAbsoluteError.DistLL = (MeanAbsoluteError.DistLL * NumberOfMeasurements + CurrentAbsoluteError.DistLL)/(NumberOfMeasurements+1);
  MeanAbsoluteError.DistMM = (MeanAbsoluteError.DistMM * NumberOfMeasurements + CurrentAbsoluteError.DistMM)/(NumberOfMeasurements+1);
  MeanAbsoluteError.DistRR = (MeanAbsoluteError.DistRR * NumberOfMeasurements + CurrentAbsoluteError.DistRR)/(NumberOfMeasurements+1);
  MeanAbsoluteError.LL     = (MeanAbsoluteError.LL     * NumberOfMeasurements + CurrentAbsoluteError.LL    )/(NumberOfMeasurements+1);
  MeanAbsoluteError.ML     = (MeanAbsoluteError.ML     * NumberOfMeasurements + CurrentAbsoluteError.ML    )/(NumberOfMeasurements+1);
  MeanAbsoluteError.MR     = (MeanAbsoluteError.MR     * NumberOfMeasurements + CurrentAbsoluteError.MR    )/(NumberOfMeasurements+1);
  MeanAbsoluteError.RR     = (MeanAbsoluteError.RR     * NumberOfMeasurements + CurrentAbsoluteError.RR    )/(NumberOfMeasurements+1);
  MeanAbsoluteError.DistL  = (MeanAbsoluteError.DistL  * NumberOfMeasurements + CurrentAbsoluteError.DistL )/(NumberOfMeasurements+1);
  MeanAbsoluteError.DistR  = (MeanAbsoluteError.DistL  * NumberOfMeasurements + CurrentAbsoluteError.DistL )/(NumberOfMeasurements+1);
  MeanAbsoluteError.L      = (MeanAbsoluteError.L      * NumberOfMeasurements + CurrentAbsoluteError.L     )/(NumberOfMeasurements+1);
  MeanAbsoluteError.M      = (MeanAbsoluteError.M      * NumberOfMeasurements + CurrentAbsoluteError.M     )/(NumberOfMeasurements+1);
  MeanAbsoluteError.R      = (MeanAbsoluteError.R      * NumberOfMeasurements + CurrentAbsoluteError.R     )/(NumberOfMeasurements+1);
  MeanAbsoluteError.Angle  = (MeanAbsoluteError.Angle  * NumberOfMeasurements + CurrentAbsoluteError.Angle )/(NumberOfMeasurements+1);
  MeanAbsoluteError.Fast   = (MeanAbsoluteError.Fast   * NumberOfMeasurements + CurrentAbsoluteError.Fast  )/(NumberOfMeasurements+1);

  NumberOfMeasurements++;
}

ErrorSummary_t const & CErrorMeasurement::getStandardDeviation()
{
  calculateStandardDeviation();

  return StandardDeviation;
}

void CErrorMeasurement::calculateStandardDeviation()
{
  StandardDeviation.DistLL = std::sqrt(MeanSquaredError.DistLL - MeanAbsoluteError.DistLL * MeanAbsoluteError.DistLL);
  StandardDeviation.DistMM = std::sqrt(MeanSquaredError.DistMM - MeanAbsoluteError.DistMM * MeanAbsoluteError.DistMM);
  StandardDeviation.DistRR = std::sqrt(MeanSquaredError.DistRR - MeanAbsoluteError.DistRR * MeanAbsoluteError.DistRR);
  StandardDeviation.LL     = std::sqrt(MeanSquaredError.LL     - MeanAbsoluteError.LL     * MeanAbsoluteError.LL);
  StandardDeviation.ML     = std::sqrt(MeanSquaredError.ML     - MeanAbsoluteError.ML     * MeanAbsoluteError.ML);
  StandardDeviation.MR     = std::sqrt(MeanSquaredError.MR     - MeanAbsoluteError.MR     * MeanAbsoluteError.MR);
  StandardDeviation.RR     = std::sqrt(MeanSquaredError.RR     - MeanAbsoluteError.RR     * MeanAbsoluteError.RR);
  StandardDeviation.DistL  = std::sqrt(MeanSquaredError.DistL  - MeanAbsoluteError.DistL  * MeanAbsoluteError.DistL);
  StandardDeviation.DistR  = std::sqrt(MeanSquaredError.DistR  - MeanAbsoluteError.DistR  * MeanAbsoluteError.DistR);
  StandardDeviation.L      = std::sqrt(MeanSquaredError.L      - MeanAbsoluteError.L      * MeanAbsoluteError.L);
  StandardDeviation.M      = std::sqrt(MeanSquaredError.M      - MeanAbsoluteError.M      * MeanAbsoluteError.M);
  StandardDeviation.R      = std::sqrt(MeanSquaredError.R      - MeanAbsoluteError.R      * MeanAbsoluteError.R);
  StandardDeviation.Angle  = std::sqrt(MeanSquaredError.Angle  - MeanAbsoluteError.Angle  * MeanAbsoluteError.Angle);
  StandardDeviation.Fast   = std::sqrt(MeanSquaredError.Fast   - MeanAbsoluteError.Fast   * MeanAbsoluteError.Fast);
}

void CErrorMeasurement::print(std::ostream &rStream)
{
  calculateStandardDeviation();

  std::cout << "Error Measurements:" << std::endl;
  std::cout << " Type   \t MAE \t\t SD \t\t MSE " << std::endl;
  std::cout << " DistLL \t " << MeanAbsoluteError.DistLL     << " \t " << StandardDeviation.DistLL     << " \t " << MeanSquaredError.DistLL     << std::endl;
  std::cout << " DistMM \t " << MeanAbsoluteError.DistMM     << " \t " << StandardDeviation.DistMM     << " \t " << MeanSquaredError.DistMM     << std::endl;
  std::cout << " DistRR \t " << MeanAbsoluteError.DistRR     << " \t " << StandardDeviation.DistRR     << " \t " << MeanSquaredError.DistRR     << std::endl;
  std::cout << " LL     \t " << MeanAbsoluteError.LL         << " \t " << StandardDeviation.LL         << " \t " << MeanSquaredError.LL         << std::endl;
  std::cout << " ML     \t " << MeanAbsoluteError.ML         << " \t " << StandardDeviation.ML         << " \t " << MeanSquaredError.ML         << std::endl;
  std::cout << " MR     \t " << MeanAbsoluteError.MR         << " \t " << StandardDeviation.MR         << " \t " << MeanSquaredError.MR         << std::endl;
  std::cout << " RR     \t " << MeanAbsoluteError.RR         << " \t " << StandardDeviation.RR         << " \t " << MeanSquaredError.RR         << std::endl;
  std::cout << " DistL  \t " << MeanAbsoluteError.DistL      << " \t " << StandardDeviation.DistL      << " \t " << MeanSquaredError.DistL      << std::endl;
  std::cout << " DistR  \t " << MeanAbsoluteError.DistR      << " \t " << StandardDeviation.DistR      << " \t " << MeanSquaredError.DistR      << std::endl;
  std::cout << " L      \t " << MeanAbsoluteError.L          << " \t " << StandardDeviation.L          << " \t " << MeanSquaredError.L          << std::endl;
  std::cout << " M      \t " << MeanAbsoluteError.M          << " \t " << StandardDeviation.M          << " \t " << MeanSquaredError.M          << std::endl;
  std::cout << " R      \t " << MeanAbsoluteError.R          << " \t " << StandardDeviation.R          << " \t " << MeanSquaredError.R          << std::endl;
  std::cout << " Angle  \t " << MeanAbsoluteError.Angle      << " \t " << StandardDeviation.Angle      << " \t " << MeanSquaredError.Angle      << std::endl;
  std::cout << " Fast   \t " << MeanAbsoluteError.Fast       << " \t " << StandardDeviation.Fast       << " \t " << MeanSquaredError.Fast       << std::endl;
  std::cout << " Sum    \t " << MeanAbsoluteError.calcLoss() << " \t " << StandardDeviation.calcLoss() << " \t " << MeanSquaredError.calcLoss() << std::endl;
  std::cout << "Number of Error Measurements: " << NumberOfMeasurements << std::endl;
}



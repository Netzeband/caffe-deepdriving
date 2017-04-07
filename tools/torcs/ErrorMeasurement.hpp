/*
 * ErrorMeasurement.hpp
 *
 *  Created on: Mar 30, 2017
 *      Author: netzeband
 */

#ifndef ERRORMEASUREMENT_HPP_
#define ERRORMEASUREMENT_HPP_

#include "Indicators.hpp"
#include "Database.hpp"

#include <iostream>

typedef struct ErrorSummary_t
{
  public:
    float DistLL;
    float DistMM;
    float DistRR;
    float LL;
    float ML;
    float MR;
    float RR;
    float DistL;
    float DistR;
    float L;
    float M;
    float R;
    float Angle;
    float Fast;

    void reset();
    float calcLoss() const;
    void print(std::ostream &rStream) const;

} ErrorSummary_t;

class CErrorMeasurement
{
  public:
    /// @brief Constructor.
    CErrorMeasurement();

    /// @brief Destructor.
    ~CErrorMeasurement();

    /// @brief Measures the error between estimated and ground truth indicators.
    /// @param pGroundTruth Is the ground truth indicator.
    /// @param pEstimated   Is the estimated indicator.
    void measure(Indicators_t * pGroundTruth, Indicators_t * pEstimated);

    void measureBatch(Indicators_t * pEstimatedArray, CLabel * pLabelArray, int BatchSize);

    /// @return Returns the squared error measurement values for the last measurement.
    ErrorSummary_t const & getCurrentSquaredError() const { return CurrentSquaredError; }

    /// @return Returns the mean squared error measurement values for all measurements.
    ErrorSummary_t const & getMeanSequaredError() const { return MeanSquaredError; }

    /// @return Returns the absolute error measurement values for the last measurement.
    ErrorSummary_t const & getCurrentAbsuluteError() const { return CurrentAbsoluteError; }

    /// @return Returns the mean absolute error measurement values for all measurements.
    ErrorSummary_t const & getMeanAbsoluteError() const { return MeanAbsoluteError; }

    /// @return Returns the standard deviation of the error measurement values.
    ErrorSummary_t const & getStandardDeviation();

    /// @brief Prints the error mean statistics.
    void print(std::ostream &rStream);

    /// @brief Returns the number of measurements.
    int getMeasurements() const { return NumberOfMeasurements;}

  private:
    ErrorSummary_t CurrentSquaredError;
    ErrorSummary_t MeanSquaredError;
    ErrorSummary_t CurrentAbsoluteError;
    ErrorSummary_t MeanAbsoluteError;
    ErrorSummary_t StandardDeviation;
    unsigned long  NumberOfMeasurements;

    void calculateMeanValues();
    void calculateStandardDeviation();
};



#endif /* ERRORMEASUREMENT_HPP_ */

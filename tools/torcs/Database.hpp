/**
 * Database.hpp
 *
 *  Created on: Mar 25, 2017
 *      Author: Andre Netzeband
 */

#ifndef DATABASE_HPP_
#define DATABASE_HPP_

#include "Indicators.hpp"

#include "caffe/caffe.hpp"

#include <opencv2/core/core.hpp>

class CLabel
{
  public:
    /// @brief The affordance indicators from the labels.
    Indicators_t Indicators;

    /// @brief Constructor.
    CLabel();

    /// @brief Reads the labels from a datum.
    void readFromDatum(caffe::Datum const &rData);

    /// @brief Writes the labels to a datum.
    static void writeToDatum(caffe::Datum &rData, Indicators_t const &rIndicators);
};

#endif /* DATABASE_HPP_ */

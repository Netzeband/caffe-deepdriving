/*
 * NeuralNet.hpp
 *
 *  Created on: Mar 28, 2017
 *      Author: netzeband
 */

#ifndef NEURALNET_HPP_
#define NEURALNET_HPP_

#include <boost/filesystem/path.hpp>

#include <string>
#include <iostream>

#include <caffe/caffe.hpp>
#include "caffe/util/db_leveldb.hpp"

#include "Image.hpp"
#include "Indicators.hpp"
#include "Database.hpp"

class CNeuralNet
{
  public:
    /// @brief Constructor.
    CNeuralNet(boost::filesystem::path &rModelPath, boost::filesystem::path &rWeightsPath, boost::filesystem::path &rMeanPath, int GPUDevice);

    /// @brief Constructor.
    CNeuralNet(std::string &rModelPath, std::string &rWeightsPath, std::string &rMeanPath, int GPUDevice);

    /// @brief Destructor.
    ~CNeuralNet();

    /// @brief Processes an image and delivers all output indicators.
    void process(Indicators_t * pOutput, CImage &rInputImage);

    /// @brief Processes a whole batch.
    /// @return Returns true, if the last batch was processed.
    bool processBatch(Indicators_t * pResultArray, CLabel * pLabelArray, caffe::db::LevelDBCursor * pCursor, int BatchSize);

    float getMaxProcessTime() const;
    float getMeanProcessTime() const;
    float getMaxForwardTime() const;
    float getMeanForwardTime() const;
    void printTimeSummery(std::ostream &rStream) const;

  private:
    caffe::Net<float> * pNetwork;
    cv::Mat             MeanImage;
    float ProcessTime;
    float MaxProcessTime;
    float ForwardTime;
    float MaxForwardTime;
    long  NumberOfInferences;

    void initNetwork(boost::filesystem::path &rModelPath, boost::filesystem::path &rWeightsPath, boost::filesystem::path &rMeanPath, int GPUDevice);

    void setMean(boost::filesystem::path &rMeanPath);

    void copyImageToInput(IplImage * pImage, int BatchElement);

    void copyOutputToIndicators(Indicators_t * pOutput, int BatchElement);

    void resizeInput(IplImage * pExampleImage, int BatchSize);
};



#endif /* NEURALNET_HPP_ */

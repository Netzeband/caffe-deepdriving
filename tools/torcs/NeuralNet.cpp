/*
 * NeuralNet.cpp
 *
 *  Created on: Mar 28, 2017
 *      Author: netzeband
 */

#include "NeuralNet.hpp"

#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>

using namespace caffe;

CNeuralNet::CNeuralNet(std::string &rModelPath, std::string &rWeightsPath, std::string &rMeanPath, int GPUDevice)
{
  boost::filesystem::path ModelPath(rModelPath);
  boost::filesystem::path WeightsPath(rWeightsPath);
  boost::filesystem::path MeanPath(rMeanPath);

  initNetwork(ModelPath, WeightsPath, MeanPath, GPUDevice);

  ProcessTime = 0;
  MaxProcessTime = 0;
  ForwardTime = 0;
  MaxForwardTime = 0;
  NumberOfInferences = 0;
}

CNeuralNet::CNeuralNet(boost::filesystem::path &rModelPath, boost::filesystem::path &rWeightsPath, boost::filesystem::path &rMeanPath, int GPUDevice)
{
  initNetwork(rModelPath, rWeightsPath, rMeanPath, GPUDevice);
}

CNeuralNet::~CNeuralNet()
{
  if (pNetwork)
  {
    delete(pNetwork);
  }
}

void CNeuralNet::initNetwork(boost::filesystem::path &rModelPath, boost::filesystem::path &rWeightsPath, boost::filesystem::path &rMeanPath, int GPUDevice)
{
  if (GPUDevice >= 0)
  {
    Caffe::set_mode(Caffe::GPU);
    Caffe::SetDevice(GPUDevice);
  }
  else
  {
    Caffe::set_mode(Caffe::CPU);
  }

  pNetwork = new Net<float>(rModelPath.string(), TEST);

  CHECK(pNetwork) << "Could not create a network object!";

  pNetwork->CopyTrainedLayersFrom(rWeightsPath.string());

  setMean(rMeanPath);
}

void CNeuralNet::setMean(boost::filesystem::path &rMeanPath)
{
  BlobProto MeanBinaryBlob;
  ReadProtoFromBinaryFileOrDie(rMeanPath.c_str(), &MeanBinaryBlob);

  // Convert from BlobProto to Blob<float>
  Blob<float> MeanBlob;
  MeanBlob.FromProto(MeanBinaryBlob);
  CHECK_EQ(MeanBlob.channels(), 3) << "Number of channels of mean file doesn't match input layer.";

  // The format of the mean file is planar 32-bit float BGR or grayscale.
  std::vector<cv::Mat> MeanChannels;
  float * pData = MeanBlob.mutable_cpu_data();

  for (int i = 0; i < 3; ++i)
  {
    // Extract an individual channel.
    cv::Mat Channel(MeanBlob.height(), MeanBlob.width(), CV_32FC1, &pData[MeanBlob.height() * MeanBlob.width() * i]);
    MeanChannels.push_back(Channel);
  }

  cv::merge(MeanChannels, MeanImage);
}

bool CNeuralNet::processBatch(Indicators_t * pResultArray, CLabel * pLabelArray, caffe::db::LevelDBCursor * pCursor, int BatchSize)
{
  Timer ProcessTimer;
  Timer ForwardTimer;

  ProcessTimer.Start();

  bool WasLastBatch = false;
  CHECK(BatchSize > 0) << "Invalid Batch Size";
  CHECK(pCursor->valid()) << "Invalid Cursor";

  CImage Image[BatchSize];

  for (int i = 0; i < BatchSize; i++)
  {
    Datum  Data;

    Data.ParseFromString(pCursor->value());
    pLabelArray[i].readFromDatum(Data);

    Image[i].readFromDatum(Data);

    if (i == 0)
    {
      resizeInput(Image[i].getImage(), BatchSize);
    }

    copyImageToInput(Image[i].getImage(), i);

    pCursor->Next();
    if (!pCursor->valid())
    {
      pCursor->SeekToFirst();
      WasLastBatch = true;
    }
  }

  ForwardTimer.Start();

  pNetwork->Forward();

  float Time = (ForwardTimer.MicroSeconds() / 1000000)/BatchSize;
  ForwardTime += Time;
  MaxForwardTime = std::max(MaxForwardTime, Time);

  for (int i = 0; i < BatchSize; i++)
  {
    copyOutputToIndicators(&pResultArray[i], i);
  }

  Time = (ProcessTimer.MicroSeconds() / 1000000)/BatchSize;
  ProcessTime += Time;
  MaxProcessTime = std::max(MaxProcessTime, Time);

  NumberOfInferences += BatchSize;

  return WasLastBatch;
}

void CNeuralNet::process(Indicators_t * pOutput, CImage &rInputImage)
{
  Timer ProcessTimer;
  Timer ForwardTimer;

  ProcessTimer.Start();

  resizeInput(rInputImage.getImage(), 1);
  copyImageToInput(rInputImage.getImage(), 0);

  ForwardTimer.Start();

  pNetwork->Forward();

  float Time = ForwardTimer.MicroSeconds() / 1000000;
  ForwardTime += Time;
  MaxForwardTime = std::max(MaxForwardTime, Time);

  copyOutputToIndicators(pOutput, 0);

  Time = ProcessTimer.MicroSeconds() / 1000000;
  ProcessTime += Time;
  MaxProcessTime = std::max(MaxProcessTime, Time);

  NumberOfInferences++;
}

void CNeuralNet::resizeInput(IplImage * pExampleImage, int BatchSize)
{
  int Height = pExampleImage->height;
  int Width  = pExampleImage->width;

  // reshape input layer if necessary
  Blob<float>* pInputLayer = pNetwork->input_blobs()[0];

  if (pInputLayer->num() != BatchSize || pInputLayer->width() != Width || pInputLayer->height() != Height || pInputLayer->channels() != 3)
  {
    std::cout << "Reshape of input-layer to 3 channels, height " << Height << " and width " << Width << ", witch batch-size " << BatchSize << "." << std::endl;
    pInputLayer->Reshape(BatchSize, 3, Height, Width);
    pNetwork->Reshape();
  }

  CHECK(pInputLayer->num() == BatchSize) << "BatchSize was not changed correctly.";
}

void CNeuralNet::copyImageToInput(IplImage * pImage, int BatchElement)
{
  int Height = pImage->height;
  int Width  = pImage->width;

  Blob<float>* pInputLayer = pNetwork->input_blobs()[0];

  CHECK(pInputLayer->num() > BatchElement) << "BatchElement Index higher than batch-size of input-layer";

  int BatchOffset = BatchElement * Height * Width * pInputLayer->channels();

  // create channels for the input image
  float * pInputData = pInputLayer->mutable_cpu_data();
  std::vector<cv::Mat> InputChannels;
  for (int ChannelNumber = 0; ChannelNumber < pInputLayer->channels(); ChannelNumber++)
  {
    cv::Mat Channel(Height, Width, CV_32FC1, &pInputData[BatchOffset + ChannelNumber * Height * Width]);
    InputChannels.push_back(Channel);
  }

  // convert image to float values
  cv::Mat Image(pImage);
  cv::Mat FloatImage;
  Image.convertTo(FloatImage, CV_32FC3);

  // normalize image
  cv::Mat NormalizedImage;
  cv::subtract(FloatImage, MeanImage, NormalizedImage);

  // copy to input channel
  cv::split(NormalizedImage, InputChannels);

  CHECK(reinterpret_cast<float*>(InputChannels.at(0).data) == &pInputLayer->cpu_data()[BatchOffset]) << "Could not copy the input image to the network!";
}

void CNeuralNet::copyOutputToIndicators(Indicators_t * pOutput, int BatchElement)
{
  Blob<float>* pOutputLayer = pNetwork->output_blobs()[0];
  const float* pResult = pOutputLayer->cpu_data();

  CHECK(pOutputLayer->num() > BatchElement) << "BatchElement Index higher than batch-size of input-layer";

  int const BatchOffset = BatchElement * 14;

  pOutput->Angle                              = (pResult[BatchOffset + 0]-0.5)*1.1;

  pOutput->DistanceToLeftMarking              = (pResult[BatchOffset + 1]-1.34445)*5.6249;
  pOutput->DistanceToCenterMarking            = (pResult[BatchOffset + 2]-0.39091)*6.8752;
  pOutput->DistanceToRightMarking             = (pResult[BatchOffset + 3]+0.34445)*5.6249;

  pOutput->DistanceToLeftObstacle             = (pResult[BatchOffset + 4]-0.12)*95;
  pOutput->DistanceToRightObstacle            = (pResult[BatchOffset + 5]-0.12)*95;

  pOutput->DistanceToLeftMarkingOfLeftLane    = (pResult[BatchOffset + 6]-1.48181)*6.8752;
  pOutput->DistanceToLeftMarkingOfCenterLane  = (pResult[BatchOffset + 7]-0.98)*6.25;
  pOutput->DistanceToRightMarkingOfCenterLane = (pResult[BatchOffset + 8]-0.02)*6.25;
  pOutput->DistanceToRightMarkingOfRightLane  = (pResult[BatchOffset + 9]+0.48181)*6.8752;

  pOutput->DistanceToLeftObstacleInLane       = (pResult[BatchOffset + 10]-0.12)*95;
  pOutput->DistanceToCenterObstacleInLane     = (pResult[BatchOffset + 11]-0.12)*95;
  pOutput->DistanceToRightObstacleInLane      = (pResult[BatchOffset + 12]-0.12)*95;

  if (pResult[BatchOffset + 13]>0.5)
  {
    pOutput->Fast = 1;
  }
  else
  {
    pOutput->Fast = 0;
  }

  /* TODO: Depending on the lane, the car is driving on, the lane marking needs to be corrected if they are invalid.
  if (goto_lane==2 && toMarking_LL<-8) toMarking_LL=-7.5;  // correct error output
  if (goto_lane==1 && toMarking_RR>8) toMarking_RR=7.5;  // correct error output
  */

}

float CNeuralNet::getMaxProcessTime() const
{
  return MaxProcessTime;
}

float CNeuralNet::getMeanProcessTime() const
{
  return ProcessTime/NumberOfInferences;
}

float CNeuralNet::getMaxForwardTime() const
{
  return MaxForwardTime;
}

float CNeuralNet::getMeanForwardTime() const
{
  return ForwardTime/NumberOfInferences;
}

void CNeuralNet::printTimeSummery(std::ostream &rStream) const
{
  rStream << std::endl << "*** Time Summary for the Neural Net ***" << std::endl;
  rStream << "Number of Inferences: " << NumberOfInferences << std::endl;
  rStream << "Mean Process Time   : " << getMeanProcessTime() << "s" << std::endl;
  rStream << "Mean Forward Time   : " << getMeanForwardTime() << "s" << std::endl;
  rStream << "Max Process Time   : " << getMaxProcessTime() << "s" << std::endl;
  rStream << "Max Forward Time   : " << getMaxForwardTime() << "s" << std::endl;
}

/**
 * Semantic.hpp
 *
 *  Created on: Mar 25, 2017
 *      Author: Andre Netzeband
 */

#ifndef SEMANTIC_H_
#define SEMANTIC_H_

#include "Indicators.hpp"
#include "SharedMemory.hpp"
#include "Image.hpp"
#include "ErrorMeasurement.hpp"

#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui_c.h>

class CSemantic
{
  public:
    /// @brief Constructor.
    CSemantic();

    /// @brief Destructor.
    ~CSemantic();

    /// @brief Shows and updates the semantic window.
    /// @param pGroundTruth Is the indicator object that contains the ground truth. Can be 0 to suppress.
    /// @param pGuessed     Is the indicator object that contains the guessed data. Can be 0 to suppress.
    /// @param WasUpdated   Indicated, that the values where updated.
    void show(Indicators const * pGroundTruth, Indicators const * pGuessed, bool WasUpdated, int Lanes = 0);

    /// @brief Adds a frame image to the semantic visualization. If the frame image is 0, this option is disabled.
    /// @param pImage is the pointer to the frame image to add.
    void setFrameImage(CImage * pImage);

    /// @brief Adds additional data to the semantic visualization, like speed and driving states.
    /// @param pAdditionalData is a pointer to the additional data to show.
    void setAdditionalData(TorcsData_t * pAdditionalData);

    /// @brief Adds error measurement data to the semantic visualization.
    /// @param pErrorMeasurementObject Is the object that performs the error measurement.
    void setErrorMeasurement(CErrorMeasurement * pErrorMeasurementObject);

  private:
    IplImage *            pSemanticImage;
    IplImage *            pBackground;
    IplImage *            pLane1;
    IplImage *            pLane2;
    IplImage *            pLane3;
    IplImage *            pErrorMeasurementBackground;
    CImage   *            pFrameImage;
    int                   MarkingHead;
    TorcsData_t *         pAdditionalData;
    CvFont                Font;
    CErrorMeasurement *   pErrorMeasurement;

    bool isBigWindow() const;
    void setupBackground();

    float getSpeed() const;

    int guessLanes(Indicators const * pGroundTruth, Indicators const * pGuessed, int Lanes);
    void copyBackground();
    void addLanes(int Lanes, int LanePosition);
    void addHostCar(Indicators const * pGroundTruth, Indicators const * pGuessed, int Lanes);
    void addLaneMarkings(int Lanes, int LanePosition, bool WasUpdated);
    void addObstacles(Indicators const * pGroundTruth, Indicators const * pGuessed, int Lanes);
    void addObstacles(Indicators const * pIndicator, int LanePosition, bool Filled);
    void drawObstacle(int X, int Y, bool Filled);
    void drawHostCar(Indicators const * pIndicator, float CarPosition, bool IsGroundTruth);
    void drawFrame();
    void drawAdditionalData();
    void drawErrorMeasurement();
    int  getLanePosition(Indicators const * pGroundTruth, Indicators const * pGuessed, int MidOfSemantic);
    int  getLanePosition(Indicators const * pIndicator, int MidOfSemantic);
};


#endif /* SEMANTIC_H_ */

/**
 * Image.hpp
 *
 *  Created on: Mar 26, 2017
 *      Author: Andre Netzeband
 */

#ifndef IMAGE_HPP_
#define IMAGE_HPP_

#include "caffe/caffe.hpp"

#include <opencv2/core/core.hpp>

class CImage
{
  public:
    /// @brief Constructor.
    CImage();

    /// @brief Destructor.
    ~CImage();

    /// @brief Reads the image form a datum.
    void readFromDatum(caffe::Datum const &rData);

    /// @brief Writes the image to a datum.
    void writeToDatum(caffe::Datum &rData) const;

    /// @brief Read the image from memory location.
    void readFromMemory(uint8_t * pMemory, int SourceWidth, int SourceHeight);
    void readFromMemory(uint8_t * pMemory, int SourceWidth, int SourceHeight, int TargetWidth, int TargetHeight);

    /// @brief Sets a random black/white image with the text "No Video".
    void setNoVideo(int TargetWidth, int TargetHeight);

    /// @brief Shows the image in a window.
    void show(std::string &rWindowName);
    void show(char const * const pName);

    /// @brief Returns the image pointer.
    IplImage * getImage();

  private:
    void setWindow(char const * const pName);
    void destroyWindow();

    void setImage(int32_t Height, int32_t Weight, int32_t Channels);
    void destroyImage();

    char const * pCurrentWindowName;
    int32_t      ImageHeight;
    int32_t      ImageWidth;
    int32_t      ImageChannels;
    IplImage   * pImage;
    IplImage   * pOriginalImage;
};

#endif /* IMAGE_HPP_ */

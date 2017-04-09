#!/usr/bin/env sh

if [ -z "$DEEPDRIVING_CAFFE_PATH" ]; then
  echo "Error: Please specify path to DeepDriving caffe installation with: "
  echo "export DEEPDRIVING_CAFFE_PATH=<path-to-installation>"
  echo " "
  exit -1

else

  DATA=$1
  FRAMES=$2

  if [ -z "$DATA" ]; then
    echo "ERROR: Please specify a data path as first argument."
    echo ""
    exit -1
  fi

  echo "Use Database: " $DATA
  echo "Use Weights: " $WEIGTHS

  BIN_PATH=$DEEPDRIVING_CAFFE_PATH/bin
  if [ -z "$FRAMES" ]; then
    GLOG_logtostderr=1 $BIN_PATH/torcs_record --data $DATA
  else
    GLOG_logtostderr=1 $BIN_PATH/torcs_record --data $DATA --frames $FRAMES
  fi
fi



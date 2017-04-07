#!/usr/bin/env sh

if [ -z "$DEEPDRIVING_CAFFE_PATH" ]; then
  echo "Error: Please specify path to DeepDriving caffe installation with: "
  echo "export DEEPDRIVING_CAFFE_PATH=<path-to-installation>"
  echo " "
  exit -1

else

  GPU=$1
  STATE=$2
  SOLVER=$3

  if [ -z "$SOLVER" ]; then
    if [ -z "$DEEPDRIVING_SOLVER_PATH" ]; then
      if [ -e pre_trained/driving_solver_1F.prototxt ]; then
        SOLVER=`pwd`"/pre_trained/driving_solver_1F.prototxt"

      else
        echo "Error: Please specify path to DeepDriving solver model with: "
        echo "export DEEPDRIVING_SOLVER_PATH=<path-to-solver-model>"
        echo " "
        exit -1

      fi

    else
      SOLVER=$DEEPDRIVING_SOLVER_PATH

    fi
  fi

  if [ -z "$GPU" ]; then
    if [ -z "$DEEPDRIVING_GPU" ]; then
	echo "WARNING: Disable GPU by default. Specify GPU ID to enable it:"
        echo "export DEEPDRIVING_GPU=<gpu-id>"
        echo " "
	GPU=-1

    else
      GPU=$DEEPDRIVING_GPU

    fi

  fi

  BIN_PATH=$DEEPDRIVING_CAFFE_PATH/bin

  if [ -z "$STATE" ]; then
    GLOG_logtostderr=1 $BIN_PATH/caffe train --solver=$SOLVER --gpu=$GPU
  else
    GLOG_logtostderr=1 $BIN_PATH/caffe train --solver=$SOLVER --gpu=$GPU --snapshot=$STATE
  fi
fi


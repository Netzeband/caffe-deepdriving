#!/usr/bin/env sh

if [ -z "$DEEPDRIVING_CAFFE_PATH" ]; then
  echo "Error: Please specify path to DeepDriving caffe installation with: "
  echo "export DEEPDRIVING_CAFFE_PATH=<path-to-installation>"
  echo " "
  exit -1

else

  LANES=$1
  MODEL=$2
  WEIGTHS=$3
  MEAN=$4
  GPU=$5

  if [ -z "$MODEL" ]; then
    if [ -z "$DEEPDRIVING_MODEL_PATH" ]; then
      if [ -e pre_trained/driving_run_1F.prototxt ]; then
        MODEL=`pwd`"/pre_trained/driving_run_1F.prototxt"
        echo $MODEL

      else
        echo "Error: Please specify path to DeepDriving model with: "
        echo "export DEEPDRIVING_MODEL_PATH=<path-to-model>"
        echo " "
        exit -1

      fi

    else
      MODEL=$DEEPDRIVING_MODEL_PATH

    fi
  fi

  if [ -z "$WEIGTHS" ]; then
    if [ -z "$DEEPDRIVING_WEIGHTS_PATH" ]; then
      if [ -e pre_trained/driving_train_1F_iter_140000.caffemodel ]; then
        WEIGHTS=`pwd`"/pre_trained/driving_train_1F_iter_140000.caffemodel"

      else
        echo "Error: Please specify path to DeepDriving weights with: "
        echo "export DEEPDRIVING_WEIGHTS_PATH=<path-to-weights>"
        echo " "
        exit -1

      fi

    else
      WEIGTHS=$DEEPDRIVING_WEIGHTS_PATH

    fi
  fi

  if [ -z "$MEAN" ]; then
    if [ -z "$DEEPDRIVING_MEAN_PATH" ]; then
      if [ -e pre_trained/driving_mean_1F.binaryproto ]; then
        MEAN=`pwd`"/pre_trained/driving_mean_1F.binaryproto"

      else
        echo "Error: Please specify path to DeepDriving mean-file with: "
        echo "export DEEPDRIVING_MEAN_PATH=<path-to-mean-file>"
        echo " "
        exit -1

      fi

    else
      MEAN=$DEEPDRIVING_MEAN_PATH

    fi
  fi

  if [ -z "$LANES" ]; then
    echo "Error: Please specify the number of lanes as first parameter: "
    echo "./torcs_run.sh 3"
    echo " "
    exit -1

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

  echo "Use Model:   " $MODEL
  echo "Use Weights: " $WEIGTHS

  BIN_PATH=$DEEPDRIVING_CAFFE_PATH/bin
#  cd $BIN_PATH && (GLOG_logtostderr=1 ./torcs_run --model $MODEL --weights $WEIGHTS --mean $MEAN --lanes $LANES --gpu $GPU)
  GLOG_logtostderr=1 $BIN_PATH/torcs_run --model $MODEL --weights $WEIGTHS --mean $MEAN --lanes $LANES --gpu $GPU
fi



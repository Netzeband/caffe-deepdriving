#!/usr/bin/env sh

if [ -z "$DEEPDRIVING_CAFFE_PATH" ]; then
  echo "Error: Please specify path to DeepDriving caffe installation with: "
  echo "export DEEPDRIVING_CAFFE_PATH=<path-to-installation>"
  echo " "
  exit -1

else

  DATABASE=$1

  if [ -z "$DATABASE" ]; then
    if [ -z "$DEEPDRIVING_DATABASE_PATH" ]; then
      echo "Error: Please specify path to DeepDriving database with: "
      echo "export DEEPDRIVING_DATABASE_PATH=<path-to-database>"
      echo " "
      exit -1

    else
      DATABASE=$DEEPDRIVING_DATABASE_PATH

    fi
  fi

  BIN_PATH=$DEEPDRIVING_CAFFE_PATH/bin
  GLOG_logtostderr=1 $BIN_PATH/torcs_visualize_database --data-path $DATABASE
fi



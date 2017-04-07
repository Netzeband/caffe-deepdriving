# DeepDriving for Caffe 1.0.0rc5

This is a version of the DeepDriving project (see: http://deepdriving.cs.princeton.edu/) which is based on the caffe release 1.0.0rc5 instead of an unknown release from the year 2014. It is basically the original DeepDriving implementation with some small enhancements.

Caffe is a deep learning framework which can be found here: http://caffe.berkeleyvision.org/ In this version the data layer and euclidean-loss layer have been adapted to the needs of the DeepDriving project. Furthermore several DeepDriving specific applications have been added to the tools directory.

In the directory "patches", patch files are included, which define all changes on caffe-files compared to the original caffe version 1.0.0rc5.

***Please take the original licenses of both projects (Caffe and DeepDriving) into account when using this project.***

For further details see the instructions:
* [Install Caffe on Ubuntu 14.04](https://github.com/Netzeband/caffe-deepdriving/wiki/Install)
* [Visualize the Trainings Data](https://github.com/Netzeband/caffe-deepdriving/wiki/CheckDatabase)
* [Drive in Torcs](https://github.com/Netzeband/caffe-deepdriving/wiki/Test)
* [Train the Network](https://github.com/Netzeband/caffe-deepdriving/wiki/Training)
* [Verify the Database](https://github.com/Netzeband/caffe-deepdriving/wiki/ErrorMeas)

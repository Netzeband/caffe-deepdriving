#include <vector>

#include "caffe/layers/euclidean_loss_layer.hpp"
#include "caffe/util/math_functions.hpp"

namespace caffe {

template <typename Dtype>
void EuclideanLossLayer<Dtype>::Forward_gpu(const vector<Blob<Dtype>*>& bottom,
    const vector<Blob<Dtype>*>& top) {
  int count = bottom[0]->count();
  
  // DeepDriving Changes: Normalize the training data before calculating the loss
  //
  // For any reason, the original DeepDriving training data is not normalized, thus
  // we have to do it here before calculating the loss. This is very... ugly!
  // Maybe it would be better to implement a script which preprocess the training
  // data by normalizing it before training starts. I will think about it in a further
  // implementation.
  
  int BatchSize = bottom[0]->num();   
  int OutputDimension = count/BatchSize;
  
  const Dtype* CudaResult = bottom[0]->gpu_data();
  const Dtype* CudaLabel  = bottom[1]->gpu_data();
  
  Dtype Label[BatchSize*14];
  Dtype Result[count]; 
  Dtype LabelArray[count];
  
  cudaMemcpy(Result, CudaResult, sizeof(Dtype) * count,          cudaMemcpyDeviceToHost);
  cudaMemcpy(Label,  CudaLabel,  sizeof(Dtype) * BatchSize * 14, cudaMemcpyDeviceToHost);
    
  for (int i = 0; i < BatchSize; ++i) 
  {
    LabelArray[i * OutputDimension]      = Label[i*14+0]/1.1+0.5;     // angle range ~ [-0.5, 0.5]
    if (LabelArray[i * OutputDimension]>1.0) LabelArray[i * OutputDimension]=1.0;
    if (LabelArray[i * OutputDimension]<0.0) LabelArray[i * OutputDimension]=0.0;

    LabelArray[i * OutputDimension + 1]  = Label[i*14+1]*0.17778+1.34445;   // toMarking_L range ~ [-7, -2.5]
    LabelArray[i * OutputDimension + 2]  = Label[i*14+2]*0.14545+0.39091;   // toMarking_M range ~ [-2, 3.5]
    LabelArray[i * OutputDimension + 3]  = Label[i*14+3]*0.17778-0.34445;   // toMarking_R range ~ [2.5, 7]
    LabelArray[i * OutputDimension + 4]  = Label[i*14+4]/95.0+0.12;   // dist_L range ~ [0, 75]
    LabelArray[i * OutputDimension + 5]  = Label[i*14+5]/95.0+0.12;   // dist_R range ~ [0, 75]
    LabelArray[i * OutputDimension + 6]  = Label[i*14+6]*0.14545+1.48181;   // toMarking_LL range ~ [-9.5, -4]
    LabelArray[i * OutputDimension + 7]  = Label[i*14+7]*0.16+0.98;   // toMarking_ML range ~ [-5.5, -0.5]
    LabelArray[i * OutputDimension + 8]  = Label[i*14+8]*0.16+0.02;   // toMarking_MR range ~ [0.5, 5.5]
    LabelArray[i * OutputDimension + 9]  = Label[i*14+9]*0.14545-0.48181;   // toMarking_RR range ~ [4, 9.5]
    LabelArray[i * OutputDimension + 10] = Label[i*14+10]/95.0+0.12;   // dist_LL range ~ [0, 75]
    LabelArray[i * OutputDimension + 11] = Label[i*14+11]/95.0+0.12;   // dist_MM range ~ [0, 75]
    LabelArray[i * OutputDimension + 12] = Label[i*14+12]/95.0+0.12;   // dist_RR range ~ [0, 75]
    LabelArray[i * OutputDimension + 13] = Label[i*14+13]*0.6+0.2;   // fast range ~ {0, 1}
  }
  
  Dtype* CudaLabelArray;
  cudaMalloc((void**)&CudaLabelArray,    sizeof(Dtype) * count);
  cudaMemcpy(CudaLabelArray, LabelArray, sizeof(Dtype) * count, cudaMemcpyHostToDevice);
  
  caffe_gpu_sub(
      count,
      CudaResult,
      CudaLabelArray,
      diff_.mutable_gpu_data());
      
  Dtype dot;
  caffe_gpu_dot(count, diff_.gpu_data(), diff_.gpu_data(), &dot);
  Dtype loss = dot / bottom[0]->num() / Dtype(2);
  top[0]->mutable_cpu_data()[0] = loss;

  cudaFree(CudaLabelArray);
  
  /*
  Dtype Differences[count]; 
  cudaMemcpy(Differences, diff_.gpu_data(), sizeof(Dtype) * count, cudaMemcpyDeviceToHost);
  
  //for (int i = 0; i < BatchSize; ++i) 
  {
    int i=0;
    for (int j = 0; j < OutputDimension; ++j) 
    {
      printf("BatchElement: %d, Value: %d, Result: %f, LabelArray: %f, diff: %f \n", 
      	i, 
      	j, 
      	Result[i*OutputDimension+j], 
      	LabelArray[i*OutputDimension+j], 
      	Differences[i*OutputDimension+j]); 
      fflush(stdout);
    }    
  }
  printf("Current Loss: %f\n", loss);
  */
}

template <typename Dtype>
void EuclideanLossLayer<Dtype>::Backward_gpu(const vector<Blob<Dtype>*>& top,
    const vector<bool>& propagate_down, const vector<Blob<Dtype>*>& bottom) {
  for (int i = 0; i < 2; ++i) {
    if (propagate_down[i]) {
      const Dtype sign = (i == 0) ? 1 : -1;
      const Dtype alpha = sign * top[0]->cpu_diff()[0] / bottom[i]->num();
      caffe_gpu_axpby(
          bottom[i]->count(),              // count
          alpha,                              // alpha
          diff_.gpu_data(),                   // a
          Dtype(0),                           // beta
          bottom[i]->mutable_gpu_diff());  // b
    }
  }
}

INSTANTIATE_LAYER_GPU_FUNCS(EuclideanLossLayer);

}  // namespace caffe

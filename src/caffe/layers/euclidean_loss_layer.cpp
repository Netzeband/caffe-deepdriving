#include <vector>

#include "caffe/layers/euclidean_loss_layer.hpp"
#include "caffe/util/math_functions.hpp"

namespace caffe {

template <typename Dtype>
void EuclideanLossLayer<Dtype>::Reshape(
  const vector<Blob<Dtype>*>& bottom, const vector<Blob<Dtype>*>& top) {
  LossLayer<Dtype>::Reshape(bottom, top);
  CHECK_EQ(bottom[0]->count(1), bottom[1]->count(1))
      << "Inputs must have the same dimension.";
  diff_.ReshapeLike(*bottom[0]);
}

template <typename Dtype>
void EuclideanLossLayer<Dtype>::Forward_cpu(const vector<Blob<Dtype>*>& bottom,
    const vector<Blob<Dtype>*>& top) {
  int count = bottom[0]->count();

  // DeepDriving Changes: Normalize the training data before calculating the loss
  //
  // For any reason, the original DeepDriving training data is not normalized, thus
  // we have to do it here before calculating the loss. This is very... ugly!
  // Maybe it would be better to implement a script which preprocess the training
  // data by normalizing it before training starts. I will think about it in a further
  // implementation.

  Dtype LabelArray[count];
  const Dtype* Result = bottom[0]->cpu_data();
  const Dtype* Label  = bottom[1]->cpu_data();
  int BatchSize       = bottom[0]->num();
  int OutputDimension = count/BatchSize;
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

  // calculates the difference between result and label vector
  caffe_sub(
      count,
      Result,
      LabelArray,
      diff_.mutable_cpu_data());

  // the dot product of the difference
  Dtype dot = caffe_cpu_dot(count, diff_.cpu_data(), diff_.cpu_data());

  // normalize it with the batch size and divide by 2 = the loss
  Dtype loss = dot / BatchSize / Dtype(2);
  top[0]->mutable_cpu_data()[0] = loss;
}

template <typename Dtype>
void EuclideanLossLayer<Dtype>::Backward_cpu(const vector<Blob<Dtype>*>& top,
    const vector<bool>& propagate_down, const vector<Blob<Dtype>*>& bottom) {
  for (int i = 0; i < 2; ++i) {
    if (propagate_down[i]) {
      const Dtype sign = (i == 0) ? 1 : -1;
      const Dtype alpha = sign * top[0]->cpu_diff()[0] / bottom[i]->num();
      caffe_cpu_axpby(
          bottom[i]->count(),              // count
          alpha,                              // alpha
          diff_.cpu_data(),                   // a
          Dtype(0),                           // beta
          bottom[i]->mutable_cpu_diff());  // b
    }
  }
}

#ifdef CPU_ONLY
STUB_GPU(EuclideanLossLayer);
#endif

INSTANTIATE_CLASS(EuclideanLossLayer);
REGISTER_LAYER_CLASS(EuclideanLoss);

}  // namespace caffe

#include "FloatArray.h"
#include "basicmaths.h"
#include "message.h"

void FloatArray::getMin(float* value, long unsigned int* index){
#ifdef ARM_CORTEX  
  arm_min_f32((float *)data, size, value, index);
#else
  *value=data[0];
  *index=0;
  for(int n=1; n<size; n++){
    float currentValue=data[n];
    if(currentValue<*value){
      *value=currentValue;
      *index=n;
    }
  }
#endif
}

float FloatArray::getMinValue(){
  float value;
  long unsigned int index;
  getMin(&value, &index);
  return value;
}

int FloatArray::getMinIndex(){
  float value;
  long unsigned int index;
  getMin(&value, &index);
  return index;
}

void FloatArray::getMax(float* value, long unsigned int* index){
#ifdef ARM_CORTEX  
  arm_max_f32((float *)data, size, value, index);
#else
  *value=data[0];
  *index=0;
  for(int n=1; n<size; n++){
    float currentValue=data[n];
    if(currentValue>*value){
      *value=currentValue;
      *index=n;
    }
  }
#endif
}

float FloatArray::getMaxValue(){
  float value;
  long unsigned int index;
  getMax(&value, &index);
  return value;
}

int FloatArray::getMaxIndex(){
  float value;
  long unsigned int index;
  getMax(&value, &index);
  return index;
}

void FloatArray::rectify(FloatArray& destination){ //this is actually "copy data with rectifify"
  int minSize= min(size,destination.getSize()); //TODO: shall we take this out and allow it to segfault?
#ifdef ARM_CORTEX  
  arm_abs_f32( (float*)data, (float*)destination, size);
#else
  for(int n=0; n<minSize; n++){
    destination[n]=abs(data[n]);
  }
#endif  
}

void FloatArray::rectify(){//in place
  rectify(*this);
}

void FloatArray::reverse(FloatArray& destination){ //this is actually "copy data with reverse"
  if(destination==*this){ //make sure it is not called "in-place"
    reverse();
    return;
  }
  for(int n=0; n<size; n++){
    destination[n]=data[size-n-1];
  }
}

void FloatArray::reverse(){//in place
  for(int n=0; n<size/2; n++){
    float temp=data[n];
    data[n]=data[size-n-1];
    data[size-n-1]=temp;
  }
}

float FloatArray::getRms(){
  float result;
#ifdef ARM_CORTEX  
  arm_rms_f32 ((float *)data, size, &result);
#else
  result=0;
  float *pSrc=(float *)data;
  for(int n=0; n<size; n++){
    result+=pSrc[n]*pSrc[n];
  }
  result=sqrtf(result/size);
#endif
  return result;
}

float FloatArray::getMean(){
  float result;
#ifdef ARM_CORTEX  
  arm_mean_f32 ((float *)data, size, &result);
#else
  result=0;
  for(int n=0; n<size; n++){
    result+=data[n];
  }
  result=result/size;
#endif
  return result;
}

float FloatArray::getPower(){
  float result;
#ifdef ARM_CORTEX  
  arm_power_f32 ((float *)data, size, &result);
#else
  result=0;
  float *pSrc=(float *)data;
  for(int n=0; n<size; n++){
    result+=pSrc[n]*pSrc[n];
  }
#endif
  return result;
}

float FloatArray::getStandardDeviation(){
  float result;
#ifdef ARM_CORTEX  
  arm_std_f32 ((float *)data, size, &result);
#else
  result=sqrtf(getVariance());
#endif
  return result;
}

float FloatArray::getVariance(){
  float result;
#ifdef ARM_CORTEX  
  arm_var_f32((float *)data, size, &result);
#else
  float sumOfSquares=getPower();
  float sum=0;
  for(int n=0; n<size; n++){
    sum+=data[n];
  }
  result=(sumOfSquares - sum*sum/size) / (size - 1);
#endif
  return result;
}

void FloatArray::scale(float factor){
#ifdef ARM_CORTEX  
  arm_scale_f32 ( (float*)data, factor, (float*)data, size);
#else
  for(int n=0; n<size; n++){
    data[n]*=factor;
  }
#endif
}

FloatArray FloatArray::create(int size){
  return FloatArray(new float[size], size);
}

void FloatArray::destroy(FloatArray array){
  delete array.data;
}

void FloatArray::copyTo(FloatArray other){
  ASSERT(other.size >= size, "Destination array too small");
  arm_copy_f32(data, other.data, size);
  // memcpy(other.data, data, size*sizeof(float));
}

void FloatArray::copyFrom(FloatArray other){
  ASSERT(size >= other.size, "Destination array too small");
  arm_copy_f32(other.data, data, other.size);
}

void FloatArray::setAll(float value){
  arm_fill_f32(value, data, size);
}

void FloatArray::add(FloatArray other){
  ASSERT(other.size == size, "Arrays must be same size");
  arm_add_f32(data, other.data, data, size);
}

void FloatArray::subtract(FloatArray other){
  ASSERT(other.size == size, "Arrays must be same size");
  arm_sub_f32(data, other.data, data, size);
}

void FloatArray::multiply(FloatArray other){
  ASSERT(other.size == size, "Arrays must be same size");
  arm_mult_f32(data, other.data, data, size);
}

void FloatArray::negate(){
  arm_negate_f32(data, data, size);
}

/**
 * Perform the convolution of this FloatArray with @other, putting the result in @destination.
 * @destination must have a minimum size of this+other-1.
 */
void FloatArray::convolve(FloatArray other, FloatArray destination){
  ASSERT(destination.size >= size + other.size -1, "Destination array too small");
  arm_conv_f32(data, size, other.data, other.size, destination);
}

/**
 * Perform partial convolution: start at @offset and compute @samples points
 */
void FloatArray::convolve(FloatArray other, FloatArray destination, int offset, int samples){
  ASSERT(destination.size >= samples, "Destination array too small");
  arm_conv_partial_f32(data, size, other.data, other.size, destination, offset, samples);
}

/*
 * @destination must have a minimum size of 2*max(srcALen, srcBLen)-1.
 */
void FloatArray::correlate(FloatArray other, FloatArray destination){
  ASSERT(destination.size >= 2 * max(size, other.size)-1, "Destination array too small");
  arm_correlate_f32(data, size, other.data, other.size, destination);
}

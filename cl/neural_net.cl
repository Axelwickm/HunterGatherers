#pragma OPENCL EXTENSION cl_arm_printf : disenable
#define RELU(x) (0 < x ? x : 0)
#define LEAKY_RELU(x) (0 < x ? x : x*0.1)
#define ELU(x) (0 < x ? x : 0.1*(exp(x) - 1.f))
#define SIGMOID(x) (1.f / (1.f + exp(-x)))
#define TANH(x) tanh(x)
#define ARCTAN(x) atan(x)


__kernel void perceptron(__constant unsigned* layerSize, __constant float* layerWeight, __constant unsigned* layerBias,
                         unsigned layerIndex, unsigned layerOffset,
                         __global float* previousLayer, __global float* currentLayer){
    size_t id = get_global_id(0);
    size_t previousLayerSize = layerSize[layerIndex];
    size_t thisLayerSize = layerSize[layerIndex+1];

    if (id >= thisLayerSize){
        return;
    }


    // Sum weighted activations of previous layer
    float sum = 0;
    for (size_t i = 0; i < previousLayerSize; i++){
        sum += previousLayer[i] * layerWeight[layerOffset + id*previousLayerSize + i];
    }

    // Add layer weight to sum
    sum += layerWeight[layerIndex];

    //currentLayer[id] = RELU(sum);
    currentLayer[id] = ARCTAN(sum);
    //currentLayer[id] = ELU(sum);
    //currentLayer[id] = SIGMOID(sum);
}
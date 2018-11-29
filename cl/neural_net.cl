#define RELU(x) (0 < x ? x : 0)

__kernel void perceptron(__constant unsigned* layerSize, __constant float* layerWeight, __constant unsigned* layerBias,
                         __local unsigned* layerIndex, __local unsigned* layerOffset,
                         __global float* previousLayer, __global float* currentLayer){
    size_t id = get_global_id(0);
    size_t previousLayerSize = layerSize[*layerIndex];
    size_t thisLayerSize = layerSize[(*layerIndex)+1];


    previousLayer[id] = 5.f;

    if (id >= thisLayerSize){
        return;
    }


    /*float sum = 0;
    for (size_t i = 0; i < previousLayerSize; i++){
        sum += lastLayer[(*layerOffset) - thisLayerSize + i];

    }
    currentLayer[id] = RELU(sum);*/
}
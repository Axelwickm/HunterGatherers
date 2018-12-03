#define RELU(x) (0 < x ? x : 0)

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

    currentLayer[id] = RELU(sum);
}
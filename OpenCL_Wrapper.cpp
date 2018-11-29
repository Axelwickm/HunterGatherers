//
// Created by Axel on 2018-11-27.
//

#include <fstream>
#include <streambuf>
#include <stdexcept>
#include <sstream>

#include "OpenCL_Wrapper.h"


OpenCL_Wrapper::OpenCL_Wrapper(std::string deviceToUse) {
    // Find the right device
    int err;
    size_t valueSize;
    cl_uint platformCount;
    cl_platform_id* platforms;
    cl_uint deviceCount;
    cl_device_id* devices;
    cl_uint maxMaxComputeUnits = 0;

    device_id = nullptr;

    // Get platforms
    clGetPlatformIDs(0, nullptr, &platformCount);
    platforms = (cl_platform_id*) malloc(sizeof(cl_platform_id) * platformCount);
    clGetPlatformIDs(platformCount, platforms, nullptr);

    for (int i = 0; i < platformCount; i++) {
        // Get devices in this platform
        clGetDeviceIDs(platforms[i], CL_DEVICE_TYPE_ALL, 0, nullptr, &deviceCount);
        devices = (cl_device_id*) malloc(sizeof(cl_device_id) * deviceCount);
        clGetDeviceIDs(platforms[i], CL_DEVICE_TYPE_ALL, deviceCount, devices, nullptr);


        for (int j = 0; j < deviceCount; j++) {

            char* value;
            clGetDeviceInfo(devices[j], CL_DEVICE_NAME, 0, nullptr, &valueSize);
            value = (char*) malloc(valueSize);
            clGetDeviceInfo(devices[j], CL_DEVICE_NAME, valueSize, value, nullptr);
            printf("%d. Device: %s\n", j+1, value);
            free(value);

            // print hardware device version

            clGetDeviceInfo(devices[j], CL_DEVICE_VERSION, 0, nullptr, &valueSize);
            value = (char*) malloc(valueSize);
            clGetDeviceInfo(devices[j], CL_DEVICE_VERSION, valueSize, value, nullptr);
            printf(" %d.%d Hardware version: %s\n", j+1, 1, value);
            free(value);

            // print software driver version
            clGetDeviceInfo(devices[j], CL_DRIVER_VERSION, 0, nullptr, &valueSize);
            value = (char*) malloc(valueSize);
            clGetDeviceInfo(devices[j], CL_DRIVER_VERSION, valueSize, value, nullptr);
            printf(" %d.%d Software version: %s\n\n", j+1, 2, value);
            free(value);

            clGetDeviceInfo(devices[j], CL_DEVICE_MAX_COMPUTE_UNITS, sizeof(maxComputeUnits), &maxComputeUnits, nullptr);

            // If a device has been specified in argument options.
            if (!deviceToUse.empty()){
                // Get device name and see if it matches
                clGetDeviceInfo(devices[j], CL_DEVICE_NAME, 0, nullptr, &valueSize);
                char* name = (char*) malloc(valueSize);
                clGetDeviceInfo(devices[j], CL_DEVICE_NAME, valueSize, name, nullptr);

                if (std::string(name) == deviceToUse){
                    device_id = devices[j];
                    free(name);
                    break;
                }

                free(name);
            }
            else {
                // Use the device with the most compute units
                if (maxMaxComputeUnits < maxComputeUnits){
                    maxMaxComputeUnits = maxComputeUnits;
                    device_id = devices[j];
                }
            }

        }
        free(devices);

    }

    free(platforms);

    // Check if found
    if (device_id == nullptr && deviceToUse.empty()){
        throw std::runtime_error("No device found.");
    }
    else if (device_id == nullptr){
        throw std::runtime_error("Specified device not found.");
    }

    // Print selected device name
    clGetDeviceInfo(device_id, CL_DEVICE_NAME, 0, nullptr, &valueSize);
    char* name = (char*) malloc(valueSize);
    clGetDeviceInfo(device_id, CL_DEVICE_NAME, valueSize, name, nullptr);
    printf("Using OpenCL on device: %s\n", name);
    printf("OpenCL device max compute units: %d\n", maxComputeUnits);
    free(name);

    // Create OpenCL context
    context = clCreateContext( nullptr, 1, &device_id, nullptr, nullptr, &err);
    if (!context){
        throw std::runtime_error("Failed to create OpenCL context.");
    }

    // Create OpenCL command queue
    command_queue = clCreateCommandQueue(context, device_id, CL_QUEUE_OUT_OF_ORDER_EXEC_MODE_ENABLE, &err);
    if (!command_queue){
        throw std::runtime_error("Failed to create OpenCL command queue.");
    }


    // Build the programs and create the kernels
    const std::string kernelSource = loadFile("../cl/neural_net.cl");
    cl_program  neuralNetProgram = createAndCompileProgram(kernelSource);
    perceptronKernel = clCreateKernel(neuralNetProgram, "perceptron", &err);

    if (!perceptronKernel || err != CL_SUCCESS){
        throw std::runtime_error("Failed to create OpenCL kernel-");
    }


}

OpenCL_Wrapper::~OpenCL_Wrapper() {
    for (auto& agent : agentRegister){
        clReleaseMemObject(agent.second.layerSizes);
        clReleaseMemObject(agent.second.layerBiases);
        clReleaseMemObject(agent.second.layerWeights);
    }
}

const std::string OpenCL_Wrapper::loadFile(std::string filename) {
    std::ifstream f(filename);
    if (f.fail()){
        throw std::runtime_error("Failed to load file (does it exist?): "+filename);
    }
    const std::string str((std::istreambuf_iterator<char>(f)), (std::istreambuf_iterator<char>()));
    return str;
}

cl_program OpenCL_Wrapper::createAndCompileProgram(const std::string& source) {
    const char* source_data = source.data();
    const char** source_data_p = &source_data;
    int err;
    cl_program program = clCreateProgramWithSource(context, 1, source_data_p, nullptr, &err);

    if (!program || err){
        throw std::runtime_error("Failed to create OpenCL program.");
    }
    err = clBuildProgram(program, 0, nullptr, nullptr, nullptr, nullptr);

    if (err){
        char log[10240] = "";
        clGetProgramBuildInfo(program, device_id, CL_PROGRAM_BUILD_LOG, sizeof(log), log, nullptr);
        throw std::runtime_error("Failed to build OpenCL program: "+std::to_string(err)+" \n-----\n"+std::string(log)+"\n-----\n");
    };

    return program;
}

void OpenCL_Wrapper::addAgent(std::weak_ptr<Agent> agent) {
    NeuralNet neuralNet = agent.lock()->getNeuralNet();
    AgentEntry agentEntry;
    agentEntry.agent = agent;
    agentEntry.layerSizes_Host = neuralNet.layerSizes;
    agentEntry.maxLayerSize = neuralNet.maxLayerSize;

    agentEntry.outputBandwidth = neuralNet.outputBandwidth;
    agentEntry.layerCount = neuralNet.layerCount;


    agentEntry.layerSizes = clCreateBuffer(context, CL_MEM_READ_ONLY, sizeof(unsigned)*(neuralNet.layerCount+1),nullptr, nullptr);
    agentEntry.layerWeights = clCreateBuffer(context, CL_MEM_READ_ONLY, sizeof(float)*neuralNet.layerCount, nullptr, nullptr);
    agentEntry.layerBiases = clCreateBuffer(context, CL_MEM_READ_ONLY, sizeof(float)*neuralNet.layerCount, nullptr, nullptr);


    clEnqueueWriteBuffer(command_queue, agentEntry.layerSizes, CL_TRUE, 0, sizeof(unsigned)*(neuralNet.layerCount+1),
            &neuralNet.layerSizes.front(), 0, nullptr, nullptr);
    clEnqueueWriteBuffer(command_queue, agentEntry.layerWeights, CL_TRUE, 0, sizeof(float)*neuralNet.layerCount,
            &neuralNet.layerWeights.front(), 0, nullptr, nullptr);
    clEnqueueWriteBuffer(command_queue, agentEntry.layerBiases, CL_TRUE, 0, sizeof(float)*neuralNet.layerCount,
            &neuralNet.layerBiases.front(), 0, nullptr, nullptr);

    cl_int err;
    agentEntry.netActivationA = clCreateBuffer(context, CL_MEM_READ_WRITE, sizeof(float)*agentEntry.maxLayerSize,
            nullptr, &err);
    agentEntry.netActivationB = clCreateBuffer(context, CL_MEM_READ_WRITE, sizeof(float)*agentEntry.maxLayerSize,
            nullptr, &err);

    agentRegister.insert({agent.lock().get(), agentEntry});
}

void OpenCL_Wrapper::think(Agent* agent, const std::vector<float>& percept) {
    AgentEntry& agentEntry  = agentRegister.at(agent);
    cl_event lastEvent;
    cl_event newEvent;

    clSetKernelArg(perceptronKernel, 0, sizeof(cl_mem), &agentEntry.layerSizes);
    clSetKernelArg(perceptronKernel, 1, sizeof(cl_mem), &agentEntry.layerWeights);
    clSetKernelArg(perceptronKernel, 2, sizeof(cl_mem), &agentEntry.layerBiases);

    // Buffer the percept
    // Note that this will probably not fill the whole buffer, but the kernel is fine with that.
    clEnqueueWriteBuffer(command_queue, agentEntry.netActivationA, CL_FALSE, 0,
            sizeof(float)*agentEntry.maxLayerSize, &percept[0], 0, nullptr, &lastEvent);


    unsigned layerOffset = 0;
    for (unsigned i = 0; i < agentEntry.layerCount; i++){
        clSetKernelArg(perceptronKernel, 3, sizeof(unsigned), &i);
        clSetKernelArg(perceptronKernel, 4, sizeof(unsigned), &layerOffset);

        if (i%2 == 0){
            clSetKernelArg(perceptronKernel, 5, sizeof(cl_mem), &agentEntry.netActivationA);
            clSetKernelArg(perceptronKernel, 6, sizeof(cl_mem), &agentEntry.netActivationB);
        }
        else {
            clSetKernelArg(perceptronKernel, 6, sizeof(cl_mem), &agentEntry.netActivationA);
            clSetKernelArg(perceptronKernel, 5, sizeof(cl_mem), &agentEntry.netActivationB);
        }


        size_t localSize = std::min(maxComputeUnits, agentEntry.layerSizes_Host[i]);
        size_t globalSize = (size_t) ceil((double) agentEntry.layerSizes_Host[i]/localSize)*localSize;

        printf("%d %d\n", localSize, globalSize);

        clEnqueueNDRangeKernel(command_queue, perceptronKernel, 1, nullptr, &globalSize, &localSize, 0, &lastEvent,
                               &newEvent);

        if (i == 0) {
            float o[agentEntry.outputBandwidth];

            clEnqueueReadBuffer(command_queue, agentEntry.netActivationA, CL_FALSE, 0,
                                sizeof(float)*agentEntry.outputBandwidth, &o[0], 1, &lastEvent, &newEvent);

            lastEvent = newEvent;

            clWaitForEvents(1, &lastEvent);
            for (auto &x : o) {
                printf("%f ", x);
            }
            printf("\n");
        }

        lastEvent = newEvent;
        layerOffset += agentEntry.layerSizes_Host[i];


    }

    /*float output[agentEntry.outputBandwidth];
    if (agentEntry.layerCount-1 % 2 == 0){
        clEnqueueReadBuffer(command_queue, agentEntry.netActivationB, CL_FALSE, 0,
                sizeof(float)*agentEntry.outputBandwidth, &output[0], 1, &lastEvent, &newEvent);
    }
    else {
        clEnqueueReadBuffer(command_queue, agentEntry.netActivationA, CL_FALSE, 0,
                            sizeof(float)*agentEntry.outputBandwidth, &output[0], 1, &lastEvent, &newEvent);
    }

    lastEvent = newEvent;
    clWaitForEvents(1, &lastEvent);
    for (auto& x : output){
        printf("%f ", x);
    }
    printf("\n");*/
}

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
    cl_int err;
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

    cl_int err;
    agentEntry.layerSizes = clCreateBuffer(context, CL_MEM_READ_ONLY, sizeof(unsigned)*(neuralNet.layerCount+1),nullptr, &err);
    if (err){
        throw std::runtime_error("Failed to create OpenCL neural net layer sizes buffer: "+std::to_string(err));
    }
    agentEntry.layerWeights = clCreateBuffer(context, CL_MEM_READ_ONLY, sizeof(float)*neuralNet.layerWeights.size(), nullptr, &err);
    if (err){
        throw std::runtime_error("Failed to create OpenCL neural net layer weight buffer: "+std::to_string(err));
    }
    agentEntry.layerBiases = clCreateBuffer(context, CL_MEM_READ_ONLY, sizeof(float)*neuralNet.layerCount, nullptr, &err);
    if (err){
        throw std::runtime_error("Failed to create OpenCL neural net layer weight buffer: "+std::to_string(err));
    }


    err = clEnqueueWriteBuffer(command_queue, agentEntry.layerSizes, CL_TRUE, 0, sizeof(unsigned)*(neuralNet.layerCount+1),
            &neuralNet.layerSizes.front(), 0, nullptr, nullptr);
    if (err){
        throw std::runtime_error("Failed to enqueue layer sizes buffer write: "+std::to_string(err));
    }

    err = clEnqueueWriteBuffer(command_queue, agentEntry.layerWeights, CL_TRUE, 0, sizeof(float)*neuralNet.layerWeights.size(),
            &neuralNet.layerWeights.front(), 0, nullptr, nullptr);
    if (err){
        throw std::runtime_error("Failed to enqueue layer sizes buffer write: "+std::to_string(err));
    }

    err = clEnqueueWriteBuffer(command_queue, agentEntry.layerBiases, CL_TRUE, 0, sizeof(float)*neuralNet.layerCount,
            &neuralNet.layerBiases.front(), 0, nullptr, nullptr);
    if (err){
        throw std::runtime_error("Failed to enqueue layer sizes buffer write: "+std::to_string(err));
    }

    agentEntry.netActivationA = clCreateBuffer(context, CL_MEM_READ_WRITE, sizeof(float)*agentEntry.maxLayerSize,
            nullptr, &err);
    agentEntry.netActivationB = clCreateBuffer(context, CL_MEM_READ_WRITE, sizeof(float)*agentEntry.maxLayerSize,
            nullptr, &err);
    if (err){
        throw std::runtime_error("Failed to create net activation buffer: "+std::to_string(err));
    }

    agentRegister.insert({agent.lock().get(), agentEntry});
}

void OpenCL_Wrapper::think(Agent* agent, const std::vector<float>& percept) {
    AgentEntry& agentEntry  = agentRegister.at(agent);
    cl_event lastEvent;
    cl_event newEvent;

    cl_int err = 0;
    err |= clSetKernelArg(perceptronKernel, 0, sizeof(cl_mem), &agentEntry.layerSizes);
    err |= clSetKernelArg(perceptronKernel, 1, sizeof(cl_mem), &agentEntry.layerWeights);
    err |= clSetKernelArg(perceptronKernel, 2, sizeof(cl_mem), &agentEntry.layerBiases);
    if (err){
        throw std::runtime_error("Failed to set OpenCL neural net architecture kernel arg: "+std::to_string(err));
    }

    // Buffer the percept
    // Note that this will probably not fill the whole buffer, but the kernel is fine with that.
    err = clEnqueueWriteBuffer(command_queue, agentEntry.netActivationA, CL_FALSE, 0,
            sizeof(float)*agentEntry.maxLayerSize, &percept[0], 0, nullptr, &lastEvent);

    if (err){
        throw std::runtime_error("Failed to write to first net activation buffer: "+std::to_string(err));
    }


    unsigned layerOffset = 0;
    for (unsigned i = 0; i < agentEntry.layerCount; i++){
        err =  clSetKernelArg(perceptronKernel, 3, sizeof(unsigned), &i);
        err |= clSetKernelArg(perceptronKernel, 4, sizeof(unsigned), &layerOffset);

        if (err){
            throw std::runtime_error("Failed to set OpenCL current layer kernel arg: "+std::to_string(err));
        }

        if (i%2 == 0){
            err =  clSetKernelArg(perceptronKernel, 5, sizeof(cl_mem), &agentEntry.netActivationA);
            err |= clSetKernelArg(perceptronKernel, 6, sizeof(cl_mem), &agentEntry.netActivationB);
        }
        else {
            err =  clSetKernelArg(perceptronKernel, 6, sizeof(cl_mem), &agentEntry.netActivationA);
            err |= clSetKernelArg(perceptronKernel, 5, sizeof(cl_mem), &agentEntry.netActivationB);
        }
        if (err){
            throw std::runtime_error("Failed to set OpenCL activation kernel arg: "+std::to_string(err));
        }


        size_t localSize = std::min(maxComputeUnits, agentEntry.layerSizes_Host[i]);
        size_t globalSize = (size_t) ceil((double) agentEntry.layerSizes_Host[i]/localSize)*localSize;

        //printf("%d %d\n", localSize, globalSize);

        err = clEnqueueNDRangeKernel(command_queue, perceptronKernel, 1, nullptr, &globalSize, &localSize, 0, nullptr,
                               &newEvent);
        if (err){
            throw std::runtime_error("Failed to enqueue perceptron ND Range kernels: "+std::to_string(err));
        }

        lastEvent = newEvent;
        layerOffset += agentEntry.layerSizes_Host[i] * agentEntry.layerSizes_Host[i+1];


    }

    float output[agentEntry.outputBandwidth];
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
    printf("\n");
}

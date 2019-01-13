//
// Created by Axel on 2018-11-27.
//

#include <fstream>
#include <sstream>
#include <functional>

#include "OpenCL_Wrapper.h"


OpenCL_Wrapper::OpenCL_Wrapper(std::string deviceToUse) {
    // Find the right device
    int err;
    size_t valueSize;
    cl_uint platformCount;
    cl_platform_id *platforms;
    cl_uint deviceCount;
    cl_device_id *devices;
    cl_uint maxMaxComputeUnits = 0;

    device_id = nullptr;

    // Get platforms
    clGetPlatformIDs(0, nullptr, &platformCount);
    platforms = (cl_platform_id *) malloc(sizeof(cl_platform_id) * platformCount);
    clGetPlatformIDs(platformCount, platforms, nullptr);

    for (int i = 0; i < platformCount; i++) {
        // Get devices in this platform
        clGetDeviceIDs(platforms[i], CL_DEVICE_TYPE_ALL, 0, nullptr, &deviceCount);
        devices = (cl_device_id*) malloc(sizeof(cl_device_id) * deviceCount);
        clGetDeviceIDs(platforms[i], CL_DEVICE_TYPE_ALL, deviceCount, devices, nullptr);


        for (int j = 0; j < deviceCount; j++) {

            char *value;
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
                auto name = (char *) malloc(valueSize);
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

    // Print selectedText device name
    clGetDeviceInfo(device_id, CL_DEVICE_NAME, 0, nullptr, &valueSize);
    auto name = (char *) malloc(valueSize);
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
    while (!agentRegister.empty()){
        removeAgent(agentRegister.begin()->second.agent);
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

cl_program OpenCL_Wrapper::createAndCompileProgram(const std::string &source) {
    const char *source_data = source.data();
    const char **source_data_p = &source_data;
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

void OpenCL_Wrapper::addAgent(Agent *agent) {

    const MapGenes &genes = *agent->getGenes();

    agentRegister[agent] = AgentEntry();
    AgentEntry &agentEntry = agentRegister.at(agent);
    agentEntry.agent = agent;


    agentEntry.outputBandwidth = (unsigned int) genes.getGene<IntegerGene>("OutputCount")->getValue();
    agentEntry.output.resize(agentEntry.outputBandwidth, 0);
    agentEntry.layerCount = (unsigned int) genes.getGene<IntegerGene>("LayerCount")->getValue();

    auto layersList = genes.getGene<ListGenes>("Layers")->getList().begin();
    std::vector<float> layerBiases;
    std::generate_n(std::back_inserter(layerBiases), agentEntry.layerCount, [&]{
        auto g = ((MapGenes*) layersList->get())->getGene<FloatGene>("Bias");
        layersList++;
        return g->getValue();
    });


    layersList = genes.getGene<ListGenes>("Layers")->getList().begin();
    std::vector<unsigned> layerSizes;
    layerSizes.push_back((unsigned int) genes.getGene<IntegerGene>("InputCount")->getValue());
    agentEntry.maxLayerSize = 0;
    std::generate_n(std::back_inserter(layerSizes), agentEntry.layerCount, [&]{
        auto g = ((MapGenes*) layersList->get())->getGene<LambdaGene<int> >("PerceptronCount");
        layersList++;
        int val = g->getValue();
        agentEntry.maxLayerSize = agentEntry.maxLayerSize < val ? val : agentEntry.maxLayerSize;
        return val;
    });

    agentEntry.layerSizes_Host = layerSizes;

    std::vector<float> layerWeights;
    for (auto &layer : genes.getGene<ListGenes>("Layers")->getList()) {
        for (auto &perceptron : ((MapGenes *) layer.get())->getGene<ListGenes>("Perceptrons")->getList()) {
            for (auto &weight : ((MapGenes *) perceptron.get())->getGene<ListGenes>("Weights")->getList()) {
                layerWeights.push_back(((FloatGene*) weight.get())->getValue());
            }
        }
    }

    cl_int err;
    agentEntry.layerSizes = clCreateBuffer(context, CL_MEM_READ_ONLY, sizeof(unsigned)*(agentEntry.layerCount+1),nullptr, &err);
    if (err){
        throw std::runtime_error("Failed to create OpenCL neural net layer sizes buffer: "+std::to_string(err));
    }
    agentEntry.layerWeights = clCreateBuffer(context, CL_MEM_READ_ONLY, sizeof(float)*layerWeights.size(), nullptr, &err);
    if (err){
        throw std::runtime_error("Failed to create OpenCL neural net layer weight buffer: "+std::to_string(err));
    }
    agentEntry.layerBiases = clCreateBuffer(context, CL_MEM_READ_ONLY, sizeof(float)*agentEntry.layerCount, nullptr, &err);
    if (err){
        throw std::runtime_error("Failed to create OpenCL neural net layer weight buffer: "+std::to_string(err));
    }


    err = clEnqueueWriteBuffer(command_queue, agentEntry.layerSizes, CL_TRUE, 0, sizeof(unsigned)*(agentEntry.layerCount+1),
            &layerSizes.front(), 0, nullptr, nullptr);
    if (err){
        throw std::runtime_error("Failed to enqueue layer sizes buffer write: "+std::to_string(err));
    }

    err = clEnqueueWriteBuffer(command_queue, agentEntry.layerWeights, CL_TRUE, 0, sizeof(float)*layerWeights.size(),
            &layerWeights.front(), 0, nullptr, nullptr);
    if (err){
        throw std::runtime_error("Failed to enqueue layer sizes buffer write: "+std::to_string(err));
    }

    err = clEnqueueWriteBuffer(command_queue, agentEntry.layerBiases, CL_TRUE, 0, sizeof(float)*agentEntry.layerCount,
            &layerBiases.front(), 0, nullptr, nullptr);
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

}

void OpenCL_Wrapper::removeAgent(Agent *agent) {
    AgentEntry &a = agentRegister.at(agent);

    cl_int err = clReleaseMemObject(a.netActivationA);
    err |= clReleaseMemObject(a.netActivationB);

    err |= clReleaseMemObject(a.layerSizes);
    err |= clReleaseMemObject(a.layerBiases);
    err |= clReleaseMemObject(a.layerWeights);

    agentRegister.erase(agent);
    if (err){
        throw std::runtime_error("Failed to release memory objects: "+std::to_string(err));
    }
}

void OpenCL_Wrapper::think(std::shared_ptr<Agent> agent, const std::vector<float> &percept) {
    AgentEntry &agentEntry = agentRegister.at(agent.get());

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
    // This will probably not fill the whole buffer, but the kernel is fine with that.
    err = clEnqueueWriteBuffer(command_queue, agentEntry.netActivationA, CL_FALSE, 0,
            sizeof(float)*agentEntry.maxLayerSize, &percept.at(0), 0, nullptr, &lastEvent);

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


        size_t localSize = std::min(maxComputeUnits, agentEntry.layerSizes_Host[i+1]);
        size_t globalSize = (size_t) ceil((double) agentEntry.layerSizes_Host[i+1]/localSize)*localSize;

        //printf("%d %d\n", i, globalSize);

        err = clEnqueueNDRangeKernel(command_queue, perceptronKernel, 1, nullptr, &globalSize, &localSize,
                1, &lastEvent, &newEvent);

        if (err){
            throw std::runtime_error("Failed to enqueue perceptron ND Range kernels: "+std::to_string(err));
        }
        err = clReleaseEvent(lastEvent);

        if (err){
            throw std::runtime_error("Failed to release event: "+std::to_string(err));
        }
        lastEvent = newEvent;
        layerOffset += agentEntry.layerSizes_Host[i] * agentEntry.layerSizes_Host[i+1];


    }


    if (agentEntry.layerCount % 2 == 0){
        err = clEnqueueReadBuffer(command_queue, agentEntry.netActivationA, CL_FALSE, 0,
                            sizeof(float)*agentEntry.output.size(), &agentEntry.output.front(), 1, &lastEvent, &newEvent);
    }
    else {
        err = clEnqueueReadBuffer(command_queue, agentEntry.netActivationB, CL_FALSE, 0,
                            sizeof(float)*agentEntry.output.size(), &agentEntry.output.front(), 1, &lastEvent, &newEvent);
    }

    if (err == -30){
        throw std::runtime_error("Err -30 D: <- me");

    }
    else if (err){
        throw std::runtime_error("Failed to read output buffer from network: "+std::to_string(err));
    }

    clReleaseEvent(lastEvent);
    lastEvent = newEvent;

    clSetEventCallback(lastEvent, CL_COMPLETE, responseCallback, (void*) &agentEntry);

}

void OpenCL_Wrapper::clFinishAll() {
    clFinish(command_queue);
}

void OpenCL_Wrapper::responseCallback(cl_event e, cl_int status, void *data) {
    auto entry = (AgentEntry*) data;
    //printf("net out: %f, ", entry->output[0]);
    for (float &f : entry->output) {
        f = 1.f / (1.f + expf(-f));
    }
    //printf("normalized: %f\n", entry->output[0]);
    entry->agent->setActions(entry->output);
    clReleaseEvent(e);
}



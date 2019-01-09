//
// Created by Axel on 2018-11-27.
//

#ifndef HUNTERGATHERERS_OPENCL_WRAPPER_H
#define HUNTERGATHERERS_OPENCL_WRAPPER_H


#define CL_HPP_TARGET_OPENCL_VERSION 120
#ifdef __APPLE__
#include "OpenCL/opencl.h"
#else
#include <CL/cl.h>
#endif


#include "WorldObject.h"
#include "Agent.h"

#include <unordered_map>



struct AgentEntry {
    Agent* agent;
    std::vector<float> output;

    unsigned outputBandwidth;
    unsigned layerCount;

    std::vector<unsigned> layerSizes_Host;
    unsigned maxLayerSize;

    cl_mem layerSizes;
    cl_mem layerBiases;
    cl_mem layerWeights;

    cl_mem netActivationA;
    cl_mem netActivationB;
};

class OpenCL_Wrapper {
public:
    explicit OpenCL_Wrapper(std::string deviceToUse);
    virtual ~OpenCL_Wrapper();

    void addAgent(Agent* agent);
    void removeAgent(Agent* agent);
    void think(std::shared_ptr<Agent> agent, const std::vector<float> &percept);

    void clFinishAll();
    std::unordered_map<Agent*, AgentEntry> agentRegister;

private:
    cl_device_id device_id;
    cl_context context;
    cl_command_queue command_queue;
    cl_uint maxComputeUnits;

    cl_kernel perceptronKernel;

    const std::string loadFile(std::string filename);
    cl_program createAndCompileProgram(const std::string& source);


    static void responseCallback(cl_event e, cl_int status, void* data);

};


#endif //HUNTERGATHERERS_OPENCL_WRAPPER_H

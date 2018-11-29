//
// Created by Axel on 2018-11-27.
//

#ifndef HUNTERGATHERERS_OPENCL_WRAPPER_H
#define HUNTERGATHERERS_OPENCL_WRAPPER_H



#ifdef __APPLE__
#include "OpenCL/opencl.h"
#else
#include <CL/cl.h>
#endif


#include "WorldObject.h"
#include "Agent.h"

#include <unordered_map>



struct AgentEntry {
    std::weak_ptr<Agent> agent;

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
    OpenCL_Wrapper(std::string deviceToUse);
    virtual ~OpenCL_Wrapper();

    void addAgent(std::weak_ptr<Agent> agent);
    void think(Agent* agent, const std::vector<float>& percept);

private:
    cl_device_id device_id;
    cl_context context;
    cl_command_queue command_queue;
    cl_uint maxComputeUnits;

    cl_kernel perceptronKernel;

    const std::string loadFile(std::string filename);
    cl_program createAndCompileProgram(const std::string& source);

    std::unordered_map<Agent*, AgentEntry> agentRegister;

};


#endif //HUNTERGATHERERS_OPENCL_WRAPPER_H

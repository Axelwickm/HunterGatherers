//
// Created by Axel on 2018-11-21.
//

#ifndef FAMILYISEVERYTHING_AGENT_H
#define FAMILYISEVERYTHING_AGENT_H

#include "WorldObject.h"

class Agent : WorldObject {
public:
    void update(float deltaTime) override;
};


#endif //FAMILYISEVERYTHING_AGENT_H

//
// Created by pierr on 16/03/2022.
//

#include "RandomGenerator.h"
#include "Commons/AstraException.h"

#include <chrono>

namespace Astra {

    RandomGenerator* RandomGenerator::s_instance = nullptr;

    RandomGenerator::RandomGenerator() {
        AstraException::assertV(s_instance == nullptr, "RandomGenerator already Init");
        m_engine = std::mt19937_64(m_randomDevice());
        m_engine.seed(std::chrono::system_clock::now().time_since_epoch().count());
        s_instance = this;
    }

    RandomGenerator::~RandomGenerator() {
        s_instance = nullptr;
    }

    uint64_t RandomGenerator::rndInt() {
        return m_uniformDistribution(m_engine);
    }

    float RandomGenerator::rndFloat(float range) {
        std::uniform_real_distribution<float> rndDist(0.0f, range);
        return rndDist(m_engine);
    }

    RandomGenerator* RandomGenerator::Get() {
        AstraException::assertV(s_instance != nullptr, "RandomGenerator not Init");
        return s_instance;
    }
}

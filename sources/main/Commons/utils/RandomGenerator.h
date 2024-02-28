//
// Created by pierr on 16/03/2022.
//


#pragma once

// std
#include <random>
#include <cstdint>

namespace Astra {

    class AstraEngine;

    class RandomGenerator
    {
    private:
        static RandomGenerator* s_instance;

        std::random_device m_randomDevice{};
        std::mt19937_64 m_engine;
        std::uniform_int_distribution<uint64_t> m_uniformDistribution{};

    public:
        RandomGenerator();
        ~RandomGenerator();

        uint64_t rndInt();
        float rndFloat(float range);

        static RandomGenerator* Get();
    };

}

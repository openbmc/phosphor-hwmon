#pragma once

class SensorSet;

std::string getEnv(
    const char* prefix, const SensorSet::key_type& sensor);

std::string getIndirectLabelEnv(const char* prefix,
                                std::string path,
                                const SensorSet::key_type& sensor);

#pragma once

class SensorSet;

std::string getEnv(
    const char* prefix, const SensorSet::key_type& sensor);

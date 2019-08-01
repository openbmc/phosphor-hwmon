#include "average.hpp"

#include <cassert>
#include <cstdlib>

std::optional<Average::averageValue>
    Average::getAverageValue(const Average::averageKey& sensorKey) const
{
    const auto it = _averageSet.find(sensorKey);
    if (it == _averageSet.end())
    {
        return {};
    }
    else
    {
        return std::optional(it->second);
    }
}

void Average::setAverageValue(const Average::averageKey& sensorKey,
                              const averageValue& sensorValue)
{
    _averageSet[sensorKey] = sensorValue;
}

int64_t Average::calAverage(int64_t preAverage, int64_t preInterval,
                            int64_t curAverage, int64_t delta)
{
    int64_t value = 0;

    assert(delta > 0);

    // Change formula (a2*i2-a1*i1)/(i2-i1) to be the
    // following formula, to avoid multiplication overflow.
    // (a2*i2-a1*i1)/(i2-i1) =
    // (a2*(i1+delta)-a1*i1)/delta =
    // (a2-a1)(i1/delta)+a2
    value =
        (curAverage - preAverage) * (static_cast<double>(preInterval) / delta) +
        curAverage;

    return value;
}

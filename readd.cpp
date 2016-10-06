#include <iostream>
#include <memory>
#include <thread>
#include "argument.hpp"
#include "sensorset.hpp"
#include "sensorcache.hpp"
#include "hwmon.hpp"
#include "sysfs.hpp"

static void exit_with_error(const char* err, char** argv)
{
    ArgumentParser::usage(argv);
    std::cerr << std::endl;
    std::cerr << "ERROR: " << err << std::endl;
    exit(-1);
}

int main(int argc, char** argv)
{
    // Read arguments.
    auto options = std::make_unique<ArgumentParser>(argc, argv);

    // Parse out path argument.
    auto path = (*options)["path"];
    if (path == ArgumentParser::empty_string)
    {
        exit_with_error("Path not specified.", argv);
    }

    // Finished getting options out, so release the parser.
    options.release();

    // Check sysfs for available sensors.
    auto sensors = std::make_unique<SensorSet>(path);
    auto sensor_cache = std::make_unique<SensorCache>();

    // TODO: Issue#3 - Need to make calls to the dbus sensor cache here to
    //       ensure the objects all exist?

    // Polling loop.
    while(true)
    {
        // Iterate through all the sensors.
        for(auto& i : *sensors)
        {
            if (i.second.find(hwmon::entry::input) != i.second.end())
            {
                // Read value from sensor.
                int value = 0;
                read_sysfs(make_sysfs_path(path,
                                           i.first.first, i.first.second,
                                           hwmon::entry::input),
                           value);

                // Update sensor cache.
                if (sensor_cache->update(i.first, value))
                {
                    // TODO: Issue#4 - dbus event here.
                    std::cout << i.first.first << i.first.second << " = "
                              << value << std::endl;
                }
            }
        }

        // Sleep until next interval.
        // TODO: Issue#5 - Make this configurable.
        // TODO: Issue#6 - Optionally look at polling interval sysfs entry.
        {
            using namespace std::literals::chrono_literals;
            std::this_thread::sleep_for(1s);
        }

        // TODO: Issue#7 - Should probably periodically check the SensorSet
        //       for new entries.
    }

    return 0;
}


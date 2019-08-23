#pragma once

#include "env.hpp"

#include <sstream>
#include <string>

#include <gmock/gmock.h>

namespace env
{

/**
 * @class: a fake std::env class
 * @brief: trivially implements a keyval map whose value can be injected
 */
class FakeEnv : public Env
{
  public:
    /**
     * @brief: parse the input string into key/pair map.
     *         Each line of string constructor is splitted by the first '=' to
     *         produce the key/val pair.
     * @param[in] envStr: input string to the ctor. Expected to be in the format
     *         of:
     *
     *         ENV1=abc\n
     *         ENV2=def\n
     */
    void SetEnv(const std::string envStr)
    {
        std::stringstream i;
        std::string line;

        _keyVals.clear();
        i << envStr;
        while (std::getline(i, line))
        {

            if (line.empty())
            {
                continue;
            }
            auto pos = line.find('=');
            _keyVals[line.substr(0, pos)] = line.substr(pos + 1);
        }
    }

    std::string get(const char* key) const override
    {
        if (_keyVals.find(key) == _keyVals.end())
            return {};
        return _keyVals.at(std::string(key));
    }

  private:
    std::map<std::string, std::string> _keyVals;
};

class EnvMock : public Env
{
  public:
    MOCK_CONST_METHOD1(get, std::string(const char*));

    /* Set this before each test */
    void setFakeEnv(const std::string& envStr)
    {
        _fake.SetEnv(envStr);
        ON_CALL(*this, get).WillByDefault([this](const char* key) {
            return _fake.get(key);
        });
    }

  private:
    FakeEnv _fake;
};

static inline EnvMock mockEnv;

} // namespace env

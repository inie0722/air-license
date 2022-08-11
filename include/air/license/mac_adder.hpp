#pragma once

#include "air/license/identification.hpp"
#include "air/license/tool.hpp"

#include <boost/algorithm/string.hpp>

namespace air
{
    namespace license
    {
        class mac_adder : public identification
        {
        public:
            mac_adder() = default;

            virtual ~mac_adder() = default;

            virtual Json::Value info()
            {
                Json::Value ret;

                auto res = tool::system({ENCRYPT_STRING("ip addr show eth0 | grep 'link/ether' | awk '{print $2}' 2>/dev/null")});

                if (res.second == 0)
                {
                    std::vector<std::string> strs;
                    boost::split(strs, res.first, boost::is_any_of("\n"));

                    int i = 0;
                    for (auto &str : strs)
                    {
                        if (str.length() > 0)
                        {
                            ret[i] = str;
                            ++i;
                        }
                    }
                }

                return ret;
            }

            virtual Json::Value generate(const Json::Value &arg)
            {
                return arg;
            }

            virtual bool validate(const Json::Value &arg)
            {
                return arg == this->info();
            }
        };
    }
}
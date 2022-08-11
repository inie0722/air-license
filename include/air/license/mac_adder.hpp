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

                auto res = tool::system({ENCRYPT_STRING("ip addr show | grep -B 1 'link/ether' | awk '{print $2}' 2>/dev/null | awk '{if($0!=\"\") print}'")});

                if (res.second == 0)
                {
                    std::vector<std::string> strs;
                    boost::split(strs, res.first, boost::is_any_of("\n"));

                    for (size_t i = 0; i < strs.size() / 2; i++)
                    {
                        strs[i * 2].pop_back();
                        ret[strs[i * 2]] = strs[i * 2 + 1];
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
                auto info = this->info();
                for (auto &member : arg.getMemberNames())
                {
                    if (arg[member] != info[member])
                        return false;
                }

                return true;
            }
        };
    }
}
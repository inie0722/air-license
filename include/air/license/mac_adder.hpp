#pragma once

#include "air/license/identification.hpp"
#include "air/license/utility.hpp"

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
#if defined __linux__ || defined __APPLE__
                auto res = utility::system({AIR_ENCRYPT_STRING("ip addr show | grep 'link/ether'")});

                if (res.second == 0)
                {
                    std::vector<std::string> strs;
                    boost::split(strs, res.first, boost::is_any_of("\n"));

                    int i = 0;
                    for (auto &str : strs)
                    {
                        if (str.length())
                        {
                            ret[i] = str.substr(15, 17);
                            ++i;
                        }
                    }
                }
#elif defined _WIN32
                auto res = utility::system({AIR_ENCRYPT_STRING("wmic nicconfig get macaddress")});
                if (res.second == 0)
                {
                    std::vector<std::string> strs;
                    boost::split(strs, res.first, boost::is_any_of("\r\r\n"));
                    strs.erase(strs.begin());

                    int i = 0;
                    std::string tmp;
                    for (auto &str : strs)
                    {
                        //丢弃重复的
                        auto id = str;

                        if (str.length() > 0 and tmp != id)
                        {
                            tmp = id;
                            ret[i] = id;
                            ++i;
                        }
                    }
                }
#endif
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
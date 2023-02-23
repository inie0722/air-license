#pragma once

#include "air/license/identification.hpp"
#include "air/license/utility.hpp"

#include <boost/algorithm/string.hpp>

namespace air
{
    namespace license
    {
        class disk_serial : public identification
        {
        public:
            disk_serial() = default;

            virtual ~disk_serial() = default;

            virtual Json::Value info() override
            {
                Json::Value ret;
#if defined __linux__ || defined __APPLE__
                auto res = utility::system({AIR_ENCRYPT_STRING("lsblk --nodeps -no serial")});

                if (res.second == 0)
                {
                    std::vector<std::string> strs;
                    boost::split(strs, res.first, boost::is_any_of("\n"));
                    strs.pop_back();

                    int i = 0;
                    for (auto &str : strs)
                    {
                        if (str.length())
                        {
                            ret[i] = str;
                            ++i;
                        }
                    }
                }
#elif defined _WIN32
                auto res = utility::system({AIR_ENCRYPT_STRING("wmic diskdrive get serialnumber")});
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

            virtual Json::Value generate(const Json::Value &arg) override
            {
                return arg;
            }

            virtual bool validate(const Json::Value &arg) override
            {
                return arg == this->info();
            }
        };
    }
}
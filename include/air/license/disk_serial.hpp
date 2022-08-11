#pragma once

#include "air/license/identification.hpp"
#include "air/license/tool.hpp"

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

            virtual Json::Value info()
            {
                Json::Value ret;

                auto res = tool::system({AIR_ENCRYPT_STRING("lsblk --nodeps -no serial 2>/dev/null | awk '{if($0!=\"\") print}'")});

                if (res.second == 0)
                {
                    std::vector<std::string> strs;
                    boost::split(strs, res.first, boost::is_any_of("\n"));
                    strs.pop_back();

                    int i = 0;
                    for (auto &str : strs)
                    {
                        ret[i] = str;
                        ++i;
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
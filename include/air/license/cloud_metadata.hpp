#pragma once

#include "air/license/identification.hpp"
#include "air/license/tool.hpp"

#include <boost/algorithm/string.hpp>

namespace air
{
    namespace license
    {
        class cloud_metadata : public identification
        {
        public:
            cloud_metadata() = default;

            virtual ~cloud_metadata() = default;

            virtual Json::Value info()
            {
                Json::Value ret;

                auto res = tool::system({ENCRYPT_STRING("curl --connect-timeout 1 http://metadata.tencentyun.com/latest/meta-data/instance-id 2>/dev/null")});

                if (res.second == 0)
                {
                    std::vector<std::string> strs;
                    boost::split(strs, res.first, boost::is_any_of("\n"));

                    int i = 0;
                    for (auto &str : strs)
                    {
                        if (str.length() > 0)
                        {
                            ret[i] = "tencent." + str;
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
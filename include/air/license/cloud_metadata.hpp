#pragma once

#include "air/license/identification.hpp"
#include "air/license/utility.hpp"

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

            virtual Json::Value info() override
            {
                Json::Value ret;

                auto res = utility::system({AIR_ENCRYPT_STRING("curl --connect-timeout 1 http://metadata.tencentyun.com/latest/meta-data/instance-id")});

                if (res.second == 0)
                {
                    res.first.pop_back();
                    ret["tencent"] = res.first;
                }

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
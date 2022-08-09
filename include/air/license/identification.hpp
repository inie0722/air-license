#pragma once

#include <json/value.h>

namespace air
{
    namespace license
    {
        class identification
        {
        public:
            identification() = default;

            virtual ~identification() = default;

            virtual Json::Value info() = 0;

            virtual Json::Value generate(const Json::Value &arg) = 0;

            virtual bool validate(const Json::Value &arg) = 0;
        };
    }
}
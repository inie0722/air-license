#pragma

#include <json/value.h>

namespace air
{
    namespace license
    {
        class identification_strategy
        {
        public:
            identification_strategy() = default;

            virtual ~identification_strategy() = default;

            virtual Json::Value info() = 0;

            virtual Json::Value generate(const Json::Value &arg) = 0;

            virtual bool validate(const Json::Value &arg) = 0;
        };
    }
}
#pragma once

#include "air/license/identification.hpp"
#include "air/license/tool.hpp"

#include <iomanip>
#include <chrono>
#include <sstream>

#include <boost/algorithm/string.hpp>

namespace air
{
    namespace license
    {
        class time_limit : public identification
        {
        public:
            time_limit() = default;

            virtual ~time_limit() = default;

            virtual Json::Value info()
            {
                std::stringstream ss;
                auto tm = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
                ss << std::put_time(std::localtime(&tm), "%F");

                return ss.str();
            }

            virtual Json::Value generate(const Json::Value &arg)
            {
                return arg;
            }

            virtual bool validate(const Json::Value &arg)
            {
                std::tm tm;
                std::stringstream ss(arg["end"].asString());
                ss >> std::get_time(&tm, "%F");
                auto tp = std::chrono::system_clock::from_time_t(std::mktime(&tm));

                return tp <= std::chrono::system_clock::now();
            }
        };
    }
}
#pragma once

#include "air/license/identification.hpp"
#include "air/license/utility.hpp"

#include <iomanip>
#include <sstream>
#include <chrono>
#include <string>
#include <cstring>
#include <cstddef>
#include <cstdint>
#include <optional>

namespace air
{
    namespace license
    {
        class time_limit : public identification
        {
        public:
            time_limit() = default;

            virtual ~time_limit() = default;

            virtual Json::Value info() override
            {
                std::stringstream ss;
                auto tm = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
                ss << std::put_time(std::localtime(&tm), "%Y-%m-%d");

                return ss.str();
            }

            virtual Json::Value generate(const Json::Value &arg) override
            {
                return arg;
            }

            virtual bool validate(const Json::Value &arg) override
            {
                using namespace std::chrono_literals;
                auto now = std::chrono::system_clock::now();

                std::chrono::time_point<std::chrono::system_clock, std::chrono::nanoseconds> begin;
                std::chrono::time_point<std::chrono::system_clock, std::chrono::nanoseconds> end;
                {
                    std::tm tm = {};
                    std::stringstream ss(arg["begin"].asString());
                    ss >> std::get_time(&tm, "%Y-%m-%d");
                    begin = std::chrono::system_clock::from_time_t(std::mktime(&tm));
                }
                {
                    std::tm tm = {};
                    std::stringstream ss(arg["end"].asString());
                    ss >> std::get_time(&tm, "%Y-%m-%d");
                    end = std::chrono::system_clock::from_time_t(std::mktime(&tm));
                }
                return now >= begin and now <= end;
            }
        };
    }
}
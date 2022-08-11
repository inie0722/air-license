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
                ss << std::put_time(std::localtime(&tm), "%Y-%m-%d");

                return ss.str();
            }

            virtual Json::Value generate(const Json::Value &arg)
            {
                return arg;
            }

            virtual bool validate(const Json::Value &arg)
            {
                using namespace std::chrono_literals;
                auto now = std::chrono::system_clock::now();
                std::chrono::time_point<std::chrono::system_clock> begin;
                std::chrono::time_point<std::chrono::system_clock> end;
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
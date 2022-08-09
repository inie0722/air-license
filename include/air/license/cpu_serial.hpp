#pragma once

#include "air/license/identification.hpp"
#include "air/license/tool.hpp"

#include <boost/algorithm/string.hpp>

namespace air
{
    namespace license
    {
        class cpu_serial : public identification
        {
        public:
            cpu_serial() = default;

            virtual ~cpu_serial() = default;

            virtual Json::Value info()
            {
                Json::Value ret;

                std::vector<std::string> strs;

                boost::split(strs, tool::system("cpuid | grep 'processor serial number = ' | awk '{print $5}'"), boost::is_any_of("\n"));

                int i = 0;
                for (auto &str : strs)
                {
                    if (str.length() > 0)
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
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

#include <boost/asio.hpp>
#include <boost/algorithm/string.hpp>

namespace air
{
    namespace license
    {
        class time_limit : public identification
        {
        private:
            struct ntp_packet
            {
                std::uint8_t li_vn_mode; // Eight bits. li, vn, and mode.
                                         // li.   Two bits.   Leap indicator.
                                         // vn.   Three bits. Version number of the protocol.
                                         // mode. Three bits. Client will pick mode 3 for client.

                std::uint8_t stratum;   // Eight bits. Stratum level of the local clock.
                std::uint8_t poll;      // Eight bits. Maximum interval between successive messages.
                std::uint8_t precision; // Eight bits. Precision of the local clock.

                std::uint32_t rootDelay;      // 32 bits. Total round trip delay time.
                std::uint32_t rootDispersion; // 32 bits. Max error aloud from primary clock source.
                std::uint32_t refId;          // 32 bits. Reference clock identifier.

                std::uint32_t refTm_s; // 32 bits. Reference time-stamp seconds.
                std::uint32_t refTm_f; // 32 bits. Reference time-stamp fraction of a second.

                std::uint32_t origTm_s; // 32 bits. Originate time-stamp seconds.
                std::uint32_t origTm_f; // 32 bits. Originate time-stamp fraction of a second.

                std::uint32_t rxTm_s; // 32 bits. Received time-stamp seconds.
                std::uint32_t rxTm_f; // 32 bits. Received time-stamp fraction of a second.

                std::uint32_t txTm_s; // 32 bits and the most important field the client cares about. Transmit time-stamp seconds.
                std::uint32_t txTm_f; // 32 bits. Transmit time-stamp fraction of a second.
            };

            std::optional<std::chrono::time_point<std::chrono::system_clock, std::chrono::nanoseconds>> get_ntp_time(const std::string &host, std::size_t max_size, std::chrono::milliseconds timeout)
            {
                using namespace boost::asio;

                io_context io_context;

                ip::udp::resolver resolver(io_context);
                ip::udp::resolver::query query(ip::udp::v4(), host, "ntp");

                ip::udp::socket socket(io_context, ip::udp::v4());
                boost::asio::system_timer timer(io_context);

                ip::udp::endpoint local;
                ip::udp::endpoint remote = *resolver.resolve(query);

                std::size_t i = 0;
                ntp_packet packet;

                std::function<void(const boost::system::error_code &)> timer_handle = [&](const boost::system::error_code &ec)
                {
                    if (ec)
                        return;

                    socket.cancel();
                };
                std::function<void(const boost::system::error_code &, size_t)> receive_handle;

                auto send = [&]()
                {
                    if (i < max_size)
                    {
                        ++i;
                        std::memset(&packet, 0, sizeof(ntp_packet));
                        *((char *)&packet + 0) = 0x1b;

                        socket.send_to(buffer(&packet, sizeof(ntp_packet)), remote);
                        socket.async_receive_from(buffer(&packet, sizeof(ntp_packet)), local, receive_handle);
                        timer.expires_from_now(timeout);
                        timer.async_wait(timer_handle);
                    }
                };

                receive_handle = [&](const boost::system::error_code &ec, size_t)
                {
                    if (ec)
                    {

                        send();
                    }
                    else
                    {
                        timer.cancel();
                    }
                };

                send();
                io_context.run();

                if (packet.rxTm_s == 0)
                {
                    return std::nullopt;
                }
                else
                {
                    std::chrono::time_point<std::chrono::system_clock, std::chrono::nanoseconds> ret;
                    ret += std::chrono::seconds(ntohl(packet.rxTm_s) - 2208988800ull) + std::chrono::nanoseconds(ntohl(packet.rxTm_f));

                    return ret;
                }
            }

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
                auto now = get_ntp_time({AIR_ENCRYPT_STRING("pool.ntp.org")}, 10, 1s);
                if (now == std::nullopt)
                    return false;

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
                return *now >= begin and *now <= end;
            }
        };
    }
}
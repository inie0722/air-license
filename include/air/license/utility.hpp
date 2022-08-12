#pragma once

#include <cstddef>
#include <memory>
#include <string>
#include <array>
#include <stdexcept>
#include <utility>

#include <stdio.h>

#include <openssl/bio.h>
#include <openssl/evp.h>

#define AIR_ENCRYPT_STRING(_string) []() {                             \
    constexpr std::size_t key =                                        \
        []() {                                                         \
            std::size_t sum = 0;                                       \
            for (std::size_t i = 0; i < sizeof(__TIME__); i++)         \
            {                                                          \
                sum += __TIME__[i] * 0xc6a4a7935bd1e995ULL ^ __LINE__; \
            }                                                          \
                                                                       \
            for (std::size_t i = 0; i < sizeof(__DATE__); i++)         \
            {                                                          \
                sum += __DATE__[i] * 0xc6a4a7935bd1e995ULL ^ __LINE__; \
            }                                                          \
                                                                       \
            for (std::size_t i = 0; i < sizeof(__FILE__); i++)         \
            {                                                          \
                sum += __FILE__[i] * 0xc6a4a7935bd1e995ULL ^ __LINE__; \
            }                                                          \
            return sum;                                                \
        }();                                                           \
                                                                       \
    struct A                                                           \
    {                                                                  \
        bool is_encrypted = true;                                      \
        std::array<char, sizeof(_string)> data = {};                   \
        constexpr A()                                                  \
        {                                                              \
            for (std::size_t i = 0; i < data.size(); i++)              \
            {                                                          \
                data[i] = _string[i] ^ key;                            \
            }                                                          \
        }                                                              \
                                                                       \
        operator const char *()                                        \
        {                                                              \
            if (is_encrypted)                                          \
            {                                                          \
                is_encrypted = false;                                  \
                for (std::size_t i = 0; i < data.size(); i++)          \
                {                                                      \
                    data[i] ^= key;                                    \
                }                                                      \
            }                                                          \
                                                                       \
            return data.data();                                        \
        }                                                              \
    } constexpr ret;                                                   \
    return ret;                                                        \
}()

namespace air
{
    namespace license
    {
        namespace utility
        {
            std::string base64_encode(void *data, std::size_t size)
            {
                std::size_t base64_len = 4 * ((size + 2) / 3);
                auto base64_sign = std::make_unique<unsigned char[]>(base64_len + 1);
                EVP_EncodeBlock(base64_sign.get(), (unsigned char *)data, size);
                return std::string((char *)base64_sign.get());
            }

            std::pair<std::unique_ptr<unsigned char[]>, std::size_t> base64_decode(const std::string &base64)
            {
                std::size_t sign_len = base64.length() * 6 / 8;
                auto sign = std::make_unique<unsigned char[]>(sign_len);
                EVP_DecodeBlock(sign.get(), (unsigned char *)base64.c_str(), base64.length());

                return {std::move(sign), sign_len - 1};
            }

            std::pair<std::string, int> system(const std::string &cmd)
            {
                std::array<char, 512> buffer;
                std::string result;

                FILE *pipe = popen(cmd.c_str(), "r");
                if (!pipe)
                {
                    throw std::runtime_error("popen() failed!");
                }
                while (fgets(buffer.data(), buffer.size(), pipe) != nullptr)
                {
                    result += buffer.data();
                }

                return {result, pclose(pipe)};
            }
        }
    }
}
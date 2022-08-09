#pragma once

#include <cstddef>
#include <memory>
#include <string>
#include <array>
#include <stdexcept>

#include <stdio.h>

#include <openssl/bio.h>
#include <openssl/evp.h>

namespace air
{
    namespace license
    {
        namespace tool
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

            std::string system(const std::string &cmd)
            {
                std::array<char, 4096> buffer;
                std::string result;
                std::unique_ptr<FILE, decltype(&pclose)> pipe(popen(cmd.c_str(), "r"), pclose);
                if (!pipe)
                {
                    throw std::runtime_error("popen() failed!");
                }
                while (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr)
                {
                    result += buffer.data();
                }
                return result;
            }
        }

    }
}
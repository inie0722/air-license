#pragma once

#include <stdexcept>
#include <unordered_map>
#include <string>
#include <vector>
#include <initializer_list>
#include <memory>
#include <map>
#include <cstddef>

#include <json/json.h>

#include <openssl/rsa.h>
#include <openssl/pem.h>
#include <openssl/bio.h>

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

        class license_manager
        {
        private:
            std::unordered_map<std::string, std::shared_ptr<identification_strategy>> identification_strategy_;
            std::string key_;

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

            std::string generate_signature(const std::string &date)
            {
                //生成数字签名
                auto bio = BIO_new(BIO_s_mem());
                auto ctx = EVP_MD_CTX_new();

                BIO_puts(bio, key_.c_str());

                auto evpkey = PEM_read_bio_PrivateKey(bio, nullptr, 0, nullptr);
                EVP_DigestSignInit(ctx, nullptr, EVP_sha256(), nullptr, evpkey);

                EVP_DigestSignUpdate(ctx, date.c_str(), date.length());

                size_t sign_len;
                EVP_DigestSignFinal(ctx, nullptr, &sign_len);
                auto sign = std::make_unique<unsigned char[]>(sign_len);
                EVP_DigestSignFinal(ctx, sign.get(), &sign_len);

                EVP_MD_CTX_free(ctx);
                EVP_PKEY_free(evpkey);
                BIO_free(bio);

                return base64_encode(sign.get(), sign_len);
            }

            bool validate_signature(const std::string &signature, const std::string &date)
            {
                //验证数字签名
                auto bio = BIO_new(BIO_s_mem());
                auto ctx = EVP_MD_CTX_new();

                BIO_puts(bio, key_.c_str());
                auto evpkey = PEM_read_bio_PUBKEY(bio, NULL, 0, NULL);
                EVP_DigestVerifyInit(ctx, NULL, EVP_sha256(), NULL, evpkey);

                EVP_DigestVerifyUpdate(ctx, date.c_str(), date.length());

                auto sign = base64_decode(signature);
                auto ret = EVP_DigestVerifyFinal(ctx, sign.first.get(), sign.second);

                EVP_MD_CTX_free(ctx);
                EVP_PKEY_free(evpkey);
                BIO_free(bio);

                return ret;
            }

        public:
            license_manager(const std::string &key)
                : key_(key)
            {
            }

            ~license_manager() = default;

            void add_identification_strategy(const std::string &strategy_name, const std::shared_ptr<identification_strategy> &strategy)
            {
                if (identification_strategy_.find(strategy_name) != identification_strategy_.end())
                    throw std::runtime_error("Duplicate addition strategy");

                identification_strategy_[strategy_name] = strategy;
            }

            Json::Value generate(const Json::Value &arg)
            {
                Json::Value ret;

                for (auto &strategy : arg.getMemberNames())
                {
                    ret[strategy] = identification_strategy_[strategy]->generate(arg[strategy]);
                }

                ret["signature"] = generate_signature(ret.toStyledString());
                return ret;
            }

            bool validate(Json::Value arg)
            {
                std::string signature = arg["signature"].asString();
                arg.removeMember("signature");
                return validate_signature(signature, arg.toStyledString());
            }
        };
    }
}
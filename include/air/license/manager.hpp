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
#include <openssl/evp.h>

#include "air/license/identification.hpp"
#include "air/license/utility.hpp"

namespace air
{
    namespace license
    {
        class manager
        {
        private:
            std::unordered_map<std::string, std::shared_ptr<identification>> identification_;
            std::string key_;

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

                return utility::base64_encode(sign.get(), sign_len);
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

                auto sign = utility::base64_decode(signature);
                auto ret = EVP_DigestVerifyFinal(ctx, sign.first.get(), sign.second);

                EVP_MD_CTX_free(ctx);
                EVP_PKEY_free(evpkey);
                BIO_free(bio);

                return ret;
            }

        public:
            manager() = default;

            manager(const std::string &key)
                : key_(key)
            {
            }

            ~manager() = default;

            void add_identification(const std::string &strategy_name, const std::shared_ptr<identification> &strategy)
            {
                if (identification_.find(strategy_name) != identification_.end())
                    throw std::runtime_error("Duplicate addition strategy");

                identification_[strategy_name] = strategy;
            }

            Json::Value info()
            {
                Json::Value ret;

                for (auto &strategy : identification_)
                {
                    ret[strategy.first] = strategy.second->info();
                }

                return ret;
            }

            Json::Value generate(const Json::Value &arg)
            {
                Json::Value ret;

                for (auto &strategy : arg.getMemberNames())
                {
                    ret[strategy] = identification_[strategy]->generate(arg[strategy]);
                }

                ret["signature"] = generate_signature(ret.toStyledString());
                return ret;
            }

            bool validate(Json::Value arg)
            {
                std::string signature = arg["signature"].asString();
                arg.removeMember("signature");
                if (!validate_signature(signature, arg.toStyledString()))
                {

                    return false;
                }
                else
                {
                    for (auto &strategy : arg.getMemberNames())
                    {
                        if (!identification_[strategy]->validate(arg[strategy]))
                            return false;
                    }

                    return true;
                }
            }
        };
    }
}
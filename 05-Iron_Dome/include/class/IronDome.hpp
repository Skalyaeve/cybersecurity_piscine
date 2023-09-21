#ifndef IRONDOME
#define IRONDOME

#define VERSION 1
#define DATA_DIR "data"
#define MEM_LIMIT 100   // MB
#define BUFFERSIZE 1024 // BUFFERSIZE * (event_size + 16)
#define SLEEPTIME 30
#define CRYPTO_CMD_FILE "crypto_list.txt"
#define ENTROPY_FILE "entropy.txt"
#define ENTROPY_TEMP_FILE "entropy.tmp"

#define DEFAULT_ROOT "."
#define DEFAULT_LOGFILE "var/log/IronDome/IronDome.log"
#define DEFAULT_READ_TRESHOLD 100
#define DEFAULT_ENTROPY_TRESHOLD 0.9

#include <iostream>
#include <fstream>
#include <vector>
#include <map>
#include <algorithm>
#include <cmath>
#include <cstring>
#include <cerrno>
#include <unistd.h>
#include <dirent.h>
#include <sys/stat.h>
#include <sys/resource.h>
#include <sys/inotify.h>

class IronDome
{
public:
        IronDome();
        IronDome(const IronDome &other);
        IronDome(const std::vector<std::string> *workin_dirs,
                 const std::string *logfile,
                 const unsigned short *read_treshold,
                 const float *entropy_treshold);
        virtual ~IronDome();

        IronDome &operator=(const IronDome &other);

        const std::vector<std::string> &get_workin_dirs() const;
        const std::string &get_logfile() const;
        const unsigned short &get_read_treshold() const;
        const float &get_entropy_treshold() const;

        void set_workin_dirs(const std::vector<std::string> &workin_dirs);
        void set_logfile(const std::string &workin_dirs);
        void set_read_treshold(const unsigned short &read_treshold);
        void set_entropy_treshold(const float &entropy_treshold);

        void print_version() const;
        int launch();

protected:
        std::vector<std::string> _workin_dirs;
        std::string _logfile;
        unsigned short _read_treshold;
        float _entropy_treshold;

        std::string _logs;
        std::map<int, std::string> _watch_map;
        int _inotify_fd;

        void _save_logs();
        bool _create_dir(const std::string &path) const;
        std::vector<std::string> _split_str(const std::string &src, char delimiter) const;

        void _work();
        void _inotify_check(struct inotify_event *event, std::map<std::string, unsigned short> &read_counter);
        float _calc_entropy(std::vector<unsigned char> &bytes);
        void _crypto_cmd_check();
        std::vector<std::string> _get_running_process();
        bool _is_crypto_cmd(const std::string &cmd) const;
};
#endif
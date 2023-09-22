#ifndef IRONDOME
#define IRONDOME

#define VERSION 1
#define MEM_LIMIT 100   // MB
#define BUFFERSIZE 1024 // BUFFERSIZE * (event_size + 16) bytes
#define SLEEPTIME 30    // seconds
#define DATA_DIR "/mnt/d/Code/42/Security/Cybersecurity Piscine/05-Iron_Dome/data"
#define CRYPTO_CMD_FILE "crypto_list.txt"
#define ENTROPY_FILE "entropy.txt"
#define ENTROPY_TEMP_FILE "entropy.tmp"

#define DEFAULT_ROOT "/mnt/d/Code/42/Security/Cybersecurity Piscine/05-Iron_Dome"
#define DEFAULT_LOGFILE "/mnt/d/Code/42/Security/Cybersecurity Piscine/05-Iron_Dome/data/log/IronDome.log"
#define DEFAULT_READ_TRESHOLD 100
#define DEFAULT_CRYPTO_USE_TRESHOLD 10
#define DEFAULT_ENTROPY_TRESHOLD 0.9

#include <iostream>
#include <fstream>
#include <vector>
#include <unordered_map>
#include <cmath>
#include <cstring>
#include <unistd.h>
#include <dirent.h>
#include <fcntl.h>
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
                 const std::string *backup_dir,
                 const size_t *backup_interval,
                 const unsigned short *read_treshold,
                 const unsigned short *crypto_use_treshold,
                 const float *entropy_treshold);
        virtual ~IronDome();

        IronDome &operator=(const IronDome &other);

        const std::vector<std::string> &get_workin_dirs() const;
        const std::string &get_logfile() const;
        const std::string &get_backup_dir() const;
        const size_t &get_backup_interval() const;
        const unsigned short &get_read_treshold() const;
        const unsigned short &get_crypto_use_treshold() const;
        const float &get_entropy_treshold() const;

        void set_workin_dirs(const std::vector<std::string> &workin_dirs);
        void set_logfile(const std::string &workin_dirs);
        void set_backup_dir(const std::string &backup_dir);
        void set_backup_interval(const size_t &backup_interval);
        void set_read_treshold(const unsigned short &read_treshold);
        void set_crypto_use_treshold(const unsigned short &crypto_use_treshold);
        void set_entropy_treshold(const float &entropy_treshold);

        void print_version() const;
        int launch();

protected:
        std::vector<std::string> _workin_dirs;
        std::string _logfile;
        std::string _backup_dir;
        unsigned short _read_treshold;
        unsigned short _crypto_use_treshold;
        float _entropy_treshold;
        size_t _backup_interval;

        std::string _logs;
        std::unordered_map<int, std::string> _watch_map;
        int _inotify_fd;
        size_t _my_time;

        void _save_logs();
        void _add_watches(const std::string &path);
        void _work();
        void _inotify_check(struct inotify_event *event, std::unordered_map<std::string, unsigned short> &read_counter);
        void _update_entropy(const float &entropy, const std::string &path);
        float _calc_entropy(std::vector<unsigned char> &bytes);
        std::vector<std::string> _split_str(const std::string &src, char delimiter) const;
        bool _is_file(const std::string &path) const;
        void _do_backup();
        void _fill_that_repo(const std::string &dir, const std::string &backup_path);
};

#endif
#include "../../include/class/IronDome.hpp"

IronDome::IronDome()
    : _workin_dirs(std::vector<std::string>(1, DEFAULT_ROOT)),
      _logfile(DEFAULT_LOGFILE),
      _read_treshold(DEFAULT_READ_TRESHOLD),
      _backup_interval(0),
      _crypto_use_treshold(DEFAULT_CRYPTO_USE_TRESHOLD),
      _entropy_treshold(DEFAULT_ENTROPY_TRESHOLD)
{
        const char *env_home = std::getenv("HOME");
        this->_backup_dir = env_home ? std::string(env_home) : "/";
}

IronDome::IronDome(const IronDome &other)
    : _workin_dirs(other._workin_dirs),
      _logfile(other._logfile),
      _backup_dir(other._backup_dir),
      _backup_interval(other._backup_interval),
      _read_treshold(other._read_treshold),
      _crypto_use_treshold(other._crypto_use_treshold),
      _entropy_treshold(other._entropy_treshold)
{
}

IronDome::IronDome(const std::vector<std::string> *workin_dirs,
                   const std::string *logfile,
                   const std::string *backup_dir,
                   const size_t *backup_interval,
                   const unsigned short *read_treshold,
                   const unsigned short *crypto_use_treshold,
                   const float *entropy_treshold)
    : _workin_dirs(workin_dirs ? *workin_dirs : std::vector<std::string>(1, DEFAULT_ROOT)),
      _logfile(logfile ? *logfile : DEFAULT_LOGFILE),
      _backup_interval(backup_interval ? *backup_interval : 0),
      _read_treshold(read_treshold ? *read_treshold : DEFAULT_READ_TRESHOLD),
      _crypto_use_treshold(crypto_use_treshold ? *crypto_use_treshold : DEFAULT_CRYPTO_USE_TRESHOLD),
      _entropy_treshold(entropy_treshold ? *entropy_treshold : DEFAULT_ENTROPY_TRESHOLD)
{
        const char *env_home = std::getenv("HOME");
        this->_backup_dir = backup_dir ? *backup_dir : (env_home ? std::string(env_home) : "/");
}

IronDome::~IronDome()
{
        this->_save_logs();
        exit(errno);
}

IronDome &IronDome::operator=(const IronDome &other)
{
        if (this != &other)
        {
                this->_workin_dirs = other._workin_dirs;
                this->_logfile = other._logfile;
                this->_backup_dir = other._backup_dir;
                this->_backup_interval = other._backup_interval;
                this->_read_treshold = other._read_treshold;
                this->_crypto_use_treshold = other._crypto_use_treshold;
                this->_entropy_treshold = other._entropy_treshold;
        }
        return *this;
}

const std::vector<std::string> &IronDome::get_workin_dirs() const
{
        return this->_workin_dirs;
}

const std::string &IronDome::get_logfile() const
{
        return this->_logfile;
}

const std::string &IronDome::get_backup_dir() const
{
        return this->_backup_dir;
}

const size_t &IronDome::get_backup_interval() const
{
        return this->_backup_interval;
}

const unsigned short &IronDome::get_read_treshold() const
{
        return this->_read_treshold;
}

const unsigned short &IronDome::get_crypto_use_treshold() const
{
        return this->_crypto_use_treshold;
}

const float &IronDome::get_entropy_treshold() const
{
        return this->_entropy_treshold;
}

void IronDome::set_workin_dirs(const std::vector<std::string> &workin_dirs)
{
        this->_workin_dirs = workin_dirs;
}

void IronDome::set_logfile(const std::string &logfile)
{
        this->_logfile = logfile;
}

void IronDome::set_backup_dir(const std::string &backup_dir)
{
        this->_backup_dir = backup_dir;
}

void IronDome::set_backup_interval(const size_t &backup_interval)
{
        this->_backup_interval = backup_interval;
}

void IronDome::set_read_treshold(const unsigned short &read_treshold)
{
        this->_read_treshold = read_treshold;
}

void IronDome::set_crypto_use_treshold(const unsigned short &crypto_use_treshold)
{
        this->_crypto_use_treshold = crypto_use_treshold;
}

void IronDome::set_entropy_treshold(const float &entropy_treshold)
{
        this->_entropy_treshold = entropy_treshold;
}

void IronDome::print_version() const
{
        std::cout << "Version " << VERSION << std::endl;
}

int IronDome::launch()
{
        pid_t pid = fork();
        if (pid < 0)
                return 1;
        if (pid > 0)
                return 0;

        pid_t sid = setsid();
        std::cout << "[ PID " << sid << " ] Iron Dome daemon running." << std::endl;
        if (sid < 0)
                exit(-1);

        umask(0);
        close(STDIN_FILENO);
        close(STDOUT_FILENO);
        close(STDERR_FILENO);
        if (chdir("/"))
                exit(errno);
        this->_my_time = 0;
        this->_logs = "----------------| H E L L O   W O R L D |----------------\n";

        struct rlimit mem_limit;
        mem_limit.rlim_cur = mem_limit.rlim_max = MEM_LIMIT * 1024 * 1024;
        if (setrlimit(RLIMIT_AS, &mem_limit) == -1)
        {
                this->_logs += "[ ERROR ] setrlimit() failed: " + std::string(strerror(errno)) + '\n';
                this->~IronDome();
        }
        this->_logs += "[ INFO ] Memory usage limit set to " + std::to_string(MEM_LIMIT) + " MB.\n";

        this->_inotify_fd = inotify_init();
        if (this->_inotify_fd < 0)
        {
                this->_logs += "[ ERROR ] inotify_init() failed: " + std::string(strerror(errno)) + '\n';
                this->~IronDome();
        }
        int flags = fcntl(this->_inotify_fd, F_GETFL, 0);
        if (flags == -1)
        {
                this->_logs += "[ ERROR ] fcntl(F_GETFL) failed: " + std::string(strerror(errno)) + '\n';
                this->~IronDome();
        }
        if (fcntl(this->_inotify_fd, F_SETFL, flags | O_NONBLOCK) == -1)
        {
                this->_logs += "[ ERROR ] fcntl(F_SETFL) failed: " + std::string(strerror(errno)) + '\n';
                this->~IronDome();
        }
        this->_logs += "[ INFO ] inotify_fd set: " + std::to_string(this->_inotify_fd) + '\n';

        for (std::string dir : this->_workin_dirs)
                this->_add_watches(dir);
        this->_work();
        return -1;
}

void IronDome::_save_logs()
{
        if (!this->_logs.length())
                return;

        std::ofstream logfile(this->_logfile, std::ios_base::app);
        if (!logfile.is_open())
                return;
        logfile << this->_logs;
        logfile.close();
        this->_logs.clear();
}

std::vector<std::string> IronDome::_split_str(const std::string &src, char delimiter) const
{
        std::vector<std::string> tokens;
        std::string token;
        size_t end;
        size_t pos = 0;

        while ((end = src.find(delimiter, pos)) != std::string::npos)
        {
                token = src.substr(pos, end - pos);
                tokens.push_back(token);
                pos = end + 1;
        }
        token = src.substr(pos);
        tokens.push_back(token);
        return tokens;
}

void IronDome::_add_watches(const std::string &path)
{
        const int wd = inotify_add_watch(this->_inotify_fd, path.c_str(), IN_ACCESS | IN_MODIFY);
        if (wd < 0)
        {
                this->_logs += "[ ERROR ] inotify_add_watch(" + path + ") failed: " + std::string(strerror(errno)) + '\n';
                return;
        }
        this->_watch_map[wd] = path;
        this->_logs += "[ INFO ] Watching: " + path + '\n';
        this->_save_logs();

        DIR *dir = opendir(path.c_str());
        if (!dir)
        {
                this->_logs += "[ ERROR ] opendir(" + path + ") failed: " + std::string(strerror(errno)) + '\n';
                return;
        }
        struct dirent *entry;
        while ((entry = readdir(dir)))
                if (entry->d_type == DT_DIR && strcmp(entry->d_name, ".") && strcmp(entry->d_name, ".."))
                        this->_add_watches(path + '/' + entry->d_name);
        closedir(dir);
}

void IronDome::_work()
{
        const int event_size = sizeof(struct inotify_event);
        const int buf_len = BUFFERSIZE * (event_size + 16);
        char buffer[buf_len];

        std::unordered_map<std::string, unsigned short> read_counter;
        size_t length;
        size_t i;
        struct inotify_event *event;

        this->_logs += "[ INFO ] Ready.\n";
        this->_save_logs();
        while (true)
        {
                if (this->_backup_interval > 0)
                {
                        this->_my_time++;
                        if (this->_my_time * (SLEEPTIME / 60) >= this->_backup_interval)
                        {
                                this->_my_time = 0;
                                this->_do_backup();
                        }
                }
                read_counter.clear();
                while ((length = read(this->_inotify_fd, buffer, buf_len)) > 0)
                {
                        i = 0;
                        while (i < length)
                        {
                                event = reinterpret_cast<struct inotify_event *>(&buffer[i]);
                                this->_inotify_check(event, read_counter);
                                i += event_size + event->len;
                        }
                        memset(buffer, 0, length);
                        this->_save_logs();
                }
                if (length < 0 && (errno != EAGAIN && errno != EWOULDBLOCK))
                        this->_logs += "[ ERROR ] read() failed: " + std::string(strerror(errno)) + '\n';

                for (auto &it : read_counter)
                        if (!system(std::string("grep " + it.first + " < " + DATA_DIR + '/' + CRYPTO_CMD_FILE).c_str()) &&
                            it.second > this->_crypto_use_treshold)
                                this->_logs += "[ WARNING ] CRYPTO: " + it.first + '\n';
                this->_save_logs();
                sleep(SLEEPTIME);
        }
        close(this->_inotify_fd);
        for (auto &it : this->_watch_map)
                close(it.first);
        exit(EXIT_SUCCESS);
}

void IronDome::_inotify_check(struct inotify_event *event, std::unordered_map<std::string, unsigned short> &read_counter)
{
        if (event->mask & IN_ACCESS)
        {
                if (read_counter[event->name] > this->_read_treshold)
                        return;
                read_counter[event->name]++;
                if (read_counter[event->name] > this->_read_treshold)
                        this->_logs += "[ WARNING ] READ: " + std::string(event->name) + '\n';
        }
        else if (event->mask & IN_MODIFY)
        {
                const std::string filePath = this->_watch_map[event->wd] + '/' + event->name;
                if (!this->_is_file(filePath))
                        return;
                std::ifstream file(filePath, std::ios::binary);
                if (!file.is_open())
                {
                        this->_logs += "[ ERROR ] Can't open " + filePath + ": " + std::string(strerror(errno)) + '\n';
                        return;
                }
                std::vector<unsigned char> bytes(
                    (std::istreambuf_iterator<char>(file)),
                    (std::istreambuf_iterator<char>()));
                file.close();
                this->_update_entropy(this->_calc_entropy(bytes), filePath);
        }
}

float IronDome::_calc_entropy(std::vector<unsigned char> &bytes)
{
        std::unordered_map<unsigned char, unsigned int> frequency;
        for (unsigned char byte : bytes)
                frequency[byte]++;

        float entropy = 0;
        for (auto &it : frequency)
        {
                float prob = static_cast<float>(it.second) / bytes.size();
                entropy += prob * log2(prob);
        }
        return -entropy;
}

void IronDome::_update_entropy(const float &entropy, const std::string &path)
{
        std::string entropy_file(DATA_DIR);
        entropy_file += '/';
        entropy_file += ENTROPY_FILE;
        std::string entropy_temp_file(DATA_DIR);
        entropy_temp_file += '/';
        entropy_temp_file += ENTROPY_TEMP_FILE;

        if (system(std::string("grep " + path + " < " + entropy_file + " > " + entropy_temp_file).c_str()))
        {
                this->_logs += "[ INFO ] Adding entropy record for " + path + '\n';
                std::ofstream outfile(entropy_file, std::ios_base::app);
                if (!outfile.is_open())
                {
                        this->_logs += "[ ERROR ] Can't open " + entropy_file + ": " + std::string(strerror(errno)) + '\n';
                        return;
                }
                outfile << path << ':' << std::to_string(entropy) << std::endl;
                outfile.close();
                return;
        }

        std::ifstream file = std::ifstream(entropy_temp_file);
        if (!file.is_open())
        {
                this->_logs += "[ ERROR ] Can't open " + entropy_temp_file + ": " + std::string(strerror(errno)) + '\n';
                return;
        }
        std::string line;
        std::getline(file, line);
        file.close();
        if (!line.length())
        {
                this->_logs += "[ ERROR ] " + entropy_temp_file + " corrupted.\n";
                return;
        }
        std::vector<std::string> content = this->_split_str(line, ':');

        if (content.size() != 2 || content[0] != path)
                this->_logs += "[ ERROR ] " + entropy_temp_file + " corrupted.\n";
        else if (std::stof(content[1]) < entropy)
                this->_logs += "[ WARNING ] ENTROPY: " + path + '\n';
        system(std::string("tr \"" + path + ":" + content[1] + "\" \"" + path + ":" + std::to_string(entropy) + "\"").c_str());
}

bool IronDome::_is_file(const std::string &path) const
{
        struct stat path_stat;
        if (stat(path.c_str(), &path_stat) == -1)
                return false;
        return S_ISREG(path_stat.st_mode);
}

void IronDome::_do_backup()
{
        std::string backup_dir = this->_backup_dir + '/' + std::to_string(time(NULL));
        if (mkdir(backup_dir.c_str(), 0777) == -1)
        {
                this->_logs += "[ ERROR ] mkdir(" + backup_dir + ") failed: " + std::string(strerror(errno)) + '\n';
                return;
        }
        for (std::string dir : this->_workin_dirs)
        {
                std::string backup_path = backup_dir + dir.substr(dir.find_last_of('/'));
                if (mkdir(backup_path.c_str(), 0777) == -1)
                {
                        this->_logs += "[ ERROR ] mkdir(" + backup_path + ") failed: " + std::string(strerror(errno)) + '\n';
                        continue;
                }
                _fill_that_repo(dir, backup_path);
        }
}

void IronDome::_fill_that_repo(const std::string &dir, const std::string &backup_path)
{
        DIR *dirp = opendir(dir.c_str());
        if (!dirp)
        {
                this->_logs += "[ ERROR ] opendir(" + dir + ") failed: " + std::string(strerror(errno)) + '\n';
                return;
        }
        struct dirent *entry;
        while ((entry = readdir(dirp)))
        {
                if (std::string(entry->d_name) == "." || std::string(entry->d_name) == "..")
                        continue;
                std::string file_path = dir + '/' + entry->d_name;
                std::string backup_file_path = backup_path + '/' + entry->d_name;

                if (entry->d_type == DT_DIR)
                {
                        if (mkdir(backup_file_path.c_str(), 0777) == -1)
                        {
                                this->_logs += "[ ERROR ] mkdir(" + backup_file_path + ") failed: " + std::string(strerror(errno)) + '\n';
                                continue;
                        }
                        _fill_that_repo(file_path, backup_file_path);
                }
                else
                {
                        std::ifstream src(file_path, std::ios::binary);
                        if (!src.is_open())
                        {
                                this->_logs += "[ ERROR ] Can't open " + file_path + ": " + std::string(strerror(errno)) + '\n';
                                continue;
                        }
                        std::ofstream dst(backup_file_path, std::ios::binary);
                        if (!dst.is_open())
                        {
                                src.close();
                                this->_logs += "[ ERROR ] Can't open " + backup_file_path + ": " + std::string(strerror(errno)) + '\n';
                                continue;
                        }
                        dst << src.rdbuf();
                        src.close();
                        dst.close();
                }
        }
        closedir(dirp);
}
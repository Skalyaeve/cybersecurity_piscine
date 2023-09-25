#include "../../include/class/IronDome.hpp"

IronDome::IronDome()
    : _workin_dirs(std::vector<std::string>(1, DEFAULT_ROOT)),
      _logfile(DEFAULT_LOGFILE),
      _backup_interval(DEFAULT_BACKUP_INTERVAL),
      _read_treshold(DEFAULT_READ_TRESHOLD),
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
      _backup_interval(backup_interval ? *backup_interval : DEFAULT_BACKUP_INTERVAL),
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
        this->_logs = "----------------| I R O N   D O M E |----------------\n";

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
        this->_watch_crypto_cmds();
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

bool IronDome::_is_file(const std::string &path) const
{
        struct stat path_stat;
        if (stat(path.c_str(), &path_stat) == -1)
                return false;
        return S_ISREG(path_stat.st_mode);
}

std::vector<std::string> IronDome::_split_str(const std::string &src, const char &delimiter) const
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
        struct stat s;
        if (stat(path.c_str(), &s) == 0)
        {
                if ((s.st_mode & S_IFMT) == S_IFDIR)
                {
                        const int wd = inotify_add_watch(this->_inotify_fd, path.c_str(), IN_ACCESS | IN_MODIFY | IN_CREATE);
                        if (wd < 0)
                        {
                                this->_logs += "[ ERROR ] inotify_add_watch(" + path + ") failed: " + std::string(strerror(errno)) + '\n';
                                return;
                        }
                        this->_watch_map[wd] = path;
                        this->_logs += "[ INFO ] Watching directory: " + path + '\n';

                        DIR *dir = opendir(path.c_str());
                        if (!dir)
                        {
                                this->_logs += "[ ERROR ] opendir(" + path + ") failed: " + std::string(strerror(errno)) + '\n';
                                return;
                        }

                        struct dirent *entry;
                        while ((entry = readdir(dir)))
                        {
                                if (entry->d_type == DT_DIR && strcmp(entry->d_name, ".") && strcmp(entry->d_name, ".."))
                                        this->_add_watches(path + '/' + entry->d_name);
                                else if (entry->d_type == DT_REG)
                                        this->_add_watches(path + '/' + entry->d_name);
                        }
                        closedir(dir);
                }
                else if ((s.st_mode & S_IFMT) == S_IFREG)
                {
                        const int wd = inotify_add_watch(this->_inotify_fd, path.c_str(), IN_ACCESS | IN_MODIFY);
                        if (wd < 0)
                        {
                                this->_logs += "[ ERROR ] inotify_add_watch(" + path + ") failed: " + std::string(strerror(errno)) + '\n';
                                return;
                        }
                        this->_watch_map[wd] = path;
                        this->_logs += "[ INFO ] Watching file: " + path + '\n';
                }
        }
        else
                this->_logs += "[ ERROR ] stat(" + path + ") failed: " + std::string(strerror(errno)) + '\n';
}

void IronDome::_watch_crypto_cmds()
{
        std::string path(DATA_DIR);
        path += '/';
        path += CRYPTO_CMD_FILE;
        std::ifstream file(path);
        if (!file.is_open())
        {
                this->_logs += "[ ERROR ] Can't open " + path + ": " + std::string(strerror(errno)) + '\n';
                return;
        }

        struct stat s;
        std::string line;
        while (std::getline(file, line))
        {
                path = "/usr/bin/" + line;
                if (!stat(path.c_str(), &s))
                        this->_add_watches(path);
        }
        file.close();
}

void IronDome::_work()
{
        const int event_size = sizeof(struct inotify_event);
        const int buf_len = BUFFERSIZE * (event_size + 16);
        char buffer[buf_len];

        std::unordered_map<std::string, unsigned short> read_counter;
        ssize_t length;
        ssize_t i;
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
                                this->_logs += "[ INFO ] Backup done.\n";
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

                this->_check_crypto_use(read_counter);
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
        const std::string path = this->_watch_map[event->wd];

        if (event->mask & IN_ACCESS)
        {
                if (read_counter[path] > this->_read_treshold)
                        return;
                read_counter[path]++;
                if (read_counter[path] > this->_read_treshold)
                        this->_logs += "[ WARNING ] READ: " + std::string(path) + '\n';
        }
        else if (event->mask & IN_MODIFY)
                this->_launch_entropy_update(path);
        else if (event->mask & IN_CREATE)
        {
                const std::string new_item = this->_watch_map[event->wd] + '/' + std::string(event->name);
                this->_launch_entropy_update(new_item);

                for (const auto &it : this->_watch_map)
                        if (it.second == new_item)
                                return;
                const int wd = inotify_add_watch(this->_inotify_fd, new_item.c_str(), IN_ACCESS | IN_MODIFY);
                if (wd >= 0)
                {
                        this->_watch_map[wd] = new_item;
                        this->_logs += "[ INFO ] Watching new file: " + new_item + '\n';
                }
                else
                        this->_logs += "[ ERROR ] inotify_add_watch(" + new_item + ") failed: " + std::string(strerror(errno)) + '\n';
        }
}

void IronDome::_launch_entropy_update(const std::string &path)
{
        if (!this->_is_file(path))
                return;
        std::ifstream file(path, std::ios::binary);
        if (!file.is_open())
        {
                this->_logs += "[ ERROR ] Can't open " + path + ": " + std::string(strerror(errno)) + '\n';
                return;
        }
        std::vector<unsigned char> bytes(
            (std::istreambuf_iterator<char>(file)),
            (std::istreambuf_iterator<char>()));
        file.close();
        this->_update_entropy(this->_calc_entropy(bytes), path);
}

float IronDome::_calc_entropy(std::vector<unsigned char> &bytes) const
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
        this->_save_logs();
        std::string entropy_file(DATA_DIR);
        entropy_file += '/';
        entropy_file += ENTROPY_FILE;
        const std::string rec = this->_check_entropy_file(entropy_file, path, entropy);
        if (rec.empty())
                return;

        std::vector<std::string> content = this->_split_str(rec, ':');
        if (content.size() != 2 || content[0] != path)
        {
                this->_logs += "[ ERROR ] " + entropy_file + " corrupted.\n";
                return;
        }
        else if (entropy - std::stof(content[1]) > this->_entropy_treshold)
                this->_logs += "[ WARNING ] ENTROPY: " + path + '\n';

        this->_update_entropy_file(entropy_file, path, entropy, content[1]);
}

void IronDome::_update_entropy_file(const std::string &entropy_file, const std::string &path, const float &entropy, const std::string &old_entropy)
{
        std::string temp_file(DATA_DIR);
        temp_file += '/';
        temp_file += ENTROPY_FILE;
        temp_file += ".tmp";

        std::ifstream infile(entropy_file);
        std::ofstream outfile(temp_file);
        std::string line;

        if (!infile.is_open() || !outfile.is_open())
        {
                this->_logs += "[ ERROR ] Can't open files\n";
                return;
        }

        while (std::getline(infile, line))
        {
                std::size_t found = line.find(path + ":" + old_entropy);
                if (found != std::string::npos)
                        line.replace(found, (path + ":" + old_entropy).length(), path + ":" + std::to_string(entropy));
                outfile << line << "\n";
        }
        infile.close();
        outfile.close();

        if (rename(temp_file.c_str(), entropy_file.c_str()) != 0)
                this->_logs += "[ ERROR ] Could not rename temp file\n";
}

std::string IronDome::_check_entropy_file(const std::string &entropy_file, const std::string &path, const float &entropy)
{
        std::ifstream infile(entropy_file);

        if (!infile.is_open())
        {
                this->_logs += "[ ERROR ] Can't open " + entropy_file + ": " + std::string(strerror(errno)) + '\n';
                return std::string();
        }
        std::string line;
        std::string ret;
        while (std::getline(infile, line))
        {
                std::vector<std::string> content = this->_split_str(line, ':');
                if (content.size() != 2 || content[0] != path)
                        continue;
                ret = line;
                break;
        }
        infile.close();
        if (!ret.empty())
                return ret;

        this->_logs += "[ INFO ] Adding entropy record for " + path + '\n';
        std::ofstream outfile(entropy_file, std::ios_base::app);
        if (!outfile.is_open())
                this->_logs += "[ ERROR ] Can't open " + entropy_file + ": " + std::string(strerror(errno)) + '\n';
        else
        {
                outfile << path << ":" << std::to_string(entropy) << std::endl;
                outfile.close();
        }
        return std::string();
}

void IronDome::_check_crypto_use(const std::unordered_map<std::string, unsigned short> &read_counter)
{
        std::string path(DATA_DIR);
        path += '/';
        path += CRYPTO_CMD_FILE;
        std::ifstream file(path);
        if (!file.is_open())
        {
                this->_logs += "[ ERROR ] Can't open " + std::string(DATA_DIR) + '/' + CRYPTO_CMD_FILE + ": " + std::string(strerror(errno)) + '\n';
                return;
        }
        std::string line;
        std::string keyword;
        for (auto &it : read_counter)
        {
                if (it.first.empty())
                        continue;
                keyword = this->_split_str(it.first, '/').back();
                bool found = false;

                file.clear();
                file.seekg(0, std::ios::beg);
                while (std::getline(file, line))
                        if (line == keyword && (found = true))
                                break;
                if (found && it.second > this->_crypto_use_treshold)
                        this->_logs += "[ WARNING ] CRYPTO: " + it.first + '\n';
        }
        file.close();
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
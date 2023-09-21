#include "../../include/class/IronDome.hpp"

IronDome::IronDome()
    : _workin_dirs(std::vector<std::string>(1, DEFAULT_ROOT)),
      _logfile(DEFAULT_LOGFILE),
      _read_treshold(DEFAULT_READ_TRESHOLD),
      _entropy_treshold(DEFAULT_ENTROPY_TRESHOLD)
{
}

IronDome::IronDome(const IronDome &other)
    : _workin_dirs(other._workin_dirs),
      _logfile(other._logfile),
      _read_treshold(other._read_treshold),
      _entropy_treshold(other._entropy_treshold)
{
}

IronDome::IronDome(const std::vector<std::string> *workin_dirs,
                   const std::string *logfile,
                   const unsigned short *read_treshold,
                   const float *entropy_treshold)
    : _workin_dirs(workin_dirs ? *workin_dirs : std::vector<std::string>(1, DEFAULT_ROOT)),
      _logfile(logfile ? *logfile : DEFAULT_LOGFILE),
      _read_treshold(read_treshold ? *read_treshold : DEFAULT_READ_TRESHOLD),
      _entropy_treshold(entropy_treshold ? *entropy_treshold : DEFAULT_ENTROPY_TRESHOLD)
{
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
                this->_read_treshold = other._read_treshold;
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

const unsigned short &IronDome::get_read_treshold() const
{
        return this->_read_treshold;
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

void IronDome::set_read_treshold(const unsigned short &read_treshold)
{
        this->_read_treshold = read_treshold;
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
        this->_logs = std::string();
        if (this->_create_dir(this->_logfile))
                exit(errno);

        struct rlimit mem_limit;
        mem_limit.rlim_cur = mem_limit.rlim_max = MEM_LIMIT * 1024 * 1024;
        if (setrlimit(RLIMIT_AS, &mem_limit) == -1)
        {
                this->_logs += "[ ERROR ] Can't set memory limit: " + std::string(strerror(errno)) + "\n";
                this->~IronDome();
        }
        this->_inotify_fd = inotify_init();
        if (this->_inotify_fd < 0)
        {
                this->_logs += "[ ERROR ] Can't initialize inotify: " + std::string(strerror(errno)) + "\n";
                this->~IronDome();
        }
        for (std::string dir : this->_workin_dirs)
        {
                const int watch_fd = inotify_add_watch(this->_inotify_fd, dir.c_str(), IN_ACCESS | IN_MODIFY);
                if (watch_fd < 0)
                        this->_logs += "[ ERROR ] Can't add watch: " + std::string(strerror(errno)) + "\n";
                else
                        this->_watch_map[watch_fd] = dir;
        }
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

bool IronDome::_create_dir(const std::string &path) const
{
        struct stat st;
        std::string temp = std::string();
        std::vector<std::string> tokens = this->_split_str(path, '/');

        for (std::size_t i = 0; i < tokens.size() - 1; ++i)
        {
                temp += tokens[i];
                memset(&st, 0, sizeof(st));
                if (stat(temp.c_str(), &st) == -1)
                        if (mkdir(temp.c_str(), 0700))
                                return true;
                temp += '/';
        }
        return false;
}

std::vector<std::string> IronDome::_split_str(const std::string &src, char delimiter) const
{
        std::vector<std::string> tokens;
        std::string token;
        std::size_t end;
        std::size_t pos = 0;

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

void IronDome::_work()
{
        const int event_size = sizeof(struct inotify_event);
        const int buf_len = BUFFERSIZE * (event_size + 16);
        char buffer[buf_len];
        this->_logs += "[ INFO ] Daemon running.\n";
        this->_save_logs();
        while (true)
        {
                this->_logs += "[ INFO ] +1.\n";
                std::map<std::string, unsigned short> read_counter = std::map<std::string, unsigned short>();
                ssize_t length;
                while ((length = read(this->_inotify_fd, buffer, buf_len)) > 0)
                {
                        ssize_t i = 0;
                        while (i < length)
                        {
                                struct inotify_event *event = reinterpret_cast<struct inotify_event *>(&buffer[i]);
                                this->_inotify_check(event, read_counter);
                                i += event_size + event->len;
                        }
                        memset(buffer, 0, buf_len);
                }
                if (length < 0)
                        this->_logs += "[ ERROR ] Can't read inotify_fd: " + std::string(strerror(errno)) + "\n";
                this->_crypto_cmd_check();
                this->_logs += "[ INFO ] Looped.\n";
                this->_save_logs();
                sleep(SLEEPTIME);
        }
        close(this->_inotify_fd);
        for (auto &it : this->_watch_map)
                close(it.first);
        exit(EXIT_SUCCESS);
}

void IronDome::_inotify_check(struct inotify_event *event, std::map<std::string, unsigned short> &read_counter)
{
        if (event->mask & IN_ACCESS)
        {
                read_counter[event->name]++;
                if (read_counter[event->name] > this->_read_treshold)
                        this->_logs += "[ WARNING - READ ] " + std::string(event->name) + '\n';
        }
        if (event->mask & IN_MODIFY)
        {
                const std::string filePath = this->_watch_map[event->wd] + "/" + event->name;
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

                const float entropy = this->_calc_entropy(bytes);

                std::string entropy_file = DATA_DIR;
                entropy_file += '/' + ENTROPY_FILE;
                std::string entropy_temp_file = DATA_DIR;
                entropy_temp_file += '/' + ENTROPY_TEMP_FILE;

                if (system(std::string("grep " + filePath + " < " + entropy_file + " > " + entropy_temp_file).c_str()))
                {
                        std::ofstream outfile(entropy_file, std::ios_base::app);
                        if (!outfile.is_open())
                        {
                                this->_logs += "[ ERROR ] open " + entropy_file + ": " + std::string(strerror(errno)) + '\n';
                                return;
                        }
                        outfile << filePath << ":" << std::to_string(entropy) << std::endl;
                        outfile.close();
                }
                file = std::ifstream(entropy_temp_file);
                if (!file.is_open())
                {
                        this->_logs += "[ ERROR ] open " + entropy_temp_file + ": " + std::string(strerror(errno)) + '\n';
                        return;
                }
                std::string line;
                std::getline(file, line);
                file.close();
                if (!line.length())
                {
                        this->_logs += "[ ERROR ] " + entropy_temp_file + " is empty.\n";
                        return;
                }
                std::vector<std::string> content = this->_split_str(line, ':');
                if (content.size() != 2)
                        this->_logs += "[ ERROR ] " + entropy_temp_file + " is corrupted.\n";
                else if (content[0] != filePath)
                        this->_logs += "[ ERROR ] " + entropy_temp_file + " is corrupted.\n";
                else if (std::stof(content[1]) < entropy)
                        this->_logs += "[ WARNING - ENTROPY ] " + filePath + '\n';
        }
}

float IronDome::_calc_entropy(std::vector<unsigned char> &bytes)
{
        std::map<unsigned char, unsigned int> frequency;
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

void IronDome::_crypto_cmd_check()
{
        std::vector<std::string> running_process = this->_get_running_process();
        for (std::string cmd : running_process)
                if (this->_is_crypto_cmd(cmd))
                        this->_logs += "[ WARNING - CRYPTO ] " + cmd + '\n';
}

std::vector<std::string> IronDome::_get_running_process()
{
        std::vector<std::string> list;
        DIR *dir = opendir("/proc");
        if (!dir)
        {
                this->_logs += "[ ERROR ] Can't open /proc: " + std::string(strerror(errno)) + '\n';
                return list;
        }
        struct dirent *entry;
        while ((entry = readdir(dir)))
        {
                if (entry->d_type != DT_DIR)
                        continue;
                if (atoi(entry->d_name) <= 0)
                        continue;

                std::ifstream file(std::string("/proc/") + entry->d_name + "/comm");
                if (!file.is_open())
                {
                        this->_logs += "[ ERROR ] Can't open /proc/" + std::string(entry->d_name) + "/comm: " + std::string(strerror(errno)) + '\n';
                        continue;
                }
                std::string cmd;
                if (getline(file, cmd))
                        list.push_back(cmd);
                file.close();
        }
        closedir(dir);
        return list;
}

bool IronDome::_is_crypto_cmd(const std::string &cmd) const
{
        return system(std::string("grep " + cmd + " < " + DATA_DIR + '/' + CRYPTO_CMD_FILE).c_str()) == 0;
}
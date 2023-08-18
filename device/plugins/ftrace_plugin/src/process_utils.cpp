/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2021. All rights reserved.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#include "process_utils.h"

#include <fcntl.h>
#include <poll.h>
#include <sstream>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <unordered_map>

#include "file_utils.h"
#include "logging.h"
#include "string_utils.h"

namespace {
constexpr int RD = 0;
constexpr int WR = 1;
} // namespace

struct PipedSigHandler {
    explicit PipedSigHandler(int sig) : sig_(sig)
    {
        Init();
    }

    ~PipedSigHandler()
    {
        Finalize();
    }

    int GetNotifyFd() const
    {
        return pipe_[RD];
    }

private:
    int Init()
    {
        handler_ = signal(sig_, &PipedSigHandler::SigHandler);
        HILOG_INFO(LOG_CORE, "set signal handler for sig %d done!", sig_);

        CHECK_TRUE(pipe(pipe_) != -1, -1, "create pipe failed, %d", errno);
        CHECK_TRUE(fcntl(pipe_[RD], F_SETFL, O_NONBLOCK) != -1, -1, "set non block to pipe[WR] failed!");
        return 0;
    }

    int Finalize()
    {
        CHECK_TRUE(close(pipe_[RD]) != -1, -1, "close pipe_[RD] failed, %d", errno);
        CHECK_TRUE(close(pipe_[WR]) != -1, -1, "close pipe_[WR] failed, %d", errno);
        signal(sig_, nullptr);
        HILOG_INFO(LOG_CORE, "restore signal handler for sig %d done!", sig_);
        return 0;
    }

    static void SigHandler(int sig)
    {
        write(pipe_[WR], &sig, sizeof(sig));
    }

private:
    int sig_ = 0;
    static int pipe_[2];
    sighandler_t handler_ = nullptr;
};

int PipedSigHandler::pipe_[2] = {-1, -1};

struct Poller {
    using EventCallback = std::function<void(void)>;

    void AddFd(int fd, short events, const EventCallback& onEvent)
    {
        struct pollfd pfd = {0};
        pfd.fd = fd;
        pfd.events = events;
        pollSet_.push_back(pfd);
        callbacks_[fd] = onEvent;
        HILOG_INFO(LOG_CORE, "Add fd %d to poll set done!", fd);
    }

    int PollEvents(int timeout)
    {
        int nready = poll(pollSet_.data(), pollSet_.size(), timeout);
        CHECK_TRUE(nready >= 0, -1, "poll failed, %d", errno);
        if (nready == 0) {
            HILOG_INFO(LOG_CORE, "poll %dms timeout!", timeout);
            return 0;
        }
        return nready;
    }

    void DispatchEvents(int nready) const
    {
        for (int i = 0; i < nready; i++) {
            int fd = pollSet_[i].fd;
            if (pollSet_[i].revents & pollSet_[i].events) { // match interests events
                auto it = callbacks_.find(fd);
                if (it != callbacks_.end()) {
                    it->second();
                }
            }
        }
    }

private:
    std::vector<struct pollfd> pollSet_;
    std::unordered_map<int, EventCallback> callbacks_;
};

static bool ExecuteProcess(const std::string& bin,
                           const std::vector<std::string>& args,
                           int pipeFd,
                           bool out2pipe,
                           bool err2pipe)
{
    // redirect /dev/null to stdin
    int nullFd = open("/dev/null", O_RDONLY);
    int inFd = nullFd;
    int outFd = nullFd;
    int errFd = nullFd;
    CHECK_TRUE(inFd >= 0, false, "open /dev/null failed, %d", errno);
    CHECK_TRUE(dup2(inFd, STDIN_FILENO) != -1, false, "dup nullFD to stdin failed, %d", errno);

    // redirect outFd to stdout
    if (out2pipe) {
        outFd = pipeFd;
    }
    CHECK_TRUE(dup2(outFd, STDOUT_FILENO) != -1, false, "dup fd %d to stdout failed, %d", outFd, errno);

    // redirect errFd to stderr
    if (err2pipe) {
        errFd = pipeFd;
    }
    CHECK_TRUE(dup2(errFd, STDERR_FILENO) != -1, false, "dup fd %d to stderr failed, %d", errFd, errno);

    CHECK_TRUE(close(nullFd) != -1, false, "close nullFd failed, %d", errno);
    CHECK_TRUE(close(pipeFd) != -1, false, "close pipeFd failed, %d", errno);

    std::vector<char*> argv;
    for (size_t i = 0; i < args.size(); i++) {
        argv.push_back(const_cast<char*>(args[i].c_str()));
    }
    argv.push_back(nullptr); // last item in argv must be NULL

    int retval = execv(bin.c_str(), argv.data());
    std::string cmdline = StringUtils::Join(args, " ");
    CHECK_TRUE(retval != -1, false, "execv %s failed, %d!", cmdline.c_str(), errno);
    _exit(EXIT_FAILURE);
    return true;
}

static std::string ReceiveOutputAndSigchld(int pipeFd, const PipedSigHandler& handler)
{
    std::string output;
    Poller poller = {};
    poller.AddFd(pipeFd, POLLIN, [&]() {
        std::string out = FileUtils::ReadFile(pipeFd);
        output += out;
    });
    volatile bool childExit = false;
    poller.AddFd(handler.GetNotifyFd(), POLLIN, [&]() {
        childExit = true;
        int sig = 0;
        read(handler.GetNotifyFd(), &sig, sizeof(sig));
        HILOG_INFO(LOG_CORE, "sig %d received!", sig);
    });

    while (!childExit) {
        int timeout = 1000;
        int events = poller.PollEvents(timeout);
        if (events < 0) {
            HILOG_INFO(LOG_CORE, "poll failed!");
            break;
        }
        poller.DispatchEvents(events);
    }
    return output;
}

static int GetProcessExitCode(int pid)
{
    // get child exit code
    int wstatus = 0;
    pid_t w = waitpid(pid, &wstatus, 0);
    CHECK_TRUE(w > 0, -1, "waitpid failed, %d!", errno);

    // determine child exit status
    int retval = 0;
    if (WIFEXITED(wstatus)) {
        retval = WEXITSTATUS(wstatus);
        HILOG_INFO(LOG_CORE, "process %d exited with status %d", pid, retval);
    } else if (WIFSIGNALED(wstatus)) {
        retval = -1;
        HILOG_INFO(LOG_CORE, "process %d killed by signal %d\n", pid, WTERMSIG(wstatus));
    } else {
        retval = -1;
        HILOG_WARN(LOG_CORE, "process %d exited with unknow status!", pid);
    }
    return retval;
}

int ProcessUtils::Execute(const ExecuteArgs& args, std::string& output)
{
    CHECK_TRUE(args.bin_.size() > 0, -1, "bin_ empty");
    CHECK_TRUE(args.argv_.size() > 0, -1, "args_ empty");

    int pipeFds[2] = {-1, -1};
    CHECK_TRUE(pipe(pipeFds) != -1, -1, "create pipe failed, %d", errno);
    CHECK_TRUE(fcntl(pipeFds[RD], F_SETFL, O_NONBLOCK) != -1, -1, "set non block to pipe[WR] failed!");

    std::string cmdline = StringUtils::Join(args.argv_, " ");
    HILOG_INFO(LOG_CORE, "ExecuteCommand(%s): prepare ...", cmdline.c_str());

    PipedSigHandler sigChldHandler(SIGCHLD);
    pid_t pid = fork();
    CHECK_TRUE(pid >= 0, -1, "fork failed!");
    if (pid == 0) {
        // child process
        CHECK_TRUE(close(pipeFds[RD]) != -1, -1, "close pipeFds[RD] failed, %d", errno);
        ExecuteProcess(args.bin_, args.argv_, pipeFds[WR], args.out2pipe_, args.err2pipe_);
    }

    // parent process only read data from pipe, so close write end as soon as possible
    CHECK_TRUE(close(pipeFds[WR]) != -1, -1, "close pipeFds[WR] failed, %d", errno);

    output = ReceiveOutputAndSigchld(pipeFds[RD], sigChldHandler);
    auto lines = StringUtils::Split(output, "\n");
    HILOG_INFO(LOG_CORE, "ExecuteCommand(%s): output %zuB, %zuLn", cmdline.c_str(), output.size(), lines.size());

    int retval = GetProcessExitCode(pid);

    // close pipe fds
    CHECK_TRUE(close(pipeFds[RD]) != -1, -1, "close pipe[RD] failed, %d", errno);
    if (retval != 0 && cmdline != "hitrace -l") {
        HILOG_ERROR(LOG_CORE, "ExecuteCommand(%s): exit with %d, hitrace output is %s", cmdline.c_str(),
            retval, output.c_str());
    } else {
        HILOG_INFO(LOG_CORE, "ExecuteCommand(%s): exit with %d", cmdline.c_str(), retval);
    }
    return retval;
}

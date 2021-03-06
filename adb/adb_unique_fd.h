/*
 * Copyright (C) 2016 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#pragma once

#include <unistd.h>

#include <android-base/unique_fd.h>

// Helper to automatically close an FD when it goes out of scope.
struct AdbCloser {
    static void Close(int fd);
};

using unique_fd = android::base::unique_fd_impl<AdbCloser>;

#if !defined(_WIN32)
inline bool Pipe(unique_fd* read, unique_fd* write, int flags = 0) {
    int pipefd[2];
#if !defined(__APPLE__)
    if (pipe2(pipefd, flags) != 0) {
        return false;
    }
#else
    // Darwin doesn't have pipe2. Implement it ourselves.
    if (flags != 0 && (flags & ~(O_CLOEXEC | O_NONBLOCK)) != 0) {
        errno = EINVAL;
        return false;
    }

    if (pipe(pipefd) != 0) {
        return false;
    }

    if (flags & O_CLOEXEC) {
        if (fcntl(pipefd[0], F_SETFD, FD_CLOEXEC) != 0 ||
            fcntl(pipefd[1], F_SETFD, FD_CLOEXEC) != 0) {
            PLOG(FATAL) << "failed to set FD_CLOEXEC on newly created pipe";
        }
    }

    if (flags & O_NONBLOCK) {
        if (fcntl(pipefd[0], F_SETFL, O_NONBLOCK) != 0 ||
            fcntl(pipefd[1], F_SETFL, O_NONBLOCK) != 0) {
            PLOG(FATAL) << "failed to set O_NONBLOCK  on newly created pipe";
        }
    }
#endif

    read->reset(pipefd[0]);
    write->reset(pipefd[1]);
    return true;
}
#endif

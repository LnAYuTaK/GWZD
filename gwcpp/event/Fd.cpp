#include "Fd.h"

#include <errno.h>
#include <fcntl.h>
#include <utility>

#include "gwcpp/base/CLog.h"

using namespace gwcpp;
Fd::Fd() {}

Fd::Fd(int fd) {
  detail_ = new Detail;
  detail_->fd = fd;
}

Fd::Fd(int fd, const CloseFunc &close_func) {
  detail_ = new Detail;
  detail_->fd = fd;
  detail_->close_func = close_func;
}

Fd::~Fd() {
  if (detail_ == nullptr) return;
  if (detail_->ref_count > 0) {
    --detail_->ref_count;
  }
  if (detail_->ref_count == 0) {
    if (detail_->fd >= 0) {
      if (detail_->close_func)
        detail_->close_func(detail_->fd);
      else
        ::close(detail_->fd);
    }
    delete detail_;
  }
}

Fd::Fd(const Fd &other) {
  if (other.detail_ != nullptr) {
    ++other.detail_->ref_count;
    detail_ = other.detail_;
  }
}

Fd &Fd::operator=(const Fd &other) {
  if (this != &other) {
    if (other.detail_ != nullptr) {
      ++other.detail_->ref_count;
      detail_ = other.detail_;
    }
  }
  return *this;
}

IMP_MOVE_RESET_FUNC_BASE_ON_SWAP(Fd)

void Fd::swap(Fd &other) { std::swap(detail_, other.detail_); }

void Fd::close() {
  if (detail_ != nullptr && detail_->fd >= 0) {
    if (detail_->close_func) {
      detail_->close_func(detail_->fd);
      detail_->close_func = nullptr;
    } else
      ::close(detail_->fd);
    detail_->fd = -1;
  }
}

ssize_t Fd::read(void *ptr, size_t size) const {
  if (detail_ == nullptr) return -1;

  return ::read(detail_->fd, ptr, size);
}

ssize_t Fd::readv(const struct iovec *iov, int iovcnt) const {
  if (detail_ == nullptr) return -1;

  return ::readv(detail_->fd, iov, iovcnt);
}

ssize_t Fd::write(const void *ptr, size_t size) const {
  if (detail_ == nullptr) return -1;

  return ::write(detail_->fd, ptr, size);
}

ssize_t Fd::writev(const struct iovec *iov, int iovcnt) const {
  if (detail_ == nullptr) return -1;

  return ::writev(detail_->fd, iov, iovcnt);
}

void Fd::setNonBlock(bool enable) const {
  if (detail_ == nullptr) return;

#ifdef O_NONBLOCK
  int old_flags = fcntl(detail_->fd, F_GETFL, 0);
  int new_flags = old_flags;

  if (enable)
    new_flags |= O_NONBLOCK;
  else
    new_flags &= ~O_NONBLOCK;

  if (new_flags != old_flags) {
    int ret = fcntl(detail_->fd, F_SETFL, new_flags);
    if (ret == -1) {
      CLOG_ERROR() << " Fcntl Error : ";
    }
  }
#else
#error No way found to set non-blocking mode for fds.
#endif
}

bool Fd::isNonBlock() const {
  if (detail_ == nullptr) return false;

#ifdef O_NONBLOCK
  int flags = fcntl(detail_->fd, F_GETFL, 0);
  return (flags & O_NONBLOCK) != 0;
#else
#error No way found to set non-blocking mode for fds.
#endif
}

void Fd::setCloseOnExec() const {
  if (detail_ == nullptr) return;

#ifdef FD_CLOEXEC
  int old_flags = fcntl(detail_->fd, F_GETFD, 0);
  int new_flags = old_flags | FD_CLOEXEC;
  if (new_flags != old_flags) {
    int ret = fcntl(detail_->fd, F_SETFL, new_flags);
    if (ret == -1) {
      CLOG_ERROR() << "fcntl error";
    }
    // LogErr("fcntl error, errno:%d", errno);
  }
#else
  UNUSED(fd);
#endif
}

Fd Fd::Open(const char *filename, int flags) {
  int fd = ::open(filename, flags);
  if (fd < 0) {
    CLOG_ERROR() << "Open File :" << filename;
    return Fd();
  }
  return Fd(fd);
}


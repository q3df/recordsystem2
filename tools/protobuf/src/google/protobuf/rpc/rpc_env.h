// Copyright 2013 <chaishushan{AT}gmail.com>. All rights reserved.
// Use of this source code is governed by a BSD-style
// license that can be found in the LICENSE file.

#ifndef GOOGLE_PROTOBUF_RPC_ENV_H__
#define GOOGLE_PROTOBUF_RPC_ENV_H__

#include <google/protobuf/stubs/common.h>
#include <deque>
#include <cstdarg>
#include <pthread.h>
#include <google/protobuf/stubs/once.h>
#include <pthread.h>

namespace google {
namespace protobuf {
namespace rpc {

class Conn;

class LIBPROTOBUF_EXPORT Env {
 public:
	Env();
	virtual ~Env() {}

  // Return a default environment suitable for the current operating
  // system.  Sophisticated users may wish to provide their own Env
  // implementation instead of relying on this default environment.
  //
  // The result of Default() belongs to protobuf and must never be deleted.
  static Env* Default();

  // Write an entry to the log file with the specified format.
  virtual void Logv(const char* fmt, va_list ap);
  virtual void Logf(const char* fmt, ...);

  virtual bool Handshake(Conn *con);

  virtual void ClientDisconnect(Conn *con) {}

  // Start a new thread, invoking "function(arg)" within the new thread.
  // When "function(arg)" returns, the thread will be destroyed.
  virtual void StartThread(void (*function)(void* arg), void* arg);

  // Arrange to run "(*function)(arg)" once in a background thread.
  //
  // "function" may run in an unspecified thread.  Multiple functions
  // added to the same Env may run concurrently in different threads.
  // I.e., the caller may not assume that background work items are
  // serialized.
  virtual void Schedule(void (*function)(void* arg), void* arg);

 private:
  void PthreadCall(const char* label, int result);
  void BGThread();
  static void* BGThreadWrapper(void* arg);


  size_t page_size_;
  pthread_mutex_t mu_;
  pthread_cond_t bgsignal_;
  pthread_t bgthread_;
  bool started_bgthread_;

  // Entry per Schedule() call
  struct BGItem { void* arg; void (*function)(void*); };
  typedef std::deque<BGItem> BGQueue;
  BGQueue queue_;

  void* zmq_ctx_;

  GOOGLE_DISALLOW_EVIL_CONSTRUCTORS(Env);
};

}  // namespace rpc
}  // namespace protobuf
}  // namespace google

#endif // GOOGLE_PROTOBUF_RPC_ENV_H__


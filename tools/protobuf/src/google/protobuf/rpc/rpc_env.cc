// Copyright 2013 <chaishushan{AT}gmail.com>. All rights reserved.
// Use of this source code is governed by a BSD-style
// license that can be found in the LICENSE file.

#include "google/protobuf/rpc/rpc_env.h"

namespace {
/* This implementation is only for native Win32 systems.  */
#if (defined _WIN32 || defined __WIN32__) && ! defined __CYGWIN__
#	define WIN32_LEAN_AND_MEAN
#	include <windows.h>
	int getpagesize (void) {
		SYSTEM_INFO system_info;
		GetSystemInfo (&system_info);
		return system_info.dwPageSize;
	}
#endif

	struct StartThreadState {
	  void (*user_function)(void*);
	  void* arg;
	};

	static void* StartThreadWrapper(void* arg) {
	  StartThreadState* state = reinterpret_cast<StartThreadState*>(arg);
	  state->user_function(state->arg);
	  delete state;
	  return NULL;
	}
} // namespace

namespace google {
namespace protobuf {
namespace rpc {

Env::Env() {
	page_size_ = getpagesize();
	started_bgthread_ = false;
	PthreadCall("mutex_init", pthread_mutex_init(&mu_, NULL));
	PthreadCall("cvar_init", pthread_cond_init(&bgsignal_, NULL));
}

// Write an entry to the log file with the specified format.
void Env::Logv(const char* format, va_list ap) {
	const size_t kBufSize = 4096;
	char buffer[kBufSize+1];
	int written = vsnprintf(buffer, kBufSize, format, ap);
	buffer[kBufSize] = '\0';
	fprintf(stderr, "%s\n", buffer);
}

void Env::Logf(const char* fmt, ...) {
  va_list ap;
  va_start(ap, fmt);
  Logv(fmt, ap);
  va_end(ap);
}

bool Env::Handshake(Conn *con) {
	return true;
}

void Env::StartThread(void (*function)(void* arg), void* arg) {
	pthread_t t;
	StartThreadState* state = new StartThreadState;
	state->user_function = function;
	state->arg = arg;
	PthreadCall("start thread", pthread_create(&t, NULL,  &StartThreadWrapper, state));
}

void Env::Schedule(void (*function)(void*), void* arg) {
	PthreadCall("lock", pthread_mutex_lock(&mu_));
  
	// Start background thread if necessary
	if (!started_bgthread_) {
		started_bgthread_ = true;
		PthreadCall(
		"create thread",
		pthread_create(&bgthread_, NULL,  &Env::BGThreadWrapper, this)
		);
	}
  
	// If the queue is currently empty, the background thread may currently be
	// waiting.
	if(queue_.empty()) {
		PthreadCall("signal", pthread_cond_signal(&bgsignal_));
	}
  
	// Add to priority queue
	queue_.push_back(BGItem());
	queue_.back().function = function;
	queue_.back().arg = arg;
  
	PthreadCall("unlock", pthread_mutex_unlock(&mu_));
}

void Env::PthreadCall(const char* label, int result) {
	if(result != 0) {
		fprintf(stderr, "pthread %s: %s\n", label, strerror(result));
		exit(1);
	}
}

// BGThread() is the body of the background thread
void Env::BGThread() {
	while (true) {
		// Wait until there is an item that is ready to run
		PthreadCall("lock", pthread_mutex_lock(&mu_));
		while (queue_.empty()) {
			PthreadCall("wait", pthread_cond_wait(&bgsignal_, &mu_));
		}
  
		void (*function)(void*) = queue_.front().function;
		void* arg = queue_.front().arg;
		queue_.pop_front();
  
		PthreadCall("unlock", pthread_mutex_unlock(&mu_));
		this->StartThread(function, arg);
	}
}

void* Env::BGThreadWrapper(void* arg) {
	reinterpret_cast<Env*>(arg)->BGThread();
	return NULL;
}


static ::google::protobuf::ProtobufOnceType g_env_default_init_once;
static Env* g_env_default;
static void InitDefaultEnv() {
  g_env_default = new Env();
}

Env* Env::Default() {
  ::google::protobuf::GoogleOnceInit(&g_env_default_init_once, InitDefaultEnv);
  return static_cast<Env *>(g_env_default);
}

}  // namespace rpc
}  // namespace protobuf
}  // namespace google

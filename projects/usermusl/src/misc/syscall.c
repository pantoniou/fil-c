#define _BSD_SOURCE
#define _GNU_SOURCE
#include <errno.h>
#include <sched.h>
#include <signal.h>
#include <stdarg.h>
#include <unistd.h>
#include "syscall.h"
#include "fork_impl.h"

#undef syscall

/* The only clone that Fil-C supports is one that is a fork: it also creates the child in new
   namespaces, and the parent keeps its own, so it keeps its runtime threads. The runtime stops its
   threads for the fork and restarts them in both processes. Something that would share memory
   with the child, or make a thread, cannot be given the state the runtime needs. All five
   arguments have to be there, and everything after the flags has to be zero, whatever order the
   architecture gives them. */
#define CLONE_FORK_FLAGS (CLONE_NEWNS | CLONE_NEWCGROUP | CLONE_NEWUTS | CLONE_NEWIPC | \
	CLONE_NEWUSER | CLONE_NEWPID | CLONE_NEWNET | CLONE_UNTRACED)

static long clone_as_fork(long flags, long a1, long a2, long a3, long a4)
{
	if (a1 || a2 || a3 || a4 || (flags & 0xff) != SIGCHLD ||
	    (flags & ~(long)(0xff | CLONE_FORK_FLAGS))) {
		errno = EINVAL;
		return -1;
	}
	return __Fork_flags((int)(flags & CLONE_FORK_FLAGS));
}

long syscall(long n, ...)
{
	if (n == SYS_clone) {
		va_list ap;
		long flags, a1, a2, a3, a4;
		va_start(ap, n);
		flags = va_arg(ap, long);
		a1 = va_arg(ap, long);
		a2 = va_arg(ap, long);
		a3 = va_arg(ap, long);
		a4 = va_arg(ap, long);
		va_end(ap);
		return clone_as_fork(flags, a1, a2, a3, a4);
	}
	return *(long*)zcall(zsys_syscall, zargs());
}

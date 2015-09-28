/*
 * @file   forkUtil.c
 * @brief  util for process fork
 *
 * this code is under 3-Cause BSD license.
 *
 * @author taktod
 * @date   2015/09/28
 */

#ifdef __ENABLE_FILE__

#include "forkUtil.h"
#include "../log.h"
#include <sys/wait.h>
#include <string.h>
#include <unistd.h>

static void ForkUtil_catch(int sig_type) {
	if(sig_type == SIGCHLD) {
		pid_t child_pid = 0;
		do {
			int child_ret;
			child_pid = waitpid(-1, &child_ret, WNOHANG);
		} while(child_pid > 0);
	}
}

void ttLibC_ForkUtil_setup() {
	struct sigaction act;
	memset(&act, 0, sizeof(act));
	act.sa_handler = ForkUtil_catch;
	sigemptyset(&act.sa_mask);
	act.sa_flags = SA_NOCLDSTOP | SA_RESTART;
	sigaction(SIGCHLD, &act, NULL);
}

pid_t ttLibC_ForkUtil_fork() {
	return fork();
}

#endif

/*
 * This file is part of the Advance project.
 *
 * Copyright (C) 2002 Andrea Mazzoleni
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details. 
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 * In addition, as a special exception, Andrea Mazzoleni
 * gives permission to link the code of this program with
 * the MAME library (or with modified versions of MAME that use the
 * same license as MAME), and distribute linked combinations including
 * the two.  You must obey the GNU General Public License in all
 * respects for all of the code used other than MAME.  If you modify
 * this file, you may extend this exception to your version of the
 * file, but you are not obligated to do so.  If you do not wish to
 * do so, delete this exception statement from your version.
 */

#include "target.h"
#include "log.h"
#include "file.h"

#include <stdlib.h>
#include <sched.h>
#include <time.h>
#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <string.h>

/***************************************************************************/
/* Init */

int target_init(void) {
	return 0;
}

void target_done(void) {
}

/***************************************************************************/
/* Scheduling */

void target_yield(void) {
	sched_yield();
}

void target_idle(void) {
	struct timespec req;
	req.tv_sec = 0;
	req.tv_nsec = 1000000; /* 1 ms */
	nanosleep(&req, 0);
}

void target_usleep(unsigned us) {
}

/***************************************************************************/
/* Hardware */

void target_port_set(unsigned addr, unsigned value) {
}

unsigned target_port_get(unsigned addr) {
	return 0;
}

void target_writeb(unsigned addr, unsigned char c) {
}

unsigned char target_readb(unsigned addr) {
	return 0;
}

/***************************************************************************/
/* Mode */

void target_mode_reset(void) {
	/* nothing */
}

/***************************************************************************/
/* Sound */

void target_sound_error(void) {
	/* nothing */
}

void target_sound_warn(void) {
	/* nothing */
}

void target_sound_signal(void) {
	/* nothing */
}

/***************************************************************************/
/* APM */

int target_apm_shutdown(void) {
	/* nothing */
	return 0;
}

int target_apm_standby(void) {
	/* nothing */
	return 0;
}

int target_apm_wakeup(void) {
	/* nothing */
	return 0;
}

/***************************************************************************/
/* System */

int target_system(const char* cmd) {
	log_std(("linux: system %s\n",cmd));

	return system(cmd);
}

int target_spawn(const char* file, const char** argv) {
	int pid, status;
	int i;

	log_std(("linux: spawn %s\n",file));
	for(i=0;argv[i];++i)
		log_std(("linux: spawn arg %s\n",argv[i]));

	pid = fork();
	if (pid == -1)
		return -1;

	if (pid == 0) {
		execvp(file, (char**)argv);
		exit(127);
	} else {
		while (1) {
			if (waitpid(pid, &status, 0) == -1) {
				if (errno != EINTR) {
					status = -1;
					break;
				}
			} else
				break;
		}

		return status;
	}
}

int target_mkdir(const char* file) {
	return mkdir(file, S_IRWXU | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH);
}

void target_sync(void) {
	sync();
}

int target_search(char* path, unsigned path_size, const char* file) {
	char separator[2];
	const char* path_env;
	char* path_list;
	char* dir;

	log_std(("linux: target_search(%s)\n", file));

	/* if it's an absolute path */
	if (file[0] == file_dir_slash()) {
		strcpy(path, file);

		if (access(path,F_OK) == 0) {
			log_std(("linux: target_search() return %s\n", path));
			return 0;
		}

		return -1;
	}

	/* get the path list */
	path_env = getenv("PATH");
	if (!path_env) {
		log_std(("linux: genenv(PATH) failed\n"));
		return -1;
	}

	/* duplicate for the strtok use */
	path_list = strdup(path_env);
	if (!path_env) {
		return -1;
	}

	separator[0] = file_dir_separator();
	separator[1] = 0;
	dir = strtok(path_list, separator);
	while (dir) {
		unsigned l;

		strcpy(path, dir);

		l = strlen(path);
		if (l>0 && path[l-1] != file_dir_slash()) {
			path[l] = file_dir_slash();
			path[l+1] = 0;
		}

		strcat(path, file);

		if (access(path,F_OK) == 0) {
			free(path_list);
			log_std(("linux: target_search() return %s\n", path));
			return 0;
		}

		dir = strtok(0, separator);
	}

	log_std(("linux: target_search failed\n"));

	free(path_list);
	return -1;
}

void target_out_va(const char* text, va_list arg) {
	vfprintf(stdout, text, arg);
}

void target_err_va(const char *text, va_list arg) {
	vfprintf(stderr, text, arg);
}

void target_nfo_va(const char *text, va_list arg) {
	vfprintf(stderr, text, arg);
}

void target_out(const char *text, ...) {
	va_list arg;
	va_start(arg, text);
	target_out_va(text, arg);
	va_end(arg);
}

void target_err(const char *text, ...) {
	va_list arg;
	va_start(arg, text);
	target_err_va(text, arg);
	va_end(arg);
}

void target_nfo(const char *text, ...) {
	va_list arg;
	va_start(arg, text);
	target_nfo_va(text, arg);
	va_end(arg);
}

void target_flush(void) {
	fflush(stdout);
	fflush(stderr);
}
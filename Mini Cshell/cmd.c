// SPDX-License-Identifier: BSD-3-Clause

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>

#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include "cmd.h"
#include "utils.h"

#define READ		0
#define WRITE		1

#define IN			0
#define OUT			1
#define ERR			2

/**
 * Internal change-directory command.
 */
static bool shell_cd(word_t *dir)
{
	if (dir == NULL || dir->string == NULL)
		return false;

	return chdir(dir->string);
}

/**
 * Internal exit/quit command.
 */
static int shell_exit(void)
{

	return SHELL_EXIT;
}

/**
 * Parse a simple command (internal, environment variable assignment,
 * external command).
 */
static int parse_simple(simple_command_t *s, int level, command_t *father)
{
	if (s == NULL || s->verb == NULL)
		return false;

	if (s->verb->string != NULL && strcmp(s->verb->string, "cd") == 0) {
		char *in_redirect = get_word(s->in);

		if (in_redirect) {
			int fd = open(in_redirect, O_RDONLY);

			close(fd);
		}

		char *out_redirect = get_word(s->out);

		if (out_redirect) {
			int fd = open(out_redirect, O_WRONLY | O_CREAT | O_TRUNC, 0666);

			close(fd);
		}

		char *err_redirect = get_word(s->err);

		if (err_redirect) {
			int fd = open(err_redirect, O_WRONLY | O_CREAT | O_TRUNC, 0666);

			close(fd);
		}

		return shell_cd(s->params);
	}
	if ((s->verb->string != NULL && strcmp(s->verb->string, "exit") == 0)
		|| (s->verb->string != NULL && strcmp(s->verb->string, "quit") == 0)) {
		return shell_exit();
	}

	if (s->verb->next_part && s->verb->next_part->next_part &&
		s->verb->next_part->string && *s->verb->next_part->string == '=') {
		return setenv(s->verb->string, get_word(s->verb->next_part->next_part), 1);
	}

	pid_t child_pid = fork();
	int ret = 0;

	if (child_pid == 0) {
		char *in_redirect = get_word(s->in);

		if (in_redirect) {
			int fd = open(in_redirect, O_RDONLY);

			dup2(fd, IN);
			close(fd);
		}

		char *out_redirect = get_word(s->out);
		if (out_redirect) {
			int fd;

			if (s->io_flags == IO_OUT_APPEND)
				fd = open(out_redirect, O_WRONLY | O_CREAT | O_APPEND, 0666);
			else
				fd = open(out_redirect, O_WRONLY | O_CREAT | O_TRUNC, 0666);

			dup2(fd, OUT);
			close(fd);
		}

		char *err_redirect = get_word(s->err);
		if (err_redirect) {
			int fd;

			if (s->io_flags == IO_ERR_APPEND)
				fd = open(err_redirect, O_WRONLY | O_CREAT | O_APPEND, 0666);
			else
				fd = open(err_redirect, O_WRONLY | O_CREAT | O_TRUNC, 0666);

			dup2(fd, ERR);
			if (out_redirect && strcmp(out_redirect, err_redirect) == 0)
				dup2(fd, OUT);
			close(fd);
		}

		int size = 0;

		ret = execvp(s->verb->string, get_argv(s, &size));

		if (ret == -1) {
			printf("Execution failed for '%s'\n", s->verb->string);
			exit(ret);
		}

	} else {
		waitpid(child_pid, &ret, 0);
	}

	return ret;
}

/**
 * Process two commands in parallel, by creating two children.
 */
static bool run_in_parallel(command_t *cmd1, command_t *cmd2, int level,
		command_t *father)
{
	int ret;
	pid_t child_pid1 = fork();

	if (child_pid1 == 0) {
		ret = parse_command(cmd1, level, father);
		exit(ret);
	}

	pid_t child_pid2 = fork();

	if (child_pid2 == 0) {
		ret = parse_command(cmd2, level, father);
		exit(ret);
	} else {
		waitpid(child_pid1, &ret, 0);
		waitpid(child_pid2, &ret, 0);
	}
	return ret;
}

/**
 * Run commands by creating an anonymous pipe (cmd1 | cmd2).
 */
static bool run_on_pipe(command_t *cmd1, command_t *cmd2, int level,
		command_t *father)
{
	int fd_pipe[2];
	int ret = 0;

	pipe(fd_pipe);

	pid_t child_pid1 = fork();

	if (child_pid1 == 0) {
		dup2(fd_pipe[OUT], OUT);
		close(fd_pipe[OUT]);

		ret = parse_command(cmd1, level, father);
		exit(ret);
	}
	close(fd_pipe[OUT]);

	pid_t child_pid2 = fork();

	if (child_pid2 == 0) {
		dup2(fd_pipe[IN], IN);
		close(fd_pipe[IN]);

		ret = parse_command(cmd2, level, father);
		exit(ret);
	} else {
		close(fd_pipe[OUT]);
		close(fd_pipe[IN]);
		waitpid(child_pid1, &ret, 0);
		waitpid(child_pid2, &ret, 0);
	}
	return ret;
}

/**
 * Parse and execute a command.
 */
int parse_command(command_t *c, int level, command_t *father)
{
	if (c == NULL)
		return false;

	if (c->op == OP_NONE) {
		return parse_simple(c->scmd, level, father);
	}

	int ret = 0;

	switch (c->op) {
	case OP_SEQUENTIAL:
		ret = parse_command(c->cmd1, level, c);
		// if (ret == 0) return ret;
		ret = parse_command(c->cmd2, level, c);
		break;

	case OP_PARALLEL:
		ret = run_in_parallel(c->cmd1, c->cmd2, level, c);
		break;

	case OP_CONDITIONAL_NZERO:
		ret = parse_command(c->cmd1, level, c);
		if (ret == 0)
			return ret;
		ret = parse_command(c->cmd2, level, c);

		break;

	case OP_CONDITIONAL_ZERO:
		ret = parse_command(c->cmd1, level, c);
		if (ret != 0)
			return ret;
		ret = parse_command(c->cmd2, level, c);

		break;

	case OP_PIPE:
		ret = run_on_pipe(c->cmd1, c->cmd2, level, c);
		break;

	default:
		return shell_exit();
	}

	return ret;
}

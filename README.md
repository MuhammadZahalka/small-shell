# Small Shell

A Unix-style command shell written in C++, supporting built-in commands, external command execution, job control, and I/O redirection.

## Overview

The shell reads commands from standard input, parses them, and executes them either as built-in operations handled internally or as external programs run in child processes via `fork` and `execv`. Background jobs are tracked in a jobs list, and signal handlers manage foreground process interruption and suspension.

## Features

- **Built-in commands** — implemented directly in the shell process (e.g. `cd`, `pwd`, `jobs`, `fg`, `bg`, `quit`, `kill`).
- **External command execution** — forks a child process and executes the requested binary, with the parent waiting for foreground jobs.
- **Job control** — background jobs (`&`) are tracked in a jobs list with job IDs, and can be brought to the foreground or resumed.
- **Signal handling** — custom handlers for `SIGINT` (Ctrl-C) and `SIGTSTP` (Ctrl-Z) to interrupt or suspend the current foreground process without killing the shell itself.
- **I/O redirection and pipes** — supports redirecting command input/output and chaining commands.

## Build

```bash
mkdir build && cd build
cmake ..
make
```

## Usage

```bash
./smash
```

Then use it like a normal shell:

```
smash> pwd
smash> sleep 100 &
smash> jobs
smash> fg 1
```

## Notes

Written in C++ using POSIX system calls (`fork`, `execv`, `waitpid`, `kill`, `signal`). Developed as part of an Operating Systems course, focusing on process management, signals, and the mechanics of how a shell interacts with the kernel.

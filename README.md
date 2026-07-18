# A Small Unix Shell in C

An educational Unix-style command shell that explores process creation, signal handling, background work, built-in commands, output redirection, and a two-command pipe.

## Features

- Runs external programs with `fork` and `execvp`.
- Waits for foreground processes with `waitpid`.
- Tracks background jobs started with `&` in a linked list.
- Supports the built-ins `cd`, `pwd`, `echo`, `jobs`, `fg`, and `exit`.
- Handles a single `command | command` pipeline.
- Supports append-style output redirection with `>`.
- Installs handlers for `SIGINT` and `SIGTSTP` behavior.

## Build

```bash
make
```

Or compile directly:

```bash
cc -std=gnu99 -Wall -Wextra shell.c -o ege-shell
```

## Run

```bash
./ege-shell
```

Example session:

```text
$-->Ege's Shell>> pwd
$-->Ege's Shell>> echo hello from the shell
$-->Ege's Shell>> ls | wc
$-->Ege's Shell>> sleep 10 &
$-->Ege's Shell>> jobs
```

## Implementation map

| Area | Main functions |
| --- | --- |
| Parsing | `getcmd` |
| Built-ins | `runCd`, `runPwd`, `runEcho`, `runJobs`, `runFg` |
| Process execution | `main`, `fork`, `execvp`, `waitpid` |
| Piping | `checkPipe`, `runPipe` |
| Redirection | `runRedirection` |
| Job storage | `create_job`, `addJob`, `resetJobs` |
| Signals | `sigint_handler`, `handle_sigtstp` |

## Scope

This project makes core process-control concepts visible in a single C file. It intentionally supports a small grammar: one pipe, simple whitespace tokenization, and one output-redirection form. It is not a POSIX-complete shell and should not be used as a security boundary.

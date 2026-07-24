# ST5004CEM Operating Systems and Security — Coursework

Four C programs, one per task. Each task lives in its own folder with a
`Makefile`. Everything was written and tested on Ubuntu (WSL2 on Windows 11)
using `gcc` from the `build-essential` package.

## Requirements

- A Linux system (Debian/Ubuntu recommended)
- `gcc` and `make`:
  ```
  sudo apt update && sudo apt install -y build-essential
  ```

> Note: build and run the programs from a normal Linux filesystem (your home
> directory). Task 3 uses real file permissions, which do not work correctly on
> Windows-backed mounts such as `/mnt/c` or `/mnt/d`.

## Task 1 — Process Management and Threading

```
cd task1_threads
make
./task1
```
Menu-driven: fork demo, race condition (with and without a mutex),
producer–consumer with semaphores, deadlock prevention, and a round-robin
scheduler simulation. Each demo asks for its parameters (number of children,
threads, buffer size, burst times, time quantum, ...) — press Enter at any
prompt to keep the default value shown in brackets.

## Task 2 — Memory Management Simulation (paging)

```
cd task2_paging
make
./paging
```
Simulates paging with FIFO and LRU page replacement, tracks page faults and
hit/miss ratios, and prints the frame contents at every step. Also includes a
Belady's anomaly demo.

## Task 3 — File System Operations and Security

```
cd task3_filesec
make
./task3
```
Secure file manager: user registration/login (salted, hashed passwords),
per-file rwx permissions for owner/group/others, XOR encryption of file
contents, and an audit log. Data is kept in `store/`, `users.txt`,
`filemeta.txt` and `audit.log` (created on first run).

## Task 4 — Network Programming and IPC

```
cd task4_network
make            # builds both server and client
./server        # terminal 1
./client        # terminal 2 (or: ./client <server-ip>)
```
Concurrent TCP server (one thread per client) with a simple line-based text
protocol and login authentication. Demo accounts: `alice/wonderland` and
`bob/builder`. See `common.h` for the protocol. Commands: `LOGIN`, `ECHO`,
`TIME`, `ADD`, `QUIT`.

## Cleaning up

```
make clean      # in any task folder, removes binaries and object files
```

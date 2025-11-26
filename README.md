# OS Take-Home: Async Tasks + Local Sockets (`Mn`–`fn1`–`fn2`)

This project is a **take-home assignment** for an Operating Systems course.  
The goal is to implement a small distributed computation system with:

- separate **processes** (`Mn`, `fn1`, `fn2`);
- **local (Unix domain) sockets** as the IPC mechanism;
- an **asynchronous Task-style API** in the main component (`Mn`) to call `fn1(x)` and `fn2(x)` in parallel.

## High-Level Design

The system consists of three executables:

- **`mn`** – main process:
  - reads input `x` from the user;
  - spawns two worker processes `fn1` and `fn2` using `fork()` + `execl()`;
  - creates a separate Unix socket pair for each worker (`socketpair(AF_UNIX, SOCK_STREAM, ...)`);
  - sends requests `REQ <x>` to both workers over their sockets;
  - waits for both results asynchronously using a simple `Task<FnResult>` abstraction (internally built on top of `std::thread` + `std::promise/std::future`);
  - combines the two results via an operation `⊗` (in this implementation: sum of two real values with status propagation: `FAIL > UNDEF > OK`);
  - on exit, sends a `STOP` command to both workers and waits for them with `waitpid()`.

- **`fn1`** – worker process 1:
  - reads messages from file descriptor `3` (one end of the Unix socket pair, passed via `dup2()` in `Mn`);
  - expects requests of the form `REQ <x>`, where `x` is a floating-point value;
  - computes `fn1(x)` (for example, `x^2` with range checks);
  - replies with one of:
    - `OK <result>`
    - `FAIL <reason>`
    - `UNDEF <reason>`;
  - terminates cleanly on `STOP`.

- **`fn2`** – worker process 2:
  - uses the same communication protocol as `fn1`;
  - computes a different function `fn2(x)` (for example, `sqrt(|x|)` with a special case for `x = 0`);
  - replies with `OK` / `FAIL` / `UNDEF` and exits on `STOP`.

The main process combines the two responses:

- if at least one status is `FAIL` → overall status is `FAIL`;
- else if at least one status is `UNDEF` → overall status is `UNDEF`;
- else (both `OK`) → parses both numeric values and outputs their sum.

This setup demonstrates:

- multiple OS processes;
- IPC via Unix domain sockets;
- asynchronous Task-style programming in the main component.

---

## Build Instructions

**Requirements:**

- Linux or WSL (Ubuntu)
- `g++` with C++20 support
- `make`

**To build:**

    make

This will produce three binaries in the project root:

    mn
    fn1
    fn2

---

## Run Instructions

Start the main process:

    ./mn

Example interaction:

    Mn: starting workers fn1/fn2 via local sockets...
    Enter x (or q to quit): 2
    Mn: fn1(x) = [4.000000], status=OK; fn2(x) = [1.414214], status=OK
    Mn: combined (⊗ = sum) => status=OK, value = 5.414214

    Enter x (or q to quit): 0
    Mn: fn1(x) = [0.000000], status=OK; fn2(x) = [no_meaning_for_zero], status=UNDEF
    Mn: combined (⊗ = sum) => status=UNDEF, value = fn1=0.000000; fn2=no_meaning_for_zero

    Enter x (or q to quit): 1001
    Mn: fn1(x) = [x_out_of_range], status=FAIL; fn2(x) = [31.638584], status=OK
    Mn: combined (⊗ = sum) => status=FAIL, value = fn1=x_out_of_range; fn2=31.638584

To terminate the program, type:

    q

The main process will send `STOP` to both workers, close the sockets, wait for their termination, and then exit cleanly.
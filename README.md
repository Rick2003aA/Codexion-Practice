*This project has been created as part of the 42 curriculum by shinnunohis, rtsubuku.*

# Codexion

## Description
Codexion is a concurrency simulation inspired by the classic resource-sharing problems taught in the 42 curriculum. A set of coders sit around a table and repeatedly execute the cycle:

`compile -> debug -> refactor`

Compiling is the critical section. To compile, a coder must acquire two shared USB dongles placed between adjacent seats. If a coder cannot start compiling again before `time_to_burnout` expires, that coder burns out and the simulation stops.

The project focuses on:

- shared-resource coordination with threads
- deadlock prevention
- starvation-aware scheduling
- time-based monitoring
- deterministic logging under concurrency

The simulator also supports two compile schedulers:

- `fifo`: first coder to queue for compile gets priority
- `edf`: earliest deadline first, based on the next burnout deadline

## Project Overview
The program starts one monitor thread and one worker thread per coder.

- Each coder thread loops through compile, debug, and refactor phases.
- The scheduler controls which coder may enter the compile phase.
- The dongle layer manages access to shared resources and cooldowns.
- The monitor checks burnout and completion conditions and requests a global stop when needed.

Main entry points:

- `coders/app/main.c`
- `coders/core/coder_routine.c`
- `coders/core/coder_actions.c`
- `coders/core/scheduler.c`
- `coders/core/monitor.c`

## Instructions
### Build
```bash
make
```

### Clean
```bash
make clean
make fclean
```

### Rebuild
```bash
make re
```

### Run
```bash
./codexion <number_of_coders> <time_to_burnout_ms> <time_to_compile_ms> \
           <time_to_debug_ms> <time_to_refactor_ms> \
           <required_compiles> <dongle_cooldown_ms> <fifo|edf>
```

Example:

```bash
./codexion 5 1400 200 100 100 3 0 fifo
```

The provided `Makefile` also includes:

```bash
make run
```

which currently runs:

```bash
./codexion 5 1400 200 100 100 3 0 fifo
```

### Output format
Each log line is printed as:

```text
<timestamp_ms> <coder_id> <message>
```

Example:

```text
12 2 is compiling
25 4 is debugging
```

## Blocking Cases Handled
This implementation addresses the following concurrency hazards.

### Deadlock prevention
Each coder always locks dongles in ascending index order instead of "left then right". This removes circular wait, which is one of Coffman's deadlock conditions.

Handled effect:

- no cycle where every coder holds one dongle and waits forever for the other

### Coffman's conditions
The solution accepts mutual exclusion, hold-and-wait, and no preemption for dongles, but breaks circular wait through global lock ordering. That is the central deadlock prevention strategy.

### Starvation reduction
Coders do not compete for compile entry purely by timing luck. They first pass through an explicit scheduler queue implemented as a priority heap.

- `fifo` reduces unfair overtaking by ordering compile requests by arrival
- `edf` prioritizes the coder whose burnout deadline is earliest

This does not mathematically eliminate starvation in every imaginable policy, but it is designed to prevent practical starvation in the simulation by making compile access explicit and policy-driven.

### Dongle cooldown handling
After release, a dongle cannot be reused immediately. Each dongle stores an availability timestamp and blocks acquisition until its cooldown expires. This models temporary unavailability without busy-spinning.

### Precise burnout detection
Internal timing is tracked in microseconds, while logs remain displayed in milliseconds. Burnout is checked against the last compile start timestamp with a dedicated monitor thread, reducing boundary errors caused by coarse millisecond-only arithmetic.

### Single-dongle edge case
If the simulation has only one coder and therefore one dongle, the compile path handles the special case explicitly instead of trying to acquire the same dongle twice.

### Log serialization
All printed output is protected by a log mutex so lines are never interleaved between threads.

## Thread Synchronization Mechanisms
This project uses standard POSIX threading primitives.

### `pthread_mutex_t`
Mutexes protect shared mutable state:

- `log_mutex`: serializes all log output
- `stop_mutex`: protects the global stop flag
- `sched_mutex`: protects the scheduler heap, queue state, and scheduler condition variable
- `action_mutex`: protects per-coder timing and compile counters
- one mutex per dongle: protects dongle availability and its condition variable

Race-condition prevention examples:

- a coder updates `last_compile_start_us` under `action_mutex` while the monitor reads it under the same mutex
- heap insertion/removal and priority checks are all done while holding `sched_mutex`
- printing is always done while holding `log_mutex`, so output lines are not mixed

### `pthread_cond_t`
Condition variables are used for thread-safe waiting without constant polling.

- `sched_cv`: coders waiting to compile sleep until scheduler state changes
- each dongle has its own `cv`: coders sleep until the dongle becomes available again after cooldown

Thread-safe communication examples:

- when the current compiling coder releases its turn, it broadcasts on `sched_cv` so waiting coders re-check heap priority
- when a dongle leaves cooldown, it broadcasts on the dongle condition variable so waiting coders can attempt acquisition again
- when the simulation stops, broadcast wakes threads blocked in scheduler or dongle waits so they can exit cleanly

### Monitor coordination
The monitor thread does not directly interrupt worker threads. Instead, it requests a stop through shared state and wakeups:

- set the global stop flag
- broadcast waiting condition variables
- let coders observe the stop flag and return safely

This keeps shutdown explicit and avoids unsafe cancellation.

## Technical Choices
### Priority heap for scheduler policy
The compile waiting queue is implemented as a binary heap over coder pointers. This makes it easy to support both FIFO and EDF with the same data structure by changing only the priority comparison rule.

### Relative simulation time
The simulator stores a `start_us` timestamp at initialization and computes all internal times as elapsed microseconds since that point. This keeps arithmetic simple and consistent across monitor checks, cooldown deadlines, and logging.

### Event timestamp reuse
For compile start, the program captures one timestamp and uses it both for internal state updates and for log output. This keeps the recorded compile start and the visible `"is compiling"` log aligned.

## Development Notes
The implementation evolved incrementally.

- time measurement was first validated with `gettimeofday`
- a single worker thread was introduced before scaling to multiple coders
- logging was serialized early with a mutex to avoid broken output
- dongles were first tested as one shared resource, then expanded to one per seat gap
- the design moved from "logic inside main" to a split structure with init, core, and common modules
- monitor logic and stop signaling were added after the basic worker loop was stable
- burnout and required compile counts were moved into configurable runtime rules
- dongle cooldown was added using per-dongle condition variables and timed waits

## Resources
Classic references related to threads, time, and synchronization:

- POSIX threads manual pages: `pthread_create(3)`, `pthread_mutex_lock(3)`, `pthread_cond_wait(3)`, `pthread_cond_timedwait(3)`
- `gettimeofday(2)` and `clock_gettime(2)` manual pages
- The Open Group Base Specifications for POSIX threads
- "The Dining Philosophers Problem" by Edsger W. Dijkstra
- "Operating Systems: Three Easy Pieces" sections on locks, condition variables, and scheduling
- 42 project materials on concurrency and synchronization

### How AI was used
AI was used as a design and review assistant, not as an automatic code generator.

Used for:

- discussing what functions were needed next
- checking whether the intended synchronization design made sense
- identifying mistakes and risky assumptions
- suggesting correction strategies in words
- clarifying concepts such as mutexes, heaps, timed waits, and scheduling policies

Not used for:

- generating the final code to paste directly into the project
- replacing manual implementation and debugging work

The workflow was iterative: ask for the role of a function or subsystem, implement it manually, receive verbal feedback about errors, then revise the code.

## Further Reading
If you want to explore the topic further, focus on:

- deadlock conditions and lock ordering
- starvation and fairness in schedulers
- monitor-thread versus event-driven shutdown models
- precision limits of time measurement in concurrent programs

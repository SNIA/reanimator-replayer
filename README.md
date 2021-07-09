# Re-animator Replayer - replay DataSeries traces

Re-animator Replayer is a tracing tool that is intended for extracting traces of any program or an application and then replaying their I/O behavior.
It uses DataSeries, a binary format which provides a fast and efficient way to store and analyze traces.
Our trace replayer follows SNIA's specification documents -- "POSIX System-Call Trace Common Semantics" and "IO Trace Common Semantics" -- which describe the system call fields, their types, and their values.

Re-animator Replayer can be used as a benchmarking tool to measure and reproduce the I/O performance under various workloads, or for analyzing the program traces for security purposes, etc.

For more information on the Re-Animator project, please see our paper [Re-Animator: Versatile High-Fidelity Storage-System Tracing and Replaying](https://doi.org/10.1145/3383669.3398276).

Re-animator Replayer is under development by Ibrahim Umit Akgun of the File Systems and Storage Lab (FSL) at Stony Brook University under Professor Erez Zadok, with assistance from Professor Geoff Kuenning at Harvey Mudd College.

## Build Instructions

Requires bash.

1. Install the following required programs and libraries:

    ```plaintext
    git cmake perl autoconf automake gcc g++ libtool libboost-dev libboost-thread-dev libboost-program-options-dev build-essential libxml2-dev zlib1g-dev libaio-dev
    ```

    On Ubuntu 16 and 18, all the above requirements are available through the APT package manager.

1. Clone this repository and run [`build-reanimator-replayer.sh`](build-reanimator-replayer.sh). This will place build files in the current directory under `build/` and install the `system-call-replayer` and other completed binaries and libraries under `reanimator_replayer_release/`.

    | Option                | Description                                               |
    | --------------------- | --------------------------------------------------------- |
    | `--build-dir DIR`     | Download repositories and place build files in DIR        |
    | `--config-args ARGS`  | Append ARGS to every ./configure command                  |
    | `--install`           | Install libraries and binaries under /usr/local           |
    | `--install-dir DIR`   | Install libraries and binaries under DIR                  |
    | `--install-packages`  | Automatically use apt-get to install missing packages     |
    | `--release`           | Build an optimized release version of reanimator replayer |
    | `-h`, `--help`        | Print this help message                                   |


## Usage

`system-call-replayer path/to/DataSeriesFile.ds`

The system call replayer supports the following optional command line options:

| Option                    | Description                                                                   |
| ------------------------- | ----------------------------------------------------------------------------- |
| `-V [ --version ]`        | Print version of system call replayer                                         |
| `-h [ --help ]`           | Produce help message                                                          |
| `-v [ --verbose ]`        | System calls replay in verbose mode                                           |
| `--verify`                | Verifies that the data being written/read is exactly what was used originally |
| `-w [ --warn ] arg`       | System call replays in warn mode                                              |
| `-p [ --pattern ] arg`    | Write repeated pattern data for write, pwrite, and writev system call         |

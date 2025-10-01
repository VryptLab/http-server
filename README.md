# High-Performance HTTP Server in C

A lightweight, high-performance HTTP server written in **pure C**, designed for speed and scalability.  
It uses **epoll** for efficient I/O multiplexing and a **thread pool** for concurrency, allowing it to handle thousands of simultaneous connections with minimal overhead.

---

## Features

- **Ultra-lightweight**: Minimal dependencies, written in pure C.
- **Fast Endpoints**:
  - `GET /` â†’ responds with raw text `"OK"`.
  - `GET /stats` â†’ responds with real-time JSON statistics:
    - `total_connections`: total number of connections since server start.
    - `rps`: requests per second (updated every second).
- **High Concurrency**: Combines `epoll` with multi-threading for maximum throughput.
- **Optimized for Linux**: Uses non-blocking sockets and atomic counters.

---

## Build & Run

### Requirements
- Linux-based system  
- GCC (supporting C11 standard)  
- POSIX threads (pthread)

### install
```bash
git clone https://github.com/VryptLab/http-server.git
make clear && make
```

### Run
```bash
./server
```

By default, the server listens on **port 8080**.

---

## Usage

- Access the root endpoint:
  ```bash
  curl http://localhost:8080/
  ```
  Response:
  ```
  OK
  ```

- Access server statistics:
  ```bash
  curl http://localhost:8080/stats
  ```
  Example response:
  ```json
  {
    "total_connections": 42,
    "rps": 7
  }
  ```

---

## Performance Notes

- Designed to handle **high numbers of concurrent connections** using `epoll` (edge-triggered I/O).
- Multithreaded with a thread pool equal to the number of CPU cores.
- Atomic counters ensure safe, lock-free statistics tracking.

---

## Use Cases

- Benchmarking and load testing tools.
- Educational reference for building scalable servers in C.
- Lightweight alternative to heavy HTTP frameworks.

---

## License

This project is released under the **MIT License**.  
You are free to use, modify, and distribute it as long as the license terms are respected.

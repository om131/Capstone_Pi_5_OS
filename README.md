
🎯 Overview
This project demonstrates advanced Linux systems programming and embedded systems concepts by building a production-ready BLE (Bluetooth Low Energy) data collection pipeline. The system efficiently scans for BLE devices, processes data with minimal latency, and reliably forwards it to cloud services (Firebase).
Built for: Raspberry Pi 5 (ARM64)
Use Case: IoT sensor networks, asset tracking, environmental monitoring
Problem Statement
Modern IoT applications require:

Real-time data collection from multiple BLE devices
Low-latency processing pipelines
Reliable cloud data transmission
Efficient resource utilization on embedded hardware

Solution
A multi-threaded, CPU-optimized system that:

Scans BLE devices continuously using BlueZ
Transfers data via optimized IPC mechanisms
Forwards to cloud with buffering and retry logic
Achieves sub-10ms end-to-end latency


✨ Key Features
🔍 BLE Scanning

BlueZ DBus API integration for device discovery
Continuous monitoring of BLE advertisements
RSSI-based filtering and device management
HCI-level optimizations for throughput

🚀 High-Performance IPC

Unix Domain Sockets for inter-process communication
POSIX Shared Memory for zero-copy data transfer
Lock-free ring buffers (optional)
Measured latency: < 5ms between processes

☁️ Cloud Integration

Firebase Realtime Database integration via cURL
Buffering for batch uploads
Retry logic with exponential backoff
Connection pooling for efficiency

⚡ System Optimizations

CPU Affinity: BLE scanner pinned to CPU 0-1, Cloud forwarder to CPU 2-3
Process Prioritization: Real-time scheduling for BLE thread
Memory Management: Pre-allocated buffers, memory pooling
Systemd Integration: Production-ready service configuration

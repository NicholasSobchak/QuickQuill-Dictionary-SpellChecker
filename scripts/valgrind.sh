#!/usr/bin/env bash
set -e

echo "=== Running Valgrind on QuickQuill ==="

# Build with debug symbols
echo "Building with debug symbols..."
cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug -DCMAKE_TOOLCHAIN_FILE=vcpkg/scripts/buildsystems/vcpkg.cmake
cmake --build build -j$(nproc)

# Valgrind options
VALGRIND_FLAGS="--leak-check=full --show-leak-kinds=all --track-origins=yes --verbose --log-file=valgrind_output.txt"

# Start server under valgrind in background
echo "Starting server under valgrind..."
valgrind $VALGRIND_FLAGS ./build/src/dict_crow &
VALGRIND_PID=$!

# Wait for server to be ready
echo "Waiting for server to start..."
for i in $(seq 1 30); do
  if curl -sf http://localhost:8080/api/health > /dev/null 2>&1; then
    echo "Server is ready!"
    break
  fi
  if ! kill -0 $VALGRIND_PID 2>/dev/null; then
    echo "ERROR: Server crashed under valgrind"
    exit 1
  fi
  sleep 1
done

# Run test requests to exercise the code
echo "Running test requests..."
for word in hello world test quickquill dictionary; do
  curl -sf "http://localhost:8080/api/word/$word" > /dev/null 2>&1 || true
  curl -sf "http://localhost:8080/api/suggest/$word" > /dev/null 2>&1 || true
done

# Give valgrind time to process
sleep 2

# Kill the server gracefully to generate leak report
echo "Stopping server..."
kill -TERM $VALGRIND_PID 2>/dev/null || true
wait $VALGRIND_PID 2>/dev/null || true

# Give valgrind time to write final report
sleep 2

echo "=== Valgrind complete ==="
echo "Output saved to valgrind_output.txt"
echo ""
echo "=== Summary (definite leaks) ==="
grep -A5 "definitely lost:" valgrind_output.txt || echo "No definite leaks found!"

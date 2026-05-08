#!/bin/sh
# Wrapper that runs the server and monitors health
# If health fails repeatedly, kill the server so Docker restarts it

HEALTH_URL="http://127.0.0.1:8080/api/health"
MAX_FAILS=6
FAIL_COUNT=0
WARMUP_INTERVAL=300  # Re-warm every 5 minutes to prevent cold threads

# Trap signals and forward to server for graceful shutdown
trap 'echo "Received signal, shutting down..."; kill -TERM "$SERVER_PID" 2>/dev/null; wait "$SERVER_PID"; exit' TERM INT

./dict_crow &
SERVER_PID=$!

# Wait for server to be ready
echo "Waiting for server to start..."
while ! curl -sf --max-time 10 "$HEALTH_URL" > /dev/null 2>&1; do
  # Log failure details for debugging
  echo "[$(date)] Startup health check failed: $(curl -sS --max-time 10 "$HEALTH_URL" 2>&1 | head -c 200)"
  if ! kill -0 "$SERVER_PID" 2>/dev/null; then
    echo "Server exited during startup"
    exit 1
  fi
  sleep 1
done

# Warmup function - hit various endpoints to initialize all thread-local resources
warmup() {
  echo "[$(date)] Warming up server (initializing thread-local DB/Redis connections)..."
  # Hit multiple endpoints to force initialization of thread-local resources
  WORDS="the and water fire earth air light dark house time year"
  for word in $WORDS; do
    curl -sf --max-time 5 "http://127.0.0.1:8080/api/word/$word" > /dev/null 2>&1 &
    curl -sf --max-time 5 "http://127.0.0.1:8080/api/suggest/$word" > /dev/null 2>&1 &
  done
  wait
  echo "[$(date)] Warmup complete"
}

# Initial warmup
warmup

# Periodic warmup in background
(
  while kill -0 "$SERVER_PID" 2>/dev/null; do
    sleep $WARMUP_INTERVAL
    warmup
  done
) &
WARMUP_PID=$!

# Monitor health
while kill -0 "$SERVER_PID" 2>/dev/null; do
  if curl -sf --max-time 10 "$HEALTH_URL" > /dev/null 2>&1; then
    FAIL_COUNT=0
  else
    FAIL_COUNT=$((FAIL_COUNT + 1))
    echo "[$(date)] Health check failed (count: $FAIL_COUNT/$MAX_FAILS, error: $(curl -sS --max-time 10 "$HEALTH_URL" 2>&1 | head -c 200))"
    if [ "$FAIL_COUNT" -ge "$MAX_FAILS" ]; then
      echo "Health check failed $MAX_FAILS times, killing server (PID $SERVER_PID)"
      kill -TERM "$SERVER_PID" 2>/dev/null
      sleep 5
      kill -9 "$SERVER_PID" 2>/dev/null
      exit 1
    fi
  fi
  sleep 10
done

# Server exited on its own — capture exit code and restart
kill "$WARMUP_PID" 2>/dev/null
# Don't wait here — trap handles server shutdown
EXIT_CODE=$?
echo "[$(date)] Server exited — restarting container"
exit 1

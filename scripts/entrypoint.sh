#!/bin/sh
# Wrapper that runs the server and monitors health
# If health fails repeatedly, kill the server so Docker restarts it

HEALTH_URL="http://localhost:8080/api/health"
MAX_FAILS=3
FAIL_COUNT=0
WARMUP_REQUESTS=8
WARMUP_INTERVAL=300  # Re-warm every 5 minutes to prevent cold threads

./dict_crow &
SERVER_PID=$!

# Wait for server to be ready
echo "Waiting for server to start..."
while ! curl -sf --max-time 3 "$HEALTH_URL" > /dev/null 2>&1; do
  if ! kill -0 "$SERVER_PID" 2>/dev/null; then
    echo "Server exited during startup"
    exit 1
  fi
  sleep 1
done

# Warmup function - hit health endpoint which already exercises DB/Redis
warmup() {
  echo "[$(date)] Warming up server..."
  for i in $(seq 1 $WARMUP_REQUESTS); do
    curl -sf --max-time 5 "$HEALTH_URL" > /dev/null 2>&1 &
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
  if curl -sf --max-time 3 "$HEALTH_URL" > /dev/null 2>&1; then
    FAIL_COUNT=0
  else
    FAIL_COUNT=$((FAIL_COUNT + 1))
    if [ "$FAIL_COUNT" -ge "$MAX_FAILS" ]; then
      echo "Health check failed $MAX_FAILS times, killing server (PID $SERVER_PID)"
      kill -9 "$SERVER_PID"
      exit 1
    fi
  fi
  sleep 5
done

# Cleanup
kill "$WARMUP_PID" 2>/dev/null
wait "$SERVER_PID"

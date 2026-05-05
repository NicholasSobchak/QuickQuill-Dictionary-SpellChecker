#!/usr/bin/env bash
set -e

IMAGE_NAME="nicksobchak/quickquill"

echo "=== Deploying to production ==="

# Verify required files exist
if [ ! -f dictionary.db ]; then
  echo "ERROR: dictionary.db not found in repo root"
  exit 1
fi
if [ ! -f config.json ]; then
  echo "ERROR: config.json not found in repo root"
  exit 1
fi

# Pull latest Docker image
echo "Pulling latest Docker image..."
docker pull "$IMAGE_NAME" || { echo "Failed to pull image"; exit 1; }

# Stop existing containers
echo "Stopping existing containers..."
docker-compose -f docker-compose.yml down 2>/dev/null || true

# Start services
echo "Starting containers..."
docker-compose -f docker-compose.yml up -d

# Wait for health check (through nginx)
echo "Waiting for backend to be healthy..."
for i in $(seq 1 24); do
  if curl -sf https://quickquill.ink/api/health > /dev/null 2>&1; then
    echo "=== Production deployment complete ==="
    echo "Frontend: https://quickquill.ink"
    exit 0
  fi
  echo "Waiting for backend... ($i/24)"
  sleep 10
done

echo "ERROR: Backend failed to become healthy"
docker-compose -f docker-compose.yml logs backend
docker-compose -f docker-compose.yml logs nginx
exit 1

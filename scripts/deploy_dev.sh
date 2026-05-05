#!/bin/sh
# Deploy development version locally

set -e

IMAGE_NAME="quickquill-dev"

echo "=== Building frontend ==="
cd web
npm install
npm run build
cd ..

echo "=== Building Docker image: $IMAGE_NAME ==="
docker build -t "$IMAGE_NAME" . || (echo "Docker build failed" && exit 1)

echo "=== Stopping existing containers ==="
docker-compose down 2>/dev/null || true

echo "=== Starting services with local image ==="
# Use local image instead of registry
export DOCKER_IMAGE="$IMAGE_NAME"
docker-compose -f docker-compose.yml -f docker-compose.override.yml up -d

echo "=== Waiting for services to be ready ==="
sleep 5

echo "=== Checking backend health ==="
for i in $(seq 1 12); do
  if curl -sf http://localhost/api/health > /dev/null 2>&1; then
    echo "Backend is healthy!"
    echo "=== Development deployment complete ==="
    echo "Frontend: http://localhost"
    echo "Backend API: http://localhost/api"
    exit 0
  fi
  echo "Waiting for backend... ($i/12)"
  sleep 5
done

echo "ERROR: Backend failed to become healthy"
docker-compose logs backend
exit 1

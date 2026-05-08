#!/usr/bin/env bash
set -e

IMAGE_NAME="nicksobchak/quickquill"

echo "=== Deploying to production ==="

# Pull latest Docker image
echo "Pulling latest Docker image..."
docker pull "$IMAGE_NAME" || { echo "Failed to pull image"; exit 1; }

# Stop existing containers
echo "Stopping existing containers..."
docker-compose -f docker-compose.yml down 2>/dev/null || true

# Start services with pulled image
echo "Starting services..."
export DOCKER_IMAGE="$IMAGE_NAME"
docker-compose -f docker-compose.yml up -d

echo "=== Production deployment complete ==="
echo "Frontend: https://quickquill.ink"

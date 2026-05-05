#!/usr/bin/env bash
echo "Deploying frontend, building backend, and starting docker-compose"

# build frontend
echo "Building frontend..."
(cd web && npm ci --silent && npm run build) || { echo "Frontend build failed"; exit 1; }

# build backend C++
echo "Building backend..."
cmake -S . -B build
cmake --build build -j

# find DB and config
if [ ! -f dictionary.db ]; then
  echo "WARNING: dictionary.db not found in repo root. Make sure that dictionary.db is in the repo root"
fi
if [ ! -f config.json ]; then
  echo "WARNING: config.json not found in the repo root. Using default values"
fi

# start
echo "Starting containers..."
docker build -t nicksobhak:quickquill .
docker-compose up -d

echo "Deployment complete"
echo "Use 'docker-compose down' to stop"

# Multi-stage Dockerfile: build the C++ backend inside the image and produce a small runtime image

### Builder stage
FROM debian:bookworm-slim AS builder
RUN apt-get update \
  && apt-get install -y --no-install-recommends build-essential cmake git ca-certificates curl pkg-config unzip tar zip python3 \
  && rm -rf /var/lib/apt/lists/*
WORKDIR /src
# Copy source and build inside the container
COPY . .
# Install vcpkg and bootstrap
RUN git clone https://github.com/microsoft/vcpkg.git /src/vcpkg \
  && /src/vcpkg/bootstrap-vcpkg.sh

# Install common dependencies via vcpkg (fallback to explicit install)
RUN /src/vcpkg/vcpkg install nlohmann-json sqlite3 crow catch2 --triplet x64-linux || true

# Clean any host-created build cache and use vcpkg toolchain for dependency resolution
RUN rm -rf build \
  && cmake -S . -B build -DCMAKE_BUILD_TYPE=Release \
     -DCMAKE_TOOLCHAIN_FILE=/src/vcpkg/scripts/buildsystems/vcpkg.cmake \
     -DVCPKG_TARGET_TRIPLET=x64-linux \
  && cmake --build build -j$(nproc)

### Final runtime image
FROM debian:bookworm-slim
RUN apt-get update \
  && apt-get install -y --no-install-recommends ca-certificates libstdc++6 curl \
  && rm -rf /var/lib/apt/lists/*
WORKDIR /app
# Copy only the built binary and config; do NOT copy dictionary.db (mount at runtime)
COPY --from=builder /src/build/src/dict_crow ./dict_crow
COPY config.json ./config.json
RUN chmod +x ./dict_crow

EXPOSE 8080
CMD ["./dict_crow"]

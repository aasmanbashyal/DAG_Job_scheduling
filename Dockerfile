# Use a base image with C++ build tools
FROM ubuntu:22.04

# Avoid interactive prompts during package installation
ENV DEBIAN_FRONTEND=noninteractive

# Install required packages
RUN apt-get update && apt-get install -y \
    build-essential \
    cmake \
    g++ \
    make \
    && rm -rf /var/lib/apt/lists/*

# Create and set working directory
WORKDIR /app

# Copy source code and build files
COPY . .

# Create build directories
RUN mkdir -p bin obj/core

# Build the project
RUN make clean && make

# Set the default command to run the scheduler
CMD ["./bin/scheduler"]
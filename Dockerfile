FROM gcc:latest

WORKDIR /usr/src/app

# Install SFML, OpenCL, and X11 (for GUI) dependencies
RUN apt-get update && apt-get install -y \
    gcc \
    g++ \
    libsfml-dev \
    opencl-headers \
    pocl-opencl-icd \
    ocl-icd-opencl-dev \
    x11-apps \
    clinfo \
    cmake \
    && rm -rf /var/lib/apt/lists/*

COPY . .

RUN cmake . && make

CMD ["./HunterGatherers"]


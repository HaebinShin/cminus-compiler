mkdir -p build
docker build -t cminus . && \
docker run -t -v $(pwd)/build:/work_root/build cminus /bin/sh -c "make && ./run.out sample_inputs/${INPUT_SOURCE}"
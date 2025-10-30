all: build
build: FORCE
        mkdir -p build
	cd build && cmake -DCMAKE_CUDA_STANDARD=17  -DENABLE_LOGGING=OFF -DSERVER_VERSION=$(SERVER_VERSION) -DLLAMA_CPP_DIR=/home/llama.cpp ..  && make -j

FORCE: ;

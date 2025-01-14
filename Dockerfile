FROM ubuntu:20.04

LABEL maintainer="nindanaoto <matsuoka.kotaro@gmail.com>"

RUN apt-get install google-perftools libgoogle-perftools-dev -y
RUN apt-get update && apt-get upgrade -y && DEBIAN_FRONTEND=noninteractive apt-get install -y build-essential g++-10 libomp-dev cmake git libgoogle-perftools-dev && git clone --recursive --depth 1 https://github.com/virtualsecureplatform/TFHEpp && git clone --recursive --depth 1 https://github.com/tfhe/tfhe.git  && mkdir TFHEpp/build && mkdir tfhe/build
RUN apt-get install wget g++ m4 zlib1g-dev make p7zip libboost-all-dev -y && wget https://gmplib.org/download/gmp/gmp-6.1.2.tar.xz && tar -xvf gmp-6.1.2.tar.xz && cd gmp-6.1.2 && ./configure --enable-cxx && make && make install
# && git clone --recursive --depth 1 https://github.com/virtualsecureplatform/tfhe-10ms.git && mkdir tfhe-10ms/build

WORKDIR TFHEpp/build

RUN cmake .. -DENABLE_TEST=ON -DCMAKE_CXX_COMPILER=g++-10 && make

#WORKDIR /tfhe/build
#
#RUN cmake ../src -DENABLE_TESTS=on -DENABLE_NAYUKI_PORTABLE=off -DENABLE_NAYUKI_AVX=off -DENABLE_SPQLIOS_AVX=off -DENABLE_SPQLIOS_FMA=on -DCMAKE_BUILD_TYPE=optim && make

# WORKDIR /tfhe-10ms/build

# RUN cmake ../src -DENABLE_TESTS=on -DENABLE_NAYUKI_PORTABLE=off -DENABLE_NAYUKI_AVX=off -DENABLE_SPQLIOS_AVX=off -DENABLE_SPQLIOS_FMA=on -DCMAKE_BUILD_TYPE=optim && make

WORKDIR /

#CMD echo "TFHEpp" &&./TFHEpp/build/test/nand && echo "original TFHE" && ./tfhe/build/test/test-gate-bootstrapping-spqlios-fma 
# && echo "TFHE-10ms" && tfhe-10ms/build/test/test-bootstrapping-fft-spqlios-fma 

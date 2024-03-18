/* Copyright (C) 2024 Gleb Bezborodov - All Rights Reserved
 * You may use, distribute and modify this code under the
 * terms of the MIT license.
 *
 * You should have received a copy of the MIT license with
 * this file. If not, please write to: bezborodoff.gleb@gmail.com, or visit : https://github.com/glensand/hope-speed-test
 */

#include "hope-io/net/stream.h"
#include "hope-io/net/acceptor.h"
#include "hope-io/net/factory.h"

#include <memory>
#include <iostream>
#include <chrono>

constexpr static std::size_t port{ 9669 };
constexpr static std::size_t chunk_size{ 64 * 1024 }; // 64k
auto* buffer = new uint8_t[chunk_size];

void test(bool upload, hope::io::stream* stream, std::size_t size) {
    auto chuncks = size / chunk_size;
    const std::string side = upload ? "upload" : "download";
    std::cout << "Testing " << side <<":" << size / 1024 << " KB\n";
    auto start = std::chrono::high_resolution_clock::now();
    while (chuncks > 0) {
        if (upload)
            stream->write(buffer, chunk_size);
        else
            stream->read(buffer, chunk_size);

        --chuncks;      
    }
    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> duration = end - start;
    std::cout << "Execution time: " << duration.count() << " seconds." << std::endl;
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        std::cout << "To run server use: ./speed-test server\nTo run client: ./speed-test ip\n";
        return - 1;
    }
    auto&& tst_bench = [] (bool upload, hope::io::stream* stream){
        test(upload, stream, 1024 * 512);
        test(upload, stream, 1024 * 1024);
        test(upload, stream, 10 * 1024 * 1024);
        test(upload, stream, 100 * 1024 * 1024);
        test(upload, stream, 1024 * 1024 * 1024);
    };

    if (argv[1] == std::string("server")) {
        auto* acceptor = hope::io::create_acceptor();
        acceptor->open(port);
        auto* stream = acceptor->accept();

        tst_bench(true, stream);
        tst_bench(false, stream);
    } else {
        auto* stream = hope::io::create_stream();
        stream->connect(argv[1], port);
        tst_bench(false, stream);
        tst_bench(true, stream);
    }

    return 0;
}
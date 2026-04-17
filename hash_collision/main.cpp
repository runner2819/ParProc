//#include <stdio.h>
//#include <stdlib.h>
#include <iostream>
#include <fstream>
#include <iomanip>
#include <omp.h>
#include <chrono>
#include <format>
//#include <stdatomic.h>
//#include <math.h>
#include <openssl/sha.h>
#include <vector>
#include <stdexcept>
#include <map>
#include <random>

#define MSG_BYTES 4

//using namespace std;
void print_hex(std::vector<uint8_t> data, std::ofstream &out) {
    for (uint8_t byte: data) {
        out << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(byte);
    }
}

std::vector<uint8_t> truncate(uint8_t *msg, int size, int n_bits) {
    int n_bytes = (n_bits + 7) / 8;
    if (n_bytes > size) {
        throw std::invalid_argument("Msg is too short");
    } else if (n_bytes <= 0) {
        throw std::invalid_argument("Invalid number of bits");
    }
    auto res = std::vector<uint8_t>(msg + size - n_bytes, msg + size);
    if (n_bits % 8 != 0) {
        res[0] = res[0] % (1 << (n_bits % 8));
    }
    return res;
}


std::vector<uint8_t> sha_xx(const std::vector<uint8_t> msg, int n_bits) {
    if (n_bits < 8 || n_bits > 24) {
        throw std::invalid_argument("Invalid number of bits");
    }
    uint8_t *hash = new uint8_t[SHA256_DIGEST_LENGTH];

    SHA256(msg.data(), msg.size(), hash);
    std::vector<uint8_t> sha_xx_hash = truncate(hash, SHA256_DIGEST_LENGTH, n_bits);
    delete[] hash;
    return sha_xx_hash;
}

long long  birthday_attack(std::string path, int n_bits, int n_collisions) {
    std::ofstream log;
    log.open(path);
    std::map<std::vector<uint8_t>, std::vector<uint8_t>> map_of_hashes;
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<uint8_t> dist(0, 255);
    int found_col = 0;
    auto start = std::chrono::high_resolution_clock::now();
    while (found_col < n_collisions) {
        std::vector<uint8_t> x(MSG_BYTES);
        for (auto &byte: x) {
            byte = dist(gen);
        }
        std::vector<uint8_t> h_x = sha_xx(x, n_bits);
        auto aa = map_of_hashes.find(h_x);
        if (aa == map_of_hashes.end()) {
            map_of_hashes.insert({h_x, x});
        } else if (aa->second != x) {
            log << "Collision ";
            print_hex(x, log);
            log << " and ";
            print_hex(aa->second, log);
            log << std::endl;
            found_col++;
        }
    }
    auto stop = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(stop - start);
    log.close();
    return duration.count();
}

int main() {
    for (int i = 8; i < 25; i++) {
        std::cout <<i<<" "<<birthday_attack(std::format("/Users/maxim/Desktop/All/code/CLionProjects/ParProc/hash_collision/data_sha{}.txt", i), i, 1000)<<"\n";
    }

    return 0;
}


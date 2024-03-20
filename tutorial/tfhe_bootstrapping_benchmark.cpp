#include <stdio.h>

#include <cassert>
#include <cereal/archives/portable_binary.hpp>
#include <cereal/types/vector.hpp>
#include <fstream>
#include <iostream>
#include <memory>
#include <random>
#include <tfhe++.hpp>
#include <vector>
#include <chrono>
#include <complex>

using namespace std::chrono;
inline double get_time_msec(void)
{
    return static_cast<double>(duration_cast<nanoseconds>(
                                   steady_clock::now().time_since_epoch())
                                   .count()) /
           1000000;
}

double CalculateApproximationError(const std::vector<double>& result,
                                   const std::vector<double>& expectedResult) {
    if (result.size() != expectedResult.size()){
        std::cout <<  "Cannot compare vectors with different numbers of elements" << std::endl;
        return 0;
    }
    // using the Euclidean norm
    double avrg = 0;
    for (size_t i = 0; i < result.size(); ++i) {
        avrg += std::pow(std::abs(result[i] - expectedResult[i]), 2);
    }
    avrg = std::sqrt(avrg/result.size());
    return avrg;
    //avrg = std::sqrt(avrg) / result.size();  // get the average
    //return std::abs(std::log2(avrg));
}

int main()
{
    // Generate a random key.
    std::unique_ptr<TFHEpp::SecretKey> sk =
        std::make_unique<TFHEpp::SecretKey>();
    std::unique_ptr<TFHEpp::GateKey> gk =
        std::make_unique<TFHEpp::GateKey>(*sk);

    double encoder_a = -50.;
    double encoder_b = 50.;
    TFHEpp::Encoder encoder(encoder_a, encoder_b, 32);

    TFHEpp::IdentityFunction identity_function = TFHEpp::IdentityFunction<TFHEpp::lvl1param>();
    //TFHEpp::SigmoidFunction identity_function = TFHEpp::SigmoidFunction<TFHEpp::lvl1param>();

    int vector_size = 48;
    std::vector<double> p(vector_size);
    for (int i = 0; i < vector_size; i++) {
        p[i] = i;
    }
    std::vector<TFHEpp::TLWE<TFHEpp::lvl0param>> ciphertext(vector_size);

    for (int i = 0; i < vector_size; i++) {
        ciphertext[i] = TFHEpp::tlweSymEncodeEncrypt<TFHEpp::lvl0param>(
            p[i], TFHEpp::lvl0param::alpha, sk->key.lvl0, encoder);
    }

    double start, end;
    start = get_time_msec();
    for (int i = 0; i < vector_size; i++) {
        TFHEpp::ProgrammableBootstrapping(ciphertext[i], ciphertext[i],
                                          *gk.get(), encoder, encoder,
                                          identity_function);
    }
    end = get_time_msec();

    std::vector<double> decrypted_answer(vector_size);
    for (int i = 0; i < vector_size; i++) {
        decrypted_answer[i] = TFHEpp::tlweSymDecryptDecode<TFHEpp::lvl0param>(
            ciphertext[i], sk->key.lvl0, encoder);
    }
    
    std::cout << "decyrpted answer is " << std::endl;
    for (int i =0; i<vector_size; i++){
        std::cout << decrypted_answer[i] << " ";
    }
    std::cout << std::endl;

    std::cout << "precision is " << CalculateApproximationError(p, decrypted_answer) << std::endl;

    std::cout << "time is " << end - start << std::endl;

    return 0;
}


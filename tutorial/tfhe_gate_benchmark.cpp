#include <cereal/archives/portable_binary.hpp>
#include <cereal/types/vector.hpp>
#include <chrono>
#include <fstream>
#include <memory>
#include <random>
#include <tfhe++.hpp>
#include <vector>
using namespace std::chrono;
inline double get_time_msec(void)
{
    return static_cast<double>(duration_cast<nanoseconds>(
                                   steady_clock::now().time_since_epoch())
                                   .count()) /
           1000000;
}

enum BinGate { OR, AND, NOR, NAND };

int cleartext_gate(int v1, int v2, BinGate gate)
{
    switch (gate) {
    case OR: return v1 || v2;
    case AND: return v1 && v2;
    case NOR: return not(v1 || v2);
    case NAND: return not(v1 && v2);
    default:
        std::cerr << "\n This gate does not exists \n";
        exit(1);
        return 0;
    }
}

int main()
{
    // Generate a random key.
    std::unique_ptr<TFHEpp::SecretKey> sk =
        std::make_unique<TFHEpp::SecretKey>();
    std::unique_ptr<TFHEpp::GateKey> gk =
        std::make_unique<TFHEpp::GateKey>(*sk);
    // Create an encoder.
    double encoder_a = -20.;
    double encoder_b = 20.;
    TFHEpp::Encoder encoder(encoder_a, encoder_b, 32);

    // Create a cleartext.
    const int vector_size = 48;
    std::srand(time(NULL));
    std::vector<double> p(vector_size), p2(vector_size);  // uint8_t double?
    for (int i = 0; i < vector_size; i++) {
        p[i] = i;
    }
    for (int i = 0; i < vector_size; i++) {
        if(i == 3){
            p2[i] = i;
            continue;
        }
        p2[i] = 0;
    }
    std::vector<TFHEpp::TLWE<TFHEpp::lvl0param>> ciphertext(vector_size),
        ciphertext2(vector_size);
    // Create first ciphertext.
    for (int i = 0; i < vector_size; i++) {
        ciphertext[i] = TFHEpp::tlweSymEncodeEncrypt<TFHEpp::lvl0param>(
            p[i], TFHEpp::lvl0param::alpha, sk->key.lvl0, encoder);
    }
    // Create second ciphertext.
    for (int i = 0; i < vector_size; i++) {
        ciphertext2[i] = TFHEpp::tlweSymEncodeEncrypt<TFHEpp::lvl0param>(
            p2[i], TFHEpp::lvl0param::alpha, sk->key.lvl0, encoder);
    }

    double start, stop;

    start = get_time_msec();
    // Calculate AND between two ciphertexts.
    for (int i = 0; i < vector_size; i++) {
        TFHEpp::HomAND(ciphertext[i], ciphertext[i], ciphertext2[i],
                       *gk.get());  // change to XOR, NAND, NOT
        //TFHEpp::HomMULT(ciphertext[i], ciphertext[i], ciphertext2[i], *gk.get(), encoder, encoder, encoder);
    }
    stop = get_time_msec();

    std::vector<double> decrypted_answer(vector_size);
    for (int i = 0; i < vector_size; i++) {
        decrypted_answer[i] = TFHEpp::tlweSymDecryptDecode<TFHEpp::lvl0param>(
            ciphertext[i], sk->key.lvl0, encoder);
    }
    std::cout << "result is " << std::endl;
    for (auto& part : decrypted_answer) {
        std::cout << part << " ";
    }
    std::cout << std::endl;
    std::cout << "time is " << stop - start << std::endl;

    return 0;
}
#include <signal.h>
#include <stdio.h>

#include <cassert>
#include <cereal/archives/portable_binary.hpp>
#include <cereal/types/vector.hpp>
#include <chrono>
#include <complex>
#include <fstream>
#include <iostream>
#include <memory>
#include <random>
#include <tfhe++.hpp>
#include <vector>
#include <thread>
#include <omp.h>

void signal_callback_handler(int signum)
{
    std::cout << "Caught signal " << signum << std::endl;
    // Terminate program
    exit(signum);
}
void HalfAdder(TFHEpp::TLWE<TFHEpp::lvl0param>& sum,
               TFHEpp::TLWE<TFHEpp::lvl0param>& carry,
               const TFHEpp::TLWE<TFHEpp::lvl0param>& ca,
               const TFHEpp::TLWE<TFHEpp::lvl0param>& cb,
               const TFHEpp::GateKey& gk)
{
    HomXOR(sum, ca, cb, gk);
    HomAND(carry, ca, cb, gk);
}
 
void FullMulter(TFHEpp::TLWE<TFHEpp::lvl0param>& mult,
                const TFHEpp::TLWE<TFHEpp::lvl0param>& cai,
                const TFHEpp::TLWE<TFHEpp::lvl0param>& cbi,
                const TFHEpp::GateKey& gk)
{
    TFHEpp::TLWE<TFHEpp::lvl0param> res, carry;
    for (int i=0; i< cai.size(); i++){
        TFHEpp::TLWE<TFHEpp::lvl0param> tmp_res;
        TFHEpp::TLWE<TFHEpp::lvl0param> tmp;
        for (int j =0; j<TFHEpp::lvl0param::n ; j++){
            tmp[j] = cbi[i];
        }
        TFHEpp::HomAND(tmp_res, cai, tmp, gk);
        std::rotate(tmp_res.begin(), tmp_res.begin()+1, tmp_res.end());  
        HalfAdder(res, carry, res, tmp_res, gk);
        TFHEpp::HomOR(res, res, carry, gk);
    }

       
}

using namespace std::chrono;
inline double get_time_msec(void)
{
    return static_cast<double>(duration_cast<nanoseconds>(
                                   steady_clock::now().time_since_epoch())
                                   .count()) /
           1000000;
}

double CalculateApproximationError(const std::vector<double>& result,
                                   const std::vector<double>& expectedResult)
{
    if (result.size() != expectedResult.size()) {
        std::cout << "Cannot compare vectors with different numbers of elements"
                  << std::endl;
        return 0;
    }
    // using the Euclidean norm
    double avrg = 0;
    for (size_t i = 0; i < result.size(); ++i) {
        avrg += std::pow(std::abs(result[i] - expectedResult[i]), 2);
    }
    avrg = std::sqrt(avrg / result.size());
    return avrg;
    // avrg = std::sqrt(avrg) / result.size();  // get the average
    // return std::abs(std::log2(avrg));
}

std::ifstream::pos_type filesize(const char* filename)
{
    std::ifstream in(filename, std::ifstream::ate | std::ifstream::binary);
    in.seekg(0, std::ios_base::end);
    return in.tellg();
}

int main()
{
    //std::system("(top -b -d 0.05 >> top_result_100.txt)&");
    //std::system("(top -b -d 0.01 >> top_result3.txt)&");
   //std::system("(mpstat -P ALL 1 >> mpstat3.txt)&");
    //std::system("(top -b -d 0.05 >> top_original_singleCPU.txt)&");
    //std::system("( vmstat -n -S m -t 1 >> tfhepp_tfhe_bootstrap_vmstat_STD128.txt)&");
//    std::system("(top -b -d 0.5 >> top_tfhepp_0750.txt)&");
    //std::system("( vmstat -n -S m -t 1 >> tfhepp_2G_4.txt)&");
//    std::system("(iotop -b -d 0.5 -k >> test.txt)&");
    std::cout << "TFHEpp::lvl0param n is " << TFHEpp::lvl0param::n << std::endl;
    std::cout << "the numbers of threads is " << omp_get_max_threads() << std::endl;
    double total_time = get_time_msec();
    // Generate a random key.
    double sk_start = get_time_msec();
    std::unique_ptr<TFHEpp::SecretKey> sk =
        std::make_unique<TFHEpp::SecretKey>();
    std::cout << "time for generating sk is " << get_time_msec() - sk_start
              << std::endl;

   /* // export the secret key to know the size
    {
        std::ofstream ofs{"secret.key", std::ios::binary};
        cereal::PortableBinaryOutputArchive ar(ofs);
        sk->serialize(ar);
    };
     
    std::cout << "secret key size is " << filesize("secret.key") << std::endl;
   */
    double gk_start = get_time_msec();
    std::unique_ptr<TFHEpp::GateKey> gk =
        std::make_unique<TFHEpp::GateKey>(*sk);
    std::cout << "time for generating gk is " << get_time_msec() - gk_start
              << std::endl;
    std::cout << "Completed the key generation. It is " << get_time_msec() - total_time << std::endl;
  /*  // export the secret key to know the size
   {
        std::ofstream ofs{"gate_keyswitch.key", std::ios::binary};
        cereal::PortableBinaryOutputArchive ar(ofs);
        gk->serialize_ksk(ar);
    };
    std::cout << "key switching key size is " << filesize("gate_keyswitch.key") << std::endl;

    // export the secret key to know the size
    {
        std::ofstream ofs{"gate_bs.key", std::ios::binary};
        cereal::PortableBinaryOutputArchive ar(ofs);
        gk->serialize_bs(ar);
    };
    std::cout << "bootstrap key size is " << filesize("gate_bs.key") << std::endl;
   */
    double encoder_a = -10.;
    double encoder_b = 10.;
    TFHEpp::Encoder encoder(encoder_a, encoder_b, 32);

    TFHEpp::IdentityFunction identity_function =
        TFHEpp::IdentityFunction<TFHEpp::lvl1param>();
    // TFHEpp::SigmoidFunction identity_function =
    // TFHEpp::SigmoidFunction<TFHEpp::lvl1param>();

    int vector_size = 3;
    std::vector<int> p(vector_size);
    for (int i = 0; i < vector_size; i++) {
        p[i] = 0;
    }
    p[0] = 1;
    p[1] = 2;
    std::vector<TFHEpp::TLWE<TFHEpp::lvl0param>> ciphertext(vector_size);
    TFHEpp::TLWE<TFHEpp::lvl0param> prev, next, res;
    std::cout << "TFHEpp::lvl0param n is " << TFHEpp::lvl0param::n << std::endl;
    double encryption_start = get_time_msec();
    for (int i = 0; i < vector_size; i++) {
        ciphertext[i] = TFHEpp::tlweSymEncodeEncrypt<TFHEpp::lvl0param>(
            p[i], TFHEpp::lvl0param::alpha, sk->key.lvl0, encoder);
    }
    next = TFHEpp::tlweSymEncodeEncrypt<TFHEpp::lvl0param>(
            50, TFHEpp::lvl0param::alpha, sk->key.lvl0, encoder);
    std::cout << "time for encryption is " << get_time_msec() - encryption_start
              << std::endl;
    std::cout << "Finish encryption. It is " << get_time_msec() - total_time << std::endl;
  /* // export the secret key to know the size
    {
        std::ofstream ofs{"ciphertext.key", std::ios::binary};
        cereal::PortableBinaryOutputArchive ar(ofs);
        ar(ciphertext[0]);
        //ciphertext[0].serialize(ar);
    };
   
    std::cout << "ciphertext size is " << filesize("ciphertext.key") << std::endl;
   */
    //std::cout << "Start bootstrapping. It is " << get_time_msec() - total_time
    //          << std::endl;
    double start, end;
   
    //std::system("(mpstat -P ALL 1 >> mpstat_100.txt)&");
    //std::system("(perf record --call-graph dwarf -F 400 -o perf_tfhepp.data)&");
    //TFHEpp::TLWE<TFHEpp::lvl0param> res;
    start = get_time_msec();
    res = ciphertext[0];
    for (int i = 0; i < vector_size-1; i++) {
        //TFHEpp::ProgrammableBootstrapping(ciphertext[i], ciphertext[i],
        //                                 *gk.get(), encoder, encoder,
        //                                  identity_function);

        //HomNAND(res, ciphertext[0], ciphertext[0], *gk);
        //TFHEpp::HomMULT(ciphertext2[i+1], ciphertext[i], ciphertext2[i], *gk.get(), encoder, encoder, encoder);
        TFHEpp::HomXOR(prev, ciphertext[i+1], res, *gk.get());
        TFHEpp::HomNOT(prev, prev);
        TFHEpp::HomAND(next, prev, next, *gk.get());
        //prev = next;
        //TFHEpp::HomADD(res, res, ciphertext2[i+1], encoder, encoder);
        //FullMulter(ciphertext2[i+1], ciphertext[i], ciphertext2[i], *gk.get());
        TFHEpp::HomSUBFixedEncoder(prev, ciphertext[i+1], res, encoder, encoder);
        
    }
    //for (int i=1; i<vector_size+1; i++){
    //    TFHEpp::HomADD(res, res, ciphertext2[i], encoder, encoder);
    //}
    end = get_time_msec();
    //std::cout << "Finished bootstrapping. It is "
    //          << get_time_msec() - total_time << std::endl;
    //std::cout << "gate time is " << end - start << std::endl;
    //std::cout << "bootstrap time is " << end - start << std::endl;
    std::cout << "programmable bootstrap time is " << end - start << std::endl;
    std::cout << "Finished bootstrapping. It is " << get_time_msec() - total_time << std::endl;
    //std::vector<double> decrypted_answer(vector_size);
    double decrypted_answer;
    double decryption_start = get_time_msec();
    decrypted_answer = TFHEpp::tlweSymDecryptDecode<TFHEpp::lvl0param>(
            prev, sk->key.lvl0, encoder);
    //for (int i = 0; i < vector_size; i++) {
    //    decrypted_answer[i] = TFHEpp::tlweSymDecryptDecode<TFHEpp::lvl0param>(
    //        ciphertext[i], sk->key.lvl0, encoder);
    //}
    std::cout << "time for decryption is " << get_time_msec() - decryption_start
              << std::endl;
    std::cout << "decyrpted answer is " << std::endl;
    //for (int i = 0; i < vector_size; i++) {
    //    std::cout << decrypted_answer[i] << " ";
    //}
    std::cout << decrypted_answer << std::endl;

    //std::cout << "precision is "
    //          << CalculateApproximationError(p, decrypted_answer) << std::endl;

    //std::this_thread::sleep_for(milliseconds(10000) );
    std::cout << "total time is " << get_time_msec() - total_time << std::endl;
    //std::cout << "key switch time is " << TFHEpp::key_switch_time << std::endl;
    //std::this_thread::sleep_for(milliseconds(10000) );
    //signal(SIGINT, signal_callback_handler);
    //std::this_thread::sleep_for(milliseconds(10000) );
    return 0;
}
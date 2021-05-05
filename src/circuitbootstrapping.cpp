#include "circuitbootstrapping.hpp"

namespace TFHEpp {
using namespace std;

template <class bkP, class privksP>
void CircuitBootstrappingPartial(TRLWE<typename privksP::targetP> &trgswupper,
                                 TRLWE<typename privksP::targetP> &trgswlower,
                                 const TLWE<typename bkP::domainP> &tlwe,
                                 const CircuitKey<bkP, privksP> &ck,
                                 const uint32_t digit)
{
    TLWE<typename bkP::targetP> tlwemiddle;
    GateBootstrappingTLWE2TLWEFFTvariableMu<bkP>(
        tlwemiddle, tlwe, ck.bkfft,
        1ULL << (numeric_limits<typename privksP::domainP::T>::digits -
                 (digit + 1) * privksP::targetP::Bgbit - 1));
    PrivKeySwitch<privksP>(trgswupper, tlwemiddle, ck.privksk[0]);
    PrivKeySwitch<privksP>(trgswlower, tlwemiddle, ck.privksk[1]);
}
#define INST(bkP, privksP)                                   \
    template void CircuitBootstrappingPartial<bkP, privksP>( \
        TRLWE<typename privksP::targetP> & trgswupper,       \
        TRLWE<typename privksP::targetP> & trgswlower,       \
        const TLWE<typename bkP::domainP> &tlwe,             \
        const CircuitKey<bkP, privksP> &ck, const uint32_t digit)
TFHEPP_EXPLICIT_INSTANTIATION_LVL0221_0222(INST)
#undef INST

template <class bkP, class privksP>
void CircuitBootstrapping(TRGSW<typename privksP::targetP> &trgsw,
                          const TLWE<typename bkP::domainP> &tlwe,
                          const CircuitKey<bkP, privksP> &ck)
{
    for (int i = 0; i < privksP::targetP::l; i++) {
        CircuitBootstrappingPartial(trgsw[i], trgsw[i + privksP::targetP::l],
                                    tlwe, ck, i);
    }
}
#define INST(bkP, privksP)                            \
    template void CircuitBootstrapping<bkP, privksP>( \
        TRGSW<typename privksP::targetP> & trgsw,     \
        const TLWE<typename bkP::domainP> &tlwe,      \
        const CircuitKey<bkP, privksP> &ck)
TFHEPP_EXPLICIT_INSTANTIATION_LVL0221_0222(INST)
#undef INST

template <class bkP, class privksP>
void CircuitBootstrappingFFT(TRGSWFFT<typename privksP::targetP> &trgswfft,
                             const TLWE<typename bkP::domainP> &tlwe,
                             const CircuitKey<bkP, privksP> &ck)
{
    for (int i = 0; i < privksP::targetP::l; i++) {
        TRLWE<typename privksP::targetP> trgswupper, trgswlower;
        CircuitBootstrappingPartial<bkP, privksP>(trgswupper, trgswlower, tlwe,
                                                  ck, i);
        for (int j = 0; j < 2; j++) {
            TwistIFFT<typename privksP::targetP>(trgswfft[i][j], trgswupper[j]);
            TwistIFFT<typename privksP::targetP>(
                trgswfft[i + privksP::targetP::l][j], trgswlower[j]);
        }
    }
}
#define INST(bkP, privksP)                               \
    template void CircuitBootstrappingFFT<bkP, privksP>( \
        TRGSWFFT<typename privksP::targetP> & trgswfft,  \
        const TLWE<typename bkP::domainP> &tlwe,         \
        const CircuitKey<bkP, privksP> &ck)
TFHEPP_EXPLICIT_INSTANTIATION_LVL0221_0222(INST)
#undef INST

template <class bkP, class privksP>
void CircuitBootstrappingFFTInv(
    TRGSWFFT<typename privksP::targetP> &invtrgswfft,
    const TLWE<typename bkP::domainP> &tlwe, const CircuitKey<bkP, privksP> &ck)
{
    TLWE<typename bkP::domainP> invtlwe;
    // HomNot
    for (int i = 0; i <= bkP::domainP::n; i++) invtlwe[i] = -tlwe[i];
    CircuitBootstrappingFFT(invtrgswfft, invtlwe, ck);
}
#define INST(bkP, privksP)                                  \
    template void CircuitBootstrappingFFTInv<bkP, privksP>( \
        TRGSWFFT<typename privksP::targetP> & invtrgswfft,  \
        const TLWE<typename bkP::domainP> &tlwe,            \
        const CircuitKey<bkP, privksP> &ck)
TFHEPP_EXPLICIT_INSTANTIATION_LVL0221_0222(INST)
#undef INST

template <class bkP, class privksP>
void CircuitBootstrappingFFTwithInvPartial(
    TRLWEInFD<typename privksP::targetP> &trgswfftupper,
    TRLWEInFD<typename privksP::targetP> &trgswfftlower,
    TRLWEInFD<typename privksP::targetP> &invtrgswfftupper,
    TRLWEInFD<typename privksP::targetP> &invtrgswfftlower,
    const TLWE<typename bkP::domainP> &tlwe, const CircuitKey<bkP, privksP> &ck,
    const uint32_t digit)
{
    constexpr array<typename privksP::targetP::T, privksP::targetP::l> h =
        hgen<typename privksP::targetP>();
    TRLWE<typename privksP::targetP> trgswupper, trgswlower;
    CircuitBootstrappingPartial(trgswupper, trgswlower, tlwe, ck, digit);
    for (int j = 0; j < 2; j++) {
        TwistIFFT<typename privksP::targetP>(trgswfftupper[j], trgswupper[j]);
        TwistIFFT<typename privksP::targetP>(trgswfftlower[j], trgswlower[j]);
    }
    for (int j = 0; j < privksP::targetP::n; j++) {
        trgswupper[0][j] *= -1;
        trgswupper[1][j] *= -1;
        trgswlower[0][j] *= -1;
        trgswlower[1][j] *= -1;
    }
    trgswupper[0][0] += h[digit];
    trgswlower[1][0] += h[digit];
    for (int j = 0; j < 2; j++) {
        TwistIFFT<typename privksP::targetP>(invtrgswfftupper[j],
                                             trgswupper[j]);
        TwistIFFT<typename privksP::targetP>(invtrgswfftlower[j],
                                             trgswlower[j]);
    }
}
#define INST(bkP, privksP)                                             \
    template void CircuitBootstrappingFFTwithInvPartial<bkP, privksP>( \
        TRLWEInFD<typename privksP::targetP> & trgswfftupper,          \
        TRLWEInFD<typename privksP::targetP> & trgswfftlower,          \
        TRLWEInFD<typename privksP::targetP> & invtrgswfftupper,       \
        TRLWEInFD<typename privksP::targetP> & invtrgswfftlower,       \
        const TLWE<typename bkP::domainP> &tlwe,                       \
        const CircuitKey<bkP, privksP> &ck, const uint32_t digit)
TFHEPP_EXPLICIT_INSTANTIATION_LVL0221_0222(INST)
#undef INST

template <class bkP, class privksP>
void CircuitBootstrappingFFTwithInv(
    TRGSWFFT<typename privksP::targetP> &trgswfft,
    TRGSWFFT<typename privksP::targetP> &invtrgswfft,
    const TLWE<typename bkP::domainP> &tlwe, const CircuitKey<bkP, privksP> &ck)
{
    for (int i = 0; i < privksP::targetP::l; i++) {
        CircuitBootstrappingFFTwithInvPartial(
            trgswfft[i], trgswfft[i + privksP::targetP::l], invtrgswfft[i],
            invtrgswfft[i + privksP::targetP::l], tlwe, ck, i);
    }
}
#define INST(bkP, privksP)                                      \
    template void CircuitBootstrappingFFTwithInv<bkP, privksP>( \
        TRGSWFFT<typename privksP::targetP> & trgswfft,         \
        TRGSWFFT<typename privksP::targetP> & invtrgswfft,      \
        const TLWE<typename bkP::domainP> &tlwe,                \
        const CircuitKey<bkP, privksP> &ck)
TFHEPP_EXPLICIT_INSTANTIATION_LVL0221_0222(INST)
#undef INST

}  // namespace TFHEpp
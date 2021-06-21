#include <gatebootstrapping.hpp>
#include <keyswitch.hpp>
#include <math.h>


namespace TFHEpp {

// No input
void HomCONSTANTONE(TLWE<lvl0param> &res)
{
    res = {};
    res[lvl0param::n] = lvl0param::mu;
}

void HomCONSTANTZERO(TLWE<lvl0param> &res)
{
    res = {};
    res[lvl0param::n] = -lvl0param::mu;
}

// 1 input
void HomNOT(TLWE<lvl0param> &res, const TLWE<lvl0param> &ca)
{
    for (int i = 0; i <= lvl0param::n; i++) res[i] = -ca[i];
}

void HomCOPY(TLWE<lvl0param> &res, const TLWE<lvl0param> &ca)
{
    for (int i = 0; i <= lvl0param::n; i++) res[i] = ca[i];
}


void HomADD(TLWE<lvl0param> &res, const TLWE<lvl0param> &ca, const TLWE<lvl0param> &cb)
{
    for (int i = 0; i <= lvl0param::n; i++) res[i] = ca[i] + cb[i];
}

void HomADDCONST(TLWE<lvl0param> &res, const TLWE<lvl0param> &ca, const double &b, Encoder &encoder)
{
    for (int i = 0; i < lvl0param::n; i++) res[i] = ca[i];
    res[lvl0param::n] = ca[lvl0param::n] + encoder.dtotx(encoder.encode_0_1(b));
    //for (int i = 0; i <= lvl0param::n; i++) res[i] = ca[i];
    //for (int i = 0; i <= lvl0param::n; i++){
    //    double tmp = encoder.decode_second(res[i]);
    //    if(i == lvl0param::n){
    //        tmp += encoder.encode_0_1(b);
    //        res[i] = encoder.dtotx(tmp);
    //    }else{
    //        res[i] = encoder.dtotx(tmp);
    //    }
    //}
}

void HomMULTCONST(TLWE<lvl0param> &res, const TLWE<lvl0param> &ca, const double &b, Encoder &encoder, int bits)
{
    double b_int = floor(b);
    double b_decimal = b - floor(b);

    double t1 = b_decimal * pow(2., bits);
    double t2 = round(t1);
    t1 = t2 / pow(2., bits);

    double new_b = b_int + t1;

    encoder.d2 = encoder.d2 / double(bits);
    printf("here d2: %f\n", encoder.d2);
    //encoder.b = encoder.a + encoder.d;
    double ind = 0;
    while(encoder.d2*(ind+1) <= b_int){
        ind += 1;
    }



    vector<double>::iterator it;

    //it = encoder.div.begin();
    //it = encoder.div.insert(it, bits);
    //it = encoder.ind.begin();
    //it = encoder.ind.insert(it, ind);
    //it = encoder.mul.begin();
    //it = encoder.mul.insert(it, (encoder.a+encoder.b)/double(bits));

    encoder.div.push_back(bits);
    encoder.ind.push_back(ind);
    encoder.mul.push_back(encoder.d2);

    //encoder.div.push_back(2.);
    //encoder.ind.push_back(0.);
    //encoder.mul.push_back((encoder.a+encoder.b)/2.);
    //if(b_decimal != 0){
    //    encoder.interp_num *= 1.0/b_decimal;
    //}else{
    //}

    //printf("interp %f\n", encoder.interp_num);



    if(b >=0){
        for (int i = 0; i <= lvl0param::n; i++){

            //double ca_i_double = encoder.decode_second(ca[i]);
            double ca_i_double = encoder.t32tod(ca[i]);
            //double b_encode = encoder.encode_0_1(b);
            double tmp_here = ca_i_double * new_b;

            //double tmp_here = ca_i_double * b;
            tmp_here = tmp_here - floor(tmp_here);
            //printf("%f\n", tmp_here);
            //printf("%f\n", tmp_here/ca_i_double);

            //while(tmp_here > 1.5){
            //    printf("mid: %f\n", tmp_here);
            //    tmp_here -= 0.5;
            //}
            //printf("while: %f\n", tmp_here);
            
            res[i] = encoder.dtotx(tmp_here);
            
            //auto tmpa = encoder.encode_0_1(b);
            //auto tmpb = encoder.dtotx(encoder.encode_0_1(b));
            //cout << "here" << endl;
            //cout << tmpa << endl;
            //cout << tmpb << endl;
            //res[i] = ca[i] * encoder.dtotx(encoder.encode_0_1(b));


        }
    //encoder.b = encoder.b * encoder2.b;
    //encoder.d = encoder.d * encoder2.b;

    }else{
        double tmp_b = abs(b);
        for (int i = 0; i <= lvl0param::n; i++){

            double b_decimal = tmp_b - int(tmp_b);
            int inv_b = int(1./b_decimal);

            lvl0param::T tmp = inv_b == 0 ? 0 : ca[i]/inv_b;
            lvl0param::T tmp1 = ca[i] * int(tmp_b) + tmp;
            res[i] = -tmp1;
        }

    }
}

void HomADD(TRLWE<lvl1param> &res, const TRLWE<lvl1param> &ca, const TRLWE<lvl1param> &cb)
{
    for (int i = 0; i <2; i++) 
        for(int j=0; j<= lvl1param::n; j++)
            res[i][j] = ca[i][j] + cb[i][j];
}

void HomADDCONST(TRLWE<lvl1param> &res, const TRLWE<lvl1param> &ca, const array<double, lvl1param::n> &b, Encoder &encoder)
{
    for (int i = 0; i < lvl1param::n; i++) res[0][i] = ca[0][i];
    for (int i = 0; i < lvl1param::n; i++) res[1][i] = ca[1][i] + encoder.dtotx(encoder.encode_0_1(b[i]));
}

void HomSUB(TLWE<lvl0param> &res, const TLWE<lvl0param> &ca, const TLWE<lvl0param> &cb)
{
    for (int i = 0; i <= lvl0param::n; i++) res[i] = ca[i] - cb[i];
}

void HomSUB(TRLWE<lvl1param> &res, const TRLWE<lvl1param> &ca, const TRLWE<lvl1param> &cb)
{
    for (int i = 0; i <2; i++) 
        for(int j=0; j<= lvl1param::n; j++)
            res[i][j] = ca[i][j] - cb[i][j];
}


template <int casign, int cbsign, typename lvl0param::T offset>
inline void HomGate(TLWE<lvl0param> &res, const TLWE<lvl0param> &ca,
                    const TLWE<lvl0param> &cb, const GateKey &gk)
{
    for (int i = 0; i <= lvl0param::n; i++)
        res[i] = casign * ca[i] + cbsign * cb[i];
    res[lvl0param::n] += offset;
    GateBootstrapping(res, res, gk);
}

void HomNAND(TLWE<lvl0param> &res, const TLWE<lvl0param> &ca,
             const TLWE<lvl0param> &cb, const GateKey &gk)
{
    HomGate<-1, -1, lvl0param::mu>(res, ca, cb, gk);
}

void HomNOR(TLWE<lvl0param> &res, const TLWE<lvl0param> &ca,
            const TLWE<lvl0param> &cb, const GateKey &gk)
{
    HomGate<-1, -1, -lvl0param::mu>(res, ca, cb, gk);
}

void HomXNOR(TLWE<lvl0param> &res, const TLWE<lvl0param> &ca,
             const TLWE<lvl0param> &cb, const GateKey &gk)
{
    HomGate<-2, -2, -2 * lvl0param::mu>(res, ca, cb, gk);
}

void HomAND(TLWE<lvl0param> &res, const TLWE<lvl0param> &ca,
            const TLWE<lvl0param> &cb, const GateKey &gk)
{
    HomGate<1, 1, -lvl0param::mu>(res, ca, cb, gk);
}

void HomOR(TLWE<lvl0param> &res, const TLWE<lvl0param> &ca,
           const TLWE<lvl0param> &cb, const GateKey &gk)
{
    HomGate<1, 1, lvl0param::mu>(res, ca, cb, gk);
}

void HomXOR(TLWE<lvl0param> &res, const TLWE<lvl0param> &ca,
            const TLWE<lvl0param> &cb, const GateKey &gk)
{
    HomGate<2, 2, 2 * lvl0param::mu>(res, ca, cb, gk);
}

void HomANDNY(TLWE<lvl0param> &res, const TLWE<lvl0param> &ca,
              const TLWE<lvl0param> &cb, const GateKey &gk)
{
    HomGate<-1, 1, -lvl0param::mu>(res, ca, cb, gk);
}

void HomANDYN(TLWE<lvl0param> &res, const TLWE<lvl0param> &ca,
              const TLWE<lvl0param> &cb, const GateKey &gk)
{
    HomGate<1, -1, -lvl0param::mu>(res, ca, cb, gk);
}

void HomORNY(TLWE<lvl0param> &res, const TLWE<lvl0param> &ca,
             const TLWE<lvl0param> &cb, const GateKey &gk)
{
    HomGate<-1, 1, lvl0param::mu>(res, ca, cb, gk);
}

void HomORYN(TLWE<lvl0param> &res, const TLWE<lvl0param> &ca,
             const TLWE<lvl0param> &cb, const GateKey &gk)
{
    HomGate<1, -1, lvl0param::mu>(res, ca, cb, gk);
}

// 3input
// cs?c1:c0
void HomMUX(TLWE<lvl0param> &res, const TLWE<lvl0param> &cs,
            const TLWE<lvl0param> &c1, const TLWE<lvl0param> &c0,
            const GateKey &gk)
{
    TLWE<lvl0param> temp;
    for (int i = 0; i <= lvl0param::n; i++) temp[i] = cs[i] + c1[i];
    for (int i = 0; i <= lvl0param::n; i++) res[i] = -cs[i] + c0[i];
    temp[lvl0param::n] -= lvl0param::mu;
    res[lvl0param::n] -= lvl0param::mu;
    TLWE<lvl1param> and1;
    TLWE<lvl1param> and0;
    GateBootstrappingTLWE2TLWEFFT<lvl01param>(and1, temp, gk.bkfftlvl01);
    GateBootstrappingTLWE2TLWEFFT<lvl01param>(and0, res, gk.bkfftlvl01);

    for (int i = 0; i <= lvl1param::n; i++) and1[i] += and0[i];
    and1[lvl1param::n] += lvl1param::mu;
    IdentityKeySwitch<lvl10param>(res, and1, gk.ksk);
}

void HomNMUX(TLWE<lvl0param> &res, const TLWE<lvl0param> &cs,
            const TLWE<lvl0param> &c1, const TLWE<lvl0param> &c0,
            const GateKey &gk)
{
    TLWE<lvl0param> temp;
    for (int i = 0; i <= lvl0param::n; i++) temp[i] = cs[i] + c1[i];
    for (int i = 0; i <= lvl0param::n; i++) res[i] = -cs[i] + c0[i];
    temp[lvl0param::n] -= lvl0param::mu;
    res[lvl0param::n] -= lvl0param::mu;
    TLWE<lvl1param> and1;
    TLWE<lvl1param> and0;
    GateBootstrappingTLWE2TLWEFFT<lvl01param>(and1, temp, gk.bkfftlvl01);
    GateBootstrappingTLWE2TLWEFFT<lvl01param>(and0, res, gk.bkfftlvl01);

    for (int i = 0; i <= lvl1param::n; i++) and1[i] = -and1[i]-and0[i];
    and1[lvl1param::n] -= lvl1param::mu;
    IdentityKeySwitch<lvl10param>(res, and1, gk.ksk);
}

template <class P>
void HomMUXwoSE(TRLWE<typename P::targetP> &res,
                const TLWE<typename P::domainP> &cs,
                const TLWE<typename P::domainP> &c1,
                const TLWE<typename P::domainP> &c0,
                const BootstrappingKeyFFT<P> &bkfft)
{
    TLWE<typename P::domainP> temp1;
    TLWE<typename P::domainP> temp0;
    for (int i = 0; i <= P::domainP::n; i++) temp1[i] = cs[i] + c1[i];
    for (int i = 0; i <= P::domainP::n; i++) temp0[i] = -cs[i] + c0[i];
    temp1[lvl0param::n] -= P::domainP::mu;
    temp0[lvl0param::n] -= P::domainP::mu;
    TRLWE<typename P::targetP> and0;
    GateBootstrappingTLWE2TRLWEFFT<P>(res, temp1, bkfft);
    GateBootstrappingTLWE2TRLWEFFT<P>(and0, temp0, bkfft);

    for (int i = 0; i < P::targetP::n; i++) {
        res[0][i] += and0[0][i];
        res[1][i] += and0[1][i];
    };
    res[1][0] += P::targetP::mu;
}
#define INST(P)                                                      \
    template void HomMUXwoSE<P>(TRLWE<typename P::targetP> & res,    \
                                const TLWE<typename P::domainP> &cs, \
                                const TLWE<typename P::domainP> &c1, \
                                const TLWE<typename P::domainP> &c0, \
                                const BootstrappingKeyFFT<P> &bkfft)
TFHEPP_EXPLICIT_INSTANTIATION_LVL01_02(INST);
#undef INST

void ExtractSwitchAndHomMUX(TRLWE<lvl1param> &res, const TRLWE<lvl1param> &csr,
                            const TRLWE<lvl1param> &c1r,
                            const TRLWE<lvl1param> &c0r, const GateKey &gk)
{
    TLWE<lvl1param> templvl1;
    TLWE<lvl0param> cs, c1, c0;
    SampleExtractIndex<lvl1param>(templvl1, csr, 0);
    IdentityKeySwitch<lvl10param>(cs, templvl1, gk.ksk);
    SampleExtractIndex<lvl1param>(templvl1, c1r, 0);
    IdentityKeySwitch<lvl10param>(c1, templvl1, gk.ksk);
    SampleExtractIndex<lvl1param>(templvl1, c0r, 0);
    IdentityKeySwitch<lvl10param>(c0, templvl1, gk.ksk);

    for (int i = 0; i <= lvl0param::n; i++) c1[i] += cs[i];
    for (int i = 0; i <= lvl0param::n; i++) c0[i] -= cs[i];
    c1[lvl0param::n] -= lvl0param::mu;
    c0[lvl0param::n] -= lvl0param::mu;
    TRLWE<lvl1param> and0;
    GateBootstrappingTLWE2TRLWEFFT<lvl01param>(res, c1, gk.bkfftlvl01);
    GateBootstrappingTLWE2TRLWEFFT<lvl01param>(and0, c0, gk.bkfftlvl01);

    for (int i = 0; i < lvl1param::n; i++) {
        res[0][i] += and0[0][i];
        res[1][i] += and0[1][i];
    };
    res[1][0] += lvl1param::n;
}

}  // namespace TFHEpp
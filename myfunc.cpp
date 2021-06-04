#include "myfunc.h"



using namespace std;
//======================================================//
//                   File reader                        //
//======================================================//
void intel32_reader(string fname, vector<TCshort> &dout){
    ifstream fin(fname, ios::in | ios::binary);
    if(!fin.is_open()){
        cout << "File can't opened" << endl;
    }else{
        TCshort din;
        while( fin.read( (char*)&din, sizeof(TCshort)) ){
            dout.emplace_back(din);
        }
    }
    fin.close();
}



//======================================================//
//                   File writer                        //
//======================================================//
void char_writer(string fname, vector<char> &din){
    ofstream fout(fname, ios::out | ios::binary);
    if(!fout.is_open()){
        cout << "File can't opened" << endl;
    }else{
        for(unsigned int i=0; i<din.size(); i++){
            fout.write( static_cast<char*>(&din[i]), sizeof(char) );
        }
    }
    fout.close();
}

void intel32_writer(string fname, const vector<TCfloat> &din /*vector<TCshort> &din*/){
    ofstream fout(fname, ios::out | ios::binary);
        if(!fout.is_open()){
            cout << "File can't opened" << endl;
        }else{
            TCshort dout;
            for(unsigned int i=0; i<din.size(); i++){
                dout = TCshort(short(din[i].real()*512.0f), short(din[i].imag()*512.0f));
                fout.write( (char*)&dout, sizeof(TCshort) );
            }
        }
        fout.close();
}

void coe_int32_writer(string fname, vector<TCshort> &din){
    ofstream fout(fname, ios::out | ios::binary);
    if(!fout.is_open()){
        cout << "File can't opened" << endl;
    }else{
        for(unsigned int i=0; i<din.size(); i++){
            fout.write( (char*)&din[i], sizeof(TCshort) );
        }
    }
    fout.close();
}

//======================================================//
//                        ARU                           //
//======================================================//
void Aru(TCfloat &in, TCfloat &out){
    const float gain0 = 1.0;
    const float treshold = 0.35f * 0.35f * 2.0f; // ideal_I * ideal_I + ideal_Q * ideal_Q
    const float betta = 5e-4f;

    static float end_gain = 1.0, gainAcc = 0;
    static float err = 0;

    out = in * end_gain;
    err   = -(out.real() * out.real() + out.imag() * out.imag()) + treshold;
    gainAcc += betta * err;
    end_gain = gain0 + gainAcc;
}

//======================================================//
//                Scrambler  V35                        //
//======================================================//
void ApplyV35Scrambler(vector<unsigned char> &m_DecodedData, vector<unsigned char> &m_DescrambledData)
{
    unsigned int  sreg=0;
    unsigned char x,z;

    for(unsigned int i=0; i<m_DecodedData.size(); i++)
    {
        x = ((sreg>>2)&1) ^ ((sreg>>19)&1);
        z = x ^ m_DecodedData[i];
        sreg<<=1;
        sreg+= m_DecodedData[i];
        m_DescrambledData.emplace_back(z^1);
    }
}

//======================================================//
//                Conv encoder 1/2                      //
//======================================================//
void ConvEncoder1_2(const vector<unsigned char> &data, vector<unsigned char> &encodeData){
    if(encodeData.size() > 0)
        encodeData.clear();

    long long unsigned int sreg = 0;
    unsigned char d = 0, p = 0;
    for(unsigned int i=0; i<data.size(); i++){
        sreg <<= 1;
        sreg += data[i];
        d = sreg & 1;
        p = ( ((sreg>>0)&1) ^ ((sreg>>1)&1) ^ ((sreg>>2)&1) ^
              ((sreg>>5)&1) ^ ((sreg>>6)&1) ^ ((sreg>>9)&1) ^
              ((sreg>>12)&1) ^ ((sreg>>13)&1) ^ ((sreg>>17)&1) ^
              ((sreg>>18)&1) ^ ((sreg>>19)&1) ^ ((sreg>>22)&1) ^
              ((sreg>>24)&1) ^ ((sreg>>26)&1) ^ ((sreg>>28)&1) ^
              ((sreg>>29)&1) ^ ((sreg>>32)&1) ^ ((sreg>>34)&1) ^
              ((sreg>>35)&1)
            );
        encodeData.emplace_back(d);
        encodeData.emplace_back(p);
    }
}

//======================================================//
//                awgn generator    by yaroslav         //
//======================================================//
void awgnGen(float &x, float &y){
    float u1, u2, s;
    float a, b, c, w, k, l;
    c = static_cast<float>(RAND_MAX);
    do
    {
        a = static_cast<float>(rand());
        b = static_cast<float>(rand());
        k = a / c;
        l = b / c;
        u1 = 2.0f * k - 1.0f;
        u2 = 2.0f * l - 1.0f;

        s = u1 * u1 + u2 * u2;
    }
    while ( s >= 1.0f || fabs(s)<1e-10f  );

    w =  sqrt(-2.0f * log(s) / s);

    x = u1 * w;
    y = u2 * w;
}

//======================================================//
//                sigma calc        by yaroslav         //
//======================================================//
float sigmaCalc(int SNR, float symbRate){
    float noiseSigma = 0;
    float a, f_snr;
    f_snr = static_cast<float>(SNR);
    a = static_cast<float>(pow(10, -f_snr/10.0f));
    noiseSigma = 0.2501f * (sqrt(a) / symbRate);  // mean power for qpsk only
    return noiseSigma;
}

void byteFormer(const QVector<quint8> &decData, QVector<char> &decDataBytes){
    int k =0;
    qint8 byte = 0;
    for(auto i: decData){
        if(i == 1)
            byte |= (1 << k);
        else
            byte &= ~(1 << k);
        k++;
        if(k==8){
            decDataBytes.append(byte);
            k = 0;
            byte = 0;
        }
    }
}

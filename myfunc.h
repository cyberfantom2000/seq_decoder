#ifndef MYFUNC_H
#define MYFUNC_H
#include <vector>
#include "stdlib.h"
#include <string>
#include <fstream>
#include <iostream>
#include <queue>
#include <complex>
//#include <cmath>
#include <vector>
#include <string>
#include <fstream>
#include <iostream>
#include <queue>
#include <complex>
#include <cstdlib>
#include <math.h>

using namespace std;

typedef complex<float> TCfloat;
typedef complex<short> TCshort;

void char_writer(string fname, vector<char> &din);
void intel32_reader(string fname, vector<TCshort> &dout);
void intel32_writer(string fname, const vector<TCfloat> &din /*vector<TCshort> &din*/);
void Aru(TCfloat &in, TCfloat &out);
void coe_int32_writer(string fname, vector<TCshort> &din);
void ApplyV35Scrambler(vector<unsigned char> &m_DecodedData, vector<unsigned char> &m_DescrambledData);
void hd_reader(string fname, vector<unsigned char> &dout);
void ConvEncoder1_2(const vector<unsigned char> &data, vector<unsigned char> &encodeData);
void awgnGen(float &x, float &y);
float sigmaCalc(int SNR, float symbRate);

#endif // MYFUNC_H

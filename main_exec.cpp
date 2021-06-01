#include "myfunc.h"
#include "seqdecoder.h"

void main_exec()
{
    vector<TCshort> fdata_i;
    intel32_reader("iq_noscramb.bin", fdata_i);
    qDebug() << "read complete";


    vector<unsigned char> decode;
    for(uint i=0; i<fdata_i.size(); i++){
        if(fdata_i[i].real() > 0 && fdata_i[i].imag() > 0){
            // I > 0, Q > 0
            decode.emplace_back(0);
            decode.emplace_back(0);
        }else if(fdata_i[i].real() < 0 && fdata_i[i].imag() > 0){
            // I < 0, Q > 0
            decode.emplace_back(0);
            decode.emplace_back(1);
        }else if(fdata_i[i].real() > 0 && fdata_i[i].imag() < 0){
            // I > 0, Q < 0
            decode.emplace_back(1);
            decode.emplace_back(0);
        }else{
            // I < 0, Q < 0
            decode.emplace_back(1);
            decode.emplace_back(1);
        }
    }
    qDebug() << "demodulate complete";

    SeqDecoder decoder(SeqDecoder::Intelsat_1_2);
    decoder.setDefaultParams(SeqDecoder::Intelsat_1_2);
    qDebug() << "default params set";

    QList<quint8> decodeList;
    QVector<quint8> decodeVector = QVector<quint8>::fromStdVector(decode);
    decodeList = decodeVector.toList();

    decoder.addSymbs(decodeList);
    qDebug() << "add symbs complete";
    decoder.decode();
    qDebug() << "decode complete";
}

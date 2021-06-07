#include <QCoreApplication>
#include "myfunc.h"
#include "seqdecoder.h"


int main( )
{
    vector<TCshort> fdata_i;
    intel32_reader("iq.bin", fdata_i);
    qDebug() << "read complete";

    // поворот на угол degree
    float PI = 3.14159265f;
    float degree = 90.0f;
    bool spectrInv = false;
    qint16 cosM = static_cast<qint16>(cos(PI*degree / 180.0f));
    qint16 sinM = static_cast<qint16>(sin(PI*degree / 180.0f));
    qint16 newI, newQ;
    for(uint j=0; j<fdata_i.size(); j++){
        if(spectrInv){
            newI = (fdata_i[j].real()*sinM + fdata_i[j].imag()*cosM);
            newQ = (fdata_i[j].real()*cosM - fdata_i[j].imag()*sinM);
        }else{
            newI = (fdata_i[j].real()*cosM - fdata_i[j].imag()*sinM);
            newQ = (fdata_i[j].real()*sinM + fdata_i[j].imag()*cosM);
        }
        fdata_i[j] = TCshort(newI, newQ);
    }

    vector<quint8> hd;
    QVector<quint8> noErrDecData;
    quint8 x, y;
    for(uint i=0; i<fdata_i.size(); i++){
        x = fdata_i[i].real() < 0;
        y = fdata_i[i].imag() < 0;
        hd.emplace_back(x);
        hd.emplace_back(y);
        noErrDecData.append(x);
    }
    qDebug() << "demodulate complete";

    /* QRandomGenerator randGen;
     * int size = hd.size();
    for(uint i=0; i<200; i++){
        hd[randGen.bounded(36, size)] = !hd[randGen.bounded(36, size)];
    } */

    hd[515] = ~hd[515];

    SeqDecoder decoder(SeqDecoder::Intelsat_1_2);
    decoder.setDefaultParams(SeqDecoder::Intelsat_1_2);
    qDebug() << "default params set";

    QList<quint8> decodeList;
    QVector<quint8> decodeVector = QVector<quint8>::fromStdVector(hd);
    decodeList = decodeVector.toList();


    if(decoder.addSymbs(decodeList)){
        qDebug() << "add symbs complete";
        decoder.decode();
        QVector<quint8> decData = decoder.getDecodeData();
        QVector<qint32> numErr;
        for(int w=0; w<decData.size()-256; w++){
            if(decData[w] != noErrDecData[w])
                numErr.append(w);
        }



        //QVector<char> decDataBytes;
        //byteFormer(decData, decDataBytes);
        //vector<char> vec = decDataBytes.toStdVector();
        //char_writer("decData.bin", vec);
        qDebug() << "decode complete";
        return 0;
    }else{
        qDebug() << "add symbs failed";
    }
}

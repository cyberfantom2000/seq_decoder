#include "seq_decoder.h"

Seq_decoder::Seq_decoder(CodeRate code, ModulationType mod)
{
    m_sh_D = QSharedPointer<QList<quint8>>(new QList<quint8>());
    m_sh_P = QSharedPointer<QList<quint8>>(new QList<quint8>());
    m_dec_data = QSharedPointer<QList<quint8>>(new QList<quint8>());

    m_mod_type = mod;
    m_code_type = code;
    if(code == Intelsat_1_2) m_coder_len = 36;
    else if(code == Intelsat_3_4) m_coder_len = 63;
    else if(code == Intelsat_7_8) m_coder_len = 89;
    else m_coder_len = 36;

}

void Seq_decoder::setDeltaT(quint8 deltaT){
    m_delta_T = deltaT;
}

void Seq_decoder::setCodeType(const CodeRate &code){
    reset();
    m_code_type = code;
}

void Seq_decoder::setModType(const ModulationType &mod){
    reset();
    m_mod_type = mod;
}

void Seq_decoder::setBackStep(quint16 backStep){
    if(backStep < 256) m_back_step = backStep;
    else qDebug() << "back step num >= 256";
}

void Seq_decoder::setNormStep(quint16 normStep){
    if(normStep < 256) m_norm_step = normStep;
    else qDebug() << "back step num >= 256";
}

void Seq_decoder::reset(){
    m_sh_D->clear();
    m_sh_P->clear();
    m_dec_data->clear();
}

QSharedPointer<QList<quint8>> Seq_decoder::getDecodeData(){
    if(m_dec_data->size() > 0){
        return m_dec_data;
    }else{
        qDebug() << "decode data shift reg is empty";
        return nullptr;
    }
}

QSharedPointer<quint8[]> Seq_decoder::scramblerV35(){
    if(m_dec_data->size() > 0){
        QSharedPointer<quint8[]> descrembledData = QSharedPointer<quint8[]>(new quint8[m_dec_data->size()]);
        quint32 sreg = 0;
        quint8 x, y;
        for(auto i=0; i<m_dec_data->size(); i++){
            x = ((sreg >> 2) & 1) ^ ((sreg >> 19) & 1);
            y = x ^ *descrembledData[i];
        }
    }else{
        qDebug() << "decode data shift reg is empty";
        return nullptr;
    }

}



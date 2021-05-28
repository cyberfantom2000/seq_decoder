#include "seq_decoder.h"

Seq_decoder::Seq_decoder(CodeRate code, ModulationType mod)
{
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
    if(backStep < MAX_BACK_STEP) m_back_step = backStep;
    else qDebug() << "back step num >= MAX_BACK_STEP =" << MAX_BACK_STEP;
}

void Seq_decoder::setNormStep(quint16 normStep){
    if(normStep < MAX_BACK_STEP) m_norm_step = normStep;
    else qDebug() << "back step num >= MAX_BACK_STEP =" << MAX_BACK_STEP;
}

const QList<quint8>& Seq_decoder::getDecodeData() const{
    if(m_dec_data.size() <= 0)
        qDebug() << "decode data shift reg is empty";
     return m_dec_data;
}

const QList<quint8>& Seq_decoder::scramblerV35(){
    m_descrembled_data.clear();
    if(m_dec_data.size() > 0){
        quint32 sreg = 0;
        quint8 x, y;
        for(auto i=0; i<m_dec_data.size(); i++){
            x = ((sreg >> 2) & 1) ^ ((sreg >> 19) & 1);
            y = x ^ m_dec_data[i];
            sreg <<= 1;
            sreg += m_dec_data[i];
            m_descrembled_data[i] = y ^ 1;
        }
    }else{
        qDebug() << "no decoded data";
    }
    return m_descrembled_data;
}

void Seq_decoder::reset(){
    m_dec_data.clear();
    m_deperf_data.clear();
    m_perf_mask.clear();
    m_sh_rib.clear();
    for(auto i=0; i<MAX_BACK_STEP; i++){
        m_sh_rib.append(Rib());
    }
}

bool Seq_decoder::addSymbs(const QList<quint8> &symbs){
    qint32 a = symbs.count(0);
    qint32 b = symbs.count(1);
    if((a + b) < symbs.size()){
        m_encode_data.clear(); // FIXME скорее всего лишнее
        m_encode_data = QList<quint8>(symbs);
        return true;
    }else{
        qDebug() << "list contains elements other tha 0 or 1";
        return false;
    }
}

// FIXME: не учитывает сдвиг маски при поиске синхронизации
void Seq_decoder::deperforate_data(){
    std::array<bool, 16> mask = {true, true,
                                 true, false,
                                 true, false,
                                 true, false,
                                 true, false,
                                 true, false,
                                 true, false};
    quint8 idx = 0;
    quint8 max_idx = 1;
    if(m_code_type == Intelsat_1_2)
        max_idx = 2;
    else if(m_code_type == Intelsat_3_4)
        max_idx = 6;
    else if(m_code_type == Intelsat_7_8)
        max_idx = 14;

    m_deperf_data.clear();
    m_perf_mask.clear();
    qint32 i = 0;
    while(i < m_encode_data.size()){
        if(mask[idx]){
            m_deperf_data.append(m_encode_data[i]);
            m_perf_mask.append(true);
            i++;
        }else{
            m_deperf_data.append(0);
            m_perf_mask.append(false);
        }
        idx++;
        if(idx == max_idx)
            idx = 0;
   }
}

void Seq_decoder::decode(){
    deperforate_data();
    Rib rib, mask;
    for(auto i = 0; i < m_deperf_data.size()/2; i++){
        rib.b0 = m_deperf_data[i];
        rib.b1 = m_deperf_data[i+1];
        mask.b0 = m_perf_mask[i];
        mask.b1 = m_perf_mask[i+1];
        seq_decode(rib, mask);
    }
}


void Seq_decoder::seq_decode(Rib curRib, Rib mask){
    m_sh_rib.removeLast();
    m_sh_rib.prepend(curRib);
    while(true){

    }
}


quint8 Seq_decoder::hamming_distance(Rib &rib0, Rib &rib1, qint32 maskIdx){
    quint8 hamm = 0;
    if(rib0.b0 != rib1.b0)
        hamm = (m_perf_mask[maskIdx]) ? hamm + 1 : hamm;
    if(rib0.b1 != rib1.b1)
        hamm = (m_perf_mask[maskIdx+1]) ? hamm + 1 : hamm;
    return hamm;
}

qint16 Seq_decoder::metric_calc(Rib &rib0, Rib &rib1, Rib &curRib, bool A, qint32 maskIdx){
    qint16 metric;
    quint8 hamm0 = hamming_distance(rib0, curRib, maskIdx);
    quint8 hamm1 = hamming_distance(rib1, curRib, maskIdx);
    if(hamm0 >= hamm1)
        metric = (A) ? hamm1 : hamm0;
    else
        metric = (A) ? hamm0 : hamm1;
    return metric;
}




#include "seqdecoder.h"

SeqDecoder::SeqDecoder(CodeRate code)
{
    m_code_type = code;
    if(code == Intelsat_1_2) m_coder_len = 36;
    else if(code == Intelsat_3_4) m_coder_len = 63;
    else if(code == Intelsat_7_8) m_coder_len = 89;
}


void SeqDecoder::setDefaultParams(const CodeRate &code){
    reset();
    m_delta_T = 5;
    m_code_type = code;
    if(code == Intelsat_1_2) m_coder_len = 36;
    else if(code == Intelsat_3_4) m_coder_len = 63;
    else if(code == Intelsat_7_8) m_coder_len = 89;
    m_back_step = 180;
    m_norm_step = 100;
}


void SeqDecoder::setDeltaT(quint8 deltaT){
    m_delta_T = deltaT;
}


void SeqDecoder::setCodeType(const CodeRate &code){
    reset();
    m_code_type = code;
}


void SeqDecoder::setBackStep(quint16 backStep){
    if(backStep < MAX_BACK_STEP) m_back_step = backStep;
    else qDebug() << "back step num >= MAX_BACK_STEP =" << MAX_BACK_STEP;
}


void SeqDecoder::setNormStep(quint16 normStep){
    if(normStep < MAX_BACK_STEP) m_norm_step = normStep;
    else qDebug() << "back step num >= MAX_BACK_STEP =" << MAX_BACK_STEP;
}


const QVector<quint8>& SeqDecoder::getDecodeData() const{
    if(m_decode_data.size() <= 0)
        qDebug() << "decode data shift reg is empty";
    return m_decode_data;
}


const QList<quint8>& SeqDecoder::scramblerV35(){
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


void SeqDecoder::reset(){
    m_sh_A.clear();
    m_sh_rib.clear();
    m_sh_mask.clear();
    m_dec_data.clear();
    m_decode_data.clear();

    m_Mp = -10000;
    m_Mc = 0;
    m_Ms = 0;
    m_forward_cnt = 0;
    m_pointer = 0;

}


bool SeqDecoder::addSymbs(const QList<quint8> &symbs){
    qint32 a = symbs.count(0);
    qint32 b = symbs.count(1);
    if((a + b) <= symbs.size()){
        m_encode_data.clear();
        m_encode_data.append(symbs);
        return true;
    }else{
        qDebug() << "list contains elements other tha 0 or 1";
        return false;
    }
}


// FIXME: не учитывает сдвиг маски при поиске синхронизации
void SeqDecoder::deperforate_data(){
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


void SeqDecoder::decode(){
    deperforate_data();
    Rib rib;
    Mask mask;
    // перебрать через итератор.
    for(auto j = 0; j < m_deperf_data.size()/2; j++){
        rib.b0 = m_deperf_data[j];
        rib.b1 = m_deperf_data[j+1];
        mask.b0 = m_perf_mask[j];
        mask.b1 = m_perf_mask[j+1];
        seq_decode(rib, mask);
        if(j%1000 == 0)
            qDebug() << j;
    }
}


void SeqDecoder::seq_decode(Rib &curRib, Mask &perfMask){
    state = Idle;
    bool cycleEn = true;
    Rib rib0, rib1;
    QList<quint8> recShData;
    qint16 metric;
    quint8 decSym;

    m_pointer++; // увеличение указателя на 1 т.к. пришел новый символ

    if(m_sh_rib.size() >= MAX_BACK_STEP){
        m_sh_rib.removeLast();
        m_sh_mask.removeLast();
        m_sh_A.removeLast();
    }
    m_sh_rib.prepend(curRib);
    m_sh_mask.prepend(perfMask);
    m_sh_A.prepend(false);

    while(cycleEn){
        switch (state){
        case Idle:
            if(m_forward_cnt > m_norm_step){
                m_T = 0;
                m_Mp = - 10000;
                m_Mc = 0;
                m_forward_cnt = 0;
                m_back_cnt = 0;
            }

         // преднамеренный fallthrough в это состояние
         [[clang::fallthrough]];
         case MetricCalc:
            recShData.clear();
            recShData = m_dec_data.mid(m_pointer, m_coder_len);  // FIXME проверить pointer-1 или pointer
            // Порождение двух возможных ребер
            recover_encoder(rib0, rib1, recShData);
            // Вычисление метрики между возможными ребрами и пришедшим ребром
            metric_calc(rib0, rib1, curRib, m_sh_A[m_pointer-1], metric, decSym); // FIXME проверить pointer-1 или pointer
            m_Ms = m_Mc + metric;
            // FIXME возможно не так добавлять декодированные данные (которые уже идут на выход)
            if(m_Ms >= m_T){
                if(m_dec_data.size() >= MAX_BACK_STEP){
                    m_decode_data.append(m_dec_data[MAX_BACK_STEP-1]);
                    m_dec_data.removeLast();
                }
                m_dec_data.prepend(decSym);
                state = ForwardMove;
            }else if(m_Mp >= m_T){
                if(m_forward_cnt == 0)
                    qDebug() << "alarm m_forward_cnt < 0";
                state = BackwardMove;
            }else{
                m_T -= m_delta_T;
                state = MetricCalc; // FIXME state = Idle ???
            }
            break;

        case ForwardMove:
            m_pointer--; // декодировали символ -> передвинули указатель на этот символ
            m_forward_cnt++;
            m_Mp = m_Mc;
            m_Mc = m_Ms;
            if(m_Mp < m_T + m_delta_T)
                m_T += m_delta_T;
            state = Idle;
            if(m_pointer == 0) // если декодировать больше нечего, то выходим из цикла
                cycleEn = false;

            break;

        case BackwardMove:
            m_pointer++; // отступаем назад и передвигаем указатель на предыдущий символ
            m_forward_cnt--;
            m_Mc = m_Mp;

            // Вычисление прошлого Mp
            if(m_pointer < m_back_step-3){ // еще есть шаги назад
                recShData.clear();
                recShData = m_dec_data.mid(m_pointer-1, m_coder_len);
                recover_encoder(rib0, rib1, recShData);
                metric_calc(rib0, rib1, m_sh_rib[m_pointer], m_sh_A[m_pointer], metric, decSym); // FIXME проверить pointer-1 или pointer
                m_Mp -= metric;
            }else if(m_pointer == m_back_step-2){ // назад можно сделать еще 1 шаг
                m_Mp = 0;
            }else{ // больше идти назад нельзя
                m_Mp = -10000;
            }

            // Проверка: в прошлый раз ходили по худшему пути?
            // если да -> пробуем вернуться еще на шаг назад
            if(m_sh_A[m_pointer-1]){
                m_sh_A[m_pointer-1] = false;
                if(m_Mp >= m_T){
                    if(m_forward_cnt == 0)
                        qDebug() << "alarm m_forward_cnt < 0";
                    state = BackwardMove;
                }else{
                    m_T -= m_delta_T;
                    state = MetricCalc; // FIXME state = Idle ???
                }
            // если нет -> пробуем идти по худшему
            }else{
                m_sh_A[m_pointer-1] = true;
                state = MetricCalc;
            }

            break;
        }
   }
}


void SeqDecoder::metric_calc(Rib &rib0, Rib &rib1, Rib &curRib, bool A, qint16 &metric, quint8 &decSym){
    quint8 hamm0 = hamming_distance(rib0, curRib, m_sh_mask[m_pointer]);
    quint8 hamm1 = hamming_distance(rib1, curRib, m_sh_mask[m_pointer]);
    if(hamm0 >= hamm1){
        metric = A ? (hamm1 * (-5) + 1) : (hamm0 * (-5) + 1);
        decSym = A ? 1 : 0;
    }else{
        metric = A ? (hamm0 * (-5) + 1) : (hamm1 * (-5) + 1);
        decSym = A ? 0 : 1;
    }

}


quint8 SeqDecoder::hamming_distance(Rib &rib0, Rib &rib1, Mask &mask){
    quint8 hamm = 0;
    if(rib0.b0 != rib1.b0)
        hamm = mask.b0 ? hamm + 1 : hamm;
    if(rib0.b1 != rib1.b1)
        hamm = mask.b1 ? hamm + 1 : hamm;
    return hamm;
}


void SeqDecoder::recover_encoder(Rib &rib0, Rib &rib1, QList<quint8> &decData){
    // FIXME добавить дифф. кодер
    QList<quint8> localDecData0(decData);
    QList<quint8> localDecData1(decData);
    if(m_code_type == Intelsat_1_2){
        localDecData0.removeLast();
        localDecData1.removeLast();
        localDecData0.prepend(0);
        localDecData1.prepend(1);

        rib0.b0 = 0;
        rib0.b1 = ( (localDecData0[0]&1) ^ (localDecData0[1]&1) ^ (localDecData0[2]&1) ^
                    (localDecData0[5]&1) ^ (localDecData0[6]&1) ^ (localDecData0[9]&1) ^
                    (localDecData0[12]&1) ^ (localDecData0[13]&1) ^ (localDecData0[17]&1) ^
                    (localDecData0[18]&1) ^ (localDecData0[19]&1) ^ (localDecData0[22]&1) ^
                    (localDecData0[24]&1) ^ (localDecData0[26]&1) ^ (localDecData0[28]&1) ^
                    (localDecData0[29]&1) ^ (localDecData0[32]&1) ^ (localDecData0[34]&1) ^
                    (localDecData0[35]&1)
                  );
        rib1.b0 = 1;
        rib1.b1 = ( (localDecData1[0]&1) ^ (localDecData1[1]&1) ^ (localDecData1[2]&1) ^
                    (localDecData1[5]&1) ^ (localDecData1[6]&1) ^ (localDecData1[9]&1) ^
                    (localDecData1[12]&1) ^ (localDecData1[13]&1) ^ (localDecData1[17]&1) ^
                    (localDecData1[18]&1) ^ (localDecData1[19]&1) ^ (localDecData1[22]&1) ^
                    (localDecData1[24]&1) ^ (localDecData1[26]&1) ^ (localDecData1[28]&1) ^
                    (localDecData1[29]&1) ^ (localDecData1[32]&1) ^ (localDecData1[34]&1) ^
                    (localDecData1[35]&1)
                  );
    }
}




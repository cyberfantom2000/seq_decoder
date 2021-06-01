#ifndef SEQDECODER_H
#define SEQDECODER_H

#include <QtCore>

class SeqDecoder
{
#define MAX_BACK_STEP 256

public:
    enum CodeRate{
      Intelsat_1_2 = 0,
      Intelsat_3_4,
      Intelsat_7_8
    };

    SeqDecoder(CodeRate code);
    ~SeqDecoder(){};
    void setDefaultParams(const CodeRate &code);
    void setCodeType(const CodeRate &code);
    void setDeltaT(quint8 deltaT);
    void setBackStep(quint16 backStep);
    void setNormStep(quint16 normStep);
    const QList<quint8>& scramblerV35();
    const QVector<quint8>& getDecodeData() const;
    void reset();
    bool addSymbs(const QList<quint8> &symbs);
    void decode();

private:
    enum State{
        Idle = 0,
        MetricCalc,
        ForwardMove,
        BackwardMove
    };

    // Р РµР±СЂРѕ РєРѕРґРµСЂР° 1/2 (РёСЃРєР»СЋС‡РёС‚РµР»СЊРЅРѕ РґР»СЏ СѓРґРѕР±СЃС‚РІР°)
    struct Rib{
        quint8 b0 = 0; // С‚РёРїРѕ 0 Р±РёС‚
        quint8 b1 = 0; // С‚РёРїРѕ 1 Р±РёС‚
    };
    struct Mask{
        bool b0 = true; // С‚РёРїРѕ 0 Р±РёС‚
        bool b1 = true; // С‚РёРїРѕ 1 Р±РёС‚
    };

    void deperforate_data();
    void metric_calc(Rib &rib0, Rib &rib1, Rib &curRib, bool A, qint16 &metric, quint8 &decSym);
    quint8 hamming_distance(Rib &rib0, Rib &rib1, Mask &mask);
    void recover_encoder(Rib &rib0, Rib &rib1, QList<quint8> &decData);
    void seq_decode(Rib &curRib, Mask &perfMask);

private:
    qint64 m_T = 0;
    qint64 m_Mp = -10000, m_Mc = 0, m_Ms = 0;
    CodeRate m_code_type = Intelsat_1_2;
    quint16 m_back_step = 180, m_norm_step = 100;
    quint16 m_forward_cnt = 0, m_back_cnt = 0;
    quint16 m_pointer = 0;
    quint8 m_delta_T = 5;
    quint8 m_coder_len = 36;
    State state = Idle;
    QList<bool> m_sh_A;
    QList<Rib> m_sh_rib;
    QList<Mask> m_sh_mask;
    QList<bool> m_perf_mask;
    QList<quint8> m_dec_data, m_descrembled_data, m_encode_data;
    QList<quint8> m_deperf_data;
    QVector<quint8> m_decode_data; // РґРµРєРѕРґРёСЂРѕРІР°РЅРЅС‹Рµ РґР°РЅРЅС‹Рµ, РєРѕС‚РѕСЂС‹Рµ СѓР¶Рµ РЅРµ Р±СѓРґСѓС‚ РёР·РјРµРЅСЏС‚СЊСЃСЏ
};

#endif // SEQDECODER_H

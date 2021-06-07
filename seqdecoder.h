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
    ~SeqDecoder(){}
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
    QVector<quint8> testDec;

private:
    enum State{
        Idle = 0,
        MetricCalc,
        ForwardMove,
        BackwardMove
    };


    struct Rib{
        quint8 b0 = 0;
        quint8 b1 = 0;
    };
    struct Mask{
        bool b0 = true;
        bool b1 = true;
    };
    struct CodePolynom{
        QString ssk1_2 = "714461625313";
        QString ssk3_4 = "736750426717050772741";
        QString ssk7_8 = "776631661776007201537633372136";
    };

    void deperforate_data();
    void metric_calc(Rib &rib0, Rib &rib1, qint16 &metric, quint8 &decSym);
    quint8 hamming_distance(Rib &rib0, Rib &rib1, Mask &mask);
    void recover_encoder(Rib &rib0, Rib &rib1);
    void seq_decode();

private:
    qint64 m_T = 0;
    qint64 m_Mp = -10000, m_Mc = 0, m_Ms = 0;
    CodeRate m_code_type = Intelsat_1_2;
    quint16 m_back_step = 180, m_norm_thresh = 100;
    quint16 m_forward_cnt = 0, m_back_cnt = 0;
    quint16 m_pointer = 0;
    quint8 m_delta_T = 5;
    quint8 m_coder_len = 36;
    State state = Idle;
    QHash<CodeRate, QString> m_code_polynom;
    QList<bool> m_sh_A;
    QList<Rib> m_sh_rib;
    QList<Mask> m_sh_mask;
    QList<quint8> m_taps_polynom;
    QList<bool> m_perf_mask;
    QList<quint8> m_dec_data, m_descrembled_data, m_encode_data;
    QList<quint8> m_deperf_data;
    QVector<quint8> m_decode_data; // decoded data that will not change and go to the output
};

#endif // SEQDECODER_H

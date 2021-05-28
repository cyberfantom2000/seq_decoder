#ifndef SEQ_DECODER_H
#define SEQ_DECODER_H

#include <QtCore>

class Seq_decoder
{
#define MAX_BACK_STEP 256

public:
    enum CodeRate{
      Intelsat_1_2 = 0,
      Intelsat_3_4,
      Intelsat_7_8
    };

    enum ModulationType{
      BPSK = 0,
      QPSK
    };

    Seq_decoder(CodeRate code, ModulationType mod);
    ~Seq_decoder();
    void setCodeType(const CodeRate &code);
    void setModType(const ModulationType &mod);
    void setDeltaT(quint8 deltaT);
    void setBackStep(quint16 backStep);
    void setNormStep(quint16 normStep);
    const QList<quint8>& scramblerV35();
    const QList<quint8>& getDecodeData() const;
    void reset();
    bool addSymbs(const QList<quint8> &symbs);
    void decode();

private:
    // Ребро кодера 1/2 (исключительно для удобства)
    struct Rib{
        quint8 b0 = 0; // типо 0 бит
        quint8 b1 = 0; // типо 1 бит
    };

private:
    void deperforate_data();
    qint16 metric_calc(Rib &rib0, Rib &rib1, Rib &curRib, bool A, qint32 maskIdx);
    quint8 hamming_distance(Rib &rib0, Rib &rib1, qint32 maskIdx);
    void recover_encoder(Rib &rib0, Rib &rib1);
    void seq_decode(Rib curRib, Rib mask);

private:
    qint64 m_T = 0;
    qint64 m_Vp = 0, m_Vc = 0, m_Vs = 0;
    qint64 m_Mp = -10000, m_Mc = 0, m_Ms = 0;
    CodeRate m_code_type = Intelsat_1_2;
    ModulationType m_mod_type = QPSK;
    quint16 m_back_step = 100, m_norm_step = 20;
    quint16 m_pointer;
    quint8 m_delta_T = 5;
    quint8 m_coder_len = 36;
    bool m_A = false;
    std::array<Rib, MAX_BACK_STEP> m_sh_rib;
    QList<quint8> m_dec_data, m_descrembled_data, m_encode_data;
    QList<bool> m_perf_mask;
    QList<quint8> m_deperf_data;
};

#endif // SEQ_DECODER_H

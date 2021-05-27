#ifndef SEQ_DECODER_H
#define SEQ_DECODER_H

#include <QtCore>

enum CodeRate{
  Intelsat_1_2,
  Intelsat_3_4,
  Intelsat_7_8
};

enum ModulationType{
  BPSK,
  QPSK
};

class Seq_decoder
{
public:
    Seq_decoder(CodeRate code, ModulationType mod);
    ~Seq_decoder();
    void setCodeType(const CodeRate &code);
    void setModType(const ModulationType &mod);
    void setDeltaT(quint8 deltaT);
    void setBackStep(quint16 backStep);
    void setNormStep(quint16 normStep);
    void addSym(quint8 sym);
    quint8 decode(quint8 sym);
    void reset();
    QSharedPointer<quint8[]> scramblerV35();
    QSharedPointer<QList<quint8>> getDecodeData();

private:
    void metric_calc(quint8 rib0, quint8 rib1, quint8 curRib, bool A, quint8 mask);
    qint16 hamming_distance(quint8 rib0, quint8 rib1, quint8 mask);
    void recover_encoder(quint8 &rib0, quint8 &rib1);

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
    QSharedPointer<QList<quint8>> m_sh_D, m_sh_P, m_dec_data;
};

#endif // SEQ_DECODER_H

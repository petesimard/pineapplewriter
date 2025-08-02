#ifndef CIRCULARBUFFERDEVICE_H
#define CIRCULARBUFFERDEVICE_H

#include "audiobuffer.h"

class CircularBufferDevice : public AudioBuffer
{
    Q_OBJECT

public:
    explicit CircularBufferDevice(QObject *parent = nullptr);
    ~CircularBufferDevice();

    void setBufferSize(int sizeInBytes) override;
    int getBufferSize() const override;
    QByteArray getData() const override;
    void clear() override;

protected:
    qint64 readData(char *data, qint64 maxSize) override;
    qint64 writeData(const char *data, qint64 maxSize) override;
    qint64 size() const override;
    bool seek(qint64 pos) override;
    qint64 pos() const override;

private:
    int m_writePosition;
    int m_readPosition;

    void resizeBuffer(int newSize) override;
    void writeToCircularBuffer(const QByteArray &data);
    QByteArray readFromCircularBuffer() const;
};

#endif // CIRCULARBUFFERDEVICE_H
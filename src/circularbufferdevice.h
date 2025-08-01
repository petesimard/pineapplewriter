#ifndef CIRCULARBUFFERDEVICE_H
#define CIRCULARBUFFERDEVICE_H

#include <QIODevice>
#include <QByteArray>
#include <QMutex>

class CircularBufferDevice : public QIODevice
{
    Q_OBJECT

public:
    explicit CircularBufferDevice(QObject *parent = nullptr);
    ~CircularBufferDevice();

    void setBufferSize(int sizeInBytes);
    int getBufferSize() const;
    QByteArray getData() const;
    void clear();

protected:
    qint64 readData(char *data, qint64 maxSize) override;
    qint64 writeData(const char *data, qint64 maxSize) override;
    bool isSequential() const override { return false; }
    qint64 size() const override;
    bool seek(qint64 pos) override;
    qint64 pos() const override;

private:
    QByteArray m_buffer;
    int m_bufferSize;
    int m_writePosition;
    int m_readPosition;
    qint64 m_totalBytesWritten;
    mutable QMutex m_mutex;

    void resizeBuffer(int newSize);
    void writeToCircularBuffer(const QByteArray &data);
    QByteArray readFromCircularBuffer() const;
};

#endif // CIRCULARBUFFERDEVICE_H
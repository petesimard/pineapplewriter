#ifndef AUDIOBUFFER_H
#define AUDIOBUFFER_H

#include <QIODevice>
#include <QByteArray>
#include <QMutex>

class AudioBuffer : public QIODevice
{
    Q_OBJECT

public:
    explicit AudioBuffer(QObject *parent = nullptr);
    virtual ~AudioBuffer() = default;

    // Common interface methods
    virtual void setBufferSize(int sizeInBytes) = 0;
    virtual int getBufferSize() const = 0;
    virtual QByteArray getData() const = 0;
    virtual void clear() = 0;

    // Optional methods that some implementations might support
    virtual QByteArray readAndClear() { return QByteArray(); }
    virtual bool isFull() const { return false; }
    virtual int availableSpace() const { return 0; }

protected:
    // QIODevice virtual methods that must be implemented
    qint64 readData(char *data, qint64 maxSize) override = 0;
    qint64 writeData(const char *data, qint64 maxSize) override = 0;
    bool isSequential() const override { return false; }
    qint64 size() const override = 0;
    bool seek(qint64 pos) override = 0;
    qint64 pos() const override = 0;

    // Common protected members
    QByteArray m_buffer;
    int m_bufferSize;
    qint64 m_totalBytesWritten;
    mutable QMutex m_mutex;

    // Common protected methods
    virtual void resizeBuffer(int newSize) = 0;
};

#endif // AUDIOBUFFER_H
#ifndef FIXEDBUFFERDEVICE_H
#define FIXEDBUFFERDEVICE_H

#include <QIODevice>
#include <QByteArray>
#include <QMutex>

class FixedBufferDevice : public QIODevice
{
    Q_OBJECT

public:
    explicit FixedBufferDevice(int bufferSize = 1024 * 1024, QObject *parent = nullptr);
    ~FixedBufferDevice();

    // Set the fixed buffer size (will clear existing data)
    void setBufferSize(int sizeInBytes);
    int getBufferSize() const;

    // Get current data without clearing
    QByteArray getData() const;

    // Read current buffer and clear it
    QByteArray readAndClear();

    // Clear the buffer
    void clear();

    // Check if buffer is full
    bool isFull() const;

    // Get available space
    int availableSpace() const;

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
    qint64 m_totalBytesWritten;
    mutable QMutex m_mutex;

    void resizeBuffer(int newSize);
    void writeToBuffer(const QByteArray &data);
    QByteArray readFromBuffer() const;
    void clearBuffer(); // Shared clearing logic
};

#endif // FIXEDBUFFERDEVICE_H
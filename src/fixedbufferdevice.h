#ifndef FIXEDBUFFERDEVICE_H
#define FIXEDBUFFERDEVICE_H

#include "audiobuffer.h"

class FixedBufferDevice : public AudioBuffer
{
    Q_OBJECT

public:
    explicit FixedBufferDevice(int bufferSize = 1024 * 1024, QObject *parent = nullptr);
    ~FixedBufferDevice();

    // Set the fixed buffer size (will clear existing data)
    void setBufferSize(int sizeInBytes) override;
    int getBufferSize() const override;

    // Get current data without clearing
    QByteArray getData() const override;

    // Read current buffer and clear it
    QByteArray readAndClear() override;

    // Clear the buffer
    void clear() override;

    // Check if buffer is full
    bool isFull() const override;

    // Get available space
    int availableSpace() const override;

protected:
    qint64 readData(char *data, qint64 maxSize) override;
    qint64 writeData(const char *data, qint64 maxSize) override;
    qint64 size() const override;
    bool seek(qint64 pos) override;
    qint64 pos() const override;

private:
    int m_writePosition;

    void resizeBuffer(int newSize) override;
    void writeToBuffer(const QByteArray &data);
    QByteArray readFromBuffer() const;
    void clearBuffer(); // Shared clearing logic
};

#endif // FIXEDBUFFERDEVICE_H
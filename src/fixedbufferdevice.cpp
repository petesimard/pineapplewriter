#include "fixedbufferdevice.h"
#include <QDebug>

FixedBufferDevice::FixedBufferDevice(int bufferSize, QObject *parent)
    : QIODevice(parent), m_bufferSize(bufferSize), m_writePosition(0), m_totalBytesWritten(0)
{
    resizeBuffer(bufferSize);
    open(QIODevice::ReadWrite);
}

FixedBufferDevice::~FixedBufferDevice()
{
}

void FixedBufferDevice::setBufferSize(int sizeInBytes)
{
    QMutexLocker locker(&m_mutex);
    m_bufferSize = sizeInBytes;
    resizeBuffer(sizeInBytes);
}

int FixedBufferDevice::getBufferSize() const
{
    return m_bufferSize;
}

QByteArray FixedBufferDevice::getData() const
{
    QMutexLocker locker(&m_mutex);
    return readFromBuffer();
}

QByteArray FixedBufferDevice::readAndClear()
{
    QMutexLocker locker(&m_mutex);
    QByteArray data = readFromBuffer();
    clearBuffer();
    return data;
}

void FixedBufferDevice::clear()
{
    QMutexLocker locker(&m_mutex);
    clearBuffer();
}

bool FixedBufferDevice::isFull() const
{
    QMutexLocker locker(&m_mutex);
    return m_totalBytesWritten >= m_bufferSize;
}

int FixedBufferDevice::availableSpace() const
{
    QMutexLocker locker(&m_mutex);
    return qMax(0, m_bufferSize - static_cast<int>(m_totalBytesWritten));
}

qint64 FixedBufferDevice::readData(char *data, qint64 maxSize)
{
    QMutexLocker locker(&m_mutex);

    if (maxSize <= 0)
        return 0;

    QByteArray readData = readFromBuffer();
    qint64 bytesToRead = qMin(maxSize, static_cast<qint64>(readData.size()));

    if (bytesToRead > 0)
    {
        memcpy(data, readData.constData(), bytesToRead);
    }

    return bytesToRead;
}

qint64 FixedBufferDevice::writeData(const char *data, qint64 maxSize)
{
    QMutexLocker locker(&m_mutex);

    if (maxSize <= 0)
        return 0;

    QByteArray writeData(data, maxSize);
    writeToBuffer(writeData);

    return maxSize;
}

qint64 FixedBufferDevice::size() const
{
    QMutexLocker locker(&m_mutex);
    return m_totalBytesWritten;
}

bool FixedBufferDevice::seek(qint64 pos)
{
    QMutexLocker locker(&m_mutex);

    if (pos < 0 || pos >= m_bufferSize)
    {
        return false;
    }

    // For fixed buffer, seeking is limited to the current data size
    if (pos > m_totalBytesWritten)
    {
        return false;
    }

    return true;
}

qint64 FixedBufferDevice::pos() const
{
    QMutexLocker locker(&m_mutex);
    return 0; // Fixed buffer always starts from beginning
}

void FixedBufferDevice::resizeBuffer(int newSize)
{
    m_buffer.resize(newSize);
    m_buffer.fill(0);
    m_writePosition = 0;
    m_totalBytesWritten = 0;
}

void FixedBufferDevice::writeToBuffer(const QByteArray &data)
{
    int dataSize = data.size();
    if (dataSize == 0)
        return;

    // qDebug() << "Writing to buffer at position:" << m_writePosition << "Size:" << dataSize;

    // Check if this write would cause overflow
    if (m_totalBytesWritten + dataSize > m_bufferSize)
    {
        qDebug() << "FixedBufferDevice: Buffer overflow detected! Clearing buffer.";
        clear();

        // If data is larger than buffer, only keep the first part
        if (dataSize > m_bufferSize)
        {
            QByteArray firstPart = data.left(m_bufferSize);
            m_buffer = firstPart;
            m_writePosition = 0;
            m_totalBytesWritten = m_bufferSize;
            return;
        }
    }

    // Write data to buffer
    for (int i = 0; i < dataSize; ++i)
    {
        m_buffer[m_writePosition] = data[i];
        m_writePosition = (m_writePosition + 1) % m_bufferSize;
    }

    m_totalBytesWritten += dataSize;
}

QByteArray FixedBufferDevice::readFromBuffer() const
{
    if (m_totalBytesWritten == 0)
    {
        return QByteArray();
    }

    int availableBytes = qMin(m_totalBytesWritten, static_cast<qint64>(m_bufferSize));
    QByteArray result;
    result.reserve(availableBytes);

    // Read from beginning of buffer
    for (int i = 0; i < availableBytes; ++i)
    {
        result.append(m_buffer[i]);
    }

    return result;
}

void FixedBufferDevice::clearBuffer()
{
    m_writePosition = 0;
    m_totalBytesWritten = 0;
    // m_buffer.fill(0);
}
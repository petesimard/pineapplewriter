#include "circularbufferdevice.h"
#include <QDebug>

CircularBufferDevice::CircularBufferDevice(QObject *parent)
    : QIODevice(parent), m_bufferSize(1024 * 1024) // 1MB default
      ,
      m_writePosition(0), m_readPosition(0), m_totalBytesWritten(0)
{
    resizeBuffer(m_bufferSize);
    open(QIODevice::ReadWrite);
}

CircularBufferDevice::~CircularBufferDevice()
{
}

void CircularBufferDevice::setBufferSize(int sizeInBytes)
{
    QMutexLocker locker(&m_mutex);
    m_bufferSize = sizeInBytes;
    resizeBuffer(sizeInBytes);
}

int CircularBufferDevice::getBufferSize() const
{
    return m_bufferSize;
}

QByteArray CircularBufferDevice::getData() const
{
    QMutexLocker locker(&m_mutex);
    return readFromCircularBuffer();
}

void CircularBufferDevice::clear()
{
    QMutexLocker locker(&m_mutex);
    m_writePosition = 0;
    m_readPosition = 0;
    m_totalBytesWritten = 0;
    m_buffer.fill(0);
}

qint64 CircularBufferDevice::readData(char *data, qint64 maxSize)
{
    QMutexLocker locker(&m_mutex);

    if (maxSize <= 0)
        return 0;

    QByteArray readData = readFromCircularBuffer();
    qint64 bytesToRead = qMin(maxSize, static_cast<qint64>(readData.size()));

    if (bytesToRead > 0)
    {
        memcpy(data, readData.constData(), bytesToRead);
    }

    return bytesToRead;
}

qint64 CircularBufferDevice::writeData(const char *data, qint64 maxSize)
{
    QMutexLocker locker(&m_mutex);

    if (maxSize <= 0)
        return 0;

    QByteArray writeData(data, maxSize);
    writeToCircularBuffer(writeData);

    return maxSize;
}

qint64 CircularBufferDevice::size() const
{
    QMutexLocker locker(&m_mutex);
    return m_totalBytesWritten;
}

bool CircularBufferDevice::seek(qint64 pos)
{
    QMutexLocker locker(&m_mutex);

    if (pos < 0 || pos >= m_bufferSize)
    {
        return false;
    }

    m_readPosition = pos % m_bufferSize;
    return true;
}

qint64 CircularBufferDevice::pos() const
{
    QMutexLocker locker(&m_mutex);
    return m_readPosition;
}

void CircularBufferDevice::resizeBuffer(int newSize)
{
    m_buffer.resize(newSize);
    m_buffer.fill(0);
    m_writePosition = 0;
    m_readPosition = 0;
    m_totalBytesWritten = 0;
}

void CircularBufferDevice::writeToCircularBuffer(const QByteArray &data)
{
    int dataSize = data.size();
    if (dataSize == 0)
        return;

    // If data is larger than buffer, only keep the last part
    if (dataSize > m_bufferSize)
    {
        QByteArray lastPart = data.right(m_bufferSize);
        m_buffer = lastPart;
        m_writePosition = 0;
        m_readPosition = 0;
        m_totalBytesWritten = m_bufferSize;
        return;
    }

    // Write data to circular buffer
    for (int i = 0; i < dataSize; ++i)
    {
        m_buffer[m_writePosition] = data[i];
        m_writePosition = (m_writePosition + 1) % m_bufferSize;
    }

    m_totalBytesWritten += dataSize;

    // Update read position if we've wrapped around
    if (m_totalBytesWritten > m_bufferSize)
    {
        m_readPosition = m_writePosition;
    }
}

QByteArray CircularBufferDevice::readFromCircularBuffer() const
{
    if (m_totalBytesWritten == 0)
    {
        return QByteArray();
    }

    int availableBytes = qMin(m_totalBytesWritten, static_cast<qint64>(m_bufferSize));
    QByteArray result;
    result.reserve(availableBytes);

    int currentPos = m_readPosition;
    for (int i = 0; i < availableBytes; ++i)
    {
        result.append(m_buffer[currentPos]);
        currentPos = (currentPos + 1) % m_bufferSize;
    }

    return result;
}
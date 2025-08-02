#include "audiobuffer.h"

AudioBuffer::AudioBuffer(QObject *parent)
    : QIODevice(parent), m_bufferSize(1024 * 1024), m_totalBytesWritten(0)
{
    open(QIODevice::ReadWrite);
} 
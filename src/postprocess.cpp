#include "postprocess.h"

PostProcess::PostProcess(QObject *parent)
    : QObject(parent)
{
}

PostProcess::~PostProcess()
{
}

void PostProcess::setApiKey(const QString &apiKey)
{
    this->apiKey = apiKey;
}
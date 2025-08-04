#ifndef POSTPROCESS_H
#define POSTPROCESS_H

#include <QObject>

class PostProcess : public QObject
{
    Q_OBJECT

    explicit PostProcess(QObject *parent = nullptr);
    ~PostProcess();

public:
    void setApiKey(const QString &apiKey);

private:
    QString apiKey;
};

#endif
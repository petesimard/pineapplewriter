#ifndef HOTKEYWIDGET_H
#define HOTKEYWIDGET_H

#include <QWidget>
#include <QPushButton>
#include <QKeySequence>
#include <QTimer>

class HotkeyWidget : public QWidget
{
    Q_OBJECT

public:
    explicit HotkeyWidget(QWidget *parent = nullptr);
    ~HotkeyWidget();

    QString getHotkey() const;
    void setHotkey(const QString &hotkey);

signals:
    void hotkeyChanged(const QString &hotkey);

protected:
    bool eventFilter(QObject *obj, QEvent *event) override;

private slots:
    void startListening();
    void stopListening();
    void updateButtonText();

private:
    QPushButton *button;
    QKeySequence currentSequence;
    bool isListening;
    QTimer *timeoutTimer;

    void processKeyEvent(QKeyEvent *event);
    QString keySequenceToString(const QKeySequence &sequence) const;
};

#endif // HOTKEYWIDGET_H
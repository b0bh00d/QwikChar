#pragma once

#include <QLabel>

class ClickableLabel : public QLabel
{
    Q_OBJECT
public:
    explicit ClickableLabel(QWidget *parent = nullptr);

signals:
    void signal_clicked();

protected:
    void mousePressEvent(QMouseEvent* /*event*/) { emit signal_clicked(); }
};

#pragma once

#include <QChar>
#include <QEvent>
#include <QWidget>
#include <QPaintEvent>
#include <QMouseEvent>

static const int Height = 30;

class SelectableChar : public QWidget
{
    Q_OBJECT
public:
    explicit SelectableChar(const QChar& seq, QWidget *parent = nullptr);
    explicit SelectableChar(const QString& seq, QWidget *parent = nullptr);

    int height() const { return Height; }
    int width() const { return my_size.width(); }

//    void set_highlighted(bool hl = true) { highlighted = hl; }

protected: // methods
    void mousePressEvent(QMouseEvent *event) Q_DECL_OVERRIDE;
    void mouseReleaseEvent(QMouseEvent *event) Q_DECL_OVERRIDE;
    void enterEvent(QEvent *event) Q_DECL_OVERRIDE;
    void leaveEvent(QEvent *event) Q_DECL_OVERRIDE;
    QSize sizeHint() const Q_DECL_OVERRIDE;
    QSize minimumSizeHint() const Q_DECL_OVERRIDE;
    void paintEvent(QPaintEvent *event) Q_DECL_OVERRIDE;

signals:
    void signal_selected(const QString);

private: // methods
    void init();

private: // data members
    QString     my_sequence;
    QSize       my_size{Height, Height};
    float       my_scale{1.0};
    bool        hover{false};
    bool        selected{false};
//    bool        highlighted{false};
};

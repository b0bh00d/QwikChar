#include <QLabel>
#include <QPainter>
#include <QApplication>

#include "selectablechar.h"

SelectableChar::SelectableChar(const QChar& seq, QWidget *parent)
    : QWidget(parent),
      my_sequence(seq)
{
    init();
}

SelectableChar::SelectableChar(const QString& seq, QWidget *parent)
    : QWidget(parent),
      my_sequence(seq)
{
    init();
}

void SelectableChar::init()
{
    auto font = QApplication::font();
    auto metrics = QFontMetrics(font);

    // first character is Height in width;
    // each subsequent character adds its
    // literal width

    int width = Height;
    for(int i = 1;i < my_sequence.length();++i)
        width += metrics.horizontalAdvance(my_sequence[i]);
    width += (width > Height) ? (Height - (width % Height)) : 0;

    my_size.setWidth(width < Height ? Height : width);
}

QSize SelectableChar::minimumSizeHint() const
{
//    QSize s = QPushButton::minimumSizeHint();
    return QSize(static_cast<int>(my_size.width() * my_scale),
                 static_cast<int>(my_size.height() * my_scale));
}

QSize SelectableChar::sizeHint() const
{
//    QSize s = QPushButton::sizeHint();
    return QSize(static_cast<int>(my_size.width() * my_scale),
                 static_cast<int>(my_size.height() * my_scale));
}

void SelectableChar::paintEvent(QPaintEvent* /*event*/)
{
    QPainter painter(this);

    auto rect_scaled = QSize(static_cast<int>(my_size.width() * my_scale),
                             static_cast<int>(my_size.height() * my_scale));
    QRect s2 = QRect(0, 0, rect_scaled.width(), rect_scaled.height());
    auto point_scaled = static_cast<int>(14 * (selected ? my_scale + 0.35f : my_scale));

    QColor border_color = QColor(Qt::gray).darker();
    QPen p(border_color);
    if(hover)
        p.setColor(QColor(Qt::blue).lighter());
//    else if(highlighted)
//        p.setColor(QColor(Qt::white));
    p.setWidth(3);
    QBrush b(border_color);

    painter.save();

    painter.setPen(p);
    painter.setBrush(b);
    painter.drawRect(s2);

    painter.setPen(selected ? QColor(Qt::blue).lighter() : Qt::white);
    painter.setFont(QFont("Arial", point_scaled));
    painter.drawText(s2, Qt::AlignHCenter|Qt::AlignVCenter, my_sequence);

    painter.restore();
}

void SelectableChar::enterEvent(QEvent* event)
{
    hover = true;
    update();
    event->accept();
}

void SelectableChar::leaveEvent(QEvent* event)
{
    hover = false;
    update();
    event->accept();
}

void SelectableChar::mousePressEvent(QMouseEvent* event)
{
    selected = true;
    update();
    event->accept();
}

void SelectableChar::mouseReleaseEvent(QMouseEvent* event)
{
//    QRect s = geometry();
    selected = false;
    update();
    event->accept();

//    if(s.contains(event->x(), event->y()))
        emit signal_selected(my_sequence);
}

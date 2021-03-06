#include "memorywidget.h"

#include <QDebug>
#include <QFont>
#include <QModelIndex>
#include <QApplication>
#include <QPainter>
#include <QPalette>
#include <QEvent>
#include <QPen>
#include <QContextMenuEvent>
#include <QMenu>
#include <QClipboard>
#include <DGuiApplicationHelper>
#include "../utils.h"

MemoryWidget::MemoryWidget(int mode, QWidget *parent)
    : QWidget(parent)
    , m_listwidget(new MemoryListWidget(this))
    , m_clearbutton(new IconButton(this, true))
    , m_isempty(true)
{
    calculatormode = mode;
    m_evaluator = Evaluator::instance();
    this->setContentsMargins(0, 0, 0, 0);

    QVBoxLayout *lay = new QVBoxLayout(this);
    QHBoxLayout *layH = new QHBoxLayout();

    lay->setSpacing(0);
    lay->setMargin(0);
    lay->addWidget(m_listwidget);

    m_listwidget->setFrameShape(QFrame::NoFrame);
    mode == 0 ? m_listwidget->setFixedHeight(260) : m_listwidget->setFixedHeight(500);
    m_listwidget->setVerticalScrollMode(QListView::ScrollPerPixel);
    m_listwidget->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
    m_listwidget->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_listwidget->setAutoScroll(false);
    m_listwidget->setSelectionRectVisible(false);
    m_listwidget->setFocusPolicy(Qt::NoFocus);
    m_listwidget->setUniformItemSizes(false);
//    m_listwidget->setWordWrap(true);
//    m_listwidget->setStyleSheet("QListWidget::item{color:black;background-color:transparent;} \
//                                 QListWidget::item:hover{color:black;background-color:rgba(0,0,0,0.1 * 255);} \
//                                 QListWidget{color:black;background-color:transparent;}");
    lay->addStretch();
    layH->addStretch();

    layH->addWidget(m_clearbutton);
    if (mode == 1)
        m_clearbutton->hide();
//    m_clearbutton->setFixedSize(40, 40);
//    m_clearbutton->setStyleSheet("QPushButton {border:none;background-color: transparent;image:url(:/images/light/clear_normal.svg);} \
//                                  QPushButton:hover {border: 1px solid #000000;image:url(:/images/light/clear_hover.svg);} \
//                                  QPushButton:pressed {image:url(:/images/light/clear_press.svg);}");
    connect(m_clearbutton, &DPushButton::clicked, this, [ = ]() {
        m_listwidget->clear();
        m_listwidget->addItem(tr("Nothing saved in memory"));
        m_listwidget->item(0)->setFlags(Qt::NoItemFlags);
        m_isempty = true;
        list.clear();
        m_clearbutton->hide();
        emit mListUnavailable();
    });
    lay->addLayout(layH);
    this->setLayout(lay);
    connect(m_listwidget, &MemoryListWidget::itemselected, this, [ = ](int row) {
        QPair<QString, Quantity> p;
        p.first = m_listwidget->item(row)->data(Qt::DisplayRole).toString();
        p.second = list.at(row);
        if (m_listwidget->item(row)->flags() != Qt::NoItemFlags)
            emit itemclick(p);
    });
//    connect(m_listwidget, &QListWidget::itemPressed, this, [ = ](QListWidgetItem * item) {
//        m_listwidget->setStyleSheet("QListWidget::item{color:black;background-color:transparent;} \
//                                     QListWidget::item:selected{color:#FFFFFF;background-color:#0081FF;} \
//                                     QListWidget{color:black;background-color:transparent;}");
//    });

}

void MemoryWidget::generateData(Quantity answer)
{
    //500 memory number limit
    if (list.count() == 500) {
        list.pop_back();
        m_listwidget->takeItem(499);
    }
    if (m_isempty == true) {
        m_listwidget->clear();
    }
    m_isempty = false;
    emit mListAvailable();
    QListWidgetItem *item1 = new QListWidgetItem();
    item1->setTextAlignment(Qt::AlignRight | Qt::AlignTop);
    QFont font;
    font.setPixelSize(30);
    item1->setFont(font);
    item1->setSizeHint(QSize(344, 40 + 45 * line));
    MemoryItemWidget *widget = new MemoryItemWidget(this);
    widget->setFixedSize(344, 40 + 45 * line);
    m_listwidget->insertItem(0, item1);
    m_listwidget->setItemWidget(item1, widget);
    if (answer == Quantity(0)) {
        item1->setData(Qt::DisplayRole, "0");
    } else {
        const QString result = DMath::format(answer, Quantity::Format::General());
        QString formatResult = Utils::formatThousandsSeparators(result);
        formatResult = setitemwordwrap(formatResult);
        item1->setData(Qt::DisplayRole, formatResult);
    }
    list.insert(0, answer); //对于新增数据，同步在list中加入对应的Quantity
    connect(widget, &MemoryItemWidget::plusbtnclicked, this, [ = ]() {
        int row = m_listwidget->row(item1);
        widget->setFocus();
        emit MemoryWidget::widgetplus(row);
    });
    connect(widget, &MemoryItemWidget::minusbtnclicked, this, [ = ]() {
        int row = m_listwidget->row(item1);
        widget->setFocus();
        emit MemoryWidget::widgetminus(row);
    });
    connect(widget, &MemoryItemWidget::cleanbtnclicked, this, [ = ]() {
        widget->setFocus();
        list.removeAt(m_listwidget->row(item1));
        m_listwidget->takeItem(m_listwidget->row(item1));
        delete item1;
        if (m_listwidget->count() == 0) {
            m_listwidget->addItem(tr("Nothing saved in memory"));
            m_listwidget->item(0)->setFlags(Qt::NoItemFlags);
            m_isempty = true;
            m_clearbutton->hide();
            emit mListUnavailable();
        }
    });
    widget->themetypechanged(m_themetype);
    connect(this, &MemoryWidget::themechange, widget, &MemoryItemWidget::themetypechanged);
    connect(widget, &MemoryItemWidget::itemchanged, this, [ = ](int type) {
        if (type == 1) {
            m_listwidget->setStyleSheet("QListWidget::item{color:black;background-color:transparent;} \
                                         QListWidget::item:hover{color:black;background-color:rgba(0,0,0,0.05 * 255);} \
                                         QListWidget{color:black;background-color:transparent;}");
        } else {
            m_listwidget->setStyleSheet("QListWidget::item{color:#B4B4B4;background-color:transparent;} \
                                         QListWidget::item:hover{color:#B4B4B4;background-color:rgba(255,255,255,0.05 * 255);} \
                                         QListWidget{color:#B4B4B4;background-color:transparent;}");
        }
    });
    connect(widget, &MemoryItemWidget::menuclean, this, [ = ]() {
        list.removeAt(m_listwidget->row(item1));
        m_listwidget->takeItem(m_listwidget->row(item1));
        delete item1;
        if (m_listwidget->count() == 0) {
            m_listwidget->addItem(tr("Nothing saved in memory"));
            m_listwidget->item(0)->setFlags(Qt::NoItemFlags);
            m_isempty = true;
            m_clearbutton->hide();
            emit mListUnavailable();
        }
    });
    connect(widget, &MemoryItemWidget::menucopy, this, [ = ]() {
        QClipboard *clipboard = QApplication::clipboard();
        QString originalText = clipboard->text();
        clipboard->setText(item1->data(Qt::EditRole).toString().remove("\n"));
    });
    connect(widget, &MemoryItemWidget::menuplus, this, [ = ]() {
        int row = m_listwidget->row(item1);
        emit MemoryWidget::widgetplus(row);
    });
    connect(widget, &MemoryItemWidget::menuminus, this, [ = ]() {
        int row = m_listwidget->row(item1);
        emit MemoryWidget::widgetminus(row);
    });
}

void MemoryWidget::paintEvent(QPaintEvent *event)
{
    QPainter painter(this);
    if (m_themetype == 1) {
        QPen pen(QBrush(QColor("#F8F8F8")), 0);
        painter.setPen(pen);
    } else {
        QPen pen(QBrush(QColor("#252525")), 0);
        painter.setPen(pen);
    }
//    painter.setBrush(QBrush(QColor("#F8F8F8")));
    QRect rect(this->rect().left(), this->rect().top(), this->rect().width(), this->rect().bottom() + 30);
    painter.drawRect(rect);
    QWidget::paintEvent(event);
}

void MemoryWidget::mousePressEvent(QMouseEvent *event)
{
    m_type = -1;
    QMouseEvent *pEvent = static_cast<QMouseEvent *>(event);
    m_mousepoint = pEvent->pos();

    QRect rect(this->frameGeometry());
    if (rect.contains(m_mousepoint) == true)
        emit insidewidget();
    QWidget::mousePressEvent(event);
}

void MemoryWidget::memoryplus(Quantity answer)
{
    const QString resultmem = DMath::format(answer, Quantity::Format::Fixed() + Quantity::Format::Precision(78));
    QString formatResultmem = Utils::formatThousandsSeparators(resultmem);
    formatResultmem = formatResultmem.replace('-', "－").replace('+', "＋");
    if (m_isempty == false) {
//        QString exp = QString(m_listwidget->item(0)->data(Qt::EditRole).toString() + "+(" + formatResultmem + ")");
        QString exp = QString(DMath::format(list.value(0), Quantity::Format::Fixed() + Quantity::Format::Precision(78)) + "+(" + formatResultmem + ")");
        m_evaluator->setExpression(formatExpression(exp));
        Quantity ans = m_evaluator->evalUpdateAns();
        const QString result = DMath::format(ans, Quantity::Format::General());
        QString formatResult = Utils::formatThousandsSeparators(result);
        formatResult = setitemwordwrap(formatResult);
        m_listwidget->item(0)->setData(Qt::DisplayRole, formatResult);
        list.replace(0, ans);
    } else {
        m_listwidget->clear();
        generateData(answer);
    }
}

void MemoryWidget::memoryminus(Quantity answer)
{
    const QString resultmem = DMath::format(answer, Quantity::Format::Fixed() + Quantity::Format::Precision(78));
    QString formatResultmem = Utils::formatThousandsSeparators(resultmem);
    formatResultmem = formatResultmem.replace('-', "－").replace('+', "＋");
    if (m_isempty == false) {
//        QString exp = QString(m_listwidget->item(0)->data(Qt::EditRole).toString() + "-(" + formatResultmem + ")");
        QString exp = QString(DMath::format(list.value(0), Quantity::Format::Fixed() + Quantity::Format::Precision(78)) + "-(" + formatResultmem + ")");
        m_evaluator->setExpression(formatExpression(exp));
        Quantity ans = m_evaluator->evalUpdateAns();
        const QString result = DMath::format(ans, Quantity::Format::General());
        QString formatResult = Utils::formatThousandsSeparators(result);
        formatResult = setitemwordwrap(formatResult);
        m_listwidget->item(0)->setData(Qt::DisplayRole, formatResult);
        list.replace(0, ans);
    } else {
        m_listwidget->clear();
        generateData(Quantity(0));
        QString exp = QString("0-(" + formatResultmem + ")");
        m_evaluator->setExpression(formatExpression(exp));
        Quantity ans = m_evaluator->evalUpdateAns();
        const QString result = DMath::format(ans, Quantity::Format::General());
        QString formatResult = Utils::formatThousandsSeparators(result);
        formatResult = setitemwordwrap(formatResult);
        m_listwidget->item(0)->setData(Qt::DisplayRole, formatResult);
        list.replace(0, ans);
    }
}

void MemoryWidget::memoryclean()
{
    m_listwidget->clear();
    m_isempty = true;
    list.clear();
    emit mListUnavailable();
}

QPair<QString, Quantity> MemoryWidget::getfirstnumber()
{
    QPair<QString, Quantity> p1;
    if (m_isempty == false) {
        QString str = m_listwidget->item(0)->data(Qt::EditRole).toString();
        p1.first = str.remove("\n");
        p1.second = list.at(0);
        return p1;
    } else {
        p1.first = QString();
        p1.second = Quantity(0);
        return p1;
    }
}

void MemoryWidget::widgetplusslot(int row, Quantity answer)
{
    const QString resultmem = DMath::format(answer, Quantity::Format::Fixed() + Quantity::Format::Precision(78));
    QString formatResultmem = Utils::formatThousandsSeparators(resultmem);
    formatResultmem = formatResultmem.replace('-', "－").replace('+', "＋");
    if (answer == Quantity(0)) {
        m_listwidget->item(row)->setData(Qt::DisplayRole, m_listwidget->item(row)->data(Qt::EditRole));
    } else {
//        QString exp = QString(m_listwidget->item(row)->data(Qt::EditRole).toString() + "+(" + formatResultmem + ")");
        QString exp = QString(DMath::format(list.value(row), Quantity::Format::Fixed() + Quantity::Format::Precision(78)) + "+(" + formatResultmem + ")");
        m_evaluator->setExpression(formatExpression(exp));
        Quantity ans = m_evaluator->evalUpdateAns();
        const QString result = DMath::format(ans, Quantity::Format::General());
        QString formatResult = Utils::formatThousandsSeparators(result);
        formatResult = setitemwordwrap(formatResult, row);
        m_listwidget->item(row)->setData(Qt::DisplayRole, formatResult);
        list.replace(row, ans);
    }
}

void MemoryWidget::widgetminusslot(int row, Quantity answer)
{
    const QString resultmem = DMath::format(answer, Quantity::Format::Fixed() + Quantity::Format::Precision(78));
    QString formatResultmem = Utils::formatThousandsSeparators(resultmem);
    formatResultmem = formatResultmem.replace('-', "－").replace('+', "＋");
    if (answer == Quantity(0))
        m_listwidget->item(row)->setData(Qt::DisplayRole, m_listwidget->item(row)->data(Qt::EditRole));
    else {
//        QString exp = QString(m_listwidget->item(row)->data(Qt::EditRole).toString() + "-(" + formatResultmem + ")");
        QString exp = QString(DMath::format(list.value(row), Quantity::Format::Fixed() + Quantity::Format::Precision(78)) + "-(" + formatResultmem + ")");
        m_evaluator->setExpression(formatExpression(exp));
        Quantity ans = m_evaluator->evalUpdateAns();
        const QString result = DMath::format(ans, Quantity::Format::General());
        QString formatResult = Utils::formatThousandsSeparators(result);
        formatResult = setitemwordwrap(formatResult, row);
        m_listwidget->item(row)->setData(Qt::DisplayRole, formatResult);
        list.replace(row, ans);
    }
}

void MemoryWidget::expressionempty(bool b)
{
    if (!m_isempty) {
        for (int i = 0; i < m_listwidget->count(); i++) {
            static_cast<MemoryItemWidget *>(m_listwidget->itemWidget(m_listwidget->item(i)))->isexpressionempty(b);
        }
    }
}

QString MemoryWidget::formatExpression(const QString &text)
{
    return QString(text)
           .replace(QString::fromUtf8("＋"), "+")
           .replace(QString::fromUtf8("－"), "-")
           .replace(QString::fromUtf8("×"), "*")
           .replace(QString::fromUtf8("÷"), "/")
           .replace(QString::fromUtf8(","), "");
}

QString MemoryWidget::setitemwordwrap(const QString &text, int row)
{
    QString result = text;
    result.replace('-', "－").replace('+', "＋");
    int index = result.indexOf("e");
    line = 1;
    if (index > 0 && result.left(index).length() > 13) {
        result.insert(index, "\n");
        line = 2;
    } else if (index <= 0 && result.length() > 21) {
        result.insert(20, "\n");
        line = 2;
    }
    if (m_listwidget->item(row)) {
        m_listwidget->item(row)->setSizeHint(QSize(344, 40 + 45 * line));
        m_listwidget->itemWidget(m_listwidget->item(row))->setFixedSize(QSize(344, 40 + 45 * line));
    }
    if (m_clearbutton->isHidden() == true)
        m_clearbutton->show();
    return result;
}

void MemoryWidget::setThemeType(int type)
{
    m_listwidget->update();
    int typeIn = type;
    if (typeIn == 0) {
        typeIn = DGuiApplicationHelper::instance()->themeType();
    }
    m_themetype = typeIn;
    emit themechange(m_themetype);
    QColor c = Dtk::Gui::DGuiApplicationHelper::instance()->applicationPalette().highlight().color().name();
    QString path;
    if (m_themetype == 1) {
        path = QString(":/images/%1/").arg("light");
        m_listwidget->setStyleSheet("QListWidget::item{color:black;background-color:transparent;} \
                                     QListWidget::item:hover{color:black;background-color:rgba(0,0,0,0.05 * 255);} \
                                     QListWidget{color:black;background-color:transparent;}");
        m_clearbutton->setIconUrl(path + "empty_normal.svg", path + "empty_hover.svg", path + "empty_press.svg", 1);
        connect(m_listwidget, &QListWidget::itemPressed, this, [ = ](QListWidgetItem * item) {
            m_listwidget->setStyleSheet("QListWidget::item{color:black;background-color:transparent;} \
                                         QListWidget::item:selected{color:black;background-color:rgba(0,0,0,0.2 * 255);} \
                                         QListWidget{color:black;background-color:transparent;height}");
        });
    } else {
        path = QString(":/images/%1/").arg("dark");
        m_listwidget->setStyleSheet("QListWidget::item{color:#B4B4B4;background-color:transparent;} \
                                     QListWidget::item:hover{color:#B4B4B4;background-color:rgba(255,255,255,0.05 * 255);} \
                                     QListWidget{color:#B4B4B4;background-color:transparent;}");
        m_clearbutton->setIconUrl(path + "empty_normal.svg", path + "empty_hover.svg", path + "empty_press.svg", 1);
        connect(m_listwidget, &QListWidget::itemPressed, this, [ = ](QListWidgetItem * item) {
            m_listwidget->setStyleSheet("QListWidget::item{color:#B4B4B4;background-color:transparent;} \
                                         QListWidget::item:selected{color:#FFFFFF;background-color:rgba(255,255,255,0.2 * 255);} \
                                         QListWidget{color:#B4B4B4;background-color:transparent;}");
        });
    }

}

MemoryWidget::~MemoryWidget()
{

}

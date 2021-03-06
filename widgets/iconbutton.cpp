/*
 * Copyright (C) 2017 ~ 2018 Deepin Technology Co., Ltd.
 *
 * Author:     rekols <rekols@foxmail.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "iconbutton.h"
#include <QGridLayout>
#include <QDebug>
#include <QTimer>
#include <QToolTip>

IconButton::IconButton(QWidget *parent, bool b)
    : TextButton("", parent),
      m_iconWidget(new DLabel),
      m_iconRenderer(new DSvgRenderer(this))
{
    m_settings = DSettings::instance(this);
    int mode = m_settings->getOption("mode").toInt();
    mode == 0 ? setFixedSize(80, 58) : setFixedSize(67, 44);
    if (b == true)
        setFixedSize(40, 40);
    QGridLayout *layout = new QGridLayout(this);
    layout->addWidget(m_iconWidget, 0, Qt::AlignCenter);
    layout->setContentsMargins(0, 0, 0, 0);
    setLayout(layout);
    m_isHover = false;
    m_isPress = false;
    m_isEmptyBtn = b;
}

IconButton::~IconButton()
{
}

void IconButton::setIconUrl(const QString &normalFileName, const QString &hoverFileName, const QString &pressFileName, int mode)
{
    m_normalUrl = normalFileName;
    m_hoverUrl = hoverFileName;
    m_pressUrl = pressFileName;
    m_currentUrl = normalFileName;

    m_iconRenderer->load(m_normalUrl);
    m_currentUrl = m_normalUrl;
    m_buttonStatus = 0;
    QPixmap pix(m_normalUrl);
    m_pixmap = pix;
    //setIcon(QIcon(m_pixmap));
    //setIconSize(QSize(30,30)*devicePixelRatioF());
    m_mode = mode;
}

void IconButton::animate(int msec)
{
    if (m_isPress == false) { //edit for bug-20492 20200416
        m_isHover = false;  //edit for bug-20508 20200414
        setDown(true);
        m_isPress = true;
        m_iconRenderer->load(m_pressUrl);
        m_currentUrl = m_pressUrl;
        m_buttonStatus = 2;
        if (m_mode == 1)
            m_mode = 2;
        QPixmap pixmap(m_pressUrl);
        m_pixmap = pixmap;

        QTimer::singleShot(msec, this, [ = ] {
            setDown(false);
            QPixmap pixmap(m_normalUrl);
            m_currentUrl = m_normalUrl;
            m_buttonStatus = 0;
            if (m_mode == 2)
                m_mode = 1;
            m_pixmap = pixmap;
            m_iconRenderer->load(m_normalUrl);
            m_isPress = false;
            update();
        });
    }
}

void IconButton::mousePressEvent(QMouseEvent *e)
{
    m_iconRenderer->load(m_pressUrl);
    m_currentUrl = m_pressUrl;
    m_buttonStatus = 2;
    if (m_mode == 1)
        m_mode = 2;
    QPixmap pixmap(m_pressUrl);
    m_pixmap = pixmap;
    m_isPress = true;
    //pixmap.setDevicePixelRatio(devicePixelRatioF());
    //DPushButton::setIcon(QIcon(pixmap));

    TextButton::mousePressEvent(e);
}

void IconButton::mouseReleaseEvent(QMouseEvent *e)
{
    m_iconRenderer->load(m_normalUrl);
    m_currentUrl = m_normalUrl;
    m_buttonStatus = 0;
    if (m_mode == 2)
        m_mode = 1;
    QPixmap pixmap(m_normalUrl);
    m_pixmap = pixmap;
    m_isPress = false;
    //pixmap.setDevicePixelRatio(devicePixelRatioF());
    //DPushButton::setIcon(QIcon(pixmap));

    TextButton::mouseReleaseEvent(e);
}

void IconButton::enterEvent(QEvent *e)
{
    m_iconRenderer->load(m_hoverUrl);
    m_currentUrl = m_hoverUrl;
    m_buttonStatus = 1;
    QPixmap pixmap(m_hoverUrl);
    m_pixmap = pixmap;
    m_isHover = true;
    //pixmap.setDevicePixelRatio(devicePixelRatioF());
    //DPushButton::setIcon(QIcon(pixmap));

    TextButton::enterEvent(e);
}

void IconButton::leaveEvent(QEvent *e)
{
    m_iconRenderer->load(m_normalUrl);
    m_currentUrl = m_normalUrl;
    m_buttonStatus = 0;
    QPixmap pixmap(m_normalUrl);
    m_pixmap = pixmap;
    m_isHover = false;
    //pixmap.setDevicePixelRatio(devicePixelRatioF());
    //DPushButton::setIcon(QIcon(pixmap));

    TextButton::leaveEvent(e);
}

void IconButton::paintEvent(QPaintEvent *)
{
    QPainter painter(this);
    if (m_isEmptyBtn == false) {
        int mode = m_settings->getOption("mode").toInt();
        mode == 0 ? setFixedSize(80, 58) : setFixedSize(67, 44);
        QRectF frameRect = this->rect();
        QRectF rect(frameRect.left() + 2, frameRect.top() + 2, frameRect.width() - 4, frameRect.height() - 4);
        QRectF hover(frameRect.left() + 3, frameRect.top() + 3, frameRect.width() - 6, frameRect.height() - 6);
        painter.setRenderHint(QPainter::Antialiasing, true);
        painter.setRenderHint(QPainter::SmoothPixmapTransform, true);
        //m_pixmap = m_pixmap.scaled(m_pixmap.size() * devicePixelRatioF());
        QRectF pixRect = m_pixmap.rect();
        pixRect.moveCenter(rect.center());
        QColor actcolor = Dtk::Gui::DGuiApplicationHelper::instance()->applicationPalette().highlight().color();//活动色
        QColor pressBrush, focus, hoverFrame, base, hoverbrush;
        int type = DGuiApplicationHelper::instance()->paletteType();
        if (type == 0)
            type = DGuiApplicationHelper::instance()->themeType();
        if (type == 1) {
            pressBrush = QColor(0, 0, 0, 0.1 * 255);
            focus = actcolor;
            hoverFrame = actcolor;
            hoverFrame.setAlphaF(0.2);
            base = Qt::white;
            hoverbrush = Qt::white;
        } else {
            pressBrush = QColor(0, 0, 0, 0.5 * 255);
            focus = actcolor;
            hoverFrame = actcolor;
            hoverFrame.setAlphaF(0.2);
            base = QColor(48, 48, 48);
            hoverbrush = QColor(255, 255, 255, 0.1 * 255);
        }
        if (hasFocus()) {
            if (m_isPress) {
                painter.setBrush(QBrush(pressBrush));
                QPen pen;
                pen.setColor(pressBrush);
                painter.setPen(pen);
                painter.drawRoundRect(rect, 25, 30);
            } else {
                painter.setPen(Qt::NoPen);
                painter.setBrush(QBrush(base));
                painter.drawRoundRect(rect, 25, 30);
                QPen pen;
                pen.setColor(focus);
                pen.setWidth(2);
                painter.setPen(pen);
                painter.setBrush(Qt::NoBrush);
                painter.drawRoundRect(rect, 25, 30);
            }
        } else {
            if (m_isHover) {
                QPen pen;
                pen.setColor(hoverFrame);
                pen.setWidth(1);
                painter.setPen(pen);
                painter.setBrush(QBrush(hoverbrush));
                painter.drawRoundRect(rect, 25, 30);

//                painter.setPen(Qt::NoPen);
//                painter.setBrush(QBrush(base));
//                painter.drawRoundRect(hover, 25, 30);
            } else if (m_isPress) {
                painter.setPen(Qt::NoPen);
                painter.setBrush(QBrush(pressBrush));
                painter.drawRoundRect(rect, 25, 30);
            } else {
                painter.setPen(Qt::NoPen);
                painter.setBrush(QBrush(base));
                painter.drawRoundRect(rect, 25, 30);
            }
            //painter.setPen(QPen(hoverFrame));
            //painter.setBrush(Qt::NoBrush);
            //painter.drawRoundRect(rect,30,40);
        }
    }
    //painter.drawPixmap(pixRect.topLeft(),m_pixmap);
    drawCenterPixMap(painter);
}

bool IconButton::event(QEvent *e)
{
    if (e->type() == QEvent::ToolTip) {
        QHelpEvent *helpEvent = static_cast<QHelpEvent *>(e);
        if (this->m_isHover == true && m_isEmptyBtn) {
            QString tooltext = tr("Clear all memory");
            QToolTip::showText(helpEvent->globalPos(), tooltext);
        } else {
            QToolTip::hideText();
            e->ignore();
        }
        return true;
    }
    return DPushButton::event(e);
}

void IconButton::SetAttrRecur(QDomElement elem, QString strtagname, QString strattr, QString strattrval)
{
    if (m_mode != 1) {
        if (elem.tagName().compare(strtagname) == 0 && elem.attribute(strattr) != "none" && elem.attribute(strattr) != "") {
            elem.setAttribute(strattr, strattrval);
            if (m_buttonStatus == 0)
                elem.setAttribute("fill-opacity", 0.75);
            if (m_buttonStatus == 1)
                elem.setAttribute("fill-opacity", 0.65);
            if (m_buttonStatus == 2)
                elem.setAttribute("fill-opacity", 1);
        }
        if (m_mode == 0) {
            elem.setAttribute(strattr, strattrval);
            if (m_buttonStatus == 0)
                elem.setAttribute("fill-opacity", 0.75);
            if (m_buttonStatus == 1)
                elem.setAttribute("fill-opacity", 0.65);
            if (m_buttonStatus == 2)
                elem.setAttribute("fill-opacity", 1);
        }
        if (m_isEmptyBtn == true) {
            strtagname = "path";
            if (elem.tagName().compare(strtagname) == 0 && elem.attribute(strattr) != "none" && elem.attribute(strattr) != "") {
                elem.setAttribute(strattr, strattrval);
                if (m_buttonStatus == 0)
                    elem.setAttribute("fill-opacity", 0.75);
                if (m_buttonStatus == 1)
                    elem.setAttribute("fill-opacity", 0.65);
                if (m_buttonStatus == 2)
                    elem.setAttribute("fill-opacity", 1);
            }
        }
        for (int i = 0; i < elem.childNodes().count(); i++) {
            if (!elem.childNodes().at(i).isElement()) {
                continue;
            }
            SetAttrRecur(elem.childNodes().at(i).toElement(), strtagname, strattr, strattrval);
        }
    }
}

void IconButton::drawCenterPixMap(QPainter &painter)
{
    painter.save();

    QFile file(m_currentUrl);
    file.open(QIODevice::ReadOnly);
    QByteArray baData = file.readAll();

    QDomDocument doc;
    doc.setContent(baData);

    file.close();

    SetAttrRecur(doc.documentElement(), "g", "fill", Dtk::Gui::DGuiApplicationHelper::instance()->applicationPalette().highlight().color().name());
    QRectF frameRect = this->rect();
    QRectF rect(frameRect.left() + 2, frameRect.top() + 2, frameRect.width() - 4, frameRect.height() - 4);
    QRectF pixRect = m_pixmap.rect();
    pixRect.moveCenter(rect.center());
//    m_iconRenderer = new DSvgRenderer(doc.toByteArray(), this);
    m_iconRenderer->load(doc.toByteArray());
    m_iconRenderer->render(&painter, pixRect);
    painter.restore();
}

/*void IconButton::setIconSize(const int &size)
{
    const int scaledSize = size * devicePixelRatioF();
    const QSize iconSize(scaledSize, scaledSize);
    const QImage image = m_iconRenderer->toImage(iconSize);

    QPixmap pix;
    pix.convertFromImage(image);
    pix.setDevicePixelRatio(devicePixelRatioF());

    //m_iconWidget->setAlignment(Qt::AlignCenter);
    //m_iconWidget->setPixmap(pix);
    this->DPushButton::setIcon(QIcon(pix));
    //m_iconWidget->setFixedSize(iconSize);
}*/

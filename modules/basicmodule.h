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

#ifndef BASICMODULE_H
#define BASICMODULE_H

#include <DWidget>
#include <DGuiApplicationHelper>
#include <QStackedLayout>
#include <QGridLayout>
#include <QTimer>

#include "../widgets/textbutton.h"
#include "../widgets/iconbutton.h"
#include "../widgets/basickeypad.h"
#include "../widgets/memorykeypad.h"
#include "../widgets/tricombobox.h"
#include "expressionbar.h"
#include "memorywidget.h"
#include "./widgets/scibasickeypad.h"
#include "../widgets/inputedit.h"

DGUI_USE_NAMESPACE
DWIDGET_USE_NAMESPACE

class BasicModule : public DWidget
{
    Q_OBJECT

public:
    BasicModule(QWidget *parent = nullptr);
    ~BasicModule();

    void setKeyPress(QKeyEvent *e);

    void mousePressEvent(QMouseEvent *event);
public slots:
    //memory func
    void mAvailableEvent();
    void mUnAvailableEvent();
    void showListWidget();
private slots:
    void initTheme(int type);
private:
    void handleEditKeyPress(QKeyEvent *);
    void handleKeypadButtonPress(int);
    void equalButtonPress();
    void handleClearStateChanged(bool isAllClear);

protected:
    //void paintEvent(QPaintEvent *);

private:
    ExpressionBar *m_expressionBar;
    BasicKeypad *m_basicKeypad;
    MemoryKeypad *m_memoryKeypad;
    QStackedLayout *m_keypadLayout;
    MemoryWidget *m_memorylistwidget;
    QColor m_expBarColor;
    QColor m_expBarSepColor;
    QColor m_btnSepColor;
    bool m_insidewidget;
    bool m_avail = false;
    bool m_isinsidefun = false;//鼠标点击在函数widget内
    bool m_memCalbtn; //m+,m-,ms
    bool m_memRCbtn;//mr,mc
    bool m_isallgray; //内存按钮全部置灰;
};

#endif

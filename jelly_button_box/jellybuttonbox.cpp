#include "jellybuttonbox.h"

JellyButtonBox::JellyButtonBox(QWidget *parent) : QWidget(parent)
{
    setAttribute(Qt::WA_DeleteOnClose, true);
    setWindowFlag(Qt::FramelessWindowHint, true);
    setFocusPolicy(Qt::StrongFocus);

    effect = new QGraphicsDropShadowEffect(this);
    effect->setOffset(0,border_size*2);
    QColor blur = Qt::gray;
    blur.setAlpha(127);
    effect->setColor(blur);
    effect->setBlurRadius(btn_radius);
    setGraphicsEffect(effect);
}

void JellyButtonBox::setColors(QColor bg, QColor fg)
{
    bg_color = bg;
    fg_color = fg;
}

void JellyButtonBox::setSize(int outer, int inner, int spacing)
{
    outer_radius = outer;
    btn_radius = inner;
    btn_spacing = spacing;
}

void JellyButtonBox::setButtonIcons(QString icon1, QString icon2, QString icon3)
{
    QList<QIcon> icons = {QIcon(icon1), QIcon(icon2), QIcon(icon3)};
    setButtons(icons);
}

void JellyButtonBox::setButtonPixmaps(QString icon1, QString icon2, QString icon3)
{
    QList<QPixmap> icons = {QPixmap(icon1), QPixmap(icon2), QPixmap(icon3)};
    setButtons(icons);
}

void JellyButtonBox::setButtons(QList<QPixmap> icons, QList<QString> texts)
{
    int size = icons.size();
    if (size > 3)
        size = 3;
    for (int i = 0; i < size; i++)
    {
        auto btn = new JellyButton(icons.at(i), this);
        buttons.append(btn);
        btn->setFixedSize(btn_radius*2, btn_radius*2);
        btn->setIconColor(fg_color);
        connect(btn, &InteractiveButtonBase::clicked, this, [=]{
            emit signalButtonClicked(i);
            if (hide_after_click)
                toHide();
        });
        if (texts.size() > i)
            btn->setToolTip(texts.at(i));
        btn->hide();
    }
}

void JellyButtonBox::setButtons(QList<QIcon> icons, QList<QString> texts)
{
    int size = icons.size();
    if (size > 3)
        size = 3;
    for (int i = 0; i < size; i++)
    {
        auto btn = new JellyButton(icons.at(i), this);
        buttons.append(btn);
        btn->setFixedSize(btn_radius*2, btn_radius*2);
        btn->setIconColor(fg_color);
        connect(btn, &InteractiveButtonBase::clicked, this, [=]{
            emit signalButtonClicked(i);
            if (hide_after_click)
                toHide();
        });
        if (texts.size() > i)
            btn->setToolTip(texts.at(i));
        btn->hide();
    }
}

void JellyButtonBox::setHideAfterClick(bool h)
{
    hide_after_click = h;
}

void JellyButtonBox::exec(QPoint start_pos, QPoint end_pos)
{
    hiding = false;
    show();
    startAnimation1(start_pos, end_pos);
    setFocus();
}

void JellyButtonBox::toHide()
{
    hiding = true;
    endAnimation3();
}

/**
 * 按钮出现动画
 * @param start_pos 开始出现的位置
 * @param end_pos   结束出现的位置
 */
void JellyButtonBox::startAnimation1(QPoint start_pos, QPoint end_pos)
{
    QRect rect(end_pos.x()-outer_radius-border_size, end_pos.y()-outer_radius-border_size, outer_radius*2+border_size*2, outer_radius*2+border_size*2);

    QPropertyAnimation* geo_ani = new QPropertyAnimation(this, "geometry");
    geo_ani->setStartValue(QRect(start_pos, QSize(1, 1)));
    geo_ani->setEndValue(rect);
    geo_ani->setEasingCurve(QEasingCurve::OutBack);
    geo_ani->setDuration(350);
    connect(geo_ani, SIGNAL(finished()), geo_ani, SLOT(deleteLater()));
    connect(geo_ani, &QPropertyAnimation::finished, this, [=]{
        total_width = (btn_radius*2 + btn_spacing + outer_radius + border_size) * 2;
        show_prop = 100;
            startAnimation2();
    });
    geo_ani->start();
}

/**
 * 整体扩展、按钮背景撕开 动画（反弹）
 */
void JellyButtonBox::startAnimation2()
{
    QRect rect(geometry().center().x() - total_width / 2, geometry().top(), total_width, height());
    int dur = 500;

    QPropertyAnimation* geo_ani = new QPropertyAnimation(this, "geometry");
    geo_ani->setStartValue(geometry());
    geo_ani->setEndValue(rect);
    geo_ani->setEasingCurve(QEasingCurve::OutBack);
    geo_ani->setDuration(dur);
    connect(geo_ani, SIGNAL(finished()), geo_ani, SLOT(deleteLater()));
    geo_ani->start();

    QPropertyAnimation* step2_ani = new QPropertyAnimation(this, "step2");
    step2_ani->setStartValue(0);
    step2_ani->setEndValue(100);
    step2_ani->setDuration(dur);
    connect(step2_ani, SIGNAL(finished()), step2_ani, SLOT(deleteLater()));
    connect(step2_ani, &QPropertyAnimation::valueChanged, this, [=](const QVariant& value){
        if (value > 64 && icon_prop == 0)
        {
            icon_prop = 1;
            startAnimation3();
        }
    });
    connect(step2_ani, &QPropertyAnimation::finished, this, [=]{
        expd_prop = 100;
    });
    step2_ani->start();
}

void JellyButtonBox::startAnimation3()
{
    QPropertyAnimation* step3_ani = new QPropertyAnimation(this, "step3");
    step3_ani->setStartValue(1);
    step3_ani->setEndValue(100);
    step3_ani->setDuration(270);
    step3_ani->setEasingCurve(QEasingCurve::OutCirc);
    connect(step3_ani, SIGNAL(finished()), step3_ani, SLOT(deleteLater()));
    connect(step3_ani, &QPropertyAnimation::finished, this, [=]{
        icon_prop = 100;
    });
    step3_ani->start();

    // 显示按钮控件
    for (int i = 0; i < buttons.size(); i++)
    {
        buttons.at(i)->show();
    }
}

void JellyButtonBox::endAnimation3()
{
    QPropertyAnimation* step3_ani = new QPropertyAnimation(this, "step3");
    step3_ani->setStartValue(icon_prop);
    step3_ani->setEndValue(0);
    step3_ani->setDuration(270 * icon_prop / 100);
    step3_ani->setEasingCurve(QEasingCurve::OutCirc);
    connect(step3_ani, SIGNAL(finished()), step3_ani, SLOT(deleteLater()));
    connect(step3_ani, &QPropertyAnimation::finished, this, [=]{
        icon_prop = 0;
        foreach (auto button, buttons)
        {
            button->hide();
        }
        endAnimation2();
    });
    step3_ani->start();
}

void JellyButtonBox::endAnimation2()
{
    int rad = outer_radius+border_size;
    QRect rect(geometry().center()-QPoint(rad, rad), QSize(rad*2,rad*2));
    int dur = 350;

    QPropertyAnimation* geo_ani = new QPropertyAnimation(this, "geometry");
    geo_ani->setStartValue(geometry());
    geo_ani->setEndValue(rect);
    geo_ani->setEasingCurve(QEasingCurve::InBack);
    geo_ani->setDuration(dur);
    connect(geo_ani, SIGNAL(finished()), geo_ani, SLOT(deleteLater()));
    geo_ani->start();

    QPropertyAnimation* step2_ani = new QPropertyAnimation(this, "step2");
    step2_ani->setStartValue(expd_prop);
    step2_ani->setEndValue(0);
    step2_ani->setDuration(dur * expd_prop / 100);
    connect(step2_ani, SIGNAL(finished()), step2_ani, SLOT(deleteLater()));
    connect(step2_ani, &QPropertyAnimation::finished, this, [=]{
        expd_prop = 100;
        endAnimation1();
    });
    step2_ani->start();
}

void JellyButtonBox::endAnimation1()
{
    QRect rect(geometry().center(), QSize(1,1));

    QPropertyAnimation* geo_ani = new QPropertyAnimation(this, "geometry");
    geo_ani->setStartValue(geometry());
    geo_ani->setEndValue(rect);
    geo_ani->setEasingCurve(QEasingCurve::OutBack);
    geo_ani->setDuration(270);
    connect(geo_ani, SIGNAL(finished()), geo_ani, SLOT(deleteLater()));
    geo_ani->start();
}

void JellyButtonBox::paintEvent(QPaintEvent *)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing, true);

    // 绘制背景
    if (show_prop < 100) // 画圆
    {
        QPainterPath bg_path;
        bg_path.addEllipse(border_size, border_size, width()-border_size*2, height() - border_size*2);
        painter.fillPath(bg_path, bg_color);
    }
    else // 圆角矩形
    {
        QPainterPath bg_path;
        bg_path.addRoundedRect(border_size, border_size, width()-border_size*2, height() - border_size*2, outer_radius, outer_radius);
        painter.fillPath(bg_path, bg_color);
    }

    // 绘制前景
    if (show_prop < 100)
    {
        QPainterPath fg_path;
        int real_radius = (width()-border_size*2) * btn_radius / outer_radius / 2;
        fg_path.addEllipse(width()/2-real_radius, height()/2-real_radius, real_radius*2, real_radius*2);
        painter.fillPath(fg_path, fg_color);
    }
    else if (expd_prop < 100)
    {
        QPainterPath fg_path;
//        fg_path.addRoundedRect(outer_radius-btn_radius+border_size, outer_radius-btn_radius+border_size, width()-border_size*2-(outer_radius-btn_radius)*2, height()-border_size*2-(outer_radius-btn_radius)*2, btn_radius, btn_radius);
        // 计算三个分开的曲线
        // 进度：expd_prop / 100
        // 总宽度：total_width
        // 最小宽度：(outer_radius + border_size)*2

        int ctry = height()/2;
        QPoint left(border_size + outer_radius, ctry),
                mid(width() / 2, ctry),
                right(width() - border_size - outer_radius, ctry);

        // 切线贝塞尔算法
        int delta_ol_om = mid.x()-left.x(); // 圆心之间的距离
        double prop = delta_ol_om/(double)(btn_spacing+btn_radius*2);
        if (prop > 1) // 反弹效果时会变成负的
            prop = 1;
        double angle = PI/2 - (PI/2) * prop * prop; // 切线角度
        int radius = btn_radius + (outer_radius-btn_radius)*prop/2;
        if (icon_prop) // step3已经开始了， 半径改变
        {
            radius = (btn_radius + (outer_radius-btn_radius)/2) * (100-icon_prop) / 100;
        }

        int qie_delta_x = radius * cos(angle);
        int qie_delta_y = radius * sin(angle);
        QPoint qie_ulm(left.x() + qie_delta_x,left.y() - qie_delta_y);
        QPoint qie_uml(mid.x() - qie_delta_x, mid.y() - qie_delta_y);
        QPoint qie_dlm(left.x() + qie_delta_x,left.y() + qie_delta_y);
        QPoint qie_dml(mid.x() - qie_delta_x, mid.y() + qie_delta_y);
        QPoint qie_umr(mid.x() + qie_delta_x, mid.y() - qie_delta_y);
        QPoint qie_urm(right.x() - qie_delta_x,right.y() - qie_delta_y);
        QPoint qie_dmr(mid.x() + qie_delta_x, mid.y() + qie_delta_y);
        QPoint qie_drm(right.x() - qie_delta_x,right.y() + qie_delta_y);

        int dis = qie_uml.x() - qie_ulm.x();
//        int dis_calc = dis*sqrt(prop) * sin(angle) * cos(angle);
        int dis_calc = dis*prop*prop;
        int ctrl_delta_x = dis_calc * cos(angle);
        int ctrl_delta_y = dis_calc * sin(angle);
        QPoint ctrl_uml = qie_uml + QPoint(-ctrl_delta_x, ctrl_delta_y);
        QPoint ctrl_ulm = qie_ulm + QPoint(ctrl_delta_x, ctrl_delta_y);
        QPoint ctrl_dlm = qie_dlm + QPoint(ctrl_delta_x, -ctrl_delta_y);
        QPoint ctrl_dml = qie_dml + QPoint(-ctrl_delta_x, -ctrl_delta_y);
        QPoint ctrl_umr = qie_umr + QPoint(ctrl_delta_x, ctrl_delta_y);
        QPoint ctrl_urm = qie_urm + QPoint(-ctrl_delta_x, ctrl_delta_y);
        QPoint ctrl_drm = qie_drm + QPoint(-ctrl_delta_x, -ctrl_delta_y);
        QPoint ctrl_dmr = qie_dmr + QPoint(ctrl_delta_x, -ctrl_delta_y);

        angle = angle * 180 / PI; // 切线弧度制转角度制，用来 arcTo
        double degle = 90 - angle; // 90-切线角度

        fg_path.moveTo(mid.x(), mid.y()-radius);
        fg_path.arcTo(mid.x()-radius, mid.y()-radius, radius*2, radius*2, 90, degle);
        fg_path.cubicTo(ctrl_uml, ctrl_ulm, qie_ulm);
        fg_path.arcTo(QRect(left.x()-radius, left.y()-radius, radius*2, radius*2), angle, 180+degle*2);
        fg_path.cubicTo(ctrl_dlm, ctrl_dml, qie_dml);
        fg_path.arcTo(mid.x()-radius, mid.y()-radius, radius*2, radius*2, 180+angle, degle*2);
        fg_path.cubicTo(ctrl_dmr, ctrl_drm, qie_drm);
        fg_path.arcTo(right.x()-radius, right.y()-radius, radius*2, radius*2, 180+angle, 180+degle*2);
        fg_path.cubicTo(ctrl_urm, ctrl_umr, qie_umr);
        fg_path.arcTo(mid.x()-radius, mid.y()-radius, radius*2, radius*2, angle, degle);
        fg_path.lineTo(mid.x(), mid.y()-radius);

        QColor c = fg_color;
        c.setAlpha(qMax(c.alpha() * (100-icon_prop*4) / 100, 0));
        painter.fillPath(fg_path, c);
    }
    else if (icon_prop < 100) // 背景收缩，图标出现
    {
        QPainterPath fg_path;
        int ctry = height()/2;
        QPoint left(border_size + outer_radius, ctry),
                mid(width() / 2, ctry),
                right(width() - border_size - outer_radius, ctry);
        int radius = (btn_radius + (outer_radius-btn_radius)/2) * (100-icon_prop) / 100; // 应该的半径
        fg_path.addEllipse(left.x()-radius, left.y()-radius, radius*2, radius*2);
        fg_path.addEllipse(mid.x()-radius, mid.y()-radius, radius*2, radius*2);
        fg_path.addEllipse(right.x()-radius, right.y()-radius, radius*2, radius*2);

        QColor c = fg_color;
        c.setAlpha(qMax(c.alpha() * (100-icon_prop*4) / 100, 0));
        painter.fillPath(fg_path, c);
    }

}

void JellyButtonBox::focusOutEvent(QFocusEvent *event)
{
    QWidget::focusOutEvent(event);

    if (!hiding)
        toHide();
}

void JellyButtonBox::setStep1(int p)
{
    show_prop = p;
}

void JellyButtonBox::setStep2(int p)
{
    expd_prop = p;
}

void JellyButtonBox::setQieAngle(int a)
{
    qie_angle = a;
}

void JellyButtonBox::setStep3(int p)
{
    icon_prop = p;
    foreach (auto button, buttons) {
        button->setIconPaddingProper(p * 0.25 / 100);
        button->update();
    }
    int width = this->width();
    int height = this->height();
    int ctry = height/2;
    QPoint left(border_size + outer_radius, ctry),
            mid(width / 2, ctry),
            right(width - border_size - outer_radius, ctry);
    if (buttons.size() > 0)
        buttons.at(0)->move(left.x()-btn_radius, left.y()-btn_radius);
    if (buttons.size() > 1)
        buttons.at(1)->move(mid.x()-btn_radius, mid.y()-btn_radius);
    if (buttons.size() > 2)
        buttons.at(2)->move(right.x()-btn_radius, right.y()-btn_radius);
    update();
}

int JellyButtonBox::getStep1()
{
    return show_prop;
}

int JellyButtonBox::getStep2()
{
    return expd_prop;
}

int JellyButtonBox::getQieAngle()
{
    return qie_angle;
}

int JellyButtonBox::getStep3()
{
    return icon_prop;
}

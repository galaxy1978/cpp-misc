/**
 * @brief 百分比控件
 */
#ifndef PERCENT_H
#define PERCENT_H

#include <QWidget>

class percent : public QWidget
{
    Q_OBJECT
private:
    float   __m_start_angle;
    float   __m_end_angle;

    float   __m_value;

    QColor  __m_back_color;
    QColor  __m_canvas_color;
    QColor  __m_arc_color;
    QFont   __m_font;
    QColor  __m_font_color;
public:
    explicit percent(QWidget *parent = nullptr);
    virtual ~percent();

    void setBackground( const QColor& c ){ __m_back_color = c; repaint(); }
    void setArcColor( const QColor& c ){ __m_arc_color = c; repaint(); }
    void setRatioFont( const QFont& f , const QColor& c );
    void setAngle( float s , float e ){ __m_start_angle = s; __m_end_angle = e; repaint();}
    void setCanvasColor( const QColor& c ){ __m_canvas_color = c;}
    /**
     * @brief 设置百分比
     * @param ratio[ I ]，百分比0~100
     */
    void setValue( float ratio ){ __m_value = ratio; repaint();}
private:
    virtual void paintEvent( QPaintEvent * evt ) final;
signals:

};

#endif // PERCENT_H

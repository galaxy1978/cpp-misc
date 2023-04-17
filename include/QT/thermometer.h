/**
 * @brief 温度计自定义控件
 * @version 1.0
 * @author 宋炜
 * @date 2023-1-15
 */

#ifndef THERMOMETER_H
#define THERMOMETER_H

#include <QWidget>

class thermometer : public QWidget
{
    Q_OBJECT
private:
    float      __m_low;       // 最低温度刻度，摄氏度
    float      __m_hight;     // 最高温度刻度，摄氏度

    float      __m_c;         // 当前摄氏度

    bool       __m_en_c;
    bool       __m_en_f;
    bool       __m_en_title;

    QFlags<Qt::AlignmentFlag>   __m_title_alignment;
    QColor     __m_line_color;
    QColor     __m_meter_color;
    QColor     __m_hg_color;
    QColor     __m_back_color;
    QColor     __m_scale_font_color;
    QColor     __m_title_color;

    QFont      __m_scale_font;
    QFont      __m_title_font;
    long       __m_scale_font_size;
    long       __m_title_font_size;
    long       __m_scale;
    long       __m_scale_detail;
    QString    __m_title;
public:
    explicit thermometer( QWidget *parent = nullptr);
    virtual ~thermometer();
    /**
     * @brief 设置标题栏
     * @param title
     * @param aligment
     */
    void setTile( const QString& title , Qt::Alignment aligment ){ __m_title = title; __m_title_alignment = aligment; }
    void setScaleColor( const QColor& c ){ __m_line_color = c; repaint();}
    void setScaleRange( long s , long d ){ __m_scale = s; __m_scale_detail = d;repaint();}
    void setRange( float l , float h ){ __m_low = l; __m_hight = h; repaint();}
    /**
     * @brief setHgColor
     * @param c
     */
    void setHgColor( const QColor& c ){ __m_hg_color = c; repaint();}
    /**
     * @brief setMeterBackgroud
     * @param c
     */
    void setMeterBackgroud( const QColor& c ){ __m_meter_color = c; repaint();}
    void setBackGround( const QColor& c ){ __m_back_color = c; repaint();}
    void setTitleFont( const QFont& f , const QColor& c , long s );
    void setScaleFont( const QFont& f , const QColor& c , long s );
    void enableTitle( bool en ){ __m_en_title = en; repaint(); }
    /**
     * @brief 允许或者不允许摄氏度
     * @param en
     */
    void enableC( bool en );
    /**
     * @brief 允许或者不允许华氏度
     * @param en[ I ]
     */
    void enableF( bool en );
    /**
     * @brief 指定数据
     * @param v
     */
    void c( float v );
    void f( float v );
private:
    virtual void paintEvent( QPaintEvent * evt ) final;
    virtual void resizeEvent( QResizeEvent *event ) final;
    /**
     * @brief __draw_bulb
     * @param art
     */
    void __draw_bulb( QPainter& art );
    /**
     * @brief __draw_hg_bar
     * @param art
     */
    void __draw_hg_bar( QPainter& art );
    /**
     * @brief __draw_title
     * @param art
     */
    void __draw_title( QPainter& art );
    /**
     * @brief __draw_grid
     * @param art
     */
    void __draw_grid( QPainter& art );
    /**
     * @brief __draw_c_grid
     * @param art
     */
    void __draw_c_grid( QPainter& art );
    /**
     * @brief __draw_f_grid
     * @param art
     */
    void __draw_f_grid( QPainter& art );
    /**
     * @brief 绘制汞柱
     * @param art
     */
    void __draw_hg( QPainter& art );
signals:

};

#endif // THERMOMETER_H

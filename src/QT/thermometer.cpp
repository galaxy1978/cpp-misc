#include <QPainter>
#include <QResizeEvent>
#include "QT/thermometer.h"

thermometer::thermometer(QWidget *parent): QWidget{parent},
    __m_low(-20),
    __m_hight(80),
    __m_c(20.0),
    __m_en_c( true ),
    __m_en_f( true ),
    __m_en_title( false ),
    __m_title_alignment( Qt::AlignCenter),
    __m_line_color( QColor( 0, 0 ,0 ) ),
    __m_meter_color( QColor( 128,128 , 128 ) ),
    __m_hg_color( QColor( 200 , 0 , 0 )),
    __m_back_color(QColor(255,255,255)),
    __m_scale_font_color(QColor(180,0,0)),
    __m_title_color( QColor( 128 , 0 , 128 ) ),
    __m_title_font_size(2),
    __m_scale_font_size(2),
    __m_scale(5),
    __m_scale_detail(10),
    __m_title( "温度" ){}

thermometer :: ~thermometer(){}

void thermometer :: paintEvent( QPaintEvent *  )
{
    QPainter p( this );
    p.setRenderHints(QPainter::Antialiasing | QPainter::TextAntialiasing );

    QSize s = size();
    // 调试边框内容
    QPen pen1( __m_back_color );
    p.setPen( pen1 );
    QBrush b( __m_back_color );
    p.setBrush( b );
    p.drawRect( 1, 1,s.width() - 2, s.height() - 2);
    // 配置绘制基本参数
    p.setViewport( 1, 1 ,s.width() - 1, s.height() - 1 );
    p.setWindow( -50,-80 , 100, 100 );
    if( __m_en_title == true ){
        __draw_title( p );
    }
    __draw_hg_bar( p );
    __draw_bulb( p );
    __draw_hg( p );
    __draw_grid(p);
}

void thermometer :: __draw_bulb( QPainter& art )
{
    QPen pen(__m_hg_color );

    QBrush brush( __m_hg_color );

    art.setPen( pen );

    art.setBrush( brush );

    art.drawEllipse( -6, -6 , 12 , 12 );
}

void thermometer :: resizeEvent( QResizeEvent *event )
{
    QSize s = event->size();
    if( s.width() < s.height() ){
        s.setWidth( s.height() );
    }else{
        s.setHeight( s.width() );
    }
    resize(s);
}

void thermometer :: __draw_hg_bar( QPainter& art )
{
    QRectF r(-3,-70,6,68);
    QPen pen( __m_line_color );
    pen.setWidthF( 0.2 );
    QBrush b( __m_meter_color );
    art.setPen( pen );
    art.setBrush( b );
    art.drawRect( r );
}

void thermometer :: __draw_title( QPainter& art )
{
    QFont f = __m_title_font;
    f.setPointSize( __m_title_font_size );
    art.setFont( f );
    QPen pen( __m_title_color );
    art.setPen( pen );
    float x = -48, y = -76;
    if( __m_title_alignment == Qt::AlignLeft ){

    }else if( __m_title_alignment == Qt::AlignCenter ){
        QFontMetricsF fm( f );
        //auto w = fm.width( __m_title );
        auto w = fm.maxWidth() * __m_title.length();
        x = (  -w ) / 2;
    }else if(__m_title_alignment == Qt::AlignRight ){
        QFontMetricsF fm( f );
        //auto w = fm.width( __m_title );
        auto w = fm.maxWidth() * __m_title.length();
        x = 48 - w;
    }else{
        QFontMetricsF fm( f );
        //auto w = fm.width( __m_title );
        auto w = fm.maxWidth() * __m_title.length();
        x = - w / 2;
    }
    art.drawText( x ,y , __m_title );
}

void thermometer :: __draw_grid( QPainter& art )
{
    if( __m_en_c == false || __m_en_f == false ){ // 如果了两个
        __draw_c_grid( art );
    }
    if( __m_en_c ){
        __draw_c_grid( art );
    }
    if( __m_en_f == true ){
        __draw_f_grid( art );
    }
}
void thermometer :: __draw_c_grid( QPainter& art )
{
    float x = 6.5 , y = -8;
    int c  = __m_scale * __m_scale_detail;
    // 计算数据范围
    float d_r = __m_hight - __m_low , d = __m_low;
    // 计算刻度范围
    float step_coord = 55 / c, step_d = d_r / c;
    QPen pen , pen1;
    pen.setColor( __m_line_color );
    pen.setWidth( 0.5 );
    pen1.setColor(__m_scale_font_color );
    QFont f = __m_scale_font;

    f.setPointSizeF( __m_scale_font_size );
    art.setFont( f );
    art.setPen( pen );
    QString s;

    // 会长刻度
    for( int i = 0; i <= c; i ++ ){
        if ( i % __m_scale_detail == 0 ){
            QLineF l( x , y , x + 4.0 , y);
            art.drawLine( l );
        }else{
            QLineF l( x , y , x + 2, y);
            art.drawLine( l );
        }

        y -= step_coord;
        d += step_d;
    }
    // 绘制刻度文字
    art.setPen( pen1 );
    y = -8;
    d = __m_low;
    for( int i = 0;  i <= c; i ++ ){
        if ( i % __m_scale_detail == 0 ){
            s = QString :: asprintf( "%.02f" , d );
            art.drawText( QPointF( x + 7.4 , y + 1 ) , s );
        }
        y -= step_coord;
        d += step_d;
    }

    art.drawText( QPointF( x , y - 2 ) , "℃" );
}

void thermometer :: __draw_f_grid( QPainter& art )
{
    float x = -6 , y = -8;
    int c  = __m_scale * __m_scale_detail;
    // 计算数据范围
    float f_heigh = __m_hight * 9 / 5 + 32, f_low = __m_low* 9 / 5 + 32;
    float d_r = f_heigh - f_low , d = f_low;
    // 计算刻度范围
    float step_coord = 55/c , step_d = d_r / c;
    QPen pen , pen1;
    pen.setColor( __m_line_color );
    pen.setWidth( 0.5 );
    pen1.setColor(__m_scale_font_color );
    QFont f = __m_scale_font;

    f.setPointSizeF( __m_scale_font_size );
    art.setFont( f );
    art.setPen( pen );
    QString s;

    // 绘制刻度
    for( int i = 0; i <= c; i ++ ){
        if ( i % __m_scale_detail == 0 ){
            QLineF l( x - 4.0, y , x , y);
            art.drawLine( l );
        }else{
            QLineF l( x - 2 , y , x, y);
            art.drawLine( l );
        }

        y -= step_coord;
        d += step_d;
    }
    // 绘制刻度文字
    art.setPen( pen1 );
    y = -8;
    d = __m_low;
    for( int i = 0;  i <= c; i ++ ){
        if ( i % __m_scale_detail == 0 ){
            s = QString :: asprintf( "%.02f" , d );
            QFontMetricsF fm( f );
            //qreal w = fm.width(s);
            auto w = fm.maxWidth() * __m_title.length();
            art.drawText( QPointF( x - 7.4 -w , y + 1 ) , s );
        }
        y -= step_coord;
        d += step_d;
    }

     art.drawText( QPointF( x - 4, y - 2 ) , "F" );
}

void thermometer :: __draw_hg( QPainter& art )
{
    // 绘制从水银球到最低刻度之间的汞柱
    QPen pen( __m_hg_color );
    pen.setWidthF( 0.2 );
    QBrush b( __m_hg_color );

    art.setPen( pen );
    art.setBrush( b );
    QRectF r(-3 , -8 , 6 , 8);
    art.drawRect( r );
    qreal y = -8;
    // 计算数据范围
    qreal d_r = __m_hight - __m_low ;
    // 计算刻度范围
    qreal step_coord = 1.1 , step_d = d_r / 50 , curr = y;
    step_coord /= step_d;
    curr = -step_coord * ( __m_c - __m_low ) + y;
    QRectF r1(-3 , curr , 6 , -curr);
    art.drawRect( r1 );
}

void thermometer :: enableC( bool en )
{
    __m_en_c = en;
    repaint();
}

void thermometer :: enableF( bool en )
{
    __m_en_f = en;
    repaint();
}

void thermometer :: c( float v )
{
    __m_c = v;
    repaint();
}
void thermometer :: f( float v )
{
    __m_c = ( v - 32 )/1.8;
    repaint();
}

void thermometer :: setTitleFont( const QFont& f , const QColor& c , long s )
{
    __m_title_font = f;
    __m_title_color = c;
    __m_title_font_size = s;
    repaint();
}

void thermometer :: setScaleFont( const QFont& f , const QColor& c , long s )
{
    __m_scale_font = f;
    __m_scale_font_size = s;
    __m_scale_font_color = c;
    repaint();
}

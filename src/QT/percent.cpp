#include <Qpainter>
#include "QT/percent.h"

percent::percent(QWidget *parent)
    : QWidget{parent},
      __m_start_angle( -20 ),
      __m_end_angle( 220 ),
      __m_value( 0 ),
      __m_back_color( QColor(0,0,0, 100 ) ),
      __m_canvas_color( QColor(255,255,255 ) ),
      __m_arc_color( QColor( 128,200,128 ) ),
      __m_font_color( QColor( 255,255,255 ))
{

}

percent::~percent(){}

void percent::paintEvent( QPaintEvent * /*evt*/ )
{
    QPainter p( this );
    p.setRenderHints(QPainter::Antialiasing | QPainter::TextAntialiasing );

    QSize s = size();
    QPen pen2( __m_canvas_color );
    p.setPen( pen2 );
    QBrush b( __m_canvas_color );
    p.setBrush( b );
    p.drawRect( 1, 1,s.width() - 2, s.height() - 2);

    p.setViewport( 15, 15 ,s.width() - 30 , s.height() - 30 );
    p.setWindow( -50,-50 , 100, 100 );
    // 调试边框内容
    QPen pen1( __m_back_color );
    p.setPen( pen1 );
    QBrush b2( __m_back_color );
    p.setBrush( b2 );

    p.drawEllipse( -50 , -50 , 100 , 100 );

    QPen pen( __m_arc_color.lighter() );
    pen.setWidthF( 3 );
    p.setPen( pen );
    p.drawArc( -40, -40 , 80 , 80 , ( 180 -__m_start_angle) * 16,-__m_end_angle  * 16 );

    pen.setColor( __m_arc_color );
    pen.setWidthF( 5 );
    p.setPen( pen );

    float span = __m_end_angle - __m_start_angle;
    if( span < 0 ) span = -span;
    float r = __m_value / 100;
    span *= r;

    p.drawArc( -40, -40 , 80 , 80 , ( 180 -__m_start_angle) * 16, -span * 16 );

    p.setPen( pen1 );
    p.setBrush( b2 );
    p.drawEllipse( -30 , -30 , 60, 60 );

    QString str = QString::asprintf( "%.0f%%" , __m_value );
    QFontMetricsF fm( __m_font );
    QRectF s1 = fm.boundingRect( str );
    qreal x = 0, y = 0;
    x = s1.width() / 2;
    y = s1.height() / 4;
    pen.setColor( __m_font_color );
    p.setFont( __m_font );
    p.setPen( pen );
    p.drawText( -x , y , str );
}


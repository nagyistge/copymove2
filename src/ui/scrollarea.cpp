#include "scrollarea.hpp"

#include <QWheelEvent>
#include <QLabel>
#include <QScrollBar>
#include <QPainter>
#include <QDebug>

#include "log/log.hpp"

ScrollArea::ScrollArea( QWidget* parent ) :
    QScrollArea( parent ),
    mZoom( 1.f ) {

    this->setBackgroundRole( QPalette::Shadow );

    mLabel = new QLabel( this );
    mLabel->setSizePolicy( QSizePolicy::Ignored, QSizePolicy::Ignored );
    mLabel->setScaledContents( true );
    this->setWidget( mLabel );
    this->setWidgetResizable( true );
}

void ScrollArea::autoZoom() {

    if( mImageSize.isEmpty() ) {
        return;
    }

    float scaledW   = this->size().width();
    float originalW = mImageSize.width();

    float scaledH   = this->size().height();
    float originalH = mImageSize.height();

    mZoom = std::min( scaledW / originalW, scaledH / originalH ) * 0.99;
    this->zoom();
}

void ScrollArea::scrollBy( const QPointF& diff ) {
    horizontalScrollBar()->setValue( horizontalScrollBar()->value() + diff.x() );
    verticalScrollBar()->setValue( verticalScrollBar()->value() + diff.y() );
}

void ScrollArea::slotDrawImage( QImage image, bool fit ) {
    Q_ASSERT( image.format() == QImage::Format_ARGB32_Premultiplied );

    mImageSize = image.size();
    mLastRect = this->mLabel->rect();

    QPixmap pixmap = QPixmap::fromImage( image );
    mLabel->setPixmap( pixmap );

    if( fit ) {
        mLabel->adjustSize();
        autoZoom();
    }
}

void ScrollArea::centerZoom() {
    QPointF diff;

    int dw = mLabel->rect().width() - mLastRect.width();
    diff.setX( dw / 2 );

    int dh = mLabel->rect().height() - mLastRect.height();
    diff.setY( dh / 2 );

    scrollBy( diff );
    mLastRect = mLabel->rect();
}

void ScrollArea::wheelEvent( QWheelEvent* event ) {

    if( mLabel->pixmap() ) {

        this->setWidgetResizable( false );
        float delta = event->delta();

        // clip range from -1..1
        delta /= 120.f;
        delta = std::min( std::max( delta, -1.f ), 1.f );

        // zoom factor 1/25
        delta /= 25.f;

        mZoom += mZoom * delta;
        this->zoom();
    }
}

void ScrollArea::mouseDoubleClickEvent( QMouseEvent* ) {
    this->autoZoom();
}

void ScrollArea::mousePressEvent( QMouseEvent* event ) {
    mMousePressed = true;
    mMousePosition = event->pos();
    update();
}

void ScrollArea::mouseReleaseEvent( QMouseEvent* ) {
    mMousePressed = false;
    update();
}

void ScrollArea::mouseMoveEvent( QMouseEvent* event ) {
    if( mMousePressed ) {

        // get inverted delta
        QPointF position = event->pos();
        QPointF diff = mMousePosition - position;

        // scroll by delta
        scrollBy( diff );

        // update
        mMousePosition = position;
        update();
    }
}

void ScrollArea::zoom() {
    LOG( "Zoom : " + std::to_string( mZoom ) );
    mLabel->resize( mZoom * mLabel->pixmap()->size() );
    centerZoom();
}

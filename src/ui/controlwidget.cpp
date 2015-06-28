#include "controlwidget.hpp"
#include "ui_controlwidget.h"

#include <QPainter>

Q_DECLARE_METATYPE( ShiftHit )

ControlWidget::ControlWidget( QWidget* parent ) :
    QWidget( parent ),
    ui( new Ui::ControlWidget ) {
    ui->setupUi( this );
}

ControlWidget::~ControlWidget() {
    delete ui;
}

void ControlWidget::slotResults( std::vector<ShiftHit>::const_iterator begin, std::vector<ShiftHit>::const_iterator end ) {

    ui->comboHits->clear();

    for( auto it = begin; it != end; ++it ) {
        QString ranking = QString::number( it->ranking() ) + " : " + QString::number( it->hits() );
        QVariant data;
        data.setValue<ShiftHit>( *it );
        ui->comboHits->addItem( ranking, data );
    }
}

void ControlWidget::on_pushButton_clicked() {
    SorterParams params;
    params.mMinimalHits = ui->spinBoxMinHits->value();
    emit signalRun( params );
}

void ControlWidget::on_comboHits_currentIndexChanged( int ) {
    ShiftHit hit = ui->comboHits->currentData().value<ShiftHit>();
    ui->textHit->setText( QString::fromStdString( hit.toString() ) );

    QImage image( hit.imageSize().x(), hit.imageSize().y(), QImage::Format_ARGB32_Premultiplied );
    image.fill( 0 );

    QPainter painter( &image );
    // painter.setRenderHint( QPainter::Antialiasing, true );

    std::list<std::pair<PointI, PointI>>& pairs = hit.getBlocks();

    QBrush green = QBrush( QColor( 0, 255, 0, 255 ) );
    QBrush red   = QBrush( QColor( 255, 0, 0, 255 ) );

    for( std::pair<PointI, PointI>& pair : pairs ) {
        painter.fillRect( QRect( pair.first.x(), pair.first.y(), Block::size, Block::size ), red );
        painter.fillRect( QRect( pair.second.x(), pair.second.y(), Block::size, Block::size ), green );
    }

    LOG( "Sending hit " + hit.toString() );
    emit signalHit( image );
}

#include"3DViewer/CGraphicsView.h"

CGraphicsView::CGraphicsView(QWidget* parent)
	: QGraphicsView(parent), mMoveStart(false), mContinuousMove(false)
	, mMousePoint(QPoint(0, 0)) {

	init();
}

CGraphicsView::~CGraphicsView() {

}


void CGraphicsView::addImage(const QPixmap & pixmap, float xpos, float ypos, bool bClear)
{
	if (pixmap.isNull())
		return;

	const QPointF offset = QPointF(xpos, ypos);
	m_image->setPixmap(pixmap);
	m_image->setPos(offset);
	m_image->show();
	

	fitInView(m_scene->sceneRect(), Qt::KeepAspectRatio);
}

void CGraphicsView::addImage(const QString & qs_filename, float xpos, float ypos, bool bClear)
{
	pixtureName = qs_filename;
	QImage image_temp;
	image_temp.load(qs_filename);

	QPixmap pixmap = QPixmap::fromImage(image_temp);
	if (pixmap.isNull())
		return;
	else {
		const QPointF offset = QPointF(xpos, ypos);
		m_image->setPixmap(pixmap);
		m_image->setPos(offset);
		m_image->show();
	}

	fitInView(m_scene->sceneRect(), Qt::KeepAspectRatio);
	update();
}


void CGraphicsView::zoomIn(){

	scale(1.25, 1.25);
	update();
}

void CGraphicsView::zoomOut() {

	scale(0.8, .8);
	update();
}
void CGraphicsView::normalSize() {

	resetTransform();
	update();
}

void CGraphicsView::mousePressEvent(QMouseEvent* e)
{
	QWidget::mousePressEvent(e);
}


void CGraphicsView::mouseReleaseEvent(QMouseEvent *e)
{

	mMoveStart = false;

	if (QApplication::keyboardModifiers() != Qt::ShiftModifier)
	{
		setCursor(Qt::ArrowCursor);
	}
	QWidget::mouseReleaseEvent(e);

}

void CGraphicsView::mouseMoveEvent(QMouseEvent *e)
{

	if (e->buttons() & Qt::LeftButton)
	{
		if (!mMoveStart)
		{

			mMoveStart = true;
			mContinuousMove = false;
			mMousePoint = e->globalPos();
		}
		else
		{
			setCursor(Qt::ClosedHandCursor);
			QScrollBar *scrollBarx = horizontalScrollBar();
			QScrollBar *scrollBary = verticalScrollBar();

			QPoint p = e->globalPos();
			int offsetx = p.x() - mMousePoint.x();
			int offsety = p.y() - mMousePoint.y();
			if (!mContinuousMove && (offsetx > -10 && offsetx < 10) && (offsety > -10 && offsety < 10))
				return;

			mContinuousMove = true;
			scrollBarx->setValue(scrollBarx->value() - offsetx);
			scrollBary->setValue(scrollBary->value() - offsety);

			mMousePoint = p;
		}

	}
	QWidget::mouseMoveEvent(e);
}

void CGraphicsView::keyPressEvent(QKeyEvent *event)
{

}

void CGraphicsView::keyReleaseEvent(QKeyEvent *event)
{


}

void CGraphicsView::wheelEvent(QWheelEvent * e)
{

	const int numSteps = e->delta() / 15 / 8;
	if (numSteps == 0) {
		e->ignore();
		return;
	}
	if (numSteps > 0)
		zoomIn();
	else
		zoomOut();
	
	
	QPointF cursorPoint = e->pos();
	QPointF scenePos = this->mapToScene(QPoint(cursorPoint.x(), cursorPoint.y()));
	qreal viewWidth = this->viewport()->width();
	qreal viewHeight = this->viewport()->height();
	qreal hScale = cursorPoint.x() / viewWidth;
	qreal vScale = cursorPoint.y() / viewHeight;

	QPointF viewPoint = this->matrix().map(scenePos);
	horizontalScrollBar()->setValue(int(viewPoint.x() - viewWidth * hScale));
	verticalScrollBar()->setValue(int(viewPoint.y() - viewHeight * vScale));
	

	e->accept();

}

void CGraphicsView::resizeEvent(QResizeEvent* event) {

	fitInView(m_scene->sceneRect(), Qt::KeepAspectRatio);
	return QGraphicsView::resizeEvent(event);
}


void CGraphicsView::zoom(qreal factor, QPointF centerPoint)
{
	scale(factor, factor);
}


void CGraphicsView::init() {


	m_scene = new QGraphicsScene;
	setScene(m_scene);

	setBackgroundRole(QPalette::Dark);
	setAlignment(Qt::AlignCenter);
	setCacheMode(QGraphicsView::CacheBackground);

	setMouseTracking(false);
	setFocusPolicy(Qt::WheelFocus);

	m_image = new QGraphicsPixmapItem;
	m_image->setTransformationMode(Qt::FastTransformation);
	m_image->hide();
	m_scene->addItem(m_image);


	setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
	setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
}


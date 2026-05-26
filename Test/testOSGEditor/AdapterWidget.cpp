#include "AdapterWidget.h"

AdapterWidget::AdapterWidget( QWidget * parent, const char * name, const QGLWidget * shareWidget, WindowFlags f):
QGLWidget(parent, shareWidget, f)
{
	_gw = new osgViewer::GraphicsWindowEmbedded(0,0,width(),height());
	setFocusPolicy(Qt::StrongFocus);
	setMouseTracking( true );
}

void AdapterWidget::setKeyboardModifiers( QInputEvent* event )
{
	int modkey = event->modifiers() & (Qt::ShiftModifier | Qt::ControlModifier | Qt::AltModifier);
	unsigned int mask = 0;
	if ( modkey & Qt::ShiftModifier ) mask |= osgGA::GUIEventAdapter::MODKEY_SHIFT;
	if ( modkey & Qt::ControlModifier ) mask |= osgGA::GUIEventAdapter::MODKEY_CTRL;
	if ( modkey & Qt::AltModifier ) mask |= osgGA::GUIEventAdapter::MODKEY_ALT;
	_gw->getEventQueue()->getCurrentEventState()->setModKeyMask( mask );
}
void AdapterWidget::resizeGL( int width, int height )
{
	_gw->getEventQueue()->windowResize(0, 0, width, height );
	_gw->resized(0,0,width,height);
}
void AdapterWidget::keyPressEvent( QKeyEvent* event )
{
	setKeyboardModifiers( event );
	_gw->getEventQueue()->keyPress( (osgGA::GUIEventAdapter::KeySymbol)   *(event->text().toLatin1().data() ) );
}
void AdapterWidget::keyReleaseEvent( QKeyEvent* event )
{
	setKeyboardModifiers( event );
	_gw->getEventQueue()->keyRelease( (osgGA::GUIEventAdapter::KeySymbol)  *(event->text().toLatin1().data() ) );
}
void AdapterWidget::mousePressEvent( QMouseEvent* event )
{
	int button = 0;
	switch(event->button())
	{
	case(Qt::LeftButton): button = 1; break;
	case(Qt::MidButton): button = 2; break;
	case(Qt::RightButton): button = 3; break;
	case(Qt::NoButton): button = 0; break;
	default: button = 0; break;
	}
	setKeyboardModifiers( event );
	_gw->getEventQueue()->mouseButtonPress(event->x(), event->y(), button);
}
void AdapterWidget::mouseReleaseEvent( QMouseEvent* event )
{
	int button = 0;
	switch(event->button()) 
	{
	case(Qt::LeftButton): button = 1; break;
	case(Qt::MidButton): button = 2; break;
	case(Qt::RightButton): button = 3; break;
	case(Qt::NoButton): button = 0; break;
	default: button = 0; break;
	}
	setKeyboardModifiers( event );
	_gw->getEventQueue()->mouseButtonRelease(event->x(), event->y(), button);
}
void AdapterWidget::mouseDoubleClickEvent( QMouseEvent* event )
{
	int button = 0;
	switch ( event->button() )
	{
	case Qt::LeftButton: button = 1; break;
	case Qt::MidButton: button = 2; break;
	case Qt::RightButton: button = 3; break;
	case Qt::NoButton: button = 0; break;
	default: button = 0; break;
	}
	setKeyboardModifiers( event );
	_gw->getEventQueue()->mouseDoubleButtonPress( event->x(), event->y(), button );
}
void AdapterWidget::mouseMoveEvent( QMouseEvent* event )
{
	setKeyboardModifiers( event );
	_gw->getEventQueue()->mouseMotion(event->x(), event->y());
}
void AdapterWidget::wheelEvent( QWheelEvent* event )
{
	setKeyboardModifiers( event );
	_gw->getEventQueue()->mouseScroll(
		event->delta()>0 ? osgGA::GUIEventAdapter::SCROLL_UP : osgGA::GUIEventAdapter::SCROLL_DOWN );
}
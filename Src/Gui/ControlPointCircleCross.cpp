#include "Gui/controlPointCircleCross.h"
#include <QPainter>

		ControlPointCircleCross::ControlPointCircleCross(QGraphicsItem* parent) :
			Alpha_(255)
		{
			//setFlag(QGraphicsItem::ItemIgnoresTransformations);
			setFlag(ItemSendsGeometryChanges);
			setCacheMode(DeviceCoordinateCache);
			setZValue(1.);
		}

		QRectF ControlPointCircleCross::boundingRect() const
		{
			return QRectF(QPointF(-60, -60), QSize(120, 120));
		}

		QPainterPath ControlPointCircleCross::shape() const
		{
			return paintCross();
		}
		QPainterPath ControlPointCircleCross::paintCross(int x, int y, int w, int h, int step) const
		{
			QPainterPath path;

			//QRect drawRect(0,0,100,100);
			//QRegion region(drawRect, QRegion::Ellipse);
			////QRegion region(drawRect.adjusted(10, 10, -10, -10), QRegion::Ellipse);
			//drawRect.setSize(QSize(drawRect.width() / 2, drawRect.height() / 2));
			//drawRect.moveTopLeft(QPoint(drawRect.width() / 2, drawRect.height() / 2));
			//QRegion region2(drawRect, QRegion::Ellipse);
			//path.addRegion(region.subtracted(region2));

			//path.addRegion(region);
			path.addRect(x - w - step, y - h / 2, w, h);
			path.addRect(x - h / 2, y + step + h / 2, h, w - h / 2);
			path.addRect(x + step, y - h / 2, w, h);
			path.addRect(x - h / 2, y - w - step, h, w - h / 2);
			//path.addRect(x , y,1,1);
			return path;
		}
		void  ControlPointCircleCross::changeAlpha(int Alpha)
		{
			Alpha_ = Alpha;
			this->update();
		}
		void ControlPointCircleCross::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget)
		{
			//QPainter painter(this);
			//painter.save();
			////ÉèÖÃ·´¾â³Ý

			QColor color = QColor(Qt::green);
			color.setAlpha(Alpha_);
			painter->setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform | QPainter::Qt4CompatiblePainting);
			//painter->setCompositionMode(QPainter::CompositionMode_Difference);
			painter->setPen(QPen(color, 0));
			QRect drawRect(-60, -60, 120, 120);
			QRadialGradient rg(drawRect.center(), drawRect.width() / 2, drawRect.center());
			rg.setColorAt(0, Qt::transparent);
			rg.setColorAt(0.9, Qt::transparent);
			rg.setColorAt(0.91, color);
			rg.setColorAt(1, color);
			painter->setBrush(rg);
			painter->drawEllipse(drawRect);
			

			QRadialGradient gradient;
			gradient.setColorAt(0, color);
			painter->setBrush(gradient);
			painter->drawPath(paintCross(0, 0, 60, 2, 0));
		}

		QVariant ControlPointCircleCross::itemChange(GraphicsItemChange change, const QVariant& value)
		{
			const QVariant variant = QGraphicsItem::itemChange(change, value);

			return variant;
			//return QVariant();
		}

		void ControlPointCircleCross::mousePressEvent(QGraphicsSceneMouseEvent* event)
		{
			update();
			QGraphicsItem::mousePressEvent(event);
		}

		void ControlPointCircleCross::mouseReleaseEvent(QGraphicsSceneMouseEvent* event)
		{
			update();
			QGraphicsItem::mouseReleaseEvent(event);
		}


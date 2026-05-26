// This file is part of OpenMVG, an Open Multiple View Geometry C++ library.

// Copyright (c) 2015 Pierre MOULON.

// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.


#include <QGraphicsItem>
#include <QPointF>

		class GraphWidget;
		class QGraphicsSceneMouseEvent;

		// Graphical movable QtGraphicItem to represent a control_point image observation
		// A dynamic update of the control_point observation coordinates is performed thanks to variable reference.
		class ControlPointCircleCross : public QGraphicsItem
		{
		public:
			ControlPointCircleCross(QGraphicsItem* parent = nullptr);

			QRectF boundingRect() const;
			QPainterPath shape() const;
			void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget);
			void changeAlpha(int Alpha);
		protected:
			QVariant itemChange(GraphicsItemChange change, const QVariant& value);
			QPainterPath paintCross(int x = 0, int y = 0, int w = 18, int h = 2, int step = 2) const;
			void mousePressEvent(QGraphicsSceneMouseEvent* event);
			void mouseReleaseEvent(QGraphicsSceneMouseEvent* event);
		private:
			int Alpha_;
		};


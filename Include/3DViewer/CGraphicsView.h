#pragma once

#include <QObject>
#include <QApplication>
#include<QGraphicsPixmapItem>
#include<QGraphicsScene>
#include<QScrollBar>
#include<QGraphicsView>
#include<QWheelEvent>
#include<QEvent>
#include "3DViewer/qt_utils.h"


		class CGraphicsView :public QGraphicsView
		{
			Q_OBJECT

		public:
			CGraphicsView(QWidget* parent = Q_NULLPTR);
			~CGraphicsView();
			void addImage(const QString& qs_filename, float xpos = 0.f, float ypos = 0.f, bool bClear = false);
			void addImage(const QPixmap& pixmap, float xpos = 0.f, float ypos = 0.f, bool bClear = false);
		protected: 
			void mousePressEvent(QMouseEvent* e);
			void mouseReleaseEvent(QMouseEvent* e);
			void mouseMoveEvent(QMouseEvent* e);

			void keyPressEvent(QKeyEvent* event);
			void keyReleaseEvent(QKeyEvent* event);
			void wheelEvent(QWheelEvent* event);

			void resizeEvent(QResizeEvent* event);

			void zoom(qreal factor, QPointF centerPoint);

		private slots:
			void zoomIn();
			void zoomOut();
			void normalSize();

		private:
			void init();

		private:
			QGraphicsScene* m_scene;
			QGraphicsPixmapItem* m_image;

			bool mMoveStart;
			bool mContinuousMove;
			QPoint mMousePoint;
			QString pixtureName;
			
		};


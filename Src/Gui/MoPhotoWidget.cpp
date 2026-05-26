#include "Util/TaskProcess.h"
#include "Util/Settings.h"
#include "Gui/MoPhotoWidget.h"


#include <QFileDialog>
#include <QMessageBox>

#include <QDateTime>
#include <sstream>
#include <QPainter>
#include <QGraphicsDropShadowEffect>
#include <QPixmapCache>
#include "Gui/GlobalStruct.h"

#include <QToolTip>
#include <QApplication>
#include <QBitmap>
#include <QScrollBar>
#include <QtConcurrent>
//#include "Core/Image.h"
#include <QSet>
#include <QHash>
#include <QCryptographicHash>


	

		double generateRandDouble1(int minInt, int maxInt)
		{
			int diff = abs(maxInt - minInt);
			if (diff == 0)
				diff = 100;

			int m1 = qrand() % diff;

			double m2 = qrand() / 10000000.0;
			double retval = m1 + m2;

			return retval;
		}

		// PHOTOGROUP ListView 定制组件
		MoPhotoTableWidget::MoPhotoTableWidget(QWidget* parent)
			: QTableView(parent)
		{
			setMouseTracking(true);
			bLeaved = true;
			iHoverRow = -1;
			this->mode = mode;
			// todo:change it based on alter-color(one of two alter color).
			previousHoverRowBackColor = QColor(0x28, 0x28, 0x28);
			previousHoverRow = -1;
			selectedRow = -1;
			// 设置ListView 单选且按行选.
			setSelectionBehavior(QAbstractItemView::SelectRows);
			setSelectionMode(QAbstractItemView::ExtendedSelection);//SingleSelection
			qsrand(QTime(0, 0, 0).secsTo(QTime::currentTime()));

		}


		void MoPhotoTableWidget::InitHeader()
		{

			//setRowCount(10);
			//setColumnCount(8);

			QFont font;
			font.setFamily("Arial");
			font.setPixelSize(14);
			font.setBold(false);
			//?chy
			origBackColor0 = QColor(0x28, 0x28, 0x28);// alt0
			origBackColor1 = QColor(0x3C, 0x3C, 0x3c);// alt1
		//	origBackColor2 = QColor(0x2A,0x4D,0x84);// sel
		//	origBackColor3 = QColor(0x46,0x64,0x94);// sel + hover
			origBackColor2 = Qt::red;// sel
			origBackColor3 = Qt::green;// sel + hover
			origBackColor4 = QColor(0x47, 0x47, 0x47);// hover color.

			// 使用定制的GcpListView Item定制组件.
			pItemDelegate = new MoPhotoDelegate(this,mode);
			setItemDelegate(pItemDelegate);


			// 关联进入GcpListView某一行的信号(鼠标未按下时)
			connect(this, &QTableView::entered, this, &MoPhotoTableWidget::cellEntered2);
			connect(this, &QTableView::entered, pItemDelegate, &MoPhotoDelegate::cellEntered2);

			//// 关键GcpListView的双击事件.
			connect(this, &QTableView::doubleClicked, this, &MoPhotoTableWidget::doubleClicked);
			connect(this, &QTableView::doubleClicked, pItemDelegate, &MoPhotoDelegate::doubleClicked);

			connect(pItemDelegate, &MoPhotoDelegate::itemModified, this, &MoPhotoTableWidget::Slot_itemModified);



			// GCP ListView 头部定制组件
			MoPhotoHeaderView* moHeaderView = new MoPhotoHeaderView(Qt::Horizontal, this,mode);

			//QStringList headerLabels;
			//CHY 
			//mode 1 pos列表
			if (AI3D::CORE::BlockObject::isChineseVersion())
			{
				if (mode == 1)
				{
					headerLabels << "影像名称" << "影像目录" << "解算状态" << "位姿";
				}
				else //group列表
				{
					headerLabels << "序号" << "影像组" << "影像数" << "传感器尺寸" << "焦距" << "焦距35";

				}
			}
			else
			{
				if (mode == 1)
				{
					headerLabels << "Photo Name" << "Photo directory" << "Pose Status" << "Pose";
				}
				else //group列表
				{
					headerLabels << "SerialNo." << "Photogroups" << "No.of Photos" << "Sensorsize" << "focal" << "focal35";

				}
			}


			//moHeaderView->sethe

			// 使用定制的表头组件
			setHorizontalHeader(moHeaderView);
			///setHorizontalHeaderLabels(headerLabels);

			horizontalHeader()->setDefaultAlignment(Qt::AlignCenter);
			//	horizontalHeader()->setVisible(false);
			verticalHeader()->setVisible(false);

			//horizontalHeader()->setStretchLastSection(true);

		

			//setStyleSheet("selection-background-color:rgba(0,0,0,50);");
			//setStyleSheet("");

			if (mode == 1)
			{
				// 设置不可编辑模式(比如切换了坐标模式)
				setEditTriggers(QAbstractItemView::NoEditTriggers);
			}
			else
			{
				// 设置GcpListView 可编辑
				setEditTriggers(QAbstractItemView::DoubleClicked);
			}

			///setAlternatingRowColors(true);

			horizontalHeader()->setStyleSheet("QHeaderView::section{ background-color:#333333;color: #A5A5A5; align:center;}");
			//	verticalHeader()->setStyleSheet("QHeaderView::section{ color: #FFFFFF; }");
			setShowGrid(false);


			horizontalHeader()->setDefaultAlignment(Qt::AlignLeft | Qt::AlignVCenter);

			font.setPixelSize(12);
			horizontalHeader()->setFont(font);


			pStandardItemModel = new QStandardItemModel(this);

			if (mode == 1)
			{
				colCount = col_photopos_e::COUNT_POHOTOPOSCOL;
			}
			else
			{
				colCount = col_pg_e::COUNT_PGCOL;
			}

			pStandardItemModel->setColumnCount(colCount);




			// 使用定制的GcpListView Item定制组件.
			
			setModel(pStandardItemModel);

			// 表头可按列进行排序.
			setSortingEnabled(true);
			//sortItems(2);


			// 设置各列尺寸.
			horizontalHeader()->setDefaultSectionSize(100);//110

			if (mode == 0)
			{
				horizontalHeader()->resizeSection(PGSERINO_COL, 180);//60
				horizontalHeader()->resizeSection(PGNAME_COL, 180);//60
				horizontalHeader()->resizeSection(PGPHOTOCOUNT_COL, 180);//130
				horizontalHeader()->resizeSection(PGSENSORSIZE_COL, 180);//60
				horizontalHeader()->resizeSection(PGFOCALLENGTH_COL, 180);//60
			}
			else
			{
				horizontalHeader()->resizeSection(PHOTONAME_COL, 200);//60
				horizontalHeader()->resizeSection(PHOTODIR_COL, 400);//60
				horizontalHeader()->resizeSection(PHOTOPOSESTATUS_COL, 90);//
				//horizontalHeader()->resizeSection(PHOTOPOS_COL, 180);
				/*horizontalHeader()->resizeSection(4, 80);

				horizontalHeader()->resizeSection(5, 75);*/
			}
			horizontalHeader()->setStretchLastSection(true);

		}
		int MoPhotoTableWidget::ColCount()
		{
			return colCount;
		}
		int MoPhotoTableWidget::RowCount()
		{
			return pStandardItemModel->rowCount();
		}
		// 根据选择的坐标类别,显示不同的表头
		void MoPhotoTableWidget::setHeaderLabelsMode()
		{

			if (mode != 0)
				return;


			headerLabels.clear();


			std::vector<QString>  strs(column_photogroup_e::COUNT_PGCOL);
			
			if (AI3D::CORE::BlockObject::isChineseVersion())
			{
				strs[PGSERINO_COL] = "序号";
				strs[PGNAME_COL] = "影像组";
				strs[PGPHOTOCOUNT_COL] = "影像数";
				strs[PGSENSORSIZE_COL] = "传感器尺寸(um)";
				strs[PGFOCALLENGTH_COL] = "焦距长度(mm)";
				strs[PG35MMFOCAL_COL] = "35毫米等效焦距";
			}
			else
			{
				strs[PGSERINO_COL] = "SerialNO.";
				strs[PGNAME_COL] = "Photogroups";
				strs[PGPHOTOCOUNT_COL] = "No.of photos";
				strs[PGSENSORSIZE_COL] = "Sensor Size(um)";
				strs[PGFOCALLENGTH_COL] = "Focal Length(mm)";
				strs[PG35MMFOCAL_COL] = "35 mm eq.";
			}


			for (int i = 0; i < strs.size(); i++)
			{
				headerLabels << strs[i];
			}


			horizontalHeader()->repaint();

		}


		MoPhotoTableWidget::~MoPhotoTableWidget()
		{

		}

		//pStandardItemModel
		// 清空GcpListView 数据.
		void MoPhotoTableWidget::clearData()
		{
			if (!pStandardItemModel)
				return;

			//setUpdatesEnabled(false);

			int row = pStandardItemModel->rowCount();

			QList <int> delteRows;
			for (int i = 0; i < pStandardItemModel->rowCount(); i++)
			{
				int iLine = pStandardItemModel->item(i, 1)->text().toInt();
				
				
					delteRows.append(i);
				
			}
			int deleted = 0;
			for (int i = 0; i < delteRows.count(); i++)
			{
				int rowtemp = delteRows[i];
				pStandardItemModel->removeRow(rowtemp + deleted);
				--deleted;
			}
		}




		// 通过行号选中GcpListView 指定行.
		void MoPhotoTableWidget::selectOneRow(int row)
		{
			int rowCount = pStandardItemModel->rowCount();
			if (rowCount <= 0 || row < 0 || row >= rowCount)
				return;
			selectRow(row);
		}

		// 通过GcpId选中GcpListView 指定行.
		void MoPhotoTableWidget::selectOneRowByGroupId(int currentgcp_id_)
		{
			int rowCount = pStandardItemModel->rowCount();
			if (rowCount <= 0)
				return;
			for (int i = 0; i < rowCount; i++)
			{
				QStandardItem* item = pStandardItemModel->item(i, PGNAME_COL);
				if (item->data(AI3D::GUI::CRPhotoGroupID).toInt() == currentgcp_id_)
				{
					selectRow(i);
					break;
				}
			}
		}

		// 通过ImageId选中指定行.
		void MoPhotoTableWidget::selectOneRowByImageId(uint32_t image_id_)
		{
			int rowCount = pStandardItemModel->rowCount();
			if (rowCount <= 0)
				return;
			for (int i = 0; i < rowCount; i++)
			{
				QStandardItem* item = pStandardItemModel->item(i, PHOTONAME_COL);
				if (item->data(277).toUInt() == image_id_)
				{
					selectRow(i);
					break;
				}
			}
		}

		// 获取指定行的GcpId.
		uint64_t MoPhotoTableWidget::getGroupIdByRow(int row)
		{
			int rowCount = pStandardItemModel->rowCount();

			if (rowCount <= 0 || row < 0 || row >= rowCount)
				return std::numeric_limits<uint64_t>::max();;

			return pStandardItemModel->item(row, PGNAME_COL)->data(AI3D::GUI::CRPhotoGroupID).toInt();
		}
		void MoPhotoTableWidget::removeOneRow(int row)
		{

			int rowCount = pStandardItemModel->rowCount();
			if (rowCount <= 0 || row < 0 || row >= rowCount)
			{
				return;
			}
			bool bResult = pStandardItemModel->removeRow(row);
			update();
		}
		QStandardItem* MoPhotoTableWidget::getItem(int row, int col)
		{
			int rowCount = pStandardItemModel->rowCount();

			if (rowCount <= 0 || row < 0 || row >= rowCount)
				return nullptr;

			return pStandardItemModel->item(row, col);
		}

		// 获取指定行的ImageId.
		uint32_t MoPhotoTableWidget::getImageIdByRow(int row)
		{
			int rowCount = pStandardItemModel->rowCount();

			if (rowCount <= 0 || row < 0 || row >= rowCount)
				return std::numeric_limits<uint32_t>::max();;

			return pStandardItemModel->item(row, PHOTONAME_COL)->data(AI3D::GUI::CRImageID ).toUInt();
		}

		void MoPhotoTableWidget::appendRowData(photogroup_list_item_st& gcpListItem)
		{
			int row = pStandardItemModel->rowCount();

			QFont font;
			font.setFamily("Arial");
			font.setPixelSize(14);
			font.setBold(false);

			///QColor color = GetColor(gcp.color_);
			for (int i = 0; i < colCount /*pStandardItemModel->columnCount()*/; i++)
			{
				QStandardItem* itemTemp = new QStandardItem("");
				itemTemp->setFont(font);
				pStandardItemModel->setItem(row, i, itemTemp);
			}		

			pStandardItemModel->item(row, PGNAME_COL)->setData(gcpListItem.id_, AI3D::GUI::CRPhotoGroupID); //CRControlpointsID;
			///pStandardItemModel->item(row, PGNAME_COL)->setData(QString::fromStdString(gcpListItem.photogroupname_), Qt::EditRole);
			///pStandardItemModel->item(row, PGNAME_COL)->setToolTip(QString::fromStdString(gcpListItem.photogroupname_));
			pStandardItemModel->item(row, PGNAME_COL)->setData(str2qstr(gcpListItem.photogroupname_), Qt::EditRole);
			pStandardItemModel->item(row, PGNAME_COL)->setToolTip(str2qstr(gcpListItem.photogroupname_));

			pStandardItemModel->item(row, PGPHOTOCOUNT_COL)->setData(QString::number(gcpListItem.photocount_), Qt::EditRole);
			pStandardItemModel->item(row, PGPHOTOCOUNT_COL)->setTextAlignment(Qt::AlignCenter);
			QString focalstr =  QString::number(gcpListItem.focalmm_, 'f', 4);
			//QString focalstr = QString::fromStdString(std::to_string(gcpListItem.focalmm_));
			if (gcpListItem.focalmm_ <= 0)
			{
				focalstr = UNDEFINEDSTR;
			}
			pStandardItemModel->item(row, PGFOCALLENGTH_COL)->setData(focalstr, Qt::EditRole);
			QString sensorstr = QString::number(gcpListItem.sensorsize_, 'f', 4); //QString::fromStdString(std::to_string(gcpListItem.focalmm_));
			if (gcpListItem.sensorsize_ <= 0)
			{
				sensorstr = UNDEFINEDSTR;
			}
			pStandardItemModel->item(row, PGSENSORSIZE_COL)->setData(sensorstr, Qt::EditRole);
			pStandardItemModel->item(row, PGSENSORSIZE_COL)->setData(sensorstr, Qt::UserRole + 1);

			QString focal35mmstr = QString::number(gcpListItem.focal35mm_, 'f', 4);
			if (gcpListItem.focal35mm_ <= 0)
			{
				focal35mmstr = "";
			}
			pStandardItemModel->item(row, PG35MMFOCAL_COL)->setData(focal35mmstr, Qt::EditRole);
			
		}


		void MoPhotoTableWidget::appendRowData(photopose_list_item_st& gcpListItem)
		{
			int row = pStandardItemModel->rowCount();

			///QColor color = GetColor(gcp.color_);
			for (int i = 0; i < colCount /*pStandardItemModel->columnCount()*/; i++)
			{
				QStandardItem* itemTemp = new QStandardItem("");
				pStandardItemModel->setItem(row, i, itemTemp);
			}

			

			pStandardItemModel->item(row, PHOTONAME_COL)->setData(gcpListItem.image_id_, AI3D::GUI::CRImageID); //CRControlpointsID;

			
			pStandardItemModel->item(row, PHOTONAME_COL)->setData(/*QFileInfo*/(str2qstr(gcpListItem.photo_name_))/*.fileName()*/, Qt::EditRole);
			pStandardItemModel->item(row, PHOTODIR_COL)->setData(str2qstr(gcpListItem.photo_dir_), Qt::EditRole);

			QString statusstr; 
			if (AI3D::CORE::BlockObject::isChineseVersion())
			{
				statusstr = "未解算"; 
				if(gcpListItem.status_ == pose_status_e::POSE_ST_COMPLETED)
					statusstr = "完成";
			}
			else
			{
				statusstr = "Unknown"; 
				if(gcpListItem.status_ == pose_status_e::POSE_ST_COMPLETED)
					statusstr = "Completed";
			}


			pStandardItemModel->item(row, PHOTOPOSESTATUS_COL)->setData(statusstr, Qt::EditRole);

			pStandardItemModel->item(row, PHOTOPOS_COL)->setData(QString::fromStdString(gcpListItem.posvalus_str_), Qt::EditRole);
		}

		

		int MoPhotoTableWidget::getMode()
		{
			return mode;
		}

		int MoPhotoTableWidget::getColCount()
		{
			return colCount;
		}

		void MoPhotoTableWidget::doubleClicked(const QModelIndex& index)
		{
			std::cout << "1" << std::endl;
		}

		void MoPhotoTableWidget::mousePressEvent(QMouseEvent* event)
		{
			QTableView::mousePressEvent(event);
		}

		void MoPhotoTableWidget::mouseReleaseEvent(QMouseEvent* event)
		{
			QTableView::mouseReleaseEvent(event);
		}

		// 更新指定行显示.
		void MoPhotoTableWidget::updateRow(int row)
		{
			//int colCount = columnCount();
			int colCount = pStandardItemModel->columnCount();
			for (int i = 0; i < colCount; i++)
			{
				QModelIndex modelIndex = model()->index(row, i);
				update(modelIndex);
			}
		}

		//bLeaved
		// bLeaved 设置为当整体离开MoTableWidget的状态标志,避免处于MoTableWidget在上下端
		// 边界行移出时,不复位刚Hover的颜色状态.
		void MoPhotoTableWidget::cellEntered2(const QModelIndex& index)
		{

			bLeaved = false;
			iHoverRow = index.row();
		}


		void MoPhotoTableWidget::hideEvent(QHideEvent* hideEvent)
		{
			// process when hidden.
		}

		// bLeaved 设置为当整体离开MoTableWidget的状态标志,避免处于MoTableWidget在上下端
		// 边界行移出时,不复位刚Hover的颜色状态.
		void MoPhotoTableWidget::leaveEvent(QEvent* event)
		{
			//QTableViewItem* twItem = nullptr;

			bLeaved = true;

			if (iHoverRow != -1)
			{
				updateRow(iHoverRow);
			}

			iHoverRow = -1;

		}
		//?chy
		void MoPhotoTableWidget::setRowColor(int row, QColor _color)
		{

		}

		void MoPhotoTableWidget::Slot_itemModified(int row, int col, const QString& val) const
		{
			emit itemModified(row, col, val);
		}



		// 定制MoTableWidget的每个item使用该定制组件
		MoPhotoDelegate::MoPhotoDelegate(QWidget* parent,int mode)
			: QStyledItemDelegate(parent)
		{
			pTableWidget = qobject_cast<MoPhotoTableWidget*>(parent);
			iHoverRow = -1;
			mode_ = mode;
		}

		void MoPhotoDelegate::doubleClicked(const QModelIndex& index)
		{
			std::cout << "2" << std::endl;
		}
		void MoPhotoTableWidget::SetSelectionChanged(bool bchanged )
		{
			bchanged_ = bchanged;
		}

		// 对MoTableWidget的Item项按不同状态及图文信息进行定制显示.
		void MoPhotoDelegate::paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const
		{
			int row = index.row();
			int col = index.column();
			QString txt = index.data().toString();
			 
			if (true) //col != 2 && col != 4)
			{
				if (iHoverRow != -1 && iHoverRow == row)
				{

				}
				/*if (mode_ == 1 && pTableWidget->bchanged_)
				{
					std::cout << mode_ << " mode- " << option.state << std::endl;
				}*/

				if (option.state & QStyle::State_Selected)
				{

					QItemSelectionModel* selections = pTableWidget->selectionModel();

					//	//获取被选中的指针列表
					QModelIndexList IndexList = selections->selectedIndexes();
					foreach(QModelIndex _index, IndexList)
					{
						//std::cout << " index " << _index.row() << std::endl;
						/*if (mode_ == 1)
						{
							std::cout << " index " << _index.row() << std::endl;
						}*/
						// 对于当前选中行.
						if (!pTableWidget->bLeaved && (option.state & QStyle::State_MouseOver || index.row() == iHoverRow))
						{

							// 当前item选中状态且Hover,并且没有光标没有离开MoTableWidget.
							//?chy
							painter->fillRect(option.rect, QColor(0x46, 0x64, 0x94, 0xff));
							/*if (index.row() == pTableWidget->currentIndex().row())
							{
								if (mode_ == 1)
								std::cout << index.row() << " -multi- " << pTableWidget->currentIndex().row() << std::endl;
							}*/
						}
						else
						{

							// 当前item选中状态,但光标已经离开MoTableWidget或者没有Hover

							//if (index.row() == pTableWidget->currentIndex().row())
							
							if (pTableWidget->bchanged_)
							{
								/*if (mode_ == 1)
								 std::cout << mode_ << " mode " << std::endl;*/
								if (index.row() == pTableWidget->currentIndex().row())
								{
									painter->fillRect(option.rect, QColor(0x2A, 0x4D, 0x84, 0xff));
									/*if (mode_ == 1)
									std::cout << index.row() << " -multi-19-- " << pTableWidget->currentIndex().row() << std::endl;*/
								}
								else if (index.row() % 2 == 0)
								{

									// 对于其他的偶数行,显示对应的背景色.
									//paintBackgroundBase(painter, option, index);
									painter->fillRect(option.rect, QColor(0x28, 0x28, 0x28, 0xff));
								}
								else
								{


									//paintBackgroundAlternateBase(painter, option, index);
									// 对于其他的奇数行,显示对应的背景色.
									painter->fillRect(option.rect, QColor(0x3C, 0x3C, 0x3C, 0xff));
								}
								pTableWidget->bchanged_ = false;
							}
							else
							{
								painter->fillRect(option.rect, QColor(0x2A, 0x4D, 0x84, 0xff));
								/*if (mode_ == 1)
								std::cout << index.row() << " -multi-199-- " << pTableWidget->currentIndex().row() << std::endl;*/
							}
							
						}

					}
					//paintBackgroundHighLighted(painter, option, index);
				}
				else if (!pTableWidget->bLeaved && (option.state & QStyle::State_MouseOver || index.row() == iHoverRow))
				{

					// 当前item没有选中,但光标没有离开MoTableWidget且处于Hover.
					painter->fillRect(option.rect, QColor(0x47, 0x47, 0x47, 0xff));


				}
				else if (index.row() % 2 == 0)
				{

					// 对于其他的偶数行,显示对应的背景色.
					//paintBackgroundBase(painter, option, index);
					painter->fillRect(option.rect, QColor(0x28, 0x28, 0x28, 0xff));
				}
				else
				{


					//paintBackgroundAlternateBase(painter, option, index);
					// 对于其他的奇数行,显示对应的背景色.
					painter->fillRect(option.rect, QColor(0x3C, 0x3C, 0x3C, 0xff));
				}
			}

			
			
			{
				// 前两列之外显示对应的文本(EditRole属性)
				///	painter->drawText(option.rect, Qt::AlignCenter, index.data().toString());
				QString txt = pTableWidget->model()->index(row, col).data(Qt::EditRole).toString();
				//painter->drawText(option.rect, Qt::AlignCenter, index.data().toString());
				QRect r(option.rect);
				r.setLeft(option.rect.x() + 6);

				//		painter->drawText(option.rect, Qt::AlignLeft|Qt::AlignVCenter, txt);
				painter->drawText(r, Qt::AlignLeft | Qt::AlignVCenter, txt);
			}
			//if (mode_ == 0)
			{

				if ((col == 0)&& (mode_ == 0))
				{
					// 首列显示行号.
					
					painter->drawText(option.rect, Qt::AlignCenter, QString::number(row + 1));
				}
			}
			painter->save();

			// 绘制item的右边线.
			QPen pen(QColor("#1D1D1D"), 1, Qt::SolidLine);
			painter->setPen(pen);
			painter->drawLine(option.rect.x() + option.rect.width() - 1, option.rect.y(), option.rect.x() + option.rect.width() - 1, option.rect.y() + option.rect.height() - 1);

			painter->restore();
			pTableWidget->bchanged_ = false;
			//QStyledItemDelegate::paint(painter, option, index);

		}

		// 创建可编辑列
		QWidget* MoPhotoDelegate::createEditor(QWidget* parent, const QStyleOptionViewItem& option, const QModelIndex& index) const
		{
			int row = index.row();
			int col = index.column();

			// getMode():  1:为MeasurementView; 0:为GcpListView
			if (pTableWidget && pTableWidget->getMode() == 1)
				return nullptr;

			if (col == PGSERINO_COL || col == PGNAME_COL || col == PGPHOTOCOUNT_COL) // col 2 is name,not need to edit.
				return nullptr;

			if ( col == PGSENSORSIZE_COL || col == PGFOCALLENGTH_COL)
			{
				// 5/6/7列为可编辑列.
				QLineEdit* leName = new QLineEdit(parent);
				//QDoubleValidator* doubleValidator2 = new QDoubleValidator();
				//doubleValidator2->setRange(0.000001, 1000);                 // 设置验证器范围只能是 0 ~ 999
				//leName->setValidator(doubleValidator2);   // 为编辑框设置验证器

				leName->setStyleSheet("selection-background-color:#3572B8;selection-color:white;color:white;");
				return leName;
			}
			
			else
				return nullptr; //QStyledItemDelegate::createEditor(parent, option, index);
		}

		// MoTableWidget的Model对象数据发生变化,更新Model对应Item进行显示.
		void MoPhotoDelegate::setEditorData(QWidget* editor, const QModelIndex& index) const
		{
			//QStyledItemDelegate::setEditorData(editor, index);
			if (!editor)
				return;
		
			bool bOk = true;
			QString rawText = index.model()->data(index, Qt::EditRole).toString();
			double value = index.model()->data(index, Qt::EditRole).toDouble(&bOk);
			QString text;// = index.model()->data(index, Qt::EditRole).toString();
			if ((index.column() == PGFOCALLENGTH_COL) || (index.column() == PGSENSORSIZE_COL))
			{
				/*if (index.column() == PGSENSORSIZE_COL && rawText.contains(UNDEFINEDSTR, Qt::CaseInsensitive))
				{
					text = UNDEFINEDSTR;
				}
				else */
				if (rawText.contains(UNDEFINEDSTR,Qt::CaseInsensitive) || !bOk || value <= 0)
				{
					text = UNDEFINEDSTR;
				}
				else
				{
					text = index.model()->data(index, Qt::EditRole).toString();
				}
			}
			

			//if (index.column() == 4)
			//{
			//	// 若为第4列,更改第4列下拉选项.
			//	QComboBox* spin = static_cast<QComboBox*>(editor);
			//	spin->setCurrentText(value);
			//}
			//else
			{
				// 对于其他可编辑的项,更新对应item的组件显示.
				QLineEdit* le = static_cast<QLineEdit*>(editor);
				le->setText(text);
			}

		}

		// 可编辑item对应的控件修改后,更新item对应的Model内部数据.
		void MoPhotoDelegate::setModelData(QWidget* editor, QAbstractItemModel* model, const QModelIndex& index) const
		{
			if (!editor)
				return;


			//if (index.column() == 4)
			//{
			//	// 第4列选项更改,更新对应item的Model数据.
			//	QComboBox* comboBox = static_cast<QComboBox*>(editor);
			//	QString value = comboBox->currentText();
			//	model->setData(index, value, Qt::EditRole);
			//	// todo:emit signal.
			//	//emit CurrentIndexChangeSignal(index.row(), index.column(), value);
			//	emit itemModified(index.row(), index.column(), value);
			//}
			//else
			{
				// 其他列若为可编辑的控件有界面数据改变时,更新对应item的Model数据.
				QLineEdit* lineedit = static_cast<QLineEdit*>(editor);

				bool bOk = true;
				QString value = lineedit->text();
				double dValue = value.toDouble(&bOk);
				if (!bOk)
					dValue = UNDEFINEDVAL;

				bool bOk2 = true;
				QString rawSaved = index.model()->data(index, Qt::UserRole + 1).toString();
				double dValue2 = rawSaved.toDouble(&bOk2);
				if (!bOk2)
					dValue2 = UNDEFINEDVAL;

				/*
				QByteArray baValue = value.toLatin1();
				double dValue = -1;

				QString rawSaved = index.model()->data(index, Qt::UserRole + 1).toString();

				if (!value.isEmpty() && baValue.size() > 0)
				{
					char tmpStr[1024];
					int len = baValue.size();
					if (len >= 1023)
						len = 1023;
					strcpy_s(tmpStr, baValue.size(), baValue.data());
					tmpStr[len] = '\0';
					char* endPtr = nullptr;

					dValue = strtod(tmpStr, &endPtr);
					if (endPtr)
						dValue = -1;
				}

				if ((rawSaved.isEmpty() || rawSaved.contains(UNDEFINEDSTR, Qt::CaseInsensitive))
					&& dValue <= 0
					)
				{
					return;
				}
				*/

				// todo:check value whether it is valid value.
				if (dValue <= 0)
				{				
					value = UNDEFINEDSTR;
				}			

				if (rawSaved == value)
					return;
				else if (!rawSaved.contains(UNDEFINEDSTR, Qt::CaseInsensitive) && !value.contains(UNDEFINEDSTR, Qt::CaseInsensitive) && dValue > 0 && dValue2 > 0)
				{
					if (fabs(dValue - dValue2) < 0.0001)
						return;
				}

				model->setData(index, value, Qt::EditRole);
				// todo: compare new value with original value.
				model->setData(index, value, Qt::UserRole + 1);

				emit itemModified(index.row(), index.column(), value);
				// todo:emit signal?
			}


		}

		// 可编辑Item重设对应组件的几何尺寸.//?chy
		void  MoPhotoDelegate::updateEditorGeometry(QWidget* editor, const QStyleOptionViewItem& option, const QModelIndex& index) const
		{
			//if (index.column() == 4)
			//{
			//	QRect r(option.rect);
			//	r.setLeft(r.left() + 3);
			//	r.setTop(r.top() + 3);
			//	r.setRight(r.right() - 3);
			//	r.setBottom(r.bottom() - 3);
			//	//r.setBottom(r.h)

			//	if (editor)
			//	{
			//		editor->setGeometry(r);
			//		QComboBox* cbType = static_cast<QComboBox*>(editor);
			//		cbType->showPopup();
			//	}
			//}
			//else
			{
				int col = index.column();

				if (col == PGSERINO_COL || col == PGNAME_COL || col == PGPHOTOCOUNT_COL)
					;
				else if (editor)
				{
					QRect r(option.rect);

					r.setLeft(r.left() + 3);
					r.setTop(r.top() + 3);
					r.setRight(r.right() - 3);
					r.setBottom(r.bottom() - 3);

					editor->setGeometry(r);
				}
			}
		}

		// 当光标移入当前item,记录对应的Hover行号.
		void MoPhotoDelegate::cellEntered(int row, int col)
		{

			iHoverRow = row;
		}

		void MoPhotoDelegate::cellEntered2(const QModelIndex& index)
		{

			iHoverRow = index.row();
		}

		QSize MoPhotoDelegate::sizeHint(const QStyleOptionViewItem& option, const QModelIndex& index) const
		{
			QSize size = QStyledItemDelegate::sizeHint(option, index);
			if (index.column() <= 1)
			{
				if (size.width() > 50)
					return QSize(50, size.height());
				else
					return size;
			}
			else
				return size;

		}

		// MoTableWidget表头定制组件.
		MoPhotoHeaderView::MoPhotoHeaderView(Qt::Orientation orientation, QWidget* parent,int mode)
			: QHeaderView(orientation, parent)
		{
			pTableWidget = qobject_cast<MoPhotoTableWidget*>(parent);
			setStyleSheet("padding-top:5px;padding-bottom:5px;");
			setSectionsClickable(true);
			mode_ = mode;
		}

		MoPhotoHeaderView::~MoPhotoHeaderView()
		{


		}

		// MoTableWidget定制表头实际绘制
		void MoPhotoHeaderView::paintSection(QPainter* painter, const QRect& rect, int logicalIndex) const
		{
			if (mode_ == 0)
			{
				if (logicalIndex == PGSENSORSIZE_COL || logicalIndex == PGFOCALLENGTH_COL)
				{


					QStringList headerLabels;

					headerLabels = pTableWidget->getheaderLabels();
					QString text = headerLabels.at(logicalIndex);
					auto text2Pixmap = [&](QString text)
					{
						//QPixmap MainWindow::text2Pixmap(QString text)

						QFont m_font;
						QFontMetrics fmt(m_font);
						QPixmap result(fmt.width(text), fmt.height());

						QRect rect(0, 0, fmt.width(text), fmt.height());
						result.fill(Qt::transparent);
						QPainter painter(&result);
						painter.setFont(m_font);
						painter.setPen(QColor(255, 143, 36));
						//painter.drawText(const QRectF(fmt.width(text), fmt.height()),Qt::AlignLeft, text);
						painter.drawText((const QRectF)(rect), text);
						return result;
					};
					QPixmap tetmap = text2Pixmap(text);
					//多张图片拼接合成一张图片
					auto pinjie = [&](QVector<QPixmap> image)

					{

						int image_width = 0;
						int max_height = 0;
						QVector <QPixmap > ::iterator it;
						for (it = image.begin(); it != image.end(); ++it)
						{
							int width = (*it).width();
							image_width += width;
							image_width += 5;
							if ((*it).height() > max_height)
							{
								max_height = (*it).height();
							}
						}
						QPixmap result_image_h(image_width, max_height);
						result_image_h.fill(Qt::transparent);
						QPainter painter_h;
						painter_h.begin(&result_image_h);
						int x_number = 0;
						for (it = image.begin(); it != image.end(); ++it)
						{
							painter_h.drawPixmap(x_number, 0, (*it));
							x_number += (*it).width();
							x_number += 5;
						}
						painter_h.end();
						return result_image_h;
					};
					// 表头第一列位置,绘制当前行数据的状态图(不同颜色的小圆圈)
					QString iconFile = ":/new/prefix1/skin/edit.png";;// "graynor.png";
					QPixmap pix,pinjieimg;

					if (!QPixmapCache::find(iconFile, &pix))
					{
						pix = QPixmap(iconFile).scaled(22, 22);
						QPixmapCache::insert(iconFile, pix);
					}
					QVector<QPixmap> imgmaps;
					imgmaps.append(pix);
					imgmaps.append(tetmap);
					pinjieimg = pinjie(imgmaps);
					//QPixmap iconPixmap = QPixmap(iconFile).scaled(22, 22);

					int rectW = rect.width();
					int rectH = rect.height();
					int iconXoff = 0;
					int iconYoff = 0;
					if (rectW > 22)
						iconXoff = (rectW - 22) / 5;

					if (rectH > 22)
						iconYoff = (rectH - 22) / 1.2;

					painter->fillRect(rect, QColor(0x33, 0x33, 0x33, 0xff));
					painter->drawPixmap(rect.x() + iconXoff, rect.y() + iconYoff, pinjieimg);

				}
				else
				{
					// 非第1列的表土,直接绘制对应列的已设置的表头标题文本.

					QStringList headerLabels;

					headerLabels = pTableWidget->getheaderLabels();

					if (logicalIndex != PGFOCALLENGTH_COL && logicalIndex != PGSENSORSIZE_COL)
					{
						QString txt = headerLabels.at(logicalIndex);

						painter->save();

						QPen pen(QColor("#A5A5A5"), 1, Qt::SolidLine);
						painter->setPen(pen);

						painter->fillRect(rect, QColor(0x33, 0x33, 0x33, 0xff));

						QRect r(rect);
						r.setLeft(r.x() + 11);
						r.setTop(r.y() + 5);

						QFont font;
						font.setFamily("Arial");
						font.setPixelSize(12);
						font.setBold(false);
						//font.setLine

						painter->setFont(font);

						painter->drawText(r, Qt::AlignVCenter | Qt::AlignLeft, txt);

						painter->restore();
					}
					else
						QHeaderView::paintSection(painter, rect, logicalIndex);
				}
			}
			else
			{
				QStringList headerLabels;

				headerLabels = pTableWidget->getheaderLabels();

				
				{
					QString txt = headerLabels.at(logicalIndex);

					painter->save();

					QPen pen(QColor("#A5A5A5"), 1, Qt::SolidLine);
					painter->setPen(pen);

					painter->fillRect(rect, QColor(0x33, 0x33, 0x33, 0xff));

					QRect r(rect);
					r.setLeft(r.x() + 11);
					r.setTop(r.y() + 5);

					QFont font;
					font.setFamily("Arial");
					font.setPixelSize(12);
					font.setBold(false);
					//font.setLine

					painter->setFont(font);

					painter->drawText(r, Qt::AlignVCenter | Qt::AlignLeft, txt);

					painter->restore();
				}
			
			}
			

			painter->save();

			QPen pen(QColor("#1D1D1D"), 1, Qt::SolidLine);
			painter->setPen(pen);
			painter->drawLine(rect.x() + rect.width() - 1, rect.y(), rect.x() + rect.width() - 1, rect.y() + rect.height() - 1);

			painter->restore();

		}

		QSize MoPhotoHeaderView::sizeHint() const
		{
			QSize size = QHeaderView::sizeHint();

			return QSize(size.width(), size.height() + 10);
		}

	

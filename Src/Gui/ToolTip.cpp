#include "Gui/ToolTip.h"
#include "Gui/GlobalStruct.h"
#include <iostream>
#include <QHelpEvent>
#include "Gui/ProjectInfoWgt.h"
#include <QAbstractItemView>
#include <QHeaderView>
#include <QTreeView>
#include <QTableView>
#include <QToolTip>
#include "Util/TaskProcess.h"
namespace AI3D
{
    namespace GUI
    {
        AToolTipper::AToolTipper(QObject* parent) :
            QObject(parent)
        {
        }

        bool AToolTipper::eventFilter(QObject* obj, QEvent* event)
        {
            if (event->type() == QEvent::ToolTip)
            {
                QAbstractItemView* view = qobject_cast<QAbstractItemView*>(obj->parent());
                if (!view)
                {
                    return false;
                }

                QHeaderView* headerView = qobject_cast<QHeaderView*>(view);
                if (headerView)
                {
                    return headerViewEventFilter(obj, event);
                }

                QHelpEvent* helpEvent = static_cast<QHelpEvent*>(event);
                QPoint pos = helpEvent->pos();
                QModelIndex index = view->indexAt(pos);
                if (!index.isValid())
                {
                    return false;
                }
                
                QString itemText = view->model()->data(index).toString();
                if (index.data(CustomRole::CRProjectWgt).value<QWidget*>())
                {
                   //chy 0908????????????????????itproject?????????????????????????????????
                   // itemText =view->model()->data(index, CustomRole::CRProjectData).value<ProjectInfoWgt* >()->GetProjectName();

                }
                else if (index.data(CustomRole::CRItemType).value<ItemType>())
                {
                    switch (index.data(CustomRole::CRItemType).value<ItemType>()) {
                    case ItemType::ITProject:
                    {
                       
                    }
                    break;

                    case ItemType::ITBlockAT:
                    {
                       
                    }
                    break;
                    case ItemType::ITBlock:
                    {
                        AI3D::CORE::BlockObject* block = view->model()->data(index, CustomRole::CRBlockData).value<AI3D::CORE::BlockObject*>();
                       
                        int num_images, num_gcps, num_tiepoints,num_usertiepoint;
                        if (block->GetCurrentAT() == nullptr)
                        {
                            return false;
                     }
                       {
                           num_images = (block->GetCurrentAT()->GetImages().size());
                           num_gcps = (block->GetCurrentAT()->GetControlPoints().size());
                           num_usertiepoint = block->GetCurrentAT()->GetUserPoints3D().size();
                       }
                        num_tiepoints = (block->GetTiepointStatus() ? block->GetCurrentAT()->GetPoints3D().size() : block->GetTaskInfo().statisticinfo_.tiepointnum);
                        std::string tooltip;
                        if (block->GetTaskInfo().mergedFrom.empty())
                        {
                            tooltip = block->GetTaskInfo().blockString;
                           
                        }
                        else
                        {
                            tooltip = block->GetTaskInfo().mergedFrom;
                        }
                        QString blockname = str2qstr(block->GetName());
                        if (AI3D::CORE::BlockObject::isChineseVersion())
                        {
                            tooltip += +"\n????Id:" + qstr2str(blockname) + "\n???????: " + std::to_string(num_images) + "\n???????: " + \
                                std::to_string(num_gcps) + "\n???????: " + std::to_string(num_tiepoints) + "\n???????: " + std::to_string(num_usertiepoint);

                        }
                        else
                        {
                            tooltip += +"\nBlock Id:" + qstr2str(blockname) + "\nphotos: " + std::to_string(num_images) + "\ncontrolpoints: " + \
                                std::to_string(num_gcps) + "\ntie points: " + std::to_string(num_tiepoints) + "\nuser tie points: " + std::to_string(num_usertiepoint);;
                        }
                        itemText = str2qstr(tooltip);
                    }
                    break;

                    default:
                        break;
                    }
                    
                    
                }
             
                // ?????
                QSize iconSize(0, 0);
                QIcon icon = view->model()->data(index, Qt::DecorationRole).value<QIcon>();
                if (!icon.isNull())
                {
                    QList<QSize> listSize = icon.availableSizes();
                    if (listSize.size() > 0)
                    {
                        iconSize.setWidth(listSize.at(0).width());
                        iconSize.setHeight(listSize.at(0).height());
                    }
                }

                // ??????????
                int headerHeight = 0;
                int headerWidth = 0;
                QTreeView* treeView = qobject_cast<QTreeView*>(view);
                if (treeView)
                {
                    headerHeight = treeView->header()->height();
                }
                else
                {
                    QTableView* tableView = qobject_cast<QTableView*>(view);
                    if (tableView)
                    {
                        headerHeight = tableView->horizontalHeader()->height();
                        headerWidth = tableView->verticalHeader()->width();
                    }
                }
             /*   std::cout << itemText.toStdString() << std::endl;*/
                
                // ??????
                const int textMargin = view->style()->pixelMetric(QStyle::PM_FocusFrameHMargin) + 1;
                const int iconMargin = iconSize.width() > 0 ? (view->style()->pixelMetric(QStyle::PM_FocusFrameHMargin) + 1) : 0;
                QRect rect = view->visualRect(index);
                QRect textRect = rect.adjusted(textMargin + iconSize.width() + iconMargin * 2, 0, -textMargin, 0); //?????????

                QFontMetrics fm(view->font());
                int flags = view->model()->data(index, Qt::TextAlignmentRole).toInt();
                QSize itemTextSize = fm.boundingRect(textRect, flags | Qt::TextLongestVariant | Qt::TextWordWrap, itemText).size();

                if ((itemTextSize.width() > textRect.width() || itemTextSize.height() > textRect.height()) && !itemText.isEmpty())
                {
                    // ???tip?????????
                    rect.adjust(headerWidth, headerHeight, headerWidth, headerHeight);
                    QToolTip::showText(helpEvent->globalPos(), itemText, view, rect);
                   
                    QPalette palette = QToolTip::palette();
                    palette.setColor(QPalette::Inactive, QPalette::ToolTipBase, Qt::white);   //????ToolTip?????
                    palette.setColor(QPalette::Inactive, QPalette::ToolTipText, QColor(102, 102, 102, 255)); 	//????ToolTip?????
                    QToolTip::setPalette(palette);
                    //QFont font("Segoe UI", -1, 50);
                    QFont font("Arial", -1, 50);
                    font.setPixelSize(14);
                    QToolTip::setFont(font);
                }
                else
                {
                    QToolTip::hideText();
                }
                return true;
            }

            return false;
        }

        bool AToolTipper::headerViewEventFilter(QObject* obj, QEvent* event)
        {
            if (event->type() == QEvent::ToolTip)
            {
                QHeaderView* headerView = qobject_cast<QHeaderView*>(obj->parent());
                if (!headerView)
                {
                    return false;
                }

                QHelpEvent* helpEvent = static_cast<QHelpEvent*>(event);
                QPoint pos = helpEvent->pos();
                int index = headerView->logicalIndexAt(pos);
                if (index < 0)
                {
                    return false;
                }

                QString itemText = headerView->model()->headerData(index, headerView->orientation()).toString();
                const int textMargin = headerView->style()->pixelMetric(QStyle::PM_FocusFrameHMargin) + 1;
                int rectWidth = headerView->sectionSize(index) - textMargin * 2;
                int rectHeight = headerView->sizeHint().height();

                QFontMetrics fm(headerView->font());
                int flag = headerView->model()->headerData(index, headerView->orientation(), Qt::TextAlignmentRole).toInt();
                QSize itemTextSize = fm.size(flag, itemText);
                if ((itemTextSize.width() > rectWidth || itemTextSize.height() > rectHeight) && !itemText.isEmpty())
                {
                    QToolTip::showText(helpEvent->globalPos(), itemText, headerView);
                }
                else
                {
                    QToolTip::hideText();
                }
                return true;
            }

            return false;
        }
    }
}
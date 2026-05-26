
/**
  * @file      Reconstration.h
  * @brief     提交AT界面类
  * @details
  * @author    李跃
  * @attention
  */




#ifndef _AI3D_GUI_POSSIGMA_H
#define _AI3D_GUI_POSSIGMA_H



#include <QDialog>

#include "ui_PosSigmaWgt.h"
#include <tuple>
#include "Core/ATData.h"
#include "Core/ATOptions.h"
#include "Core/ATDefinition.h"
using namespace AI3D::CORE;
namespace AI3D
{
	namespace GUI
	{
		class PosSigmaDia : public QDialog
		{
			Q_OBJECT

		public:
			PosSigmaDia(QDialog* parent = Q_NULLPTR);
			~PosSigmaDia();
			void paintEvent(QPaintEvent* p1);
			void setName(const QString& fileName);
			QString getName() { return ui->TaskName->text(); };
			void getSigmaParm() {};
			//-----以下为新加的policies相关的begin---//
			//初始化界面
			void InitEstimationPolicies(AI3D::CORE::ATDefinition& definition);
			
			//-----以下为新加的policies相关的end---//
			void slot_RadioButton();
			void slot_ControlRadioButton();
			void slot_getParam();
			std::tuple<double, double, double, int> getData();
			void SetATOptions(AI3D::CORE::ATOptions& options);
			AI3D::CORE::ATOptions GetATOptions()const;
			AI3D::CORE::ATOptions& GetATOptionsMutual();
		/*	void setPosModeWidgetEnable(bool bIsEnable);*/
			//控制GCP Marked Result
			void setGCPDataEnable(bool bIsAble);
			/*void setGCPRadioEnable(bool bIsEnable);
			void setGPSRadioEnable(bool bIsEnable);*/
			void setPosModeRadioChecked(alignmode_e mode);
			void PosSigmaDia::print();
			void setGCPResult(int nGCPTotal, int nMarkGCP, int nGCPMarkedPhoto);
			//设置GCP按钮是否高亮
			void setPosModeStatus(bool bIsSelect);
			void setGcpModeStatus(bool bIsSelect);
			//控制ATSettings是否高亮
			void setATSettingWidgetEnable(bool bIsEnable,bool gcpmode=false);
			void SetPOSaccuracyEnable(bool bIsEnable);
			void SetKeyDensityVisibale(bool bIsVis);
			void SetPairSelectionModeEnable(bool bIsEnable);
			void SetATSettingLabelEnable(bool bIsEnable);
			void SetWidgetStatusByRadioCheked(alignmode_e mode);//根据checked状态选择

		private:
			void Initpolicy(QComboBox* cmb, AI3D::CORE::ATDefinition& definition,const policies_object_e& object);
			/*void InitTiepointpolicy(AI3D::CORE::ATDefinition& definition);
			void InitPosepolicy(AI3D::CORE::ATDefinition& definition);
			void InitFpolicy(AI3D::CORE::ATDefinition& definition);
			void InitPPApolicy(AI3D::CORE::ATDefinition& definition);
			void InitRdispolicy(AI3D::CORE::ATDefinition& definition);
			void InitTdispolicy(AI3D::CORE::ATDefinition& definition);*/
			
		private:
			Ui_PosSigma *ui;
			double sigmaX;
			double sigmaY;
			double sigmaZ;
			int featureNum;
			int nGCPTotalNum;
			int nMarkedGCPNum;
			int nGCPMarkedPhotoNum;
			AI3D::CORE::ATOptions at_options_;
			
		};
	}
}
#endif // !1
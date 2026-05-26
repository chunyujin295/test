#ifndef _AI3D_GUI_OPTIONS_WIDGET_H_
#define _AI3D_GUI_OPTIONS_WIDGET_H_

#include <QtCore>
#include <QtWidgets>
#pragma execution_character_set("utf-8")		
namespace AI3D
{
    namespace GUI
    {

        class OptionsWidget : public QWidget {
        public:
            explicit OptionsWidget(QWidget* parent);

        protected:
            void showEvent(QShowEvent* event);
            void closeEvent(QCloseEvent* event);
            void hideEvent(QHideEvent* event);

            void AddOptionRow(const std::string& label_text, QWidget* widget);

            QSpinBox* AddOptionInt(int* option, const std::string& label_text,
                const int min = 0,
                const int max = static_cast<int>(1e7));
            QDoubleSpinBox* AddOptionDouble(double* option, const std::string& label_text,
                const double min = 0, const double max = 1e7,
                const double step = 0.01,
                const int decimals = 2);
            QDoubleSpinBox* AddOptionDoubleLog(
                double* option, const std::string& label_text, const double min = 0,
                const double max = 1e7, const double step = 0.01, const int decimals = 2);
            QCheckBox* AddOptionBool(bool* option, const std::string& label_text);
            QLineEdit* AddOptionText(std::string* option, const std::string& label_text);
            QLineEdit* AddOptionFilePath(std::string* option,
                const std::string& label_text);
            QLineEdit* AddOptionDirPath(std::string* option,
                const std::string& label_text);
            void AddSpacer();
            void AddSection(const std::string& label_text);

            void ReadOptions();
            void WriteOptions();

            QGridLayout* grid_layout_;

            std::vector<std::pair<QSpinBox*, int*>> options_int_;
            std::vector<std::pair<QDoubleSpinBox*, double*>> options_double_;
            std::vector<std::pair<QDoubleSpinBox*, double*>> options_double_log_;
            std::vector<std::pair<QCheckBox*, bool*>> options_bool_;
            std::vector<std::pair<QLineEdit*, std::string*>> options_text_;
            std::vector<std::pair<QLineEdit*, std::string*>> options_path_;
        };

    } 
}
#endif  

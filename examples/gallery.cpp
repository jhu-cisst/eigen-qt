// SPDX-License-Identifier: MIT
#include <eigenqt/VectorWidget.h>
#include <eigenqt/MatrixWidget.h>
#include <QApplication>
#include <QTabWidget>
#include <QVBoxLayout>
#include <array>
#include <eigenqt/FrameWidget.h>
#include <eigenqt/WrenchWidget.h>
#include <eigenqt/PositionsWidget.h>
#include <eigenqt/Plot2DWidget.h>

int main(int argc, char **argv)
{
    QApplication app(argc, argv);
    QTabWidget tabs;
    tabs.setWindowTitle("EigenQt");
    auto *editors = new QWidget;
    auto *layout = new QVBoxLayout(editors);
    for (const auto mode : {EigenQt::VectorEditor::Text, EigenQt::VectorEditor::SpinBox,
                            EigenQt::VectorEditor::Slider}) {
        auto *vector = new EigenQt::VectorEditor(mode);
        vector->setRange(-10.0, 10.0);
        vector->setValue(std::array<double, 3>{1, 2, 3});
        layout->addWidget(vector);
    }
    auto *bits = new EigenQt::VectorEditor;
    bits->setValue(std::vector<bool>{true, false, true});
    layout->addWidget(bits);
    auto *matrix = new EigenQt::MatrixEditor;
    matrix->setValues(3, 3, [](size_t r, size_t c) { return r == c ? 1.0 : 0.0; });
    layout->addWidget(matrix);
    tabs.addTab(editors, "Editors");
    auto *frame = new EigenQt::FrameWidget;
    frame->setValue(Eigen::Isometry3d::Identity());
    tabs.addTab(frame, "Frame / rotation modes");
    auto *wrench = new EigenQt::WrenchWidget;
    wrench->setValue(Eigen::Vector3d(1, 2, 3), Eigen::Vector3d(.1, .2, .3));
    tabs.addTab(wrench, "Force / torque modes");
    auto *positions = new EigenQt::PositionsWidget;
    for (int i = 0; i < 100; ++i) {
        positions->setValue(
            Eigen::Vector3d(i * .001, std::sin(i * .1) * .01, std::cos(i * .1) * .01));
    }
    tabs.addTab(positions, "Positions");
    auto *plot = new EigenQt::Plot2DWidget;
    auto *signal = plot->addScale("samples")->addSignal("sin");
    for (int i = 0; i < 100; ++i) {
        signal->appendPoint(Eigen::Vector2d(i * .01, std::sin(i * .1)));
    }
    tabs.addTab(plot, "Streaming plot");
    tabs.resize(900, 600);
    tabs.show();
    return app.exec();
}

// SPDX-License-Identifier: MIT
#include <EigenQt/VectorEditor>
#include <QApplication>
#include <Eigen/Core>
#include <EigenQt/FrameWidget>

int main(int argc, char **argv)
{
    QApplication app(argc, argv);
    EigenQt::VectorEditor editor;
    if (!editor.setValue(Eigen::Vector2d(1, 2))) {
        return 1;
    }
    Eigen::Vector2d output = Eigen::Vector2d::Zero();
    if (!editor.readValues(output) || output[1] != 2) {
        return 1;
    }
    EigenQt::FrameWidget frame;
    frame.setValue(Eigen::Isometry3d::Identity());
    return 0;
}

// SPDX-License-Identifier: MIT
#include <EigenQt/PositionsWidget>
#include <EigenQt/RotationView>
#include <eigenqt/RotationWidget.h>
#include <eigenqt/FrameWidget.h>
#include <eigenqt/MatrixWidget.h>
#include <eigenqt/VectorWidget.h>
#include <eigenqt/WrenchWidget.h>
#include <eigenqt/PositionsWidget.h>
#include <eigenqt/Plot2DWidget.h>
#include <unsupported/Eigen/EulerAngles>
#include <QComboBox>
#include <QMouseEvent>
#include <QtTest>
#include <array>
#include <memory>
#include <new>

template <class Base> class InteractiveView : public Base
{
public:
    using Base::mCurrentOrientation;
    using Base::mDeltaOrientation;
};

class Vector3D : public EigenQt::Vector3DWidget
{
public:
    using EigenQt::Vector3DWidget::mVectorNorm;
};

template <class Base> void checkRoundTripDrag()
{
    InteractiveView<Base> view;
    const auto initial = view.mCurrentOrientation;
    const auto mouseEvent = [&view](QEvent::Type type, QPointF position, Qt::MouseButton button,
                                    Qt::MouseButtons buttons) {
        QMouseEvent event(type, position, position, position, button, buttons, Qt::NoModifier);
        QApplication::sendEvent(&view, &event);
    };
    mouseEvent(QEvent::MouseButtonPress, QPointF(20, 20), Qt::LeftButton, Qt::LeftButton);
    mouseEvent(QEvent::MouseMove, QPointF(100, 20), Qt::NoButton, Qt::LeftButton);
    QVERIFY(!view.mDeltaOrientation.isApprox(Eigen::Quaterniond::Identity()));
    mouseEvent(QEvent::MouseMove, QPointF(20, 20), Qt::NoButton, Qt::LeftButton);
    QVERIFY(view.mDeltaOrientation.isApprox(Eigen::Quaterniond::Identity()));
    mouseEvent(QEvent::MouseButtonRelease, QPointF(20, 20), Qt::LeftButton, Qt::NoButton);
    QVERIFY(view.mCurrentOrientation.isApprox(initial));
}

class Rotation : public EigenQt::RotationWidget
{
public:
    using EigenQt::RotationWidget::AxisAngleWidget;
    using EigenQt::RotationWidget::MatrixWidget;
    using EigenQt::RotationWidget::OpenGLWidget;
    using EigenQt::RotationWidget::QuaternionWidget;
};
class Frame : public EigenQt::FrameWidget
{
public:
    using EigenQt::FrameWidget::TranslationWidget;
};
class Positions : public EigenQt::PositionsWidget
{
public:
    using EigenQt::PositionsWidget::mBB;
    using EigenQt::PositionsWidget::mPoses;
    using EigenQt::PositionsWidget::mTable;
    using EigenQt::PositionsWidget::mViews;
};
class Wrench : public EigenQt::WrenchWidget
{
public:
    using EigenQt::WrenchWidget::ForceTorqueWidget;
    using EigenQt::WrenchWidget::Plot2DWidget;
    using EigenQt::WrenchWidget::Plot3DWidget;
    using EigenQt::WrenchWidget::WrenchWidget;
};
class GeometryTest : public QObject
{
    Q_OBJECT
private slots:
    void initialVectorNorm()
    {
        // Nonzero storage makes missing initialization visible on every run.
        alignas(Vector3D) std::array<unsigned char, sizeof(Vector3D)> storage;
        storage.fill(0xff);
        std::unique_ptr<Vector3D, void (*)(Vector3D *)> vector(
            new (storage.data()) Vector3D, [](Vector3D *widget) { widget->~Vector3D(); });
        QCOMPARE(vector->mVectorNorm, 0.0);
        vector->setValue(Eigen::Vector3d(3, 4, 0));
        QCOMPARE(vector->mVectorNorm, 5.0);
    }
    void vectorDragReturnsToStart() { checkRoundTripDrag<EigenQt::Vector3DWidget>(); }
    void rotationDragReturnsToStart() { checkRoundTripDrag<EigenQt::RotationView>(); }
    void enumMetadata()
    {
        Frame frame;
        QSignalSpy changes(&frame, &EigenQt::FrameWidget::displayModeChanged);
        auto queuedMode = EigenQt::RotationWidget::Undefined;
        QObject receiver;
        connect(
            &frame, &EigenQt::FrameWidget::displayModeChanged, &receiver,
            [&](EigenQt::RotationWidget::DisplayMode mode) { queuedMode = mode; },
            Qt::QueuedConnection);
        QVERIFY(frame.setProperty("displayMode", int(EigenQt::RotationWidget::Quaternion)));
        QCOMPARE(frame.property("displayMode").toInt(), int(EigenQt::RotationWidget::Quaternion));
        QCOMPARE(changes.count(), 1);
        QCOMPARE(changes.first().first().value<EigenQt::RotationWidget::DisplayMode>(),
                 EigenQt::RotationWidget::Quaternion);
        QTRY_COMPARE(queuedMode, EigenQt::RotationWidget::Quaternion);
        EigenQt::WrenchWidget wrench;
        QSignalSpy wrenchChanges(&wrench, &EigenQt::WrenchWidget::displayModeChanged);
        QVERIFY(wrench.setProperty("displayMode", int(EigenQt::WrenchWidget::Text)));
        QCOMPARE(wrenchChanges.count(), 1);
        QCOMPARE(wrenchChanges.first().first().value<EigenQt::WrenchWidget::DisplayMode>(),
                 EigenQt::WrenchWidget::Text);
    }
    void switchingWrenchModeDisplaysCachedValue()
    {
        Wrench wrench(EigenQt::WrenchWidget::Text);
        wrench.setValue(Eigen::Vector3d(3, 4, 0), Eigen::Vector3d(0, 0, 1), 12.0);
        wrench.setDisplayMode(EigenQt::WrenchWidget::Plot3D);
        auto *vector = wrench.Plot3DWidget->findChild<EigenQt::Vector3DWidget *>();
        QVERIFY(vector);
        QCOMPARE(vector->maximumNorm(), 5.0);
        wrench.setDisplayMode(EigenQt::WrenchWidget::Plot2D);
        auto *plot = wrench.Plot2DWidget->findChild<EigenQt::Plot2DWidget *>();
        QVERIFY(plot);
        auto *scale = plot->findScale("F");
        QVERIFY(scale);
        auto *signal = scale->signalMap().at("Axis X").get();
        QCOMPARE(signal->pointCount(), size_t(1));
        QCOMPARE(signal->pointAt(0).x(), 12.0);
        QCOMPARE(signal->pointAt(0).y(), 3.0);
        wrench.setDisplayMode(EigenQt::WrenchWidget::Text);
        wrench.setValue(Eigen::Vector3d(6, 8, 0), Eigen::Vector3d::Zero(), 13.0);
        wrench.setDisplayMode(EigenQt::WrenchWidget::Plot3D);
        QCOMPARE(vector->maximumNorm(), 10.0);
        auto *combo3D = wrench.Plot3DWidget->findChild<QComboBox *>();
        QVERIFY(combo3D);
        wrench.setValue(Eigen::Vector3d(6, 8, 0), Eigen::Vector3d(0, 3, 4), 14.0);
        combo3D->setCurrentIndex(1);
        QCOMPARE(vector->maximumNorm(), 5.0);
        wrench.setDisplayMode(EigenQt::WrenchWidget::Plot2D);
        QCOMPARE(signal->pointCount(), size_t(2));
        QCOMPARE(signal->pointAt(1).x(), 14.0);
        QCOMPARE(signal->pointAt(1).y(), 6.0);
        auto *torqueScale = plot->findScale("T");
        QVERIFY(torqueScale);
        auto *torqueZ = torqueScale->signalMap().at("Axis Z").get();
        QCOMPARE(torqueZ->pointCount(), size_t(2));
        QCOMPARE(torqueZ->pointAt(0).y(), 1.0);
        QCOMPARE(torqueZ->pointAt(1).y(), 4.0);
    }
    void eigenAdapters()
    {
        EigenQt::VectorEditor vector;
        Eigen::VectorXd input(3);
        input << 1, 2, 3;
        QVERIFY(vector.setValue(input));
        Eigen::VectorXd output(3);
        QVERIFY(vector.readValues(output));
        QVERIFY(input.isApprox(output));
        EigenQt::MatrixEditor matrix;
        Eigen::Matrix<double, 2, 3, Eigen::RowMajor> rowMajor;
        rowMajor << 1, 2, 3, 4, 5, 6;
        QVERIFY(matrix.setValue(rowMajor));
        Eigen::MatrixXd result(2, 3);
        QVERIFY(matrix.readValues(result));
        QVERIFY(result.isApprox(rowMajor));
        Eigen::MatrixXd padded = Eigen::MatrixXd::Zero(4, 5);
        auto block = padded.block(1, 1, 2, 3);
        QVERIFY(matrix.readValues(block));
        QVERIFY(block.isApprox(rowMajor));
        EigenQt::VectorEditor bits;
        Eigen::VectorX<bool> flags(3);
        flags << true, false, true;
        QVERIFY(bits.setValue(flags));
        flags.setConstant(false);
        QVERIFY(bits.readValues(flags));
        QVERIFY(flags[0] && !flags[1] && flags[2]);
    }
    void representations()
    {
        Rotation rotation;
        QCOMPARE(rotation.AxisAngleWidget->parentWidget(), &rotation);
        QVERIFY(rotation.AxisAngleWidget->isHidden());
        QVERIFY(rotation.QuaternionWidget->isHidden());
        QVERIFY(rotation.OpenGLWidget->isHidden());
        QSignalSpy modes(&rotation, &EigenQt::RotationWidget::displayModeChanged);
        rotation.setValue(Eigen::Matrix3d::Identity());
        QCOMPARE(rotation.MatrixWidget->item(0, 0)->text().toDouble(), 1.0);
        rotation.setDisplayMode(EigenQt::RotationWidget::Quaternion);
        QCOMPARE(modes.count(), 1);
        QCOMPARE(rotation.property("displayMode").toInt(),
                 int(EigenQt::RotationWidget::Quaternion));
        rotation.setDisplayMode(EigenQt::RotationWidget::Quaternion);
        QCOMPARE(modes.count(), 1);
        QCOMPARE(rotation.QuaternionWidget->item(0, 3)->text().toDouble(), 1.0);
        for (const auto mode :
             {EigenQt::RotationWidget::AxisAngle, EigenQt::RotationWidget::EulerZYZ,
              EigenQt::RotationWidget::EulerZYX}) {
            rotation.setDisplayMode(mode);
            rotation.setValue(Eigen::AngleAxisd(0.2, Eigen::Vector3d::UnitZ()));
        }
        const Eigen::AngleAxisd expectedAngleAxis(0.4, Eigen::Vector3d::UnitY());
        rotation.setValue(Eigen::Quaterniond(expectedAngleAxis));
        QVERIFY(rotation.value().isApprox(expectedAngleAxis.toRotationMatrix()));
        rotation.setValue(Eigen::Quaternionf(expectedAngleAxis.cast<float>()));
        QVERIFY(rotation.value().isApprox(expectedAngleAxis.toRotationMatrix(), 1e-5));
        const Eigen::Matrix3f floatRot = expectedAngleAxis.toRotationMatrix().cast<float>();
        rotation.setValue(floatRot);
        QVERIFY(rotation.value().isApprox(expectedAngleAxis.toRotationMatrix(), 1e-5));
        using OrderZYX = Eigen::EulerSystem<Eigen::EULER_Z, Eigen::EULER_Y, Eigen::EULER_X>;
        const Eigen::EulerAngles<double, OrderZYX> eulerRot(0.3, -0.2, 0.1);
        rotation.setValue(eulerRot);
        QVERIFY(rotation.value().isApprox(eulerRot.toRotationMatrix()));
        QVERIFY_EXCEPTION_THROWN(rotation.setValue(Eigen::MatrixXd::Identity(2, 2)),
                                 std::invalid_argument);
        EigenQt::RotationView view3d;
        view3d.setValue(Eigen::Quaterniond(expectedAngleAxis));
        view3d.setValue(expectedAngleAxis);
        view3d.setValue(floatRot);

        Frame frame;
        QCOMPARE(frame.TranslationWidget->columnCount(), 3);
        QCOMPARE(frame.TranslationWidget->item(0, 0)->text().toDouble(), 0.0);
        QSignalSpy frameModes(&frame, &EigenQt::FrameWidget::displayModeChanged);
        Eigen::Isometry3d pose = Eigen::Isometry3d::Identity();
        pose.translation().x() = 0.1;
        frame.setValue(pose);
        QVERIFY(frame.value().isApprox(pose));
        QCOMPARE(frame.TranslationWidget->item(0, 0)->text().toDouble(), 100.0);
        frame.setDisplayMode(EigenQt::RotationWidget::Quaternion);
        QCOMPARE(frameModes.count(), 1);
        QCOMPARE(frame.property("displayMode").toInt(), int(EigenQt::RotationWidget::Quaternion));
        frame.setPrismaticRevoluteFactors(1.0, 1.0);
        QCOMPARE(frame.TranslationWidget->item(0, 0)->text().toDouble(), 0.1);
        Eigen::Affine3f affine = Eigen::Affine3f::Identity();
        affine.linear() = floatRot;
        affine.translation() = Eigen::Vector3f(1.0f, 2.0f, 3.0f);
        frame.setValue(affine);
        QVERIFY(frame.value().linear().isApprox(expectedAngleAxis.toRotationMatrix(), 1e-5));
        QVERIFY(frame.value().translation().isApprox(Eigen::Vector3d(1.0, 2.0, 3.0), 1e-5));
        frame.setValue(Eigen::Quaterniond(expectedAngleAxis), Eigen::Vector3f(4.0f, 5.0f, 6.0f));
        QVERIFY(frame.value().linear().isApprox(expectedAngleAxis.toRotationMatrix()));
        QVERIFY(frame.value().translation().isApprox(Eigen::Vector3d(4.0, 5.0, 6.0)));
        Eigen::Matrix4d homogeneous = Eigen::Matrix4d::Identity();
        homogeneous.topLeftCorner<3, 3>() = expectedAngleAxis.toRotationMatrix();
        homogeneous.topRightCorner<3, 1>() = Eigen::Vector3d(7.0, 8.0, 9.0);
        frame.setValue(homogeneous);
        QVERIFY(frame.value().linear().isApprox(expectedAngleAxis.toRotationMatrix()));
        QVERIFY(frame.value().translation().isApprox(Eigen::Vector3d(7.0, 8.0, 9.0)));
        QVERIFY_EXCEPTION_THROWN(frame.setValue(Eigen::MatrixXd::Identity(3, 3)),
                                 std::invalid_argument);
        QVERIFY_EXCEPTION_THROWN(frame.setValue(expectedAngleAxis, Eigen::VectorXd::Zero(2)),
                                 std::invalid_argument);
        Wrench wrench(EigenQt::WrenchWidget::Text);
        QCOMPARE(wrench.ForceTorqueWidget->parentWidget(), &wrench);
        QVERIFY(wrench.Plot2DWidget->isHidden());
        QVERIFY(wrench.Plot3DWidget->isHidden());
        wrench.setValue(Eigen::Vector3d(1, 2, 3), Eigen::Vector3d(4, 5, 6));
        QVERIFY(wrench.Plot2DWidget->close());
        QVERIFY(wrench.Plot3DWidget->close());
        Positions positions;
        QVERIFY(positions.mBB.isEmpty());
        positions.setValue(pose);
        QCOMPARE(positions.mPoses.size(), size_t(1));
        QVERIFY(!positions.mBB.isEmpty());
        QCOMPARE(positions.mTable->item(0, 0)->text().toDouble(), 100.0);
        positions.setPrismaticRevoluteFactors(1.0, 1.0);
        QCOMPARE(positions.mTable->item(0, 0)->text().toDouble(), 0.1);
        positions.mViews.front()->resetSize();
        QVERIFY(!positions.mBB.isEmpty());
        positions.clear();
        QVERIFY(positions.mPoses.empty());
        QVERIFY(positions.mBB.isEmpty());
    }
};
QTEST_MAIN(GeometryTest)
#include "GeometryTest.moc"

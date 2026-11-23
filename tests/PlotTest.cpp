// SPDX-License-Identifier: MIT
#include <eigenqt/Plot2D.h>
#include <eigenqt/Plot2DWidget.h>
#include <QtTest>
#include <array>

class PlotModel : public EigenQt::Plot2D
{
public:
    void renderInitialize() override {}
    void renderResize(double, double) override {}
    void render() override {}
};
class PlotTest : public QObject
{
    Q_OBJECT
private slots:
    void standaloneSignal()
    {
        EigenQt::Plot2D::Signal signal("standalone", 3);
        QVERIFY(signal.parentScale() == nullptr);
        QVERIFY(signal.addSignal("sibling") == nullptr);
        signal.appendPoint(Eigen::Vector2d(1.0, 2.0));
        QCOMPARE(signal.pointCount(), size_t(1));
        QCOMPARE(signal.pointAt(0).y(), 2.0);
    }
    void removedLabelScale()
    {
        EigenQt::Plot2DWidget plot;
        EigenQt::Plot2D &model = plot;
        auto *scale = plot.addScale("selected");
        plot.setDisplayYRangeScale(scale);
        QCOMPARE(plot.displayYRangeScale(), scale);
        auto *other = plot.addScale("other");
        QVERIFY(model.removeScale(other));
        QCOMPARE(plot.displayYRangeScale(), scale);
        QVERIFY(model.removeScale(scale));
        QVERIFY(!plot.displayYRangeScale());
        scale = plot.addScale("by-name");
        plot.setDisplayYRangeScale(scale);
        QVERIFY(model.removeScale("by-name"));
        QVERIFY(!plot.displayYRangeScale());
        scale = plot.addScale("erased");
        plot.setDisplayYRangeScale(scale);
        plot.scales().clear();
        QVERIFY(!plot.displayYRangeScale());
        plot.setDisplayYRangeScale(nullptr);
        QVERIFY(!plot.displayYRangeScale());
    }
    void configuredCapacity()
    {
        PlotModel plot;
        auto *existingScale = plot.addScale("existing");
        QCOMPARE(existingScale->addSignal("default")->capacity(), size_t(200));
        plot.setNumberOfPoints(1000);
        auto *signal = existingScale->addSignal("later");
        QCOMPARE(signal->capacity(), size_t(1000));
        QCOMPARE(plot.addScale("new")->addSignal("x")->capacity(), size_t(1000));
        QCOMPARE(signal->addSignal("sibling")->capacity(), size_t(1000));
        for (int i = 0; i < 1001; ++i) {
            signal->appendPoint(Eigen::Vector2d(i, i));
        }
        QCOMPARE(signal->pointCount(), size_t(1000));
        QCOMPARE(signal->pointAt(0).x(), 1.0);
        plot.setNumberOfPoints(1);
        QCOMPARE(signal->pointCount(), size_t(3));
        QCOMPARE(existingScale->addSignal("minimum")->capacity(), size_t(3));
        QCOMPARE(plot.addScale("minimum")->addSignal("x")->capacity(), size_t(3));
    }
    void circularBuffer()
    {
        PlotModel plot;
        auto *scale = plot.addScale("positions");
        QVERIFY(scale);
        auto *signal = scale->addSignal("x");
        QCOMPARE(signal->parentScale(), scale);
        QCOMPARE(signal->pointCount(), size_t(0));
        QVERIFY_EXCEPTION_THROWN(signal->pointAt(0), std::runtime_error);
        QVERIFY_EXCEPTION_THROWN(signal->setPointAt(0, Eigen::Vector2d(0, 0)), std::runtime_error);
        signal->setSize(3);
        signal->appendPoint(Eigen::Vector2d(1, 2));
        QVERIFY_EXCEPTION_THROWN(signal->pointAt(1), std::runtime_error);
        signal->setSize(3);
        const std::array<std::array<double, 2>, 4> points{{{1, 2}, {2, 4}, {3, 6}, {4, 8}}};
        signal->appendPoints(points.begin(), points.end());
        QCOMPARE(signal->pointCount(), size_t(3));
        QCOMPARE(signal->pointAt(0).x(), 2.0);
        signal->resize(5);
        QCOMPARE(signal->pointCount(), size_t(3));
        QCOMPARE(signal->pointAt(0).x(), 2.0);
        QCOMPARE(signal->pointAt(2).y(), 8.0);
        signal->resize(2);
        QCOMPARE(signal->pointAt(0).x(), 3.0);
        signal->resize(1, false);
        QCOMPARE(signal->pointAt(0).x(), 3.0);
        QVERIFY_EXCEPTION_THROWN(signal->pointAt(1), std::runtime_error);
        signal->setSize(0);
        signal->appendPoint(Eigen::Vector2d(1, 2));
        QCOMPARE(signal->pointCount(), size_t(0));
    }
    void rangesFreezeAndVisibility()
    {
        PlotModel plot;
        auto *scale = plot.addScale("scale");
        auto *signal = scale->addSignal("signal");
        signal->appendPoint(Eigen::Vector2d(1, -4));
        signal->appendPoint(Eigen::Vector2d(2, 8));
        double low, high;
        signal->computeDataRangeY(low, high);
        QCOMPARE(low, -4.0);
        QCOMPARE(high, 8.0);
        plot.setFrozen(true);
        signal->appendPoint(Eigen::Vector2d(3, 100));
        QCOMPARE(signal->pointCount(), size_t(2));
        QVERIFY(plot.isFrozen());
        plot.setFrozen(false);
        QCOMPARE(signal->pointCount(), size_t(0));
        signal->setVisible(false);
        QVERIFY(!signal->isVisible());
        QVERIFY(!scale->computeDataRangeY(low, high));
        QCOMPARE(low, -1.0);
        QCOMPARE(high, 1.0);
        signal->setVisible(true);
        signal->appendPoint(Eigen::Vector2d(1, -10));
        signal->appendPoint(Eigen::Vector2d(2, -4));
        scale->autoExpandY(0.0);
        QCOMPARE(scale->viewingRangeY().x(), -10.0);
        QCOMPARE(scale->viewingRangeY().y(), -4.0);
        signal->setSize(2);
        signal->appendPoint(Eigen::Vector2d(3, -6));
        signal->appendPoint(Eigen::Vector2d(4, -2));
        scale->autoExpandY(0.0);
        QCOMPARE(scale->viewingRangeY().x(), -10.0);
        QCOMPARE(scale->viewingRangeY().y(), -2.0);
        QVERIFY(scale->addVerticalLine("reference"));
        QVERIFY(scale->removeSignal(signal));
        QVERIFY(plot.removeScale(scale));
        QVERIFY(!plot.findScale("scale"));
    }
};
QTEST_MAIN(PlotTest)
#include "PlotTest.moc"

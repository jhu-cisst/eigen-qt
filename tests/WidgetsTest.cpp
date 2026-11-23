// SPDX-License-Identifier: MIT
#include <EigenQt/Matrix>
#include <eigenqt/VectorWidget.h>
#include <eigenqt/MatrixWidget.h>
#include <QtTest>
#include <QCheckBox>
#include <QDoubleSpinBox>
#include <QAbstractSpinBox>
#include <QLineEdit>
#include <QSlider>
#include <QVector>
#include <array>
#include <list>
#include <forward_list>
#include <cstdint>
#include <sstream>

class WidgetsTest : public QObject
{
    Q_OBJECT
private slots:
    void integerSteppingUsesPendingText()
    {
        for (const bool isUnsigned : {false, true}) {
            EigenQt::VectorEditor editor(EigenQt::VectorEditor::SpinBox);
            editor.setRange(0, 10);
            if (isUnsigned) {
                QVERIFY(editor.setValue(std::array<unsigned, 1>{0}));
            } else {
                QVERIFY(editor.setValue(std::array<int, 1>{0}));
            }
            auto *box = qobject_cast<QAbstractSpinBox *>(editor.cellWidget(0, 0));
            auto *line = box->findChild<QLineEdit *>();
            QVERIFY(line);
            QSignalSpy changes(&editor, &EigenQt::VectorEditor::valueChanged);
            line->selectAll();
            QTest::keyClicks(line, "5");
            QTest::keyClick(box, Qt::Key_Down);
            std::array<int, 1> result{};
            QVERIFY(editor.readValues(result));
            QCOMPARE(result[0], 4);
            QCOMPARE(changes.count(), 1);
            if (isUnsigned) {
                QVERIFY(editor.setValue(std::array<unsigned, 1>{10}));
            } else {
                QVERIFY(editor.setValue(std::array<int, 1>{10}));
            }
            line->selectAll();
            QTest::keyClicks(line, "5");
            QTest::keyClick(box, Qt::Key_Up);
            QVERIFY(editor.readValues(result));
            QCOMPARE(result[0], 6);
            QCOMPARE(changes.count(), 2);
        }
    }
    void floatingSpinBoxKeepsRoundedValuesInRange()
    {
        EigenQt::VectorEditor editor(EigenQt::VectorEditor::SpinBox);
        editor.setRange(0.006, 0.016);
        QVERIFY(editor.setValue(std::array<double, 1>{0.016}));
        std::array<double, 1> result{};
        QVERIFY(editor.readValues(result));
        QCOMPARE(result[0], 0.01);
        auto *box = qobject_cast<QDoubleSpinBox *>(editor.cellWidget(0, 0));
        QCOMPARE(box->minimum(), 0.01);
        QCOMPARE(box->maximum(), 0.01);
        QSignalSpy changes(&editor, &EigenQt::VectorEditor::valueChanged);
        QVERIFY_EXCEPTION_THROWN(editor.setRange(0.004, 0.006), std::invalid_argument);
        QVERIFY(editor.readValues(result));
        QCOMPARE(result[0], 0.01);
        QCOMPARE(changes.count(), 0);
        QVERIFY_EXCEPTION_THROWN(editor.setPrecision(1), std::invalid_argument);
        QCOMPARE(editor.precision(), 2);
        QVERIFY(editor.readValues(result));
        QCOMPARE(result[0], 0.01);
        editor.setRange(-0.016, -0.006);
        QVERIFY(editor.setValue(std::array<double, 1>{-0.016}));
        QVERIFY(editor.readValues(result));
        QCOMPARE(result[0], -0.01);
        editor.setRange(-0.026, 0.026);
        QVERIFY(editor.setValue(std::array<double, 2>{-0.026, 0.026}));
        std::array<double, 2> ends{};
        QVERIFY(editor.readValues(ends));
        QCOMPARE(ends[0], -0.02);
        QCOMPARE(ends[1], 0.02);
        EigenQt::MatrixEditor matrix;
        matrix.setRange(0.006, 0.016);
        QVERIFY(matrix.setValues(1, 1, [](size_t, size_t) { return 0.016; }));
        EigenQt::Matrix<double> output(1, 1);
        QVERIFY(matrix.readValues(output));
        QCOMPARE(output(0, 0), 0.01);
        EigenQt::VectorEditor impossible(EigenQt::VectorEditor::SpinBox);
        impossible.setRange(0.004, 0.006);
        QVERIFY(!impossible.setValue(std::array<double, 1>{0.005}));
        QCOMPARE(impossible.columnCount(), 0);
        EigenQt::VectorEditor largeMagnitude(EigenQt::VectorEditor::SpinBox);
        largeMagnitude.setPrecision(1);
        const double largeLow = double(1LL << 51) + 0.25;
        largeMagnitude.setRange(largeLow, largeLow + 10.0);
        QVERIFY(largeMagnitude.setValue(std::array<double, 1>{largeLow}));
        QVERIFY(largeMagnitude.readValues(result));
        QVERIFY(result[0] >= largeLow);
    }
    void sameWidgetAcceptsDifferentContainersAndTypes()
    {
        EigenQt::VectorEditor editor;
        QVERIFY(editor.setValue(std::array<float, 2>{1.25f, -2.5f}));
        QCOMPARE(editor.numberType(), EigenQt::NumericTableWidget::FloatingPoint);
        std::list<double> floating(2);
        QVERIFY(editor.readValues(floating));
        QCOMPARE(floating.front(), 1.25);
        QVERIFY(editor.setValue(std::vector<int>{-3, 4, 5}));
        QCOMPARE(editor.numberType(), EigenQt::NumericTableWidget::SignedInteger);
        QCOMPARE(editor.columnCount(), 3);
        std::array<long, 3> integers{};
        QVERIFY(editor.readValues(integers));
        QCOMPARE(integers[0], -3L);
        QVERIFY(editor.setValue(std::vector<bool>{true, false}));
        QCOMPARE(editor.numberType(), EigenQt::NumericTableWidget::Boolean);
        QVERIFY(qobject_cast<QCheckBox *>(editor.cellWidget(0, 0)));
        QVERIFY(editor.setValue(std::array<double, 1>{2.5}));
        QCOMPARE(editor.numberType(), EigenQt::NumericTableWidget::FloatingPoint);
        QVERIFY(!editor.cellWidget(0, 0));
        QCOMPARE(editor.item(0, 0)->text(), QString("2.50"));

        EigenQt::VectorEditor spin(EigenQt::VectorEditor::SpinBox);
        spin.setRange(-10.0, 10.0);
        spin.setStep(0.5);
        QVERIFY(spin.setValue(std::array<double, 2>{1.5, -2.5}));
        QVERIFY(spin.setValue(std::array<int, 2>{-3, 4}));
        QVERIFY(spin.setValue(std::array<unsigned, 2>{1u, 5u}));
    }
    void fullWidthIntegers()
    {
        for (const auto mode : {EigenQt::VectorEditor::Text, EigenQt::VectorEditor::SpinBox}) {
            EigenQt::VectorEditor editor(mode);
            const std::array<std::int64_t, 3> signedValues{
                std::numeric_limits<std::int64_t>::min(), (std::int64_t(1) << 53) + 1,
                std::numeric_limits<std::int64_t>::max()};
            QVERIFY(editor.setValue(signedValues));
            std::array<std::int64_t, 3> restored{};
            QVERIFY(editor.readValues(restored));
            QVERIFY(restored == signedValues);
            const std::array<std::uint64_t, 2> unsignedValues{
                0, std::numeric_limits<std::uint64_t>::max()};
            QVERIFY(editor.setValue(unsignedValues));
            std::array<std::uint64_t, 2> unsignedOutput{};
            QVERIFY(editor.readValues(unsignedOutput));
            QVERIFY(unsignedValues == unsignedOutput);
            if (mode == EigenQt::VectorEditor::SpinBox) {
                auto *box = qobject_cast<QAbstractSpinBox *>(editor.cellWidget(0, 1));
                QVERIFY(box);
                box->stepDown();
                QVERIFY(editor.readValues(unsignedOutput));
                QCOMPARE(unsignedOutput[1], std::numeric_limits<std::uint64_t>::max() - 1);
                box->stepUp();
                box->stepUp();
                QVERIFY(editor.readValues(unsignedOutput));
                QCOMPARE(unsignedOutput[1], std::numeric_limits<std::uint64_t>::max());
                QVERIFY(editor.setValue(signedValues));
                box = qobject_cast<QAbstractSpinBox *>(editor.cellWidget(0, 1));
                box->stepUp();
                QVERIFY(editor.readValues(restored));
                QCOMPARE(restored[1], signedValues[1] + 1);
                editor.setStep(std::numeric_limits<std::uint64_t>::max());
                box->stepDown();
                QVERIFY(editor.readValues(restored));
                QCOMPARE(restored[1], std::numeric_limits<std::int64_t>::min());
            }
        }
    }
    void checkedConversionsAreAtomic()
    {
        EigenQt::VectorEditor editor;
        QVERIFY(editor.setValue(std::array<double, 2>{1.0, 2.5}));
        std::array<int, 2> integers{8, 9};
        QVERIFY(!editor.readValues(integers));
        QVERIFY((integers == std::array<int, 2>{8, 9}));
        QVERIFY(editor.setValue(std::array<int, 2>{2, -1}));
        std::vector<unsigned> positive{8, 9};
        QVERIFY(!editor.readValues(positive));
        QVERIFY(positive == std::vector<unsigned>({8, 9}));
        QVERIFY(editor.setValue(std::array<double, 1>{std::ldexp(1.0, 63)}));
        std::array<std::int64_t, 1> signedOutput{7};
        QVERIFY(!editor.readValues(signedOutput));
        QCOMPARE(signedOutput[0], std::int64_t(7));
        QVERIFY(editor.setValue(std::array<double, 1>{std::ldexp(1.0, 64)}));
        std::array<std::uint64_t, 1> unsignedOutput{7};
        QVERIFY(!editor.readValues(unsignedOutput));
        QCOMPARE(unsignedOutput[0], std::uint64_t(7));
        QVERIFY(editor.setValue(std::array<std::int64_t, 2>{1, (std::int64_t(1) << 53) + 1}));
        std::array<double, 2> floating{8, 9};
        QVERIFY(!editor.readValues(floating));
        QVERIFY((floating == std::array<double, 2>{8, 9}));
        QVERIFY(!editor.values<double>());
    }
    void rangesAndSignals()
    {
        EigenQt::VectorEditor editor(EigenQt::VectorEditor::Slider);
        editor.setRange(std::array<double, 2>{-10.0, 10.0}, std::array<double, 2>{10.0, 30.0});
        QVERIFY(editor.setValue(std::array<double, 2>{0.0, 15.0}));
        auto *slider = qobject_cast<QSlider *>(editor.cellWidget(0, 1));
        QVERIFY(slider);
        QSignalSpy changes(&editor, &EigenQt::VectorEditor::valueChanged);
        slider->setValue(500);
        std::array<double, 2> output{};
        QVERIFY(editor.readValues(output));
        QCOMPARE(output[1], 20.0);
        QCOMPARE(changes.count(), 1);
        editor.setRange(3.0, 3.0);
        QVERIFY(editor.readValues(output));
        QCOMPARE(output[0], 3.0);
        const auto fixedRangeChanges = changes.count();
        slider->setValue(500);
        QCOMPARE(changes.count(), fixedRangeChanges);
        QVERIFY_EXCEPTION_THROWN(editor.setRange(4.0, 3.0), std::invalid_argument);
        QVERIFY_EXCEPTION_THROWN(editor.setRange(0.0, std::numeric_limits<double>::quiet_NaN()),
                                 std::invalid_argument);
        QVERIFY(editor.setValue(std::array<double, 2>{3.0, 3.0}, false));
        const auto previous = changes.count();
        editor.blockSignals(true);
        QVERIFY(editor.setValue(std::array<double, 1>{3.0}, false));
        QVERIFY(editor.signalsBlocked());
        QCOMPARE(changes.count(), previous);

        EigenQt::VectorEditor switched(EigenQt::VectorEditor::SpinBox);
        switched.setRange(-50, -10);
        switched.setRange(std::array<unsigned, 2>{1u, 2u}, std::array<unsigned, 2>{5u, 6u});
        QVERIFY(switched.setValue(std::array<unsigned, 2>{3u, 4u}));
        std::array<unsigned, 2> unsignedOut{};
        QVERIFY(switched.readValues(unsignedOut));
        QCOMPARE(unsignedOut[0], 3u);
        QCOMPARE(unsignedOut[1], 4u);
    }
    void qtOwnershipAndMetadata()
    {
        QWidget parent;
        EigenQt::VectorEditor editor(EigenQt::VectorEditor::SpinBox, &parent);
        QCOMPARE(editor.parentWidget(), &parent);
        QCOMPARE(editor.property("displayMode").toInt(), int(EigenQt::VectorEditor::SpinBox));
        EigenQt::MatrixEditor matrix(&parent);
        QCOMPARE(matrix.parentWidget(), &parent);
    }
    void containers()
    {
        EigenQt::VectorView reader;
        const std::forward_list<double> values{2.0, 4.0};
        QVERIFY(reader.setValue(values));
        QCOMPARE(reader.columnCount(), 2);
        QCOMPARE(reader.item(0, 1)->text(), QString("4.00"));
        reader.setPrecision(3);
        reader.setFormat('e');
        QVERIFY(reader.setValue(std::array<double, 1>{1.5}));
        QCOMPARE(reader.item(0, 0)->text(), QString("1.500e+00"));
        QVERIFY(reader.setValue(std::vector<double>{}));
        QCOMPARE(reader.columnCount(), 0);
        std::istringstream stream("1 2 3");
        QVERIFY(reader.setValues(std::istream_iterator<int>(stream), std::istream_iterator<int>()));
        QCOMPARE(reader.columnCount(), 3);
    }
    void editors()
    {
        for (const auto mode : {EigenQt::VectorEditor::Text, EigenQt::VectorEditor::SpinBox,
                                EigenQt::VectorEditor::Slider}) {
            EigenQt::VectorEditor editor(mode);
            editor.setRange(-10.0, 10.0);
            QSignalSpy spy(&editor, &EigenQt::VectorEditor::valueChanged);
            const std::list<double> values{2.0, 4.0};
            QVERIFY(editor.setValue(values));
            QCOMPARE(spy.count(), 0);
            std::array<double, 2> output{};
            QVERIFY(editor.readValues(output));
            QCOMPARE(output[0], 2.0);
            QCOMPARE(output[1], 4.0);
            std::array<double, 1> wrong{-1.0};
            QVERIFY(!editor.readValues(wrong));
            QCOMPARE(wrong[0], -1.0);
            editor.blockSignals(true);
            QVERIFY(editor.setValue(values));
            QVERIFY(editor.signalsBlocked());
            editor.blockSignals(false);
            QVERIFY(editor.setValue(std::array<double, 0>{}));
            std::vector<double> empty;
            QVERIFY(editor.readValues(empty));
        }
    }
    void numericTypes()
    {
        EigenQt::VectorView view;
        EigenQt::VectorEditor editor;
        QVERIFY(view.setValue(QVector<unsigned>{1, 2}));
        QVERIFY(editor.setValue(QVector<unsigned>{1, 2}));
        QCOMPARE(editor.numberType(), EigenQt::NumericTableWidget::UnsignedInteger);
        const auto result = editor.values<unsigned>();
        QVERIFY(result);
        QVERIFY(*result == std::vector<unsigned>({1, 2}));
    }
    void invalidInputDoesNotOverwriteOutput()
    {
        EigenQt::VectorEditor editor;
        editor.setValue(std::array<double, 2>{1, 2});
        editor.item(0, 1)->setText("invalid");
        std::vector<double> output{8, 9};
        QVERIFY(!editor.readValues(output));
        QVERIFY(output == std::vector<double>({8, 9}));
        EigenQt::VectorEditor integer;
        integer.setBase(16);
        integer.setValue(std::array<int, 1>{255});
        std::array<int, 1> restored{};
        QVERIFY(integer.readValues(restored));
        QCOMPARE(restored[0], 255);
    }
    void packedBools()
    {
        EigenQt::VectorEditor editor;
        EigenQt::VectorView reader;
        const std::vector<bool> values{true, false, true};
        QVERIFY(editor.setValue(values));
        QVERIFY(reader.setValue(values));
        auto *box = qobject_cast<QCheckBox *>(editor.cellWidget(0, 1));
        QVERIFY(box);
        box->setChecked(true);
        std::vector<bool> output(3);
        QVERIFY(editor.readValues(output));
        QVERIFY(output == std::vector<bool>({true, true, true}));
        std::list<bool> linked(3);
        QVERIFY(editor.readValues(linked));
        QVERIFY(std::all_of(linked.begin(), linked.end(), [](bool b) { return b; }));
    }
    void matrixLayouts()
    {
        const std::array<double, 6> rowMajor{1, 2, 3, 4, 5, 6};
        EigenQt::StridedMatrixView<const double> input(rowMajor.data(), 2, 3, 3);
        EigenQt::MatrixView reader;
        EigenQt::MatrixEditor editor;
        QVERIFY(reader.setValue(input));
        QVERIFY(editor.setValue(input));
        QCOMPARE(reader.item(1, 2)->text().toDouble(), 6.0);
        std::array<double, 12> storage{};
        EigenQt::StridedMatrixView<double> columnMajor(storage.data(), 2, 3, 1, 4);
        QVERIFY(editor.readValues(columnMajor));
        QCOMPARE(storage[0], 1.0);
        QCOMPARE(storage[1], 4.0);
        QCOMPARE(storage[8], 3.0);
        QCOMPARE(storage[9], 6.0);
        QCOMPARE(storage[2], 0.0);
        EigenQt::StridedMatrixView<double> wrong(storage.data(), 1, 3, 3);
        QVERIFY(!editor.readValues(wrong));
        EigenQt::MatrixEditor ints;
        QVERIFY(ints.setValues(2, 2, [](size_t r, size_t c) { return int(r + c); }));
        EigenQt::MatrixEditor bools;
        QVERIFY(bools.setValues(2, 2, [](size_t r, size_t c) { return r == c; }));
        EigenQt::Matrix<bool> result(2, 2);
        QVERIFY(bools.readValues(result));
        QVERIFY(result(0, 0) && !result(0, 1));
        QVERIFY(editor.setValues(1, 2, [](size_t, size_t c) { return c ? 300 : 1; }));
        std::array<std::uint8_t, 2> narrow{9, 9};
        EigenQt::StridedMatrixView<std::uint8_t> destination(narrow.data(), 1, 2, 2);
        QVERIFY(!editor.readValues(destination));
        QVERIFY((narrow == std::array<std::uint8_t, 2>{9, 9}));
    }
};
QTEST_MAIN(WidgetsTest)
#include "WidgetsTest.moc"

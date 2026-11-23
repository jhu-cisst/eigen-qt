// SPDX-License-Identifier: MIT
// SPDX-FileCopyrightText: 2026 EigenQt contributors
#include <EigenQt/FrameWidget>
#include <EigenQt/MatrixEditor>
#include <EigenQt/MatrixView>
#include <EigenQt/Plot2DWidget>
#include <EigenQt/PositionsWidget>
#include <EigenQt/RotationWidget>
#include <EigenQt/Vector3DWidget>
#include <EigenQt/VectorEditor>
#include <EigenQt/WrenchWidget>
#include <QApplication>
#include <QCommandLineParser>
#include <QDir>
#include <QEventLoop>
#include <QFont>
#include <QGridLayout>
#include <QGroupBox>
#include <QImage>
#include <QLabel>
#include <QLocale>
#include <QOpenGLWidget>
#include <QPalette>
#include <QScreen>
#include <QStyle>
#include <QSurfaceFormat>
#include <QTextStream>
#include <QTimer>
#include <QVBoxLayout>
#include <array>
#include <cmath>
#include <cstdint>
#include <functional>
#include <limits>
#include <stdexcept>
#include <vector>

namespace {
void settle()
{
    QEventLoop events;
    QTimer::singleShot(250, &events, &QEventLoop::quit);
    events.exec();
}

QGroupBox *group(const QString &title, QWidget *widget)
{
    auto *box = new QGroupBox(title);
    auto *layout = new QVBoxLayout(box);
    layout->setContentsMargins(12, 20, 12, 12);
    layout->addWidget(widget);
    return box;
}

class Captures
{
public:
    explicit Captures(const QString &directory) : output(directory)
    {
        if (!output.mkpath(".")) {
            throw std::runtime_error("Cannot create screenshot directory");
        }
    }

    void save(const QString &name, QWidget &window, QSize size,
              const std::function<void()> &populate = {})
    {
        window.setWindowTitle("EigenQt documentation capture");
        const auto available = QApplication::primaryScreen()->availableGeometry();
        if (available.width() < size.width() || available.height() < size.height()) {
            throw std::runtime_error("Screenshot capture needs a display of at least 960 x 700");
        }
        window.resize(size);
        window.move(available.topLeft() + QPoint(24, 24));
        window.show();
        window.raise();
        settle();
        if (populate) {
            populate();
        }
        window.update();
        settle();
        for (auto *gl : window.findChildren<QOpenGLWidget *>()) {
            if (gl->isVisible() && (!gl->isValid() || gl->grabFramebuffer().isNull())) {
                throw std::runtime_error("A visible OpenGL widget has no rendered framebuffer");
            }
        }
        settle();
        auto image = window.screen()->grabWindow(window.winId()).toImage();
        if (image.isNull()) {
            throw std::runtime_error("The desktop platform cannot capture this window");
        }
        image.setText("License", "SPDX-License-Identifier: MIT");
        image.setText("Copyright", "2026 EigenQt contributors");
        image.setText("Description", "Actual EigenQt widgets rendered with Qt Fusion");
        const auto path = output.filePath(name + ".png");
        if (!image.save(path, "PNG")) {
            throw std::runtime_error("Could not save PNG screenshot");
        }
        QTextStream(stdout) << path << '\n';
        window.hide();
    }

private:
    QDir output;
};

Eigen::Matrix3d sampleRotation()
{
    return (Eigen::AngleAxisd(0.6, Eigen::Vector3d::UnitZ()) *
            Eigen::AngleAxisd(-0.3, Eigen::Vector3d::UnitY()) *
            Eigen::AngleAxisd(0.2, Eigen::Vector3d::UnitX()))
        .toRotationMatrix();
}

void numeric(Captures &captures)
{
    QWidget window;
    auto *layout = new QVBoxLayout(&window);
    const std::array<double, 4> values{1.25, -2.5, 3.75, 0.5};
    const std::array<QString, 3> labels{"Text", "SpinBox", "Slider"};
    int i = 0;
    for (const auto mode : {EigenQt::VectorEditor::Text, EigenQt::VectorEditor::SpinBox,
                            EigenQt::VectorEditor::Slider}) {
        auto *editor = new EigenQt::VectorEditor(mode);
        editor->setRange(-10.0, 10.0);
        editor->setStep(0.25);
        editor->setValue(values);
        editor->setFixedHeight(40);
        layout->addWidget(group(labels[i++] + " - floating-point vector", editor));
    }
    auto *bits = new EigenQt::VectorEditor;
    bits->setValue(std::vector<bool>{true, false, true, true});
    bits->setFixedHeight(40);
    layout->addWidget(group("Boolean vector - checkboxes", bits));
    auto *integers = new EigenQt::VectorEditor(EigenQt::VectorEditor::SpinBox);
    integers->setValue(std::array<std::int64_t, 3>{-42, (std::int64_t(1) << 53) + 1,
                                                   std::numeric_limits<std::int64_t>::max()});
    integers->setFixedHeight(40);
    layout->addWidget(group("Integer vector - exact 64-bit values", integers));
    captures.save("numeric-editors", window, {900, 510});
}

void matrices(Captures &captures)
{
    QWidget window;
    auto *layout = new QHBoxLayout(&window);
    auto *view = new EigenQt::MatrixView;
    view->setValue(sampleRotation());
    layout->addWidget(group("Read-only matrix", view));
    auto *editor = new EigenQt::MatrixEditor;
    editor->setRange(-10.0, 10.0);
    editor->setValues(3, 3, [](std::size_t row, std::size_t column) {
        return row == column ? 1.0 : (double(row) - double(column)) * 0.25;
    });
    layout->addWidget(group("Spinbox matrix", editor));
    auto *bits = new EigenQt::MatrixEditor;
    bits->setValues(3, 3, [](std::size_t row, std::size_t column) { return row == column; });
    layout->addWidget(group("Boolean matrix", bits));
    captures.save("matrix-widgets", window, {900, 280});
}

void rotations(Captures &captures)
{
    QWidget window;
    auto *layout = new QGridLayout(&window);
    const std::array<EigenQt::RotationWidget::DisplayMode, 6> modes{
        EigenQt::RotationWidget::RotationMatrix, EigenQt::RotationWidget::AxisAngle,
        EigenQt::RotationWidget::Quaternion,     EigenQt::RotationWidget::EulerZYZ,
        EigenQt::RotationWidget::EulerZYX,       EigenQt::RotationWidget::OpenGL};
    const std::array<QString, 6> labels{"Rotation matrix",         "Axis / angle (degrees)",
                                        "Quaternion (x, y, z, w)", "Euler ZYZ (degrees)",
                                        "Euler ZYX (degrees)",     "Interactive 3D frame"};
    for (int i = 0; i < 6; ++i) {
        auto *rotation = new EigenQt::RotationWidget(modes[i]);
        rotation->setValue(sampleRotation());
        layout->addWidget(group(labels[i], rotation), i / 3, i % 3);
    }
    captures.save("rotation-modes", window, {960, 520});
}

void frame(Captures &captures)
{
    QWidget window;
    auto *layout = new QVBoxLayout(&window);
    auto *frame = new EigenQt::FrameWidget;
    Eigen::Isometry3d pose = Eigen::Isometry3d::Identity();
    pose.linear() = sampleRotation();
    pose.translation() = Eigen::Vector3d(0.12, -0.04, 0.08);
    frame->setValue(pose);
    layout->addWidget(group("Rotation matrix / translation (mm)", frame));
    captures.save("frame-widget", window, {540, 320});
}

void plot(Captures &captures)
{
    QWidget window;
    auto *layout = new QVBoxLayout(&window);
    auto *legend = new QLabel("<span style='color:#2466ba'>&#9679; sin(t)</span> &nbsp; "
                              "<span style='color:#cc7029'>&#9679; 0.6 cos(2t)</span> &nbsp; "
                              "<span style='color:#777777'>&#9474; event at 2.5 s</span>");
    legend->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
    layout->addWidget(legend);
    auto *plot = new EigenQt::Plot2DWidget;
    layout->addWidget(plot, 1);
    plot->setBackgroundColor(Eigen::Vector3d::Ones());
    plot->setNumberOfPoints(500);
    auto *scale = plot->addScale("signals");
    auto *sine = scale->addSignal("sin(t)");
    auto *cosine = scale->addSignal("cos(2t)");
    sine->setColor(Eigen::Vector3d(0.14, 0.40, 0.73));
    cosine->setColor(Eigen::Vector3d(0.80, 0.44, 0.16));
    auto *event = scale->addVerticalLine("event");
    event->setX(2.5);
    event->setColor(Eigen::Vector3d(0.5, 0.5, 0.5));
    scale->setContinuousFitY(true);
    plot->setDisplayYRangeScale(scale);
    captures.save("plot-widget", window, {900, 380}, [&] {
        for (int i = 0; i < 400; ++i) {
            const double t = i * 0.02;
            sine->appendPoint(Eigen::Vector2d(t, std::sin(t)));
            cosine->appendPoint(Eigen::Vector2d(t, 0.6 * std::cos(2.0 * t)));
        }
        plot->update();
    });
}

void positions(Captures &captures)
{
    QWidget window;
    auto *layout = new QVBoxLayout(&window);
    auto *positions = new EigenQt::PositionsWidget;
    layout->addWidget(positions);
    captures.save("positions", window, {900, 620}, [&] {
        for (int i = 0; i < 240; ++i) {
            const double t = i * 0.04;
            positions->setValue(Eigen::Vector3d(0.02 * std::cos(t), 0.02 * std::sin(t), t * 0.003));
        }
    });
}

void vector(Captures &captures)
{
    QWidget window;
    auto *layout = new QVBoxLayout(&window);
    auto *vector = new EigenQt::Vector3DWidget;
    layout->addWidget(vector);
    captures.save("vector-3d", window, {520, 420},
                  [&] { vector->setValue(Eigen::Vector3d(0.65, 0.4, 0.75)); });
}

void wrenches(Captures &captures)
{
    const std::array<EigenQt::WrenchWidget::DisplayMode, 3> modes{
        EigenQt::WrenchWidget::Text, EigenQt::WrenchWidget::Plot2D, EigenQt::WrenchWidget::Plot3D};
    const std::array<QString, 3> names{"wrench-text", "wrench-plot", "wrench-3d"};
    const std::array<QSize, 3> sizes{QSize(660, 110), QSize(900, 420), QSize(560, 440)};
    for (int i = 0; i < 3; ++i) {
        QWidget window;
        auto *layout = new QVBoxLayout(&window);
        auto *wrench = new EigenQt::WrenchWidget(modes[i]);
        layout->addWidget(wrench);
        captures.save(names[i], window, sizes[i], [&] {
            if (modes[i] == EigenQt::WrenchWidget::Plot2D) {
                for (int sample = 0; sample < 180; ++sample) {
                    const double t = sample * 0.04;
                    wrench->setValue(
                        Eigen::Vector3d(2.0 * std::sin(t), 1.5 * std::cos(t), std::sin(2.0 * t)),
                        Eigen::Vector3d(0.1, 0.2, 0.3), t);
                }
            } else {
                wrench->setValue(Eigen::Vector3d(3.0, 4.0, 1.5), Eigen::Vector3d(0.1, -0.2, 0.3),
                                 12.5);
            }
        });
    }
}
} // namespace

int main(int argc, char **argv)
{
    qputenv("QT_SCALE_FACTOR", "1");
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    QApplication::setAttribute(Qt::AA_DisableHighDpiScaling);
#endif
    QSurfaceFormat format;
    format.setRenderableType(QSurfaceFormat::OpenGL);
    format.setVersion(2, 1);
    format.setProfile(QSurfaceFormat::NoProfile);
    format.setDepthBufferSize(24);
    QSurfaceFormat::setDefaultFormat(format);
    QApplication app(argc, argv);
    QApplication::setStyle("Fusion");
    app.setFont(QFont("DejaVu Sans", 10));
    QLocale::setDefault(QLocale::c());
    auto palette = QApplication::style()->standardPalette();
    palette.setColor(QPalette::Window, QColor("#f6f7f9"));
    palette.setColor(QPalette::Base, Qt::white);
    palette.setColor(QPalette::Text, QColor("#202630"));
    palette.setColor(QPalette::WindowText, QColor("#202630"));
    palette.setColor(QPalette::Highlight, QColor("#2466ba"));
    app.setPalette(palette);
    QCommandLineParser parser;
    parser.setApplicationDescription("Capture real EigenQt widgets for the manual.");
    parser.addHelpOption();
    parser.addPositionalArgument("directory", "Output PNG directory (default: doc/images)",
                                 "[directory]");
    parser.process(app);
    const auto arguments = parser.positionalArguments();
    if (arguments.size() > 1) {
        parser.showHelp(1);
    }
    try {
        Captures captures(arguments.isEmpty() ? QStringLiteral("doc/images") : arguments.first());
        numeric(captures);
        matrices(captures);
        rotations(captures);
        frame(captures);
        plot(captures);
        vector(captures);
        positions(captures);
        wrenches(captures);
    } catch (const std::exception &error) {
        QTextStream(stderr) << "Screenshot capture failed: " << error.what() << '\n';
        return 1;
    }
    return 0;
}

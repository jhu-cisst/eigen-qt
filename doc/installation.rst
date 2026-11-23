.. SPDX-License-Identifier: MIT
.. SPDX-FileCopyrightText: 2026 EigenQt contributors

Installation and first application
==================================

Requirements
------------

* CMake 3.16 or newer and a C++17 compiler.
* Qt 5.15 or Qt 6, including the Widgets development files.
* Eigen 3.4 or newer, required by every component and installed consumer.
* Desktop OpenGL development files, and Qt OpenGL (Qt 5) or OpenGLWidgets (Qt 6).

Build and install
-----------------

From the repository root:

.. code-block:: sh

   cmake -S . -B build \
     -DCMAKE_INSTALL_PREFIX="$PWD/install" \
     -DEIGENQT_BUILD_TESTS=ON \
     -DEIGENQT_BUILD_EXAMPLES=ON
   cmake --build build --parallel
   ctest --test-dir build --output-on-failure
   cmake --install build
   ./build/bin/eigenqtGallery

Use ``-DEIGENQT_QT_VERSION=5`` or ``6`` to select a Qt major version. Use
``-DBUILD_SHARED_LIBS=OFF`` for static libraries. With a multi-configuration
generator, pass ``--config Release`` to build/install and ``-C Release`` to
CTest; executables are under ``build/bin/Release``.

Every build includes the Widgets, Plot, and Geometry libraries.

Link an application
-------------------

Add this to your application's ``CMakeLists.txt``:

.. code-block:: cmake

   cmake_minimum_required(VERSION 3.16)
   project(NumericExample LANGUAGES CXX)
   find_package(EigenQt 0.1 CONFIG REQUIRED)
   add_executable(numeric-example main.cpp)
   target_link_libraries(numeric-example PRIVATE EigenQt::Widgets)

Create ``main.cpp``:

.. code-block:: cpp

   #include <EigenQt/VectorEditor>
   #include <QApplication>
   #include <Eigen/Core>

   int main(int argc, char **argv)
   {
       QApplication app(argc, argv);
       EigenQt::VectorEditor editor(EigenQt::VectorEditor::SpinBox);
       editor.setWindowTitle("Joint commands");
       editor.setRange(-10.0, 10.0);
       editor.setValue(Eigen::Vector3f(1.0f, 2.0f, 3.0f));
       editor.resize(480, 100);
       editor.show();
       return app.exec();
   }

Configure your application with the EigenQt installation prefix:

.. code-block:: sh

   cmake -S . -B build -DCMAKE_PREFIX_PATH=/path/to/eigen-qt/install
   cmake --build build --parallel

Use ``EigenQt::Plot`` or ``EigenQt::Geometry`` for their
respective widgets. Package discovery provides all three targets and their
dependencies; link the target your application uses.
Use the same Qt major version in the library and application.

cisst and colcon
----------------

The colcon package name is ``eigen_qt``. In a workspace containing
cisst, build/install this package before configuring cisst Qt libraries.
cisst Qt builds require EigenQt; consumers use its headers and CMake
targets directly. The former ``cisstVectorQt`` library and compatibility aliases
have been removed.

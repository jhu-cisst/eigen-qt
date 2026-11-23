.. SPDX-License-Identifier: MIT
.. SPDX-FileCopyrightText: 2026 EigenQt contributors

Building and maintaining the documentation
==========================================

Build HTML locally
------------------

The manual uses Sphinx and reStructuredText. From the repository root:

.. code-block:: sh

   python3 -m venv /tmp/eigenqt-doc-env
   /tmp/eigenqt-doc-env/bin/python -m pip install -r doc/requirements.txt
   /tmp/eigenqt-doc-env/bin/python -m sphinx -W --keep-going \
     -b html doc doc/_build/html

Open ``doc/_build/html/index.html`` in a browser. An environment with the
requirements already installed can instead run ``make -C doc html``.
Warnings fail the build so that missing images and broken references are caught.

Read the Docs
-------------

The repository-root ``.readthedocs.yaml`` selects ``doc/conf.py`` and installs
``doc/requirements.txt``. Once the repository is available on a supported Git
host, import it into Read the Docs and enable the versions you want to publish.
The configuration follows the
`Read the Docs configuration reference <https://docs.readthedocs.com/platform/stable/config-file/v2.html>`_.

The hosted build only needs Python and the checked-in PNG files; it does not
build Qt, require a display, or regenerate screenshots. Project version information
comes from the root ``CMakeLists.txt``.

.. _capture-screenshots:

Regenerate widget screenshots
-----------------------------

The capture program is ``doc/tools/capture_widgets.cpp``. It instantiates the
library widgets, supplies fixed data, and saves their rendered contents to PNG.
It does not create mockups. Each image includes MIT license and
copyright metadata.

Build the full library and the explicit screenshot target:

.. code-block:: sh

   cmake -S . -B build -DEIGENQT_BUILD_EXAMPLES=ON
   cmake --build build --target eigenqtScreenshots --parallel
   ./build/bin/eigenqtScreenshots doc/images

The capture target is excluded from normal example builds.
On multi-configuration generators, use ``--config Release``
and run ``build/bin/Release/eigenqtScreenshots``.

Run it in a desktop session with a compatible desktop OpenGL context. The tool
briefly opens one window at a time, checks that visible OpenGL widgets have valid
contexts, and captures only those windows. It fixes the style, palette, locale,
font, scale factor, and sample data to keep images reproducible.

On Linux, Mesa software rendering can be selected with:

.. code-block:: sh

   LIBGL_ALWAYS_SOFTWARE=1 ./build/bin/eigenqtScreenshots doc/images

An Xvfb display with GLX support is another option for CI. The Qt ``offscreen``
platform is sufficient for numeric widgets but is not a substitute for a working
OpenGL context when capturing the full gallery.

After regenerating images, inspect them and rebuild the HTML manual. This catches
blank OpenGL surfaces, clipped controls, and image paths that have become stale.

Tests
-----

.. code-block:: sh

   cmake -S . -B build -DEIGENQT_BUILD_TESTS=ON
   cmake --build build --parallel
   ctest --test-dir build --output-on-failure

The widget tests run with Qt's offscreen platform. They check data conversion,
container adapters, ownership, signals, and display state. They do not validate
GPU rendering; inspect the gallery and captured images for that purpose.

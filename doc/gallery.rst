.. SPDX-License-Identifier: MIT
.. SPDX-FileCopyrightText: 2026 EigenQt contributors

Widget gallery
==============

These images are captured from the real Qt widgets with deterministic sample
data. They use the Fusion style and a light palette. Native platform styles and
fonts may look different. See :ref:`capture-screenshots` to regenerate the images.

Numeric editors
---------------

.. figure:: images/numeric-editors.png
   :alt: Five kinds of vector editor with numeric and checkbox controls
   :width: 100%

   VectorEditor: Text, SpinBox, Slider, Boolean, and full-width Integer input.

Matrix widgets
--------------

.. figure:: images/matrix-widgets.png
   :alt: Read-only, editable and boolean three-by-three matrices
   :width: 100%

   MatrixView and MatrixEditor accept matrix-like objects and custom accessors.

Rotation representations
------------------------

.. figure:: images/rotation-modes.png
   :alt: Matrix, axis/angle, quaternion, Euler ZYZ, Euler ZYX and OpenGL rotation views
   :width: 100%

   Six representations of the same rotation.

Frame display
-------------

.. figure:: images/frame-widget.png
   :alt: Rotation matrix and three translation coordinates
   :width: 70%

   FrameWidget combines a RotationWidget and a translation VectorView.

Streaming plot
--------------

.. figure:: images/plot-widget.png
   :alt: Blue and orange signals with a vertical event marker
   :width: 100%

   Plot2DWidget with a shared scale and Y-range labels.

3D vector
---------

.. figure:: images/vector-3d.png
   :alt: Three-dimensional vector with axis projections
   :width: 65%

   Vector3DWidget includes mouse rotation and automatic scaling.

Positions
---------

.. figure:: images/positions.png
   :alt: Position table and XY, XZ and YZ projections
   :width: 100%

   PositionsWidget keeps the added positions until clear() is called.

Wrench text
-----------

.. figure:: images/wrench-text.png
   :alt: Force and torque coordinate rows with the force norm
   :width: 75%

   WrenchWidget in Text mode.

Wrench time series
------------------

.. figure:: images/wrench-plot.png
   :alt: Force components, norm and zero plotted over time with visibility controls
   :width: 100%

   WrenchWidget in Plot2D mode.

Wrench 3D
---------

.. figure:: images/wrench-3d.png
   :alt: Interactive 3D force vector and a force-or-torque selector
   :width: 70%

   WrenchWidget in Plot3D mode.

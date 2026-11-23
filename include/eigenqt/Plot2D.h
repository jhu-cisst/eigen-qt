// SPDX-License-Identifier: MIT

/*
  Author(s):  Anton Deguet
  Created on: 2010-05-05

  (C) Copyright 2010-2026 Johns Hopkins University (JHU), All Rights Reserved.

*/

#ifndef EIGENQT_PLOT2DBASE_H
#define EIGENQT_PLOT2DBASE_H

#include <map>
#include <array>
#include <vector>
#include <stdexcept>
#include <Eigen/Dense>
#include <memory>
#include <string>

#include <eigenqt/plot_export.h>

#include <utility>

namespace EigenQt {

/*! Base class for 2D plotters.  Handles storage of 2D points, colors
  and thicknesses, computation of scales and offsets.

  \todo Rename "trace" to "signal", "data set", "set", "point set", ...
  \todo Add "grid" and/or "axis" (maybe a special grid) to all scales
  \todo Add a flag to all elements to determine if they should be used or not in auto fit methods
  (e.g. user might {not} want to see y=0 line) \todo Cleanup pointSize vs lineWidth in traces,
  lines, etc.  use linewidth for everything \todo Create a base class "element"/"data" for all
  elements visible (i.e. signal, vertical line, grid, ...), modifiedSinceLastRender, maybe even
  render method ... \todo Fit/render should probably be defined at plot/scale/data level \todo Add
  flag/method Modified to all elements type to optimize fit/render
*/
class EIGENQT_PLOT_EXPORT Plot2D
{
public:
    class Scale;

    /*! Storage for a given signal.  Each signal stores the data to
      display in a contiguous circular buffer.  */
    class EIGENQT_PLOT_EXPORT Signal
    {
        friend class Plot2D;
        friend class Plot2D::Scale;
        friend class Plot2DOpenGL;

    public:
        Signal(const std::string &name, size_t numberOfPoints);

        /*! Insert point at last position and move last position
          forward.  If the circular buffer is full, this methods
          overwrite the first element. */
        void appendPoint(const Eigen::Vector2d &point);
        template <class Iterator> void appendPoints(Iterator first, Iterator last)
        {
            for (; first != last; ++first) {
                const auto &point = *first;
                appendPoint(Eigen::Vector2d(point[0], point[1]));
            }
        }

        /*! Get point value in the buffer relative to first element.
          This method will throw an std::runtime_error if the index is
          invalid, i.e. greater than the buffer size. */
        Eigen::Vector2d pointAt(size_t index) const;

        /*! Set point value at a given position, relative to first
          element.  This method will throw an std::runtime_error if
          the index is invalid, i.e. greater than the buffer size. */
        void setPointAt(size_t index, const Eigen::Vector2d &point);

        void setFrozen(bool freeze);
        bool isFrozen() const;

        /*! Compute min and max over all points to recenter display.
            If the buffer contains no point, returns 0. */
        //@{
        void computeDataRangeX(double &min, double &max, bool assumesDataSorted = false) const;
        void computeDataRangeY(double &min, double &max) const;
        void computeDataRangeXY(Eigen::Vector2d &min, Eigen::Vector2d &max) const;
        //@}

        /*! Get size of circular buffer. */
        size_t capacity() const;

        /*! Get number of points. */
        size_t pointCount() const;

        void setColor(const Eigen::Vector3d &colorInRange0To1);

        /*! Destructive resize, this method resize the circular buffer
          and sets the first and last pointers to the buffer's
          beginning. */
        void setSize(size_t numberOfPoints);

        bool isVisible() const;
        void setVisible(const bool visible);

        /*! Non destructive resize.  If the new buffer is bigger,
          preserves all points.  If the new buffer is smaller,
          preserves the data at the end by default. */
        void resize(size_t numberOfPoints, bool trimOlder = true);

        /*! Add a sibling signal. Returns nullptr for a standalone signal or a duplicate name. */
        Plot2D::Signal *addSignal(const std::string &name);
        /*! Owning scale, or nullptr for a standalone signal. */
        Plot2D::Scale *parentScale();
        const std::string &name() const;

    protected:
        Scale *Parent = nullptr;
        std::string Name;

        bool Empty;
        bool Visible;
        bool Frozen;

        Eigen::Vector3d Color;
        double LineWidth;

        // Samples are stored in a contiguous STL circular buffer.
        std::vector<std::array<double, 2>> data;

        // first/last (inclusive) valid entries in the circular buffer
        size_t IndexFirst;
        size_t IndexLast;
    };

    /*! Storage for a given vertical line. */
    class EIGENQT_PLOT_EXPORT VerticalLine
    {
        friend class Plot2D;
        friend class Plot2DOpenGL;

    public:
        VerticalLine(const std::string &name, const double x = 0.0);

        void setX(const double x);
        void setColor(const Eigen::Vector3d &colorInRange0To1);

    protected:
        std::string Name;
        double X;
        bool Visible;
        Eigen::Vector3d Color;
        double LineWidth;
    };

    class EIGENQT_PLOT_EXPORT Scale
    {
        friend class Plot2D;
        friend class Plot2DOpenGL;

    public:
        // keep signals in a std::map
        typedef std::map<std::string, std::unique_ptr<Signal>> SignalsType;

        // keep vertical lines in a std::map
        typedef std::map<std::string, std::unique_ptr<VerticalLine>> VerticalLinesType;

        Scale(const std::string &name, size_t numberOfPoints = 100);

        // Explicitly delete copy ctor so MSVC dllexport doesn't complain
        Scale(const Scale &) = delete;
        Scale &operator=(const Scale &) = delete;

        const std::string &name() const;

        Plot2D::Signal *addSignal(const std::string &name);
        bool removeSignal(const std::string &name);
        bool removeSignal(const Signal *signal);

        inline const SignalsType &signalMap() const { return Signals; }
        inline const VerticalLinesType &verticalLineMap() const { return VerticalLines; }

        Plot2D::VerticalLine *addVerticalLine(const std::string &name);

        void setColor(const Eigen::Vector3d &colorInRange0To1);

        void setFrozen(bool freeze);
        bool isFrozen() const;

        void continuousUpdate();
        /*! Set options to tell Continuous update how to recenter the
          scale based on the signals. */
        //@{
        void setContinuousFitX(bool fit);
        void setContinuousFitY(bool fit);
        void setContinuousExpandY(bool expand);
        //@}
        /*! Query automatic recentering settings. */
        //@{
        inline bool continuousFitX() const { return ContinuousFitX; }
        inline bool continuousFitY() const { return ContinuousFitY; }
        inline bool continuousExpandY() const { return ContinuousExpandY; }
        const Eigen::Vector2d &viewingRangeX() const { return this->ViewingRangeX; }
        const Eigen::Vector2d &viewingRangeY() const { return this->ViewingRangeY; }
        //@}

        /*! To fit the data in the viewport we need to compute the
          range for all signals.  To reduce the number of
          computations, three methods are provided, one that compute
          the X range only, one for the Y range only and one for both
          X and Y.  Each method calls the corresponding compute data
          range for all signals in the scale. */
        //@{
        bool computeDataRangeX(double &min, double &max, bool assumesDataSorted = false) const;
        bool computeDataRangeY(double &min, double &max);
        bool computeDataRangeXY(Eigen::Vector2d &min, Eigen::Vector2d &max);
        //@}

        /*! Data recentering, these methods re-align the data once
          only, based on all signals.  Padding is used to make sure
          the data is not plotted at the extreme edges of the window.
          The padding parameter indicates the percentage of space that
          should be left empty.  For example, a 200 pixel window with
          a padding of 0.1 (10%) will leave a band of 10 pixels empty
          on each side of the window.  By default, the padding in X
          (horizontal) is null and 10% (0.1) in Y (vertical). */
        //@{
        void autoFitX(double padding = 0.0);
        void fitX(double min, double max, double padding = 0.0);
        void autoFitY(double padding = 0.1);
        void autoExpandY(double padding = 0.1);
        void fitY(double min, double max, double padding = 0.1);
        void autoFitXY(const Eigen::Vector2d &padding = Eigen::Vector2d(0.0, 0.1));
        void autoFitXExpandY(const Eigen::Vector2d &padding = Eigen::Vector2d(0.0, 0.1));
        void fitXY(Eigen::Vector2d min, Eigen::Vector2d max,
                   const Eigen::Vector2d &padding = Eigen::Vector2d(0.0, 0.1));
        //@}

    protected:
        SignalsType Signals;
        VerticalLinesType VerticalLines;
        bool ContinuousFitX;
        bool ContinuousFitY;
        bool ContinuousExpandY;
        double ExpandYMin, ExpandYMax;

        // viewport sizes
        Eigen::Vector2d Viewport;
        // stores the min and max corresponding to the viewport
        Eigen::Vector2d ViewingRangeX, ViewingRangeY;
        Eigen::Vector2d Translation;
        Eigen::Vector2d ScaleValue;

    private:
        std::string Name;
        Eigen::Vector3d Color;
        double LineWidth;
        size_t NumberOfPoints;
    };

    /*! Type used to store all scales used by the plot.  Each scale
      can contain multiple signals, lines, ... that will be scaled
      together. */
    typedef std::map<std::string, std::unique_ptr<Scale>> ScalesType;

    /*! Create a new scale and return a pointer to the newly created
      scale.  If a scale with the same name already exists the method
      returns a pointer to the existing scale. */
    Plot2D::Scale *addScale(const std::string &name);

    /*! Find an existing scale by name. */
    Plot2D::Scale *findScale(const std::string &name);

    /*! Remove an existing scale by name. */
    bool removeScale(const std::string &name);

    /*! Remove an existing scale by address. */
    bool removeScale(const Plot2D::Scale *scale);

    /*! Reference to the map of scales. */
    const ScalesType &scales() const { return Scales; }
    inline ScalesType &scales() { return Scales; }

    Plot2D();
    virtual ~Plot2D() = default;

    // Explicitly delete copy ctor so MSVC dllexport doesn't complain
    Plot2D(const Plot2D &) = delete;
    Plot2D &operator=(const Plot2D &) = delete;

    /*! Set the number of points for all signals. */
    void setNumberOfPoints(size_t numberOfPoints);

    /*! Data recentering, these methods re-align the data once only,
      based on all signals.  Padding is used to make sure the data is
      not plotted at the extreme edges of the window.  The padding
      parameter indicates the percentage of space that should be left
      empty.  For example, a 200 pixel window with a padding of 0.1
      (10%) will leave a band of 10 pixels empty on each side of the
      window.  By default, the padding in X (horizontal) is null and
      10% (0.1) in Y (vertical). */
    //@{
    void autoFitX(double padding = 0.0);
    void fitX(double min, double max, double padding = 0.0);
    void autoFitY(double padding = 0.1);
    void fitY(double min, double max, double padding = 0.1);
    //@}

    /*! setFrozen the circular buffers, i.e. AddPoint does nothing.  When
      turned off (parameter is false), this is equivalent to starting
      with an empty data set.  These methods work on all signals at
      once. */
    void setFrozen(bool freeze);

    /*! Check if all signals are frozen. */
    bool isFrozen() const;

    /*! Automatic recentering.  Propagate settings to all scales. */
    //@{
    void setContinuousFitX(bool fit);
    void setContinuousFitY(bool fit);
    void setContinuousExpandY(bool expand);
    //@}

    /*! Check if all scales are on continous fit for X. */
    bool continuousFitX() const;

    /*! Check if all scales are on continous fit for Y. */
    bool continuousFitY() const;

    /*! Check if all scales are on continous fit for X and Y. */
    bool continuousExpandY() const;

    /*! Set background color, defined as RGB between 0 and 1. */
    void setBackgroundColor(const Eigen::Vector3d &colorInRange0To1);

protected:
    /*! Methods required in all derived classes */
    //@{
    virtual void renderInitialize() = 0;
    virtual void renderResize(double width, double height) = 0;
    virtual void render() = 0;
    //@}

protected:
    ScalesType Scales;

    // default number of points for all signals
    size_t NumberOfPoints;

    // viewport sizes
    Eigen::Vector2d Viewport;

    /*! Method called before each iteration to figure out if an
      automatic update is needed or not for each scale. */
    void continuousUpdate();

    // background color
    Eigen::Vector3d BackgroundColor;
};

} // namespace EigenQt

#endif // EIGENQT_PLOT2DBASE_H

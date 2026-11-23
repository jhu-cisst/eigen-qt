// SPDX-License-Identifier: MIT

/*
  Author(s):  Anton Deguet
  Created on: 2010-05-05

  (C) Copyright 2010-2026 Johns Hopkins University (JHU), All Rights Reserved.

*/

#include <eigenqt/Plot2D.h>

#include <limits>
#include <eigenqt/detail/Constants.h>
#include <cassert>
#include <cmath>

namespace EigenQt {

Plot2D::Scale::Scale(const std::string &name, size_t numberOfPoints)
    : ContinuousFitX(true), ContinuousFitY(false), ContinuousExpandY(true),
      ExpandYMin(std::numeric_limits<double>::max()),
      ExpandYMax(std::numeric_limits<double>::lowest()), Viewport(Eigen::Vector2d::Ones()),
      ViewingRangeX(-1.0, 1.0), ViewingRangeY(-1.0, 1.0), Translation(Eigen::Vector2d::Zero()),
      ScaleValue(Eigen::Vector2d::Ones()), Name(name), Color(Eigen::Vector3d::Ones()),
      LineWidth(1.0), NumberOfPoints(numberOfPoints)
{
    autoFitXY(Eigen::Vector2d::Ones());
    setContinuousFitX(true);
    setContinuousExpandY(true);
}

const std::string &Plot2D::Scale::name() const
{
    return this->Name;
}

void Plot2D::Scale::autoFitX(double padding)
{
    double min, max;
    this->computeDataRangeX(min, max);
    this->fitX(min, max, padding);
}

void Plot2D::Scale::fitX(double min, double max, double padding)
{
    this->ViewingRangeX = Eigen::Vector2d(min, max);
    this->ScaleValue.x() = this->Viewport.x() / ((max - min) * (1.0 + padding));
    if (!std::isfinite(ScaleValue.x())) {
        ScaleValue.x() = 1.0;
    }

    this->Translation.x() = -min * this->ScaleValue.x() + 0.5 * padding * this->Viewport.x();
}

void Plot2D::Scale::autoFitY(double padding)
{
    double min, max;
    this->computeDataRangeY(min, max);
    this->fitY(min, max, padding);
}

void Plot2D::Scale::autoExpandY(double padding)
{
    double min, max;
    this->computeDataRangeY(min, max);
    if (min < this->ExpandYMin) {
        this->ExpandYMin = min;
    }
    if (max > this->ExpandYMax) {
        this->ExpandYMax = max;
    }
    this->fitY(this->ExpandYMin, this->ExpandYMax, padding);
}

void Plot2D::Scale::fitY(double min, double max, double padding)
{
    this->ViewingRangeY = Eigen::Vector2d(min, max);
    this->ScaleValue.y() = this->Viewport.y() / ((max - min) * (1.0 + padding));
    if (!std::isfinite(ScaleValue.y())) {
        ScaleValue.y() = 1.0;
    }

    this->Translation.y() = -min * this->ScaleValue.y() + 0.5 * padding * this->Viewport.y();
}

void Plot2D::Scale::autoFitXY(const Eigen::Vector2d &padding)
{
    Eigen::Vector2d min, max;
    this->computeDataRangeXY(min, max);
    this->fitXY(min, max, padding);
}

void Plot2D::Scale::fitXY(Eigen::Vector2d min, Eigen::Vector2d max, const Eigen::Vector2d &padding)
{
    ViewingRangeX = Eigen::Vector2d(min.x(), max.x());
    ViewingRangeY = Eigen::Vector2d(min.y(), max.y());
    // compute scale
    Eigen::Vector2d dataDiff = max - min;
    this->ScaleValue = this->Viewport.cwiseQuotient(dataDiff);
    if (!std::isfinite(ScaleValue.x())) {
        ScaleValue.x() = 1.0;
    }
    if (!std::isfinite(ScaleValue.y())) {
        ScaleValue.y() = 1.0;
    }

    Eigen::Vector2d pad = padding + Eigen::Vector2d::Ones();
    this->ScaleValue = ScaleValue.cwiseQuotient(pad);
    // compute translation
    this->Translation = -min.cwiseProduct(ScaleValue);
    pad = 0.5 * padding.cwiseProduct(this->Viewport);
    this->Translation += pad;
}

void Plot2D::Scale::autoFitXExpandY(const Eigen::Vector2d &padding)
{
    Eigen::Vector2d min, max;
    computeDataRangeXY(min, max);

    min.y() = std::min(min.y(), ExpandYMin);
    ExpandYMin = min.y();

    max.y() = std::max(max.y(), ExpandYMax);
    ExpandYMax = max.y();

    fitXY(min, max, padding);
}

bool Plot2D::Scale::computeDataRangeX(double &min, double &max, bool assumesDataSorted) const
{
    min = std::numeric_limits<double>::max();
    max = std::numeric_limits<double>::lowest();
    bool hasData = false;

    for (const auto &signal : Signals) {
        if (signal.second->isVisible() && signal.second->pointCount() > 0) {
            double signal_min, signal_max;
            signal.second->computeDataRangeX(signal_min, signal_max, assumesDataSorted);
            min = std::min(min, signal_min);
            max = std::max(max, signal_max);
            hasData = true;
        }
    }

    if (!hasData) {
        min = -1.0;
        max = 1.0;
        return false;
    }
    return true;
}

bool Plot2D::Scale::computeDataRangeY(double &min, double &max)
{
    min = std::numeric_limits<double>::max();
    max = std::numeric_limits<double>::lowest();
    bool hasData = false;

    for (const auto &signal : Signals) {
        if (signal.second->isVisible() && signal.second->pointCount() > 0) {
            double signal_min, signal_max;
            signal.second->computeDataRangeY(signal_min, signal_max);
            min = std::min(min, signal_min);
            max = std::max(max, signal_max);
            hasData = true;
        }
    }

    if (!hasData) {
        min = -1.0;
        max = 1.0;
        return false;
    }
    return true;
}

bool Plot2D::Scale::computeDataRangeXY(Eigen::Vector2d &min, Eigen::Vector2d &max)
{
    min.fill(std::numeric_limits<double>::max());
    max.fill(std::numeric_limits<double>::lowest());
    bool hasData = false;

    for (const auto &signal : Signals) {
        if (signal.second->isVisible() && signal.second->pointCount() > 0) {
            Eigen::Vector2d signal_min, signal_max;

            signal.second->computeDataRangeXY(signal_min, signal_max);
            min = min.cwiseMin(signal_min);
            max = max.cwiseMax(signal_max);
            hasData = true;
        }
    }

    if (!hasData) {
        min.fill(-1.0);
        max.fill(1.0);
        return false;
    }
    return true;
}

void Plot2D::Scale::setContinuousFitX(bool fit)
{
    this->ContinuousFitX = fit;
}

void Plot2D::Scale::setContinuousFitY(bool fit)
{
    this->ContinuousExpandY = false;
    this->ContinuousFitY = fit;
}

void Plot2D::Scale::setContinuousExpandY(bool expand)
{
    this->ContinuousFitY = false;
    this->ContinuousExpandY = expand;
    // reset Y range
    this->ExpandYMin = std::numeric_limits<double>::max();
    this->ExpandYMax = std::numeric_limits<double>::lowest();
}

void Plot2D::Scale::setFrozen(bool freeze)
{
    for (auto &signal : Signals) {
        signal.second->setFrozen(freeze);
    }
}

bool Plot2D::Scale::isFrozen() const
{
    for (auto &signal : Signals) {
        if (!signal.second->isFrozen()) {
            return false;
        }
    }

    return true;
}

void Plot2D::Scale::setColor(const Eigen::Vector3d &colorInRange0To1)
{
    Color = colorInRange0To1.cwiseMax(0.0).cwiseMin(1.0);
}

Plot2D::Signal *Plot2D::Scale::addSignal(const std::string &name)
{
    // check if the name already exists
    decltype(Signals)::const_iterator found = Signals.find(name);
    if (found != Signals.end()) {
        return nullptr;
    }

    Signals[name] = std::make_unique<Signal>(name, NumberOfPoints);
    Signals[name]->Parent = this;
    return Signals[name].get();
}

bool Plot2D::Scale::removeSignal(const std::string &name)
{
    decltype(Signals)::const_iterator found = Signals.find(name);
    if (found == Signals.end()) {
        return false;
    }

    Signals.erase(found);
    return true;
}

bool Plot2D::Scale::removeSignal(const Signal *signal)
{
    if (signal == nullptr) {
        return false;
    }

    return this->removeSignal(signal->name());
}

Plot2D::VerticalLine *Plot2D::Scale::addVerticalLine(const std::string &name)
{
    // check if the name already exists
    const auto found = VerticalLines.find(name);
    if (found != VerticalLines.end()) {
        return nullptr;
    }

    VerticalLines[name] = std::make_unique<VerticalLine>(name);
    return VerticalLines[name].get();
}

void Plot2D::Scale::continuousUpdate()
{
    if (this->ContinuousFitX) {
        if (this->ContinuousFitY) {
            this->autoFitXY();
        } else if (this->ContinuousExpandY) {
            this->autoFitXExpandY();
        } else {
            this->autoFitX();
        }
    } else {
        if (this->ContinuousFitY) {
            this->autoFitY();
        } else if (this->ContinuousExpandY) {
            this->autoExpandY();
        }
    }
}

Plot2D::Signal::Signal(const std::string &name, size_t numberOfPoints)
    : Name(name), Empty(true), Visible(true), Frozen(false), Color(1.0, 1.0, 1.0), LineWidth(1.0),
      data(numberOfPoints), IndexFirst(0), IndexLast(0)
{
}

Plot2D::Signal *Plot2D::Signal::addSignal(const std::string &name)
{
    if (!Parent) {
        return nullptr;
    }
    return Parent->addSignal(name);
}

Plot2D::Scale *Plot2D::Signal::parentScale()
{
    return Parent;
}

const std::string &Plot2D::Signal::name() const
{
    return this->Name;
}

void Plot2D::Signal::appendPoint(const Eigen::Vector2d &point)
{
    if (Frozen || data.empty()) {
        return;
    }

    // look where to store this point
    if (!this->Empty) {
        IndexLast = (IndexLast + 1) % data.size();

        if (IndexFirst == IndexLast) {
            IndexFirst = (IndexFirst + 1) % data.size();
        }
    } else {
        this->Empty = false;
    }

    data[IndexLast] = {point.x(), point.y()};
}

Eigen::Vector2d Plot2D::Signal::pointAt(size_t index) const

{
    if (index >= pointCount()) {
        throw std::runtime_error("Plot2D::Signal::pointAt: index bigger than pointCount");
    }
    index = ((index + this->IndexFirst) % data.size());
    return Eigen::Vector2d(data[index][0], data[index][1]);
}

void Plot2D::Signal::setPointAt(size_t index, const Eigen::Vector2d &point)

{
    if (index >= pointCount()) {
        throw std::runtime_error("Plot2D::Signal::setPointAt: index bigger than pointCount");
    }
    index = ((index + this->IndexFirst) % data.size());
    data[index] = {point.x(), point.y()};
    return;
}

void Plot2D::Signal::setFrozen(bool freeze)
{
    if (freeze != this->Frozen) {
        this->Frozen = freeze;
        if (!freeze) {
            this->IndexFirst = 0;
            this->IndexLast = 0;
            this->Empty = true;
        }
    }
}

bool Plot2D::Signal::isFrozen() const
{
    return this->Frozen;
}

void Plot2D::Signal::computeDataRangeXY(Eigen::Vector2d &min, Eigen::Vector2d &max) const
{
    if (Empty) {
        min.setZero();
        max.setZero();
        return;
    }
    min.setConstant(std::numeric_limits<double>::max());
    max.setConstant(std::numeric_limits<double>::lowest());
    for (size_t i = 0; i < pointCount(); ++i) {
        const auto &point = data[(IndexFirst + i) % data.size()];
        for (size_t axis = 0; axis < 2; ++axis) {
            min[axis] = std::min(min[axis], point[axis]);
            max[axis] = std::max(max[axis], point[axis]);
        }
    }
}

void Plot2D::Signal::computeDataRangeX(double &min, double &max, bool assumesDataSorted) const
{
    if (Empty) {
        min = max = 0.0;
        return;
    }
    if (assumesDataSorted) {
        min = data[IndexFirst][0];
        max = data[IndexLast][0];
        return;
    }
    Eigen::Vector2d low, high;
    computeDataRangeXY(low, high);
    min = low.x();
    max = high.x();
}

void Plot2D::Signal::computeDataRangeY(double &min, double &max) const
{
    Eigen::Vector2d low, high;
    computeDataRangeXY(low, high);
    min = low.y();
    max = high.y();
}

size_t Plot2D::Signal::capacity() const
{
    return data.size();
}

size_t Plot2D::Signal::pointCount() const
{
    if (Empty) {
        return 0;
    }
    return (IndexLast + data.size() - IndexFirst) % data.size() + 1;
}

void Plot2D::Signal::setSize(size_t numberOfPoints)
{
    data.assign(numberOfPoints, {0.0, 0.0});
    IndexFirst = IndexLast = 0;
    Empty = true;
}

bool Plot2D::Signal::isVisible() const
{
    return this->Visible;
}

void Plot2D::Signal::setVisible(const bool visible)
{
    this->Visible = visible;
}

void Plot2D::Signal::resize(size_t numberOfPoints, bool trimOlder)
{
    if (data.size() == numberOfPoints) {
        return;
    }
    const size_t count = std::min(pointCount(), numberOfPoints);
    const size_t skip = trimOlder ? pointCount() - count : 0;
    std::vector<std::array<double, 2>> replacement(numberOfPoints);
    for (size_t i = 0; i < count; ++i) {
        replacement[i] = data[(IndexFirst + skip + i) % data.size()];
    }
    data = std::move(replacement);
    IndexFirst = 0;
    IndexLast = count ? count - 1 : 0;
    Empty = count == 0;
}

void Plot2D::Signal::setColor(const Eigen::Vector3d &colorInRange0To1)
{
    Color = colorInRange0To1.cwiseMax(0.0).cwiseMin(1.0);
}

Plot2D::VerticalLine::VerticalLine(const std::string &name, const double x)
    : Name(name), X(x), Visible(true), Color(1.0, 1.0, 1.0), LineWidth(1.0)
{
}

void Plot2D::VerticalLine::setX(const double x)
{
    this->X = x;
}

void Plot2D::VerticalLine::setColor(const Eigen::Vector3d &colorInRange0To1)
{
    Color = colorInRange0To1.cwiseMax(0.0).cwiseMin(1.0);
}

Plot2D::Plot2D()
    : NumberOfPoints(200), Viewport(Eigen::Vector2d::Ones()), BackgroundColor(0.1, 0.1, 0.1)
{
    setNumberOfPoints(200);
    setContinuousFitX(true);
    setContinuousFitY(true);
}

void Plot2D::setNumberOfPoints(size_t numberOfPoints)
{
    this->NumberOfPoints = numberOfPoints;
    if (this->NumberOfPoints < 3) {
        this->NumberOfPoints = 3;
    }
    for (auto &scale : Scales) {
        scale.second->NumberOfPoints = this->NumberOfPoints;
        for (auto &signal : scale.second->Signals) {
            signal.second->resize(this->NumberOfPoints);
        }
    }
}

void Plot2D::autoFitX(double padding)
{
    for (auto &scale : Scales) {
        scale.second->autoFitX(padding);
    }
}

void Plot2D::fitX(double min, double max, double padding)
{
    for (auto &scale : Scales) {
        scale.second->fitX(min, max, padding);
    }
}

void Plot2D::autoFitY(double padding)
{
    for (auto &scale : Scales) {
        scale.second->autoFitY(padding);
    }
}

void Plot2D::fitY(double min, double max, double padding)
{
    for (auto &scale : Scales) {
        scale.second->fitY(min, max, padding);
    }
}

void Plot2D::setFrozen(bool freeze)
{
    for (auto &scale : Scales) {
        scale.second->setFrozen(freeze);
    }
}

bool Plot2D::isFrozen() const
{
    bool allFrozen = true;
    for (auto &scale : Scales) {
        allFrozen = allFrozen && scale.second->isFrozen();
    }

    return allFrozen;
}

void Plot2D::setContinuousFitX(bool fit)
{
    for (auto &scale : Scales) {
        scale.second->setContinuousFitX(fit);
    }
}

void Plot2D::setContinuousFitY(bool fit)
{
    for (auto &scale : Scales) {
        scale.second->setContinuousFitY(fit);
    }
}

void Plot2D::setContinuousExpandY(bool expand)
{
    for (auto &scale : Scales) {
        scale.second->setContinuousExpandY(expand);
    }
}

bool Plot2D::continuousFitX() const
{
    bool continuous = true;
    for (auto &scale : Scales) {
        continuous = continuous && scale.second->continuousFitX();
    }
    return continuous;
}

bool Plot2D::continuousFitY() const
{
    bool continuous = true;
    for (auto &scale : Scales) {
        continuous = continuous && scale.second->continuousFitY();
    }
    return continuous;
}

bool Plot2D::continuousExpandY() const
{
    bool continuous = true;
    for (auto &scale : Scales) {
        continuous = continuous && scale.second->continuousExpandY();
    }
    return continuous;
}

void Plot2D::continuousUpdate()
{
    for (auto &scale : Scales) {
        scale.second->continuousUpdate();
    }
}

void Plot2D::setBackgroundColor(const Eigen::Vector3d &colorInRange0To1)
{
    BackgroundColor = colorInRange0To1.cwiseMax(0.0).cwiseMin(1.0);
}

Plot2D::Scale *Plot2D::addScale(const std::string &name)
{
    // check if the name already exists
    const auto found = Scales.find(name);
    if (found != Scales.end()) {
        return nullptr;
    }

    Scales[name] = std::make_unique<Scale>(name, NumberOfPoints);
    Scales[name]->Viewport = this->Viewport;
    return Scales[name].get();
}

Plot2D::Scale *Plot2D::findScale(const std::string &name)
{
    const auto found = Scales.find(name);
    if (found == Scales.end()) {
        return nullptr;
    }
    return found->second.get();
}

bool Plot2D::removeScale(const std::string &name)
{
    const auto found = Scales.find(name);
    if (found == Scales.end()) {
        return false;
    }
    Scales.erase(found);
    return true;
}

bool Plot2D::removeScale(const Scale *scale)
{
    if (scale) {
        return this->removeScale(scale->name());
    }
    return false;
}

} // namespace EigenQt

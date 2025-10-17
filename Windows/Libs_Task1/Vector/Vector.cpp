#include "pch.h"
#include "Vector.h"
#include <cmath>

namespace VectorSpace {
    Vector::Vector(const NumberSpace::Number& x_, const NumberSpace::Number& y_) : x(x_), y(y_) {}

    void Vector::setCoordinatesOfVector(const NumberSpace::Number& x_, const NumberSpace::Number& y_)
    {
        this->x = x_;
        this->y = y_;
    }

    Vector Vector::ZERO_VECTOR()
    {
        return Vector(NumberSpace::Number::ZERO, NumberSpace::Number::ZERO);
    }
    Vector Vector::ONE_VECTOR() {
        return Vector(NumberSpace::Number::ONE, NumberSpace::Number::ONE);
    }

    Vector Vector::v_add(const Vector& other) const
    {
        NumberSpace::Number sum_x = this->x.add(other.x);
        NumberSpace::Number sum_y = this->y.add(other.y);
        return Vector(sum_x, sum_y);
    }

    NumberSpace::Number Vector::polarRadius() const
    {
        double x = this->x.getValue();
        double y = this->y.getValue();
        return NumberSpace::Number(sqrt(x * x + y * y));
    }
    NumberSpace::Number Vector::polarAngle() const
    {
        double x = this->x.getValue();
        double y = this->y.getValue();
        return NumberSpace::Number(atan2(y, x));
    }

}
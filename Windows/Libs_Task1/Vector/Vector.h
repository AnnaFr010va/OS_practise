#pragma once

#ifdef VECTOR_EXPORTS
#define VECTOR_API __declspec(dllexport)
#else
#define VECTOR_API __declspec(dllimport)
#endif

//#include "Number.h"
#include "..\Number\Number.h"

namespace VectorSpace {

    class VECTOR_API Vector {
    private:
        NumberSpace::Number x;
        NumberSpace::Number y;
    public:
        Vector() = default;
        ~Vector() = default;
        Vector(const NumberSpace::Number& x, const NumberSpace::Number& y);
        NumberSpace::Number getX() { return this->x; };
        NumberSpace::Number getY() { return this->y; };
        void setCoordinatesOfVector(const NumberSpace::Number& x_, const NumberSpace::Number& y_);
        Vector v_add(const Vector& other) const;
        NumberSpace::Number polarRadius() const;
        NumberSpace::Number polarAngle() const;
        static Vector ZERO_VECTOR();
        static Vector ONE_VECTOR();
    };
}
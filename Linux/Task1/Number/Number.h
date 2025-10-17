#pragma once

#include <initializer_list>
#include <stdexcept>

namespace NumberSpace {
    class Number {
    private:
        double value;
    public:
        static const Number ZERO;
        static const Number ONE;

        Number(double value_ = 0.0);
        ~Number() = default;

        double getValue() const;
        void setValue(double newValue);

        Number add(const Number& other) const;
        Number subtract(const Number& other) const;
        Number multiply(const Number& other) const;
        Number divide(const Number& other) const;

        static double add(double num1, std::initializer_list<double> other_numbers);
        static double subtract(double num1, std::initializer_list<double> other_numbers);
        static double multiplication(double num1, std::initializer_list<double> other_numbers);
        static double division(double num1, double num2);
    };
}

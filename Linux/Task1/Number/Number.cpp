#include "Number.h"

namespace NumberSpace {
    const Number Number::ZERO = 0;
    const Number Number::ONE = 1;

    Number::Number(double value_) : value(value_) {}

    double Number::getValue() const
    {
        return value;
    }
    void Number::setValue(double value_)
    {
        value = value_;
    }

    Number Number::add(const Number& other) const {
        return Number(this->value + other.value);
    }

    Number Number::subtract(const Number& other) const {
        return Number(this->value - other.value);
    }

    Number Number::multiply(const Number& other) const {
        return Number(this->value * other.value);
    }

    Number Number::divide(const Number& other) const {
        if (other.value == 0) throw std::invalid_argument("Error: division by zero");
        return Number(this->value / other.value);
    }

    double Number::add(double num1, std::initializer_list<double> other_numbers) {
        double total_sum = num1;
        for (double num : other_numbers) {
            total_sum += num;
        }
        return total_sum;
    }

    double Number::subtract(double num1, std::initializer_list<double> other_numbers) {
        double result = num1;
        for (double num : other_numbers) {
            result -= num;
        }
        return result;
    }

    double Number::multiplication(double num1, std::initializer_list<double> other_numbers) {
        double result = num1;
        for (double num : other_numbers) {
            result *= num;
        }
        return result;
    }

    double Number::division(double num1, double num2) {
        if (num2 == 0) throw std::invalid_argument("Error: division by zero");
        return num1 / num2;
    }
}

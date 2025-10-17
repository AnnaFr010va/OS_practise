#include <iostream>
#include "Number.h"
#include "Vector.h"

int main() {
    std::cout << "1. Testing static Number methods:\n";
    double a = 3.5;
    double b = 4.5;
    double c = 7.0;
    std::cout << "   " << a << " + " << b << " + " << c << " = "
        << NumberSpace::Number::add(a, { b, c }) << std::endl;

    std::cout << "   " << a << " - " << b << " - " << c << " = "
        << NumberSpace::Number::subtract(a, { b, c }) << std::endl;

    std::cout << "   " << a << " * " << b << " * " << c << " = "
        << NumberSpace::Number::multiplication(a, { b, c }) << std::endl;

    std::cout << "   " << a << " / " << b << " = "
        << NumberSpace::Number::division(a, b) << std::endl;
        
    std::cout << "2. Testing Number objects:\n";
    NumberSpace::Number num1(10.5);
    NumberSpace::Number num2(2.5);

    NumberSpace::Number sum = num1.add(num2);
    NumberSpace::Number diff = num1.subtract(num2);
    NumberSpace::Number prod = num1.multiply(num2);
    NumberSpace::Number quot = num1.divide(num2);

    std::cout << "   " << num1.getValue() << " + " << num2.getValue() << " = " << sum.getValue() << std::endl;
    std::cout << "   " << num1.getValue() << " - " << num2.getValue() << " = " << diff.getValue() << std::endl;
    std::cout << "   " << num1.getValue() << " * " << num2.getValue() << " = " << prod.getValue() << std::endl;
    std::cout << "   " << num1.getValue() << " / " << num2.getValue() << " = " << quot.getValue() << std::endl;

    std::cout << "3. Testing Vector creation:\n";
    VectorSpace::Vector vec1(NumberSpace::Number(3.0), NumberSpace::Number(4.0));
    VectorSpace::Vector vec2(NumberSpace::Number(1.0), NumberSpace::Number(2.0));

    std::cout << "   vec1: (" << vec1.getX().getValue() << ", " << vec1.getY().getValue() << ")\n";
    std::cout << "   vec2: (" << vec2.getX().getValue() << ", " << vec2.getY().getValue() << ")\n";

    std::cout << "4. Testing Vector addition:\n";
    VectorSpace::Vector vec_sum = vec1.v_add(vec2);
    std::cout << "   vec1 + vec2 = (" << vec_sum.getX().getValue() << ", " << vec_sum.getY().getValue() << ")\n";

    std::cout << "5. Testing polar coordinates:\n";
    NumberSpace::Number radius = vec1.polarRadius();
    NumberSpace::Number angle = vec1.polarAngle();

    std::cout << "   vec1 polar radius: " << radius.getValue() << std::endl;
    std::cout << "   vec1 polar angle: " << angle.getValue() << " radians" << std::endl;
    std::cout << "   vec1 polar angle: " << angle.getValue() * 180.0 / 3.14159 << " degrees" << std::endl;

    std::cout << "6. Testing vector with negative coordinates:\n";
    VectorSpace::Vector vec3(NumberSpace::Number(-3.0), NumberSpace::Number(-4.0));
    NumberSpace::Number radius3 = vec3.polarRadius();
    NumberSpace::Number angle3 = vec3.polarAngle();

    std::cout << "   vec3: (" << vec3.getX().getValue() << ", " << vec3.getY().getValue() << ")\n";
    std::cout << "   vec3 polar radius: " << radius3.getValue() << std::endl;
    std::cout << "   vec3 polar angle: " << angle3.getValue() << " radians" << std::endl;
    std::cout << "   vec3 polar angle: " << angle3.getValue() * 180.0 / 3.14159 << " degrees" << std::endl;

    return 0;
}

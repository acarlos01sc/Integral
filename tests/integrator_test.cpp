#include <exception>
#include <iostream>
#include <cmath>

#include <numathap/integrator.h>

int main() {
    try {
        double r1=numathap::integrate("x^2","x","0","1");

        std::cout << "Integral of x^2 from 0 to 1 = " << r1 << " (expected ~0.333333)\n";

        double r2=numathap::integrate("sin(x)","x","0","pi");

        std::cout << "Integral of sin(x) from 0 to pi = " << r2 << " (expected ~2.0)\n";

        double r3=numathap::integrate("sin(x)","x","pi","0");

        std::cout << "Integral of sin(x) from pi to 0 = " << r3 << " (expected ~-2.0)\n";

        double r4=numathap::integrate("1/x","x","1","2");

        std::cout << "Integral of 1/x from 1 to 2 = " << r4 << " (expected ~" << std::log(2.0) << ")\n";
    }
    catch (const std::exception& e) {
        std::cerr << "Integration error: " << e.what() << '\n';
        return 1;
    }

    return 0;
}
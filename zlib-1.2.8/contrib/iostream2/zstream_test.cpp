#include "zstream.h"
#include <math.h>
#include <stdlib.h>
#include <iomanip.h>

void main() {
    char h[256] = "Hello";
    char* g = "Goodbye";
    ozstream out("temp.gz");
    out < "This works well" < h < g;
    out.close();

    izstream in("temp.gz"); // read it back
    char *x = read_string(in), *y = new char[256], z[256];
    in > y > z;
    in.close();
    std::cout << x << std::endl << y << std::endl << z << std::endl;

    out.open("temp.gz"); // try ascii output; zcat temp.gz to see the results
    out << setw(50) << setfill('#') << setprecision(20) << x << std::endl << y << std::endl << z << std::endl;
    out << z << std::endl << y << std::endl << x << std::endl;
    out << 1.1234567890123456789 << std::endl;

    delete[] x; delete[] y;
}

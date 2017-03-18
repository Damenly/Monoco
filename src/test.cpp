#include <iostream>
#include <vector>

using namespace std;

int
main()
{
	double x1, x2, x3, x;
	double y1, y2, y3, y;
	double foo;
	double u, v;

	cin >> x1 >> y1 >> foo;
	cin >> x2 >> y2 >> foo;
	cin >> x3 >> y3 >> foo;
	cin >> x >> y;

	foo = y1 - y2;
	double bar = x1 - x2;

	if (bar == 0 )  {
		v = (x1 - x) / (x1 - x3);
		u = ((y1 - y) - v * (y1 - y3)) / (y1 - y2);
	}
	else if (foo == 0) {
		v = (y1 - y) / (y1 - y3);
		u = ((x1 - x) - v * (x1 - x3)) / (x1 - x2);
	} else {
		double foobar = (y1 - y2) / (x1 - x2);
		v = ((y1 - y) - (x1 - x) * foobar) / (x1 + y1 - x3 - y3);
		u = ((x1 - x) - v * (x1 - x3)) / (x1 -x2);
	}

	float u1 = 1 - u;
	float v1 = 1 - v;
	vec3 b1 = u1*u1*(3-2*u1)*p0 + u*u*(3-2*u)*p1 + 3*u*u1*(u1*n0 + u*n1)*adj;
	vec3 b2 = v1*v1*(3-2*v1)*p0 + v*v*(3-2*v)*p2 + 3*v*v1*(v1*n0 + v*n2)*adj;
	float w = abs(u-v) < 0.0001 ? 0.5 : ( 1 + (u-v)/(u+v) ) * 0.5;
	p = (1-w)*b1 + w*b2;

	cout << (1 - u) * (1 - v)<< endl;
}

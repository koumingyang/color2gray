#ifndef COLOR_H
#define COLOR_H

#include <cmath>

using namespace std;

struct rgb
{
    int r, g, b;
    rgb() {}
	rgb(int r, int g, int b) : r(r), g(g), b(b) {}
	bool operator<(const rgb &c) const {
		if(r < c.r) return true;
		if(r > c.r) return false;
		if(g < c.g) return true;
		if(g > c.g) return false;		
		return b < c.b;
	}
};

inline double clamp(double x, double x_min, double x_max)
{
	if (x < x_min) return(x_min);
	else if (x > x_max) return(x_max);
	else return(x);
}

struct cie_xyz
{
    double x, y, z;
    cie_xyz(double x, double y, double z) : x(x), y(y), z(z) {}

    cie_xyz(const rgb &t)
    {
        double r = (double)(t.r) / 255.0;
        double g = (double)(t.g) / 255.0;
        double b = (double)(t.b) / 255.0;

        x = 0.412453*r + 0.357580*g + 0.180423*b;
		y = 0.212671*r + 0.715160*g + 0.072169*b;
		z = 0.019334*r + 0.119193*g + 0.950227*b;

        const double Xn = 0.9513;
		const double Yn = 1.000;
		const double Zn = 1.0886;

		x /= Xn; y /= Yn; z /= Zn;
    }

    rgb to_rgb()
    {
        double r = 3.240479 * x + -1.537150 * y +  -0.498535 * z;
		double g = -0.969256 * x +  1.875992 * y +  0.041556 * z;
		double b = 0.055648 * x +  -0.204043 * y + 1.057311 * z;
		r = clamp (r, 0.0, 1.0);
		g = clamp (g, 0.0, 1.0);
		b = clamp (b, 0.0, 1.0);

        int tr = (int)(r * 255), tg = (int)(g * 255), tb = (int)(b * 255);
        return rgb(tr, tg, tb);
    }
};

struct cie_lab
{
    double l, a, b;

    cie_lab()   {}
    cie_lab(double l, double a, double b) : l(l), a(a), b(b) {}

    cie_lab(const rgb &t)
    {
        cie_xyz tmp(t);
        *this = cie_lab(tmp);
    }

    cie_lab(cie_xyz &t)
    {
        double x_third = pow(t.x, 1.0/3.0);
        double y_third = pow(t.y, 1.0/3.0);
        double z_third = pow(t.z, 1.0/3.0);

        if (t.y > 0.008856)
			l = (116.0*(y_third)) - 16.0;
		else l = 903.3 * t.y;

		a= 500.0 * ((x_third) - (y_third));
		b= 200.0 * ((y_third) - (z_third));
    }

    rgb to_rgb()
    {
        double p = (l + 16.0) / 116.0;

		// Define as constants
		double Xn = 0.9513;
		double Yn = 1.000;
		double Zn = 1.0886;

		double x = Xn * pow(p + (a/500), 3);
		double y = Yn * pow(p, 3);
		double z = Zn * pow(p - (b/200), 3);

		return cie_xyz(x, y, z).to_rgb();
    }
};

#endif  //COLOR_H
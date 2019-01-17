#ifndef IMAGE_H
#define IMAGE_H

#include <stdexcept>
#include <vector>
#include <algorithm>
#include "color.h"

extern double alpha;
extern double theta;

extern bool quantize;
extern int q_colors;

inline double crunch(double chrom_dist) 
{
	return alpha==0 ? 0 : alpha*tanh(chrom_dist/alpha);
}

inline double sq(double s) { return s*s; }

struct ColorImage {

	cie_lab * data;
	
	typedef std::pair<cie_lab, int> cie_lab_int;
	vector<cie_lab_int> qdata;
	
	int colors;
	
	int w, h, N;

	ColorImage() : data(NULL) {}
	void clean() { delete [] data; }

	double calc_delta(int i, int j) const;
	double calc_qdelta(int i, int p) const;

	void load_quant_data(const char *fname);
	
	double * calc_d();

	double * r_calc_d(int r);
	
	void load(const char * fname);
};

struct GrayImage {

	double * data;
	const int w, h, N;
	int i,j,k;

	//this will shift our data to best match the 
	//luminance channel of s.
	void post_solve(const ColorImage &s);

	GrayImage( ColorImage &s) : data(new double[s.N]), w(s.w), h(s.h), N(s.N) {
		for(i=0;i<N;i++) 
			data[i]=(s.data)[i].l;
	}
	~GrayImage() { delete [] data; }

	void complete_solve(const double *d);
	void r_solve(const double *d, int r);
	
	void save(const char * fname) const;
	void saveColor(const char * fname, const ColorImage &source) const;
};	

#endif  //IMAGE_H

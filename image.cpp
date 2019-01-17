#include <cstdio>
#include <cstdlib>
#include <map>
#include "image.h"

using namespace std;

rgb *readPPM(const char *filename, int *cols, int *rows, int * colors) 
{
    char tag[50];
    rgb *image;
    int read, num[3], curchar;
    bool binary;
    FILE *fp;
    
    if(filename != NULL && strlen(filename))
        fp = fopen(filename, "rb");
    else
        fp = stdin;

    if (!fp)    return (NULL);

    fscanf(fp, "%s\n", tag);

    // Read the "magic number" at the beginning of the ppm
    if (strncmp(tag, "P6", 40) == 0) binary = true;
    else if(strncmp(tag, "P3", 40) == 0) binary = false;
    else throw runtime_error("not a ppm!");

    // Read the rows, columns, and color depth output by cqcam
    // need to read in three numbers and skip any lines that start with a #
    read = 0;
    while(read < 3)
    {
        curchar = fgetc(fp);
        if((char)curchar == '#') 
        { // skip this line
	        while(fgetc(fp) != '\n')
	            /* do nothing */;
        }
        else 
        {
			ungetc(curchar, fp);
			fscanf(fp, "%d", &(num[read]));
			read++;
        }
    }
    while(fgetc(fp) != '\n')
        /* pass the last newline character */;
    
    *cols = num[0];
    *rows = num[1];
    *colors = num[2];

    if(*cols > 0 && *rows > 0) 
    {
        image = new rgb[(*rows) * (*cols)];

		if(image) 
        {
			// Read the data
			if(binary)	
			{
				unsigned char crgb[3 * (*rows) * (*cols)];
				fread(crgb, sizeof(char), 3 * (*rows) * (*cols), fp);
				for(int x=0;x<*rows;x++) 
                    for(int y=0;y<*cols;y++) 
					{
						int tmp = x + *rows * y;
						image[tmp]=rgb((int)(crgb[tmp*3]), (int)(crgb[tmp*3+1]), (int)(crgb[tmp*3+2]));
					}
			}
			else 
            {
				for(int x=0;x<*rows;x++) 
                    for(int y=0;y<*cols;y++) 
                    {
					    int r, g, b;
					    fscanf(fp, "%d %d %d", &r, &g, &b);
					    image[x + *rows * y]=rgb(r, g, b);
				    }
			}

			fclose(fp);

			return image;
		}
    }
    
    return (NULL);
}

void ColorImage::load(const char * fname) 
{		
	if(data) delete [] data;

	int c;
	rgb * colors = readPPM(fname, &w, &h, &c);

	N = w * h;

	/*freopen("image.out", "w", stdout);
	for (int i = 0; i < N; i++)
		printf("%d %d %d\n", colors[i].r, colors[i].g, colors[i].b);
	printf("\n\n\n");*/
	
	printf("image loaded, w: %d, y: %d, c: %d.\n", w, h, c);

	data = new cie_lab[N];

	int i;
	for(i = 0; i < N; i++) data[i] = cie_lab(colors[i]);
	
	/*freopen("load.out", "w", stdout);
	for(int i=0;i<N;i++) 
		printf("%.2lf %.2lf %.2lf\n", data[i].l, data[i].a, data[i].b);
	printf("\n");*/

	delete [] colors;
}

void GrayImage::save(const char *fname) const 
{
	rgb * rval = new rgb[N];
	for(int i = 0; i < N; i++) 
		rval[i]=cie_lab(data[i],0,0).to_rgb();

	//write out the grayscale image.
	FILE *fp;
	fp = fopen(fname, "wb");
	if(fp) 
    {
		unsigned char crgb[3 * w * h];
		for(int x = 0; x < w; x++) 
            for(int y = 0; y < h; y++) 
			{
				int tmp = x + w * y;
				crgb[tmp*3] = (unsigned char)(rval[tmp].r);
				crgb[tmp*3+1] = (unsigned char)(rval[tmp].g);
				crgb[tmp*3+2] = (unsigned char)(rval[tmp].b);
			}
		fprintf(fp, "P6\n");
		fprintf(fp, "%d %d\n%d\n", w, h, 255);
		fwrite(crgb, sizeof(char) * 3, N, fp);
	}
	fclose(fp);

	delete [] rval;
}

void ColorImage::load_quant_data(const char *fname) 
{
	int c;
	int i;	
	rgb * colors = readPPM(fname, &w, &h, &c);
	
	N = w*h;
	printf("quantized image loaded, w: %d, y: %d, c: %d.\n", w, h, c);

	qdata.clear();

	map<rgb, int> q;
	map<rgb, int>::iterator r;
	for( i=0;i<N;i++) 
    {
		r = q.find(colors[i]);
		if(r == q.end()) q[colors[i]]=1;
		else r->second++;
	}
	
	printf("quantized image appears to use %d colors.\n", (int)(q.size()));
	qdata.resize(q.size());
	for(i=0, r=q.begin(); r != q.end(); r++, i++)
		qdata[i] = cie_lab_int(cie_lab(r->first), r->second);

	
	delete [] colors;
}

void GrayImage::saveColor(const char *fname, const ColorImage &source) const {
	rgb * rval = new rgb[N];
	printf("Saving Color2Gray + Chrominance\n");
	for(int i=0; i<N; i++) {
		rval[i] = cie_lab(data[i],((source.data)[i]).a,((source.data)[i]).b).to_rgb();
	}

	/*freopen("log.out", "w", stdout);
	for(int i=0;i<N;i++) 
		printf("%d %d %d\n", rval[i].r, rval[i].g, rval[i].b);
	printf("\n");*/

	FILE *fp;
	fp = fopen(fname, "wb");
	if(fp) {

		unsigned char crgb[3 * w * h];
		for(int x = 0; x < w; x++) 
            for(int y = 0; y < h; y++) 
			{
				int tmp = x + w * y;
				crgb[tmp*3] = (unsigned char)(rval[tmp].r);
				crgb[tmp*3+1] = (unsigned char)(rval[tmp].g);
				crgb[tmp*3+2] = (unsigned char)(rval[tmp].b);
			}
		fprintf(fp, "P6\n");
		fprintf(fp, "%d %d\n%d\n", w, h, 255);
		fwrite(crgb, sizeof(char) * 3, N, fp);
	}
	fclose(fp);

	delete [] rval;
}


double * ColorImage::calc_d() 
{
	double * d = new double [N];
	int i,j;
	for(i = 0; i < N; i++) d[i] = 0;

	if(quantize) 
    {
		int p;
		for(i = 0; i < N; i++) 
            for(p = 0; p < qdata.size(); p++)
				d[i] += calc_qdelta(i, p);
		
	}
	else {
		for(i=0; i < N; i++) 
            for(j= i+1; j < N; j++) 
            {
			    double delta = calc_delta(i, j);
			    d[i] += delta;
			    d[j] -= delta;
		    }
	}
	return d;
}

double * ColorImage::r_calc_d(int r) 
{
	double * d = new double [N];
	int i;
	for(i = 0; i < N; i++) d[i] = 0;

	int x, y;
	for(x = 0; x < w; x++) 
        for(y = 0; y < h; y++) 
        {
		    int xx, yy;
            i = x + y * w;
            for(xx = x - r; xx <= x + r; xx++) 
            {
                if(xx < 0 || xx >= w) continue;
                for(yy = y - r; yy <= y + r; yy++) 		
                {
                    if (yy >= h || yy < 0) continue;	
                    int j = xx + yy * w;
                    double delta = calc_delta(i, j);
                    d[i] += delta;
                    d[j] -= delta;				
                }
		}
	}
	return d;
}

void GrayImage::r_solve(const double *d, int r) 
{
	const int iters = 30;
	int k, x, y;

	for(k = 0; k < iters; k++) 
    {
		printf("iter %d\n.",k);

		//perform a Gauss-Seidel relaxation.
	    for(x = 0; x < w; x++) 
            for(y = 0; y < h; y++) 
            {
                double sum=0;
                int count=0;
                int xx, yy;
                for(xx = x - r; xx <= x + r; xx++) 
                {
                    if(xx < 0 || xx >= w) continue;
                    for(yy = y - r; yy <= y + r; yy++) 		
                    {
                        if (yy >= h || yy < 0) continue;	
                        sum += data[xx + yy * w];
                        count ++;
                    }
                }
			    data[x + y * w]=(d[x + w * y] + sum) / (double)count;
		    }
	}
}


void GrayImage::complete_solve(const double * d) 
{
	for(i = 1; i < N; i++) 
    {
		data[i] = d[i] - d[i-1] + N * data[i-1];
		data[i] /= (double)N;
	}
}

void GrayImage::post_solve(const ColorImage &s) 
{
	double error=0;
	for(i = 0; i < N; i++) 
		error += data[i] - (s.data)[i].l;
	error /= N;
	for(i = 0; i < N; i++) data[i] = data[i] - error;
}

double ColorImage::calc_delta(int i, int j) const 
{
	const cie_lab &a = data[i];
	const cie_lab &b = data[j];

	double dL=a.l-b.l;
	double dC= crunch(sqrt(sq(a.a-b.a)+sq(a.b-b.b)));

	if(fabs(dL)>dC) return dL;
	return dC*(  (cos(theta)*(a.a-b.a) + sin(theta)*(a.b-b.b)) > 0 ? 1 : -1);
}


double ColorImage::calc_qdelta(int i, int p) const 
{
	const cie_lab &a = data[i];
	const cie_lab &b = qdata[p].first;

	double dL=a.l-b.l;
	double dC= crunch(sqrt(sq(a.a-b.a)+sq(a.b-b.b)));

	if(fabs(dL)>dC) return qdata[p].second*dL;
	return qdata[p].second*dC*(  (cos(theta)*(a.a-b.a) + sin(theta)*(a.b-b.b)) > 0 ? 1 : -1);
}

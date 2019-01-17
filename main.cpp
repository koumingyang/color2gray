#include <iostream>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cmath>
#include <ctime>

#include "color.h"
#include "image.h"

using namespace std;

double alpha=4;

//variables controlling the quantization based acceleration.
bool quantize = true;
int q_colors = 64;

//theta is in radians.
double theta = 3.14/6.0;
int r = 0;

const double d2r = 3.14159 / 180.0;
const double r2d = 180.0 / 3.14159;

bool qchoice(char c) 
{
	if(c=='y') 
    {
		quantize=true;
		printf("\nuse how many colors (64 is usually a sensible choice)?\n");
		scanf("%d",&q_colors);
	}
	else if(c=='n') 
    {
		quantize=false;
	}
	else 
    {
		printf("\nplease enter 'y' or 'n'.\n");
		return false;
	}
	return true;
}

int parse_args(int argc, char **argv)
{
    theta = 3.14/4.0;
    alpha = 10;
    r = 0;
    quantize = false;

    for(int i=2; i< argc; i++){
        if (!strcmp(argv[i], "-theta"))
        {
            i++;
            sscanf(argv[i], "%lf", (&theta));
            theta=theta*d2r ;
            printf("Theta = %.2f\n", theta*180/M_PI);
        }
        else if (!strcmp(argv[i], "-alpha"))
        {
            i++;
            sscanf(argv[i], "%lf", &alpha );
            printf("Alpha = %.2f\n", alpha);
        }
        else if (!strcmp(argv[i], "-r"))
        {
            i++;
            sscanf(argv[i], "%d", &r );
            printf("mu = %d\n", r);
        }
        else  if (!strcmp(argv[i], "-q"))
        {
            i++;
            quantize = true;
            sscanf(argv[i], "%d", &q_colors );
            printf("q = %d\n", q_colors);
        }
        else
        {
            i++; 
            printf("%s is not a valid option: color2gray file.ppm -theta 45 -alpha 10 -r 0 -q 0\n", argv[i]);
        }
    }//end of for i on parse args
    fprintf(stderr, "Done with arg parsing\n");
    return(1);
}

int main(int argc, char * argv[]) 
{
    static int PARSE_ARGS = 1;
	
	const char * fname = argc>=2 ? argv[1] : "test.ppm";

	if (PARSE_ARGS)
	    parse_args(argc,argv);
	else 
    {
	    theta = argc>=3 ? atof(argv[2])*d2r : 3.14/4.0;
	    alpha = argc>=4 ? atof(argv[3]) : 10;
        r = argc>=5 ? atoi(argv[4]) : 0;
	}

	//print out an informative message.
	printf("Executing color2gray algorithm on \"%s\"\n with alpha=%lf, theta=%lf ", fname, alpha, theta*r2d);
	if(r==0) 
    {
		printf("(complete case.)\n");
		if (!PARSE_ARGS)
        {
            printf("\nwould you like to use the quantization based inexact version of the algorithm?\n (y/n)? \t(requires image magick's 'convert' in the path.)\n");
            char c; 
            scanf("%c",&c);
            while(!qchoice(c)) scanf("%c",&c);
		}
	}
	else printf(", r=%d\n",r);

	char outname[256];
	char outname_color[256];
	// Create a meaningful output file name for the color2gray image:
	// Note: this only works if the filename is just the file and doesn't include a directory path
	if (r == 0)
		sprintf(outname, "c2g_theta%.1f_a%.1f_mu%s_q%d.%d_%s",theta*r2d,alpha,"FULL",quantize,q_colors,fname);
	else
		sprintf(outname, "c2g_theta%.1f_a%.1f_mu%d_q%d.%d_%s",theta*r2d,alpha,r,quantize,q_colors,fname);

	if (r == 0)
		sprintf(outname_color, "c2gC_theta%.1f_a%.1f_mu%s_q%d.%d_%s",theta*r2d,alpha,"FULL",quantize,q_colors,fname);
	else
		sprintf(outname_color, "c2gC_theta%.1f_a%.1f_mu%d_q%d.%d_%s",theta*r2d,alpha,r,quantize,q_colors,fname);
	printf("Color2Gray result will be output to: \n\t%s\n and \t%s\n", outname, outname_color); 

	//load the image
	ColorImage source;
	source.load(fname);
	GrayImage dest(source);	

	clock_t start = clock();

	//solve, either using the complete case or the neighboorhod case.
	double *d;
	if(r) 
    { 
		d = source.r_calc_d(r);
		dest.r_solve(d,r);
	}
	else 
    {
		if(quantize) 
        {
			char magick_string[256];
			sprintf(magick_string,"convert -colors %d %s quant-%s",q_colors,fname,fname);
			printf("\nUsing image magick to create a quantized image: \n%s\n",magick_string);
			system(magick_string);
			printf("done.\n\n");
			sprintf(magick_string,"quant-%s",fname);
			source.load_quant_data(magick_string);
		}
	
		d = source.calc_d();
		dest.complete_solve(d);
	}
	dest.post_solve(source);

	clock_t end = clock();

	printf("\nc2g completed in %.03f seconds.\n",(end-start)/(double)CLOCKS_PER_SEC);

	dest.save(outname);
	dest.saveColor(outname_color, source);

	delete [] d;
	source.clean();
		
	return 0;
}


#include "stdafx.h"
#include "decolorize.h"

namespace dec
{

const int maxn = 2048;

Mat img, imgRes;

int h, w, n;

double res[maxn*maxn];
double delta[maxn*maxn];
double ori[maxn*maxn][3];	// in CIE l*a*b* color space

char* titleSrc = "Decolorize :: source";
char* titleDest = "Decolorize :: result";

void work();

double theta = 45, alpha = 10;

void redraw()
{
	imshow(titleSrc, img);
	imshow(titleDest, imgRes);
}

void init(CString fname)
{
	char tmps[256];
	strcpy(tmps, (LPCSTR)fname);
	img = imread(tmps, IMREAD_COLOR);
	namedWindow(titleSrc, WINDOW_NORMAL);
	namedWindow(titleDest, WINDOW_NORMAL);
	h = img.rows;
	w = img.cols;
	n = w * h;
	imgRes = Mat::zeros(h, w, CV_8UC1);
	work();
	redraw();
}

// convert rgb color space to L*A*B color space
void rgb2lab(Vec3b& col, double *res)
{
	double B = col[0] / 255.0, G = col[1] / 255.0, R = col[2] / 255.0;
	double x = (0.412453f*R + 0.357580f*G + 0.180423f*B) / 0.9513;
	double y = (0.212671f*R + 0.715160f*G + 0.072169f*B) / 1.0000;
	double z = (0.019334f*R +  0.119193f*G + 0.950227f*B) / 1.0886;
	double& l = res[0], &a = res[1], &b = res[2];
	double xx = pow(x, 1.0/3);
	double yy = pow(y, 1.0/3);
	double zz = pow(z, 1.0/3);
	if (y > 0.008856)
		l = 116.0 * yy - 16.0;
	else
		l = 903.3 * y;
	a = 500 * (xx - yy);
	b = 200 * (yy - zz);
}

int lab2rgb(double res)
{
	double p = (res + 16) / 116;
	double ppp = p * p * p;
	double x = 0.9513 * ppp;
	double y = ppp;
	double z = 1.0886 * ppp;
	double r = 3.240479f*x + -1.537150f*y +  -0.498535f*z;
	return rectify(r * 255);
}

#define crunch(a)	(abs(alpha)<1e-7?0:alpha*tanh((a)/alpha))

int now;

DWORD WINAPI calcDelta(LPVOID params)
{
	while(now < n)
	{
		int i = now++;
		for(int j = i+1; j < n; ++j)
		{
			double* p = ori[i];
			double* q = ori[j];
			double dl = p[0] - q[0];
			double da = p[1] - q[1];
			double db = p[2] - q[2];
			// color difference
			double dc = crunch(sqrt(da * da + db * db));
			double vtheta = cos(theta) * da + sin(theta) * db;
			double tmp;
			if(abs(dl) > dc)
				tmp = dl;
			else
				tmp = dc * (vtheta>0?1:-1);
			delta[i] += tmp;
			delta[j] -= tmp;
		}
	}
	return 0;
}

void work()
{
	// convert from rgb to lab
	for(int i = 0; i < n; ++i)
	{
		rgb2lab(getcol(img, i/h, i%h), ori[i]);
		res[i] = delta[i] = 0;
	}

	// calculate delta
	HANDLE thd[8];
	now = 0;
	for(int i = 0; i < 8; ++i)
		thd[i] = CreateThread(0, 0, calcDelta, 0, 0, 0);
	for(int i = 0; i < 8; ++i)
		WaitForSingleObject(thd[i], INFINITE);
	// calculate result
	for(int i = 1; i < n; ++i)
		res[i] = ( delta[i] - delta[i-1] + n * res[i-1]) / n;
	double sum = 0;
	for(int i = 0; i < n; i++) 
		sum += res[i] - ori[i][0];
	sum /= n;
	for(int i = 0; i < n; i++)
		res[i] = res[i] - sum;

	// output img
	for(int i = 0; i < n; ++i)
	{
		imgRes.at<uchar>(Point(i/h, i%h)) = lab2rgb(res[i]);
	}
}

void onThetaChanged(int _theta)
{
	theta = _theta;
	work();
	redraw();
}

void onAlphaChanged(int _alpha)
{
	alpha = _alpha;
	work();
	redraw();
}

}

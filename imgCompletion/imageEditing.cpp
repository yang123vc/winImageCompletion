
#include "stdafx.h"
#include "imageEditing.h"

namespace poi
{
	char* titleSrc = "poisson source img";
	char* titleDest = "poisson target img";

	Mat imgSrc, imgDest;
	Mat cov;	// a cov layer for img Src
	Mat imgDispSrc;	// the image actually displayed as source img
	Mat imgDispDest;
	
	int srcw, srch;
	int destw, desth;

	int mode;
	
	int moused = 0;
	int mousex, mousey;
	
	int brushr = 20;

	void redraw()
	{
		imshow(titleSrc, imgDispSrc);
		imshow(titleDest, imgDispDest);
	}

	void onMouse( int event, int x, int y, int, void* )
	{
		int bo = 0;
		switch(event)
		{
		case CV_EVENT_LBUTTONDOWN:
			bo = 1;
			mousex = x, mousey = y;
			moused = 1;
			break;
		case CV_EVENT_LBUTTONUP:
			bo = 0;
			moused = 0;
			break;
		case CV_EVENT_RBUTTONDOWN:
			bo = 1;
			mousex = x, mousey = y;
			moused = 2;
			break;
		case CV_EVENT_RBUTTONUP:
			bo = 0;
			moused = 0;
			break;
		case CV_EVENT_MOUSEMOVE:
			bo = moused;
			break;
		}
		if( bo )
		{
			line(cov, Point(mousex, mousey), Point(x, y), (moused==1?Scalar(0,0,1):Scalar(1,1,1)),brushr);
			//circle(cov, Point(x, y), brushr, Scalar(0), CV_FILLED);
			imgDispSrc = imgSrc.clone().mul(cov);
			mousex = x, mousey = y;
			redraw();
		}
	}

	void poi_init(CString fsrc, CString fdest)
	{
		char tmps[256];
		strcpy(tmps, (LPCSTR)fsrc);
		imgSrc = imread(tmps, IMREAD_COLOR);
		strcpy(tmps, (LPCSTR)fdest);
		imgDest = imread(tmps, IMREAD_COLOR);

		srcw = imgSrc.cols;
		srch = imgSrc.rows;

		cov = Mat::ones(srch, srcw, CV_8UC3);

		destw = imgDest.cols;
		desth = imgDest.rows;

		namedWindow(titleSrc , WINDOW_NORMAL ); // Create a window for display.
		namedWindow(titleDest , WINDOW_NORMAL ); // Create a window for display.

		setMouseCallback( titleSrc, onMouse, 0 );
		
		for(int i = 0; i < srcw; ++i)
			for(int j = 0; j < srch; ++j)
			{
				Vec3b& col = cov.at<Vec3b>(Point(i,j));
				col[0] = col[1] = col[2] = 1;
			}
		imgDispSrc = imgSrc.clone();
		imgDispDest = imgDest.clone();
		redraw();
	}

	void poi_work(int _mode)
	{
		mode = _mode;
	}

}
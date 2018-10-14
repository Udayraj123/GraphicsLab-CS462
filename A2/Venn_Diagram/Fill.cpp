#include <windows.h>
#include "Fill.h"
#include <math.h>
#include <iostream>
#include <vector>
#include <algorithm>
#include <utility>
using namespace std;

void fill();
/* function to enable and disable the options in the drop down menu depending upon the mode*/
void setupMenus(HWND hwnd)
{
	HMENU hmenu, hsubmenu;
	hmenu = GetMenu(hwnd);
	hsubmenu = GetSubMenu(hmenu, 0);

	switch (gDrawData.drawMode)
	{
	case READY_MODE:
	case DRAW_MODE:
		// enable 'Draw Polygon', disable 'Filling' menu
		EnableMenuItem(hsubmenu, ID_SET_RADII,
			MF_BYCOMMAND | MF_ENABLED);
		break;
	case FILL_MODE:
	case FILLED_MODE:
		// disable 'Draw Circle', 'Filling' menu 
		EnableMenuItem(hsubmenu, ID_SET_RADII,
			MF_BYCOMMAND | MF_GRAYED);
		break;
	case DRAWN_MODE:
		// enable 'Filling' menus, disable 'Draw Cirle' menu
		EnableMenuItem(hsubmenu, ID_SET_RADII,
			MF_BYCOMMAND | MF_GRAYED);
		break;
	}
}
/*This function calls fil function to colour the cirlces */
void performFilling(HWND hwnd)
{
	setDrawMode(FILL_MODE, hwnd);	
	fill();
	reDraw(hwnd);
	setDrawMode(FILLED_MODE, hwnd);
}

/*if filling is clicked then it calls performFilling function*/
void processCommand(int cmd, HWND hwnd)
{
	switch (cmd)
	{
	case ID_FILL:
		performFilling(hwnd);
		break;
	default:
		processCommonCommand(cmd, hwnd);
		break;
	}
}

/* function which checksif a point lies inside,outside or on the circle1 */
bool circle1(int x, int y) {
	int cx = gDrawData.centerPts[0].x;
	int cy = gDrawData.centerPts[0].y;
	int r = gDrawData.r1;
	if ((x - cx)*(x - cx) + (y - cy)*(y - cy) - (r)*(r) <= 0)
		return true;
	else
		return false;
}
/* function which checksif a point lies inside,outside or on the circle2 */
bool circle2(int x, int y) {
	int cx = gDrawData.centerPts[1].x;
	int cy = gDrawData.centerPts[1].y;
	int r = gDrawData.r2;
	if ((x - cx)*(x - cx) + (y - cy)*(y - cy) - (r)*(r) <= 0)
		return true;
	else
		return false;
}
/* function which checksif a point lies inside,outside or on the circle3 */
bool circle3(int x, int y) {
	int cx = gDrawData.centerPts[2].x;
	int cy = gDrawData.centerPts[2].y;
	int r = gDrawData.r3;
	if ((x - cx)*(x - cx) + (y - cy)*(y - cy) - (r)*(r) <= 0)
		return true;
	else
		return false;
}

/*main function which fills the circles*/
void fill() {
	int x = 0, i;
	float sq;
	vector<int > cx(3), cy(3), sq_r;
	/* cyi are the y coordinates of the center of the circles and cxi are the x coordinates.*/
	for (i = 0; i < 3; ++i)
	{
		cx[i] = gDrawData.centerPts[i].x;
		cy[i] = gDrawData.centerPts[i].y;
	}
	sq_r.push_back(gDrawData.r1 * gDrawData.r1);
	sq_r.push_back(gDrawData.r2 * gDrawData.r2);
	sq_r.push_back(gDrawData.r3 * gDrawData.r3);

	/* ymin and ymax are the y values for which scan_lines need to be processed*/
	int ymin = min(cy[0] - gDrawData.r1, min(cy[1] - gDrawData.r2, cy[2] - gDrawData.r3));
	int ymax = max(cy[0] + gDrawData.r1, max(cy[1] + gDrawData.r2, cy[2] + gDrawData.r3));
	int scan_y = ymin;
	/* scanning the lines here and checking the intersections*/

	const int intersectionClr[] = {
		0, 	//000
		0, 	//001
		0, 	//010
		1, 	//011 XY
		0, 	//100 
		3, 	//101 XZ
		2, 	//110 YZ
		4, 	//111 XYZ
	};
	while (scan_y <= ymax) {
		vector<int > p;
		// Update circles' intersections to scanline according to scan_y
		for (i = 0; i < 3; ++i) {
			sq = (sq_r[i] - (scan_y - cy[i])*(scan_y - cy[i]));
			if (sq >= 0) {
				sq = sqrt(sq);
				// if on the right, ceil
				p.push_back(cx[i] + (int)ceil(sq));
				// else floor
				p.push_back(cx[i] - (int)floor(sq));
			}
		}

		if (p.size())
		{
			sort(p.begin(), p.end());

			for (i = 0; i< p.size() - 1; i++)
			{
				int mid = (p[i] + p[i + 1]) / 2; //any point between the two would work
				int bit3 = 0;
				if (circle1(mid, scan_y))bit3 |= 1 << 0;
				if (circle2(mid, scan_y))bit3 |= 1 << 1;
				if (circle3(mid, scan_y))bit3 |= 1 << 2;
				//dont draw if pt not in any circle 
				if (!bit3)
					continue;
				// gDrawData.hFillPen = CreatePen(PS_SOLID, 1, intersectionClr[bit3]);
				SelectObject(gDrawData.hdcMem, gDrawData.hFillPens[intersectionClr[bit3]]);
				MoveToEx(gDrawData.hdcMem, p[i], scan_y, NULL);
				LineTo(gDrawData.hdcMem, p[i + 1] + 1, scan_y);
			}
		}
		scan_y++;
	}


}


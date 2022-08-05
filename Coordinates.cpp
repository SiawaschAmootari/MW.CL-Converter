#include "pch.h"
#include "Coordinates.h"
#include "framework.h"
#include "MW.CL Converter.h"
#include "MW.CL ConverterDlg.h"
#include "ConvertHeidenhain.h"
#include "afxdialogex.h"
#include <afxcoll.h>
#include <iostream>
#include <string>
#include <fstream>
#include <map>
#include <vector>
#include <algorithm>
#include <thread>
#include <atlstr.h>
Coordinates::Coordinates()
{
}

void Coordinates::setX_coordinate(CString string)
{
	x_coordinate = string;
}

void Coordinates::setY_coordinate(CString string)
{
	y_coordinate = string;
}

void Coordinates::setZ_coordinate(CString string)
{
	z_coordinate = string;
}

void Coordinates::setA_coordinate(CString string)
{
	a_coordinate = string;
}

void Coordinates::setC_coordinate(CString string)
{
	c_coordinate = string;
}

void Coordinates::setA_matrix(CString string)
{
	a_matrix = string;
}

void Coordinates::setC_matrix(CString string)
{
	c_matrix = string;
}

void Coordinates::setB_matrix(CString string)
{
	b_matrix = string;
}

void Coordinates::setX_cycle(CString string)
{
	x_cycle = string;
}

void Coordinates::setY_cycle(CString string)
{
	y_cycle = string;
}

void Coordinates::setZ_cycle(CString string)
{
	z_cycle = string;
}

void Coordinates::setCx(CString string)
{
	cx = string;
}

void Coordinates::setCy(CString string)
{
	cy = string;
}

void Coordinates::setCz(CString string)
{
	cz = string;
}

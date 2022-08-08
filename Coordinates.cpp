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
	this->x_coordinate = _T("+0.000");
	this->y_coordinate = _T("+0.000");
	this->z_coordinate = _T("+0.000");
	this->a_coordinate = _T("+0.000");
	this->c_coordinate = _T("+0.000");

	this->a_matrix = _T("+0.000");
	this->b_matrix = _T("+0.000");
	this->c_matrix = _T("+0.000");

	this->x_cycle = _T("+0.000");
	this->y_cycle = _T("+0.000");
	this->z_cycle = _T("+0.000");

	this->cx = _T("+0.000");
	this->cy = _T("+0.000");
	this->cz = _T("+0.000");
}

CString Coordinates::getX_coordinate()
{
	return x_coordinate;
}

CString Coordinates::getY_coordinate()
{
	return y_coordinate;
}

CString Coordinates::getZ_coordinate()
{
	return z_coordinate;
}

CString Coordinates::getA_coordinate()
{
	return a_coordinate;
}

CString Coordinates::getC_coordinate()
{
	return c_coordinate;
}

CString Coordinates::getA_matrix()
{
	return a_matrix;
}

CString Coordinates::getB_matrix()
{
	return b_matrix;
}

CString Coordinates::getC_matrix()
{
	return c_matrix;
}

CString Coordinates::getX_cycle()
{
	return x_cycle;
}

CString Coordinates::getY_cycle()
{
	return y_cycle;
}

CString Coordinates::getZ_cycle()
{
	return z_cycle;
}

CString Coordinates::getCx()
{
	return cx;
}

CString Coordinates::getCy()
{
	return cy;
}

CString Coordinates::getCz()
{
	return cz;
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

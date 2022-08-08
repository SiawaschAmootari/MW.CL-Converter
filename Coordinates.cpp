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

CString Coordinates::getX_coordinate()
{
	return this->x_coordinate;
}

CString Coordinates::getY_coordinate()
{
	return this->y_coordinate;
}

CString Coordinates::getZ_coordinate()
{
	return this->z_coordinate;
}

CString Coordinates::getA_coordinate()
{
	return this->a_coordinate;
}

CString Coordinates::getC_coordinate()
{
	return this->c_coordinate;
}

CString Coordinates::getA_matrix()
{
	return this->a_matrix;
}

CString Coordinates::getB_matrix()
{
	return this->b_matrix;
}

CString Coordinates::getC_matrix()
{
	return this->c_matrix;
}

CString Coordinates::getX_cycle()
{
	return this->x_cycle;
}

CString Coordinates::getY_cycle()
{
	return this->y_cycle;
}

CString Coordinates::getZ_cycle()
{
	return this->z_cycle;
}

CString Coordinates::getCx()
{
	return this->cx;
}

CString Coordinates::getCy()
{
	return this->cy;
}

CString Coordinates::getCz()
{
	return this->cz;
}

void Coordinates::setX_coordinate(CString string)
{
	this->x_coordinate = string;
}

void Coordinates::setY_coordinate(CString string)
{
	this->y_coordinate = string;
}

void Coordinates::setZ_coordinate(CString string)
{
	this->z_coordinate = string;
}

void Coordinates::setA_coordinate(CString string)
{
	this->a_coordinate = string;
}

void Coordinates::setC_coordinate(CString string)
{
	this->c_coordinate = string;
}

void Coordinates::setA_matrix(CString string)
{
	this->a_matrix = string;
}

void Coordinates::setC_matrix(CString string)
{
	this->c_matrix = string;
}

void Coordinates::setB_matrix(CString string)
{
	this->b_matrix = string;
}

void Coordinates::setX_cycle(CString string)
{
	this->x_cycle = string;
}

void Coordinates::setY_cycle(CString string)
{
	this->y_cycle = string;
}

void Coordinates::setZ_cycle(CString string)
{
	this->z_cycle = string;
}

void Coordinates::setCx(CString string)
{
	this->cx = string;
}

void Coordinates::setCy(CString string)
{
	this->cy = string;
}

void Coordinates::setCz(CString string)
{
	this->cz = string;
}

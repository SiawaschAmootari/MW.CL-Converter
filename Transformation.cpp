#include "pch.h"
#include "Transformation.h"
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

Transformation::Transformation() {
	this->xx = _T("0");
	this->xy = _T("0");
	this->xz = _T("0");
	this->tx = _T("0");
	this->yx = _T("0");
	this->yy = _T("0");
	this->yz = _T("0");
	this->ty = _T("0");
	this->zx = _T("0");
	this->zy = _T("0");
	this->zz = _T("0");
	this->tz = _T("0");
};

void Transformation::setXx(CString string)
{
	this->xx = string;
}

void Transformation::setXy(CString string)
{
	this->xy = string;
}

void Transformation::setXz(CString string)
{
	this->xz = string;
}

void Transformation::setTx(CString string)
{
	this->tx = string;
}

void Transformation::setYx(CString string)
{
	this->yx = string;
}

void Transformation::setYy(CString string)
{
	this->yy = string;
}

void Transformation::setYz(CString string)
{
	this->yz = string;
}

void Transformation::setTy(CString string)
{
	this->ty = string;
}

void Transformation::setZx(CString string)
{
	this->zx = string;
}

void Transformation::setZy(CString string)
{
	this->zy = string;
}

void Transformation::setZz(CString string)
{
	this->zz = string;
}

void Transformation::setTz(CString string)
{
	this->tz = string;
}

CString Transformation::getXx()
{
	return this->xx;
}

CString Transformation::getXy()
{
	return this->xy;
}

CString Transformation::getXz()
{
	return this->xz;
}

CString Transformation::getTx()
{
	return this->tx;
}

CString Transformation::getYx()
{
	return this->yx;
}

CString Transformation::getYy()
{
	return this->yy;
}

CString Transformation::getYz()
{
	return this->yz;
}

CString Transformation::getTy()
{
	return this->ty;
}

CString Transformation::getZx()
{
	return this->zx;
}

CString Transformation::getZy()
{
	return this->zy;
}

CString Transformation::getZz()
{
	return this->zz;
}

CString Transformation::getTz()
{
	return this->tz;
}

CString Transformation::fillPosition(CString line) {
	CString filling = _T("");
	for (int i = 3; i < line.GetLength(); i++) {
		filling.AppendChar(line.GetAt(i));
	}
	return filling;
}

 Transformation Transformation::updateTrans(CString trans, CString line, Transformation transform) {
	if (trans == _T("xx")) {
		transform.setXx(_T(""));
		transform.setXx(fillPosition(line));
	}
	else if (trans == _T("xy")) {
		transform.setXy(_T(""));
		transform.setXy(fillPosition(line));
	}
	else if (trans == _T("xz")) {
		transform.setXz(_T(""));
		transform.setXz(fillPosition(line));
	}
	else if (trans == _T("tx")) {
		transform.setTx(_T(""));
		transform.setTx(fillPosition(line));
	}
	else if (trans == _T("yx")) {
		transform.setYx(_T(""));
		transform.setYx(fillPosition(line));
	}
	else if (trans == _T("yy")) {
		transform.setYy(_T(""));
		transform.setYy(fillPosition(line));
	}
	else if (trans == _T("yz")) {
		transform.setYz(_T(""));
		transform.setYz(fillPosition(line));
	}
	else if (trans == _T("ty")) {
		transform.setTy(_T(""));
		transform.setTy(fillPosition(line));
	}
	else if (trans == _T("zx")) {
		transform.setZx(_T(""));
		transform.setZx(fillPosition(line));
	}
	else if (trans == _T("zy")) {
		transform.setZy(_T(""));
		transform.setZy(fillPosition(line));
	}
	else if (trans == _T("zz")) {
		transform.setZz(_T(""));
		transform.setZz(fillPosition(line));
	}
	else if (trans == _T("tz")) {
		transform.setTz(_T(""));
		transform.setTz(fillPosition(line));
	}
	return transform;
}
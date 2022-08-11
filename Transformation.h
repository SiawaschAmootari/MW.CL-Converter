#pragma once


class Transformation
{
private:
	CString xx = _T("0");
	CString xy = _T("0");
	CString xz = _T("0");
	CString tx = _T("0");
	CString yx = _T("0");
	CString yy = _T("0");
	CString yz = _T("0");
	CString ty = _T("0");
	CString zx = _T("0");
	CString zy = _T("0");
	CString zz = _T("0");
	CString tz = _T("0");

public:

	Transformation();

	void setXx(CString string);
	void setXy(CString string);
	void setXz(CString string);
	void setTx(CString string);
	
	void setYx(CString string);
	void setYy(CString string);
	void setYz(CString string);
	void setTy(CString string);

	void setZx(CString string);
	void setZy(CString string);
	void setZz(CString string);
	void setTz(CString string);

	CString getXx();
	CString getXy();
	CString getXz();
	CString getTx();

	CString getYx();
	CString getYy();
	CString getYz();
	CString getTy();

	CString getZx();
	CString getZy();
	CString getZz();
	CString getTz();

	static Transformation updateTrans(CString trans, CString line, Transformation transform);
	static CString fillPosition(CString line);
};


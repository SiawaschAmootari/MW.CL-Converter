#pragma once
class UniversalDrill
{
public:
	CString q200 = _T("");
	CString q201 = _T("");
	CString q206 = _T("");
	CString q202 = _T("");
	CString q210 = _T("");
	CString q203 = _T("");
	CString q204 = _T("");
	CString q212 = _T("");
	CString q213 = _T("");
	CString q205 = _T("");
	CString q211 = _T("");
	CString q208 = _T("");
	CString q256 = _T("");

	UniversalDrill();

	void cycleDef203();
	void fillDrill(CString lineq00, CString lineq01, CString lineq06, CString lineq02, CString lineq10, CString lineq03, CString lineq04, 
		CString lineq12, CString lineq13, CString lineq05, CString lineq11, CString lineq08, CString lineq56);
	static CString fillQValues(CString line);
	static int stringAsInt(CString line);
};


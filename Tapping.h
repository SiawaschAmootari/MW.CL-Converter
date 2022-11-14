#pragma once
class Tapping
{
public:
	CString q200 = _T("");
	CString q201 = _T("");
	CString q239 = _T("");
	CString q203 = _T("");
	CString q204 = _T("");

	Tapping();

	void cycleDef207();
	void fillDrill(CString lineq00, CString lineq01, CString lineq39, CString lineq03, CString lineq04); 
	static CString fillQValues(CString line);
	
};


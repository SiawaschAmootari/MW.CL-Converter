 #pragma once
class Drill
{
public:
	CString q200 = _T("");
	CString q201 = _T("");
	CString q202 = _T("");
	CString q203 = _T("");
	CString q204 = _T("");
	CString q206 = _T("");
	CString q210 = _T("");
	CString q211 = _T("");

	Drill();

	void cycleDef200();
	void fillDrill(CString lineq00, CString lineq01, CString lineq06, CString lineq02, CString lineq10, CString lineq03, CString lineq04, CString lineq11);
	static CString fillQValues(CString line);
};
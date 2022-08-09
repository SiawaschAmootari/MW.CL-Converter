#pragma once

class ConfigFile
{

private:
	CString version = _T("");
	CString unit = _T("");
	CString initial = _T("");
	CString outputname = _T("");
	CString safepoint = _T("");
	CString headadapter = _T("");
	CString postconfig = _T("");
	CString shortestpath = _T("");
	CString toolChangeTime = _T("");
	CString toolChangePoint_x = _T("");
	CString toolChangePoint_z = _T("");
	CString toolChangePoint_xy = _T("");
	CString mw_toolCall = _T("");
public:
	CStringArray configFileList;
	ConfigFile();
	CString getVersion();
	void setVersion(CString version);
	CString getUnit();
	void setUnit(CString unit);
	CString getInitial();
	void setInitial(CString initial);
	CString getOutputname();
	void setOutputname(CString outputname);
	CString getSafepoint();
	void setSafepoint(CString safepoint);
	CString getHeadadapter();
	void setHeadadapter(CString headadapter);
	CString getPostconfig();
	void setPostconfig(CString postconfig);
	CString getShortestpath();
	void setShortestpath(CString shortestpath);
	CString getToolChangeTime();
	void setToolChangeTime(CString toolChangeTime);
	CString getToolChangePoint_x();
	void setToolChangePoint_x(CString toolChangePoint_x);
	CString getToolChangePoint_z();
	void setToolChangePoint_z(CString toolChangePoint_z);
	CString getToolChangePoint_xy();
	void setToolChangePoint_xy(CString toolChangePoint_xy);
	CString getMw_toolCall();
	void setMw_toolCall(CString mw_toolCall);
	CStringArray getConfigFileList();
	void setConfigFileList(CStringArray configFileList);

};


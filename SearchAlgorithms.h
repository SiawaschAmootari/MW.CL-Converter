#pragma once
class SearchAlgorithms
{
public:
	static bool searchForToolChange(int index, CStringArray &file);
	static CString findFeedRate(CString line);
	static void findToolCall(CString line);
	static CString findSequenceName(CString line);
	static CString findOtherLine(CString line, CString mw_other_line);
	static CString findOtherLine(CString line, char c, CString mw_other_line);
private:
};


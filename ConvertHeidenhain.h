#pragma once
#include <afxcoll.h>
#include <winnt.h>


using namespace std;

class ConvertHeidenhain : public CMWCLConverterDlg
{
public:
	
	ConvertHeidenhain(); 
	CStringArray fileContent;
	CStringArray convertedFileContent;
	CStringArray tool_repositoryContent;
	CStringArray creoConfiContent;

	//Methods
	void startConverting(CStringArray &fileContent, CString filePath);
	void findMovement(CString line,int index);
	void fillCoordinates(CString line, char c, int index, CString& g_coordinate);
	void addDecimalPlace(CString& line);
	void findToolCall(CString line);
	void openSubFiles(CString path, CStringArray& subFileContent);
	void findFeedRate(CString line);
	void findToolName(CString toolNameComment);
	void findComment(CString line);

	CString findSubFilesPath(CString fileName);

	
	//filterStrings
	CString feedRate =_T("F");
	CString spindle = _T("F");
	CString x_coordinate=_T("0.000");
	CString y_coordinate=_T("0.000");
	CString z_coordinate=_T("0.000");
	CString findLineNr(CString line);
	CString path;
	
	//bools for algortihm
	bool foundFMAX = false;
	bool foundFQ = false;
	
	//Hardcoded Strings
	CString mw_machmove_rapid = _T("MW_MACHMOVE RAPID  TIME.1 MOVE=");
	CString mw_op_comment;
	CString mw_relmove_rapid = _T("MW_RELMOVE RAPID");
	CString mw_relmove_feed = _T("MW_RELMOVE FEED");
	CString time_move = _T(" TIME.1 MOVE=");
	CString m19 = _T("TCPM OFF");
	CString mw_tool_comment = _T("");
	CString mw_toolpath_transform = _T("");
	CString mw_tool_name = _T("");
	CString use_number = _T("USE_NUMBER");
	CString mw_op_start = _T("MW_OP_START");
	CString mw_op_end = _T("MW_OP_END");
	CString mw_op_number = _T("MW_OP_NUMBER");

	//
	int op_number_index = 0;

};

/*
13 M127; DREHACHSEN WEGOPTIMIERT AUS // -> MW_MACHMOVE RAPID  TIME.1 MOVE=13
14 M129; TCPM OFF // ->MW_MACHMOVE RAPID  TIME.1 MOVE=14
17 CYCL DEF 7.0 NULLPUNKT // -> MW_MACHMOVE RAPID  TIME.1 MOVE=17 // erstmal so später mit Verschiebung
18 CYCL DEF 7.1  X + 0 // -> MW_MACHMOVE RAPID  TIME.1 MOVE=18
19 CYCL DEF 7.2  Y + 0 // -> MW_MACHMOVE RAPID  TIME.1 MOVE=19
20 CYCL DEF 7.3  Z + 0 // -> MW_MACHMOVE RAPID  TIME.1 MOVE=20
23 PLANE RESET STAY // -> MW_MACHMOVE RAPID  TIME.1 MOVE=23
26 L  Z + 500 R0 FMAX M91 // -> MW_MACHMOVE RAPID Z+500 TIME.1 MOVE=26   ->muss eingetragen werden. umschalten auf Absolutkoordinaten vom Maschinennullpunkt
29 L  A + 0  C + 0 R0 FMAX // -> MW_MACHMOVE RAPID A+0 C+0 TIME.1 MOVE=29
31 * -PLANEN -  // -> MW_OP_COMMENT 
33 FUNCTION MODE MILL //-> MW_MACHMOVE RAPID TIME.1 MOVE=33
35 TOOL CALL "T10" Z S10000 //-> MW_MACHMOVE RAPID TIME.1 MOVE=35
36 M21 //-> MW_MACHMOVE RAPID TIME.1 MOVE=36
38 FN 0 : Q1 = 5000.; VORSCHUB //-> MW_MACHMOVE RAPID TIME.1 MOVE=38
39 M140 MB MAX //-> MW_MACHMOVE RAPID TIME.1 MOVE=39
40 L X + 20 Y + 400 R0 FMAX M91 //-> MW_RELMOVE RAPID X+20 Y+400 F10000 MOVE=40 ->kommt aus pp
42 CALL LBL 1 //->MW_MACHMOVE RAPID TIME.1 MOVE=42   //danach muss das ganze Label aufgerufen werden
43 L  X + 90  Y - 30 R0 FMAX M3 //-> MW_RELMOVE RAPID  X+90.000 Y-30.000  F10000 MOVE=43
44 L  Z + 100 R0 FMAX //->MW_RELMOVE RAPID  Z+100.000  F10000 MOVE=44
45 L Z + 12 FMAX //-> MW_RELMOVE RAPID  Z+12.000 F10000 MOVE=45
46 L Z + 0 FQ1 //-> MW_RELMOVE FEED   Z+0.000 F5000. MOVE=46
47 L X - 70 //-> MW_RELMOVE FEED  X-70.000 F5000. MOVE=47
48 L Y + 0 //-> MW_RELMOVE FEED  Y+0.000 F5000. MOVE=48
49 L X + 70 //-> MW_RELMOVE FEED  X+70.000 F5000. MOVE=49
50 L Y + 30 //-> MW_RELMOVE FEED  Y+30.000 F5000. MOVE=50
51 L X - 70 //-> MW_RELMOVE FEED  X-70.000 F5000. MOVE=51
52 L Z + 100 FMAX //-> MW_RELMOVE RAPID Z+100.000 F10000 MOVE=52
53 M05 //-> MW_MACHMOVE RAPID TIME.1 MOVE=53
*/
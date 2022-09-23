#pragma once
#include <afxcoll.h>
#include <winnt.h>
#include <map>
#include "Coordinates.h"
#include "Transformation.h"
#include "ConfigFile.h"
#include "ConversionAlgorithms.h"
using namespace std;

class ConvertHeidenhain : public CMWCLConverterDlg
{
public:
	//Objects
	ConvertHeidenhain();
	Coordinates coordinates;
	Transformation transformation;
	ConfigFile configFile;

	//Variables
	CStringArray fileContent;
	CStringArray convertedFileContent;
	CStringArray tool_repositoryContent;
	CStringArray creoConfiContent;
	CStringArray moveLines;
	CStringArray file;
	int label_index = 0;
	
	
	//Methods without return value
	void readConfigFile(CString path);
	void startConverting(CStringArray& fileContent, int& labelIndex, CString &filePath);
	void filterMovement(CString line, int index, bool isMachMove);
	void findToolCall(CString line);
	void openSubFiles(CString path, CStringArray& subFileContent);
	void filterToolName(CString toolNameComment);
	void startMachineCycle(CString line, bool& foundOpCycle, CString indexString);
	void startMachineCycle(CString indexString);
	void startMachineCycle();
	void findCircle(CString lineCC, CString lineC);
	void jumpToLabel(CString line);
	void textFilter(CString line, int& index);
	void outputTransform(CString line);
	void filterCycleDef(CString lineX, CString lineY, CString lineZ);
	void fillacCoordinates(CString line);
	void findMatrix(CString line);
	void sequenceWithoutToolChange(CString line);
	void nameInToolList(CString line);
	int initialComment();

	//Methods with boolean return value
	//bool searchForToolChange(int index);

	//filterStrings
	CString feedRate = _T("F");
	CString spindle = _T("F");
	CString path;
	CString mapKey = _T("");

	//bools for algortihms
	bool foundFMAX = false;
	bool foundFQ = false;
	bool foundRTCPOFF = false;
	bool labelZero = false;

	//Hardcoded Strings
	CString mw_machmove_rapid = _T("MW_MACHMOVE RAPID   MOVE=");
	CString mw_other_line = _T("MW_MACHMOVE RAPID  TIME.1 MOVE=");
	CStringArray mw_op_comment;
	CStringArray mw_op_number_list;
	CStringArray mw_tool_name_list;
	CStringArray mw_tool_comment_list;
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
	CString mw_transform = _T("");
	//CString sequenceNamewotc = _T("");
	map<CString, CString> toolRepositoryMap;

	int op_number_index = 0;
	int mw_list_counter = 0;
	int sequenceCounter = 0;
	int toolListCounter = 0;
};

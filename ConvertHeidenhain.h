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
	CStringArray moveLines;
	CStringArray file;
	CStringArray configFile;
	int label_index;

	//Methods without return value
	void readConfigFile();
	void startConverting(CStringArray& fileContent, int& labelIndex, CString filePath);
	void findMovement(CString line, int index, bool isMachMove);
	void fillCoordinates(CString line, char c, int index, CString& g_coordinate);
	void addDecimalPlace(CString& line);
	void findToolCall(CString line);
	void openSubFiles(CString path, CStringArray& subFileContent);
	void findFeedRate(CString line);
	void findToolName(CString toolNameComment);
	void findSequenceName(CString line);
	void startMachineCycle(CString line, bool& foundOpCycle, CString indexString);
	void startMachineCycle(CString indexString);
	void findCircle(CString lineCC, CString lineC);
	void findOtherLine(CString line);
	void findOtherLine(CString line, char c);
	void jumpToLabel(CString line);
	void textFilter(CString line, int& index);
	void outputTransform(CString line);
	void findCycleDef(CString lineX, CString lineY, CString lineZ);
	void updateTrans(CString trans, CString line);
	void fillacCoordinates(CString line);
	void findMatrix(CString line);
	void calculateMatrix(double a, double b, double c);
	void fillMatrix(CString line, CString& axis, char axisChar);
	void sequenceWithoutToolChange(CString line);
	//Methods with int return value
	int initialComment();

	//Methods with CString return value
	CString cutAtSpace(CString line, int spaces);
	CString cutAtSpace(CString line, int spaces, char c);
	CString findSubFilesPath(CString fileName);
	CString fillPosition(CString line);
	CString cutCoordinateChar(CString coordinate);
	CString addTwoStrings(CString numberOne, CString numberTwo);

	//Methods with boolean return value
	bool searchForToolChange(int index);

	//filterStrings
	CString feedRate = _T("F");
	CString spindle = _T("F");

	CString x_coordinate = _T("+0.000");
	CString y_coordinate = _T("+0.000");
	CString z_coordinate = _T("+0.000");
	CString a_coordinate = _T("+0.000");
	CString c_coordinate = _T("+0.000");

	CString a_matrix = _T("+0.000");
	CString b_matrix = _T("+0.000");
	CString c_matrix = _T("+0.000");

	CString x_cycle = _T("+0.000");
	CString y_cycle = _T("+0.000");
	CString z_cycle = _T("+0.000");

	//Variablen für die Transformation
	//------------------------------------------
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
	//-----------------------------------------


	//Variablen für die Config Datei
	//-----------------------------------------
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
	//-----------------------------------------

	CString findLineNr(CString line);
	CString path;

	//bools for algortihm
	bool foundFMAX = false;
	bool foundFQ = false;
	bool foundRTCPOFF = false;

	//Hardcoded Strings
	CString mw_machmove_rapid = _T("MW_MACHMOVE RAPID   MOVE=");
	CString mw_other_line = _T("MW_MACHMOVE RAPID  TIME.1 MOVE=");
	CStringArray mw_op_comment;
	CStringArray mw_op_number_list;
	CStringArray mw_tool_name_list;
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
	CString sequenceNamewotc = _T("");

	int op_number_index = 0;
	int mw_list_counter = 0;
};

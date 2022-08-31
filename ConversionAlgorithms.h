#pragma once
class ConversionAlgorithms
{
public:
	ConversionAlgorithms();
	static CString cutAtSpace(CString line, int spaces);
	static CString cutAtSpace(CString line, int spaces, char c);
	static CString findSubFilesPath(CString fileName,CString path);
	static CString fillPosition(CString line);
	static CString cutCoordinateChar(CString coordinate);
	static CString addTwoStrings(CString numberOne, CString numberTwo);
	static CString calculateMatrix(double a, double b, double c, Transformation transformation, Coordinates coordinates);
	static CString fillMatrix(CString line, CString axis, char axisChar);
	static CString fillCoordinates(CString line, char c, int index);
	static CString addDecimalPlace(CString line);
	static CString findLineNr(CString line);
};


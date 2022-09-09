#pragma once
class Coordinates
{
public:

	Coordinates();

	//Getter
	CString getX_coordinate();
	CString getY_coordinate();
	CString getZ_coordinate();
	CString getA_coordinate();
	CString getC_coordinate();

	CString getA_matrix();
	CString getB_matrix();
	CString getC_matrix();

	CString getX_cycle();
	CString getY_cycle();
	CString getZ_cycle();

	CString getCx();
	CString getCy();
	CString getCz();

	//Setter
	void setX_coordinate(CString string);
	void setY_coordinate(CString string);
	void setZ_coordinate(CString string);
	void setA_coordinate(CString string);
	void setC_coordinate(CString string);

	void setA_matrix(CString string);
	void setC_matrix(CString string);
	void setB_matrix(CString string);

	void setX_cycle(CString string);
	void setY_cycle(CString string);
	void setZ_cycle(CString string);

	void setCx(CString string);
	void setCy(CString string);
	void setCz(CString string);

private: 
	
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

	CString cx = _T("+0.000");
	CString cy = _T("+0.000");
	CString cz = _T("+0.000");

};


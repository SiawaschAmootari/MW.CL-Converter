#include "pch.h"
#include "ConfigFile.h"
#include "pch.h"
#include "Transformation.h"
#include "pch.h"
#include "Coordinates.h"
#include "framework.h"
#include "MW.CL Converter.h"
#include "MW.CL ConverterDlg.h"
#include "ConvertHeidenhain.h"
#include "afxdialogex.h"
#include <afxcoll.h>
#include <iostream>
#include <string>
#include <fstream>
#include <map>
#include <vector>
#include <algorithm>
#include <thread>
#include <atlstr.h>

ConfigFile::ConfigFile() {};

CString ConfigFile::getVersion() {
	return this->version;
}
void ConfigFile::setVersion(CString version) {
	this->version = version;
}


CString ConfigFile::getUnit() {
	return this->unit;
}
void ConfigFile::setUnit(CString unit) {
	this->unit = unit;
}


CString ConfigFile::getInitial() {
	return this->initial;
}
void ConfigFile::setInitial(CString initial) {
	this->initial = initial;
}


CString ConfigFile::getOutputname() {
	return this->outputname;
}
void ConfigFile::setOutputname(CString outputname) {
	this->outputname = outputname;
}


CString ConfigFile::getSafepoint() {
	return this->safepoint;
}
void ConfigFile::setSafepoint(CString safepoint) {
	this->safepoint = safepoint;
}


CString ConfigFile::getHeadadapter() {
	return this->headadapter;
}
void ConfigFile::setHeadadapter(CString headadapter) {
	this->headadapter = headadapter;
}


CString ConfigFile::getPostconfig() {
	return this->postconfig;
}
void ConfigFile::setPostconfig(CString postconfig) {
	this->postconfig = postconfig;
}


CString ConfigFile::getShortestpath() {
	return this->shortestpath;
}
void ConfigFile::setShortestpath(CString shortestpath) {
	this->shortestpath = shortestpath;
}


CString ConfigFile::getToolChangeTime() {
	return this->toolChangeTime;
}
void ConfigFile::setToolChangeTime(CString toolChangeTime) {
	this->toolChangeTime = toolChangeTime;
}


CString ConfigFile::getToolChangePoint_x() {
	return this->toolChangePoint_x;
}
void ConfigFile::setToolChangePoint_x(CString toolChangePoint_x) {
	this->toolChangePoint_x = toolChangePoint_x;
}


CString ConfigFile::getToolChangePoint_z() {
	return this->toolChangePoint_z;
}
void ConfigFile::setToolChangePoint_z(CString toolChangePoint_z) {
	this->toolChangePoint_z = toolChangePoint_z;
}


CString ConfigFile::getToolChangePoint_xy() {
	return this->toolChangePoint_xy;
}
void ConfigFile::setToolChangePoint_xy(CString toolChangePoint_xy) {
	this->toolChangePoint_xy = toolChangePoint_xy;
}


CString ConfigFile::getMw_toolCall() {
	return this->mw_toolCall;
}
void ConfigFile::setMw_toolCall(CString mw_toolCall) {
	this->mw_toolCall = mw_toolCall;
}


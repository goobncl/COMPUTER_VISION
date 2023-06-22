#pragma once

#include "stdafx.h"
#include "util.h"
#include "haarFeature.h"


class CascadeClassifier
{
public:
	std::vector<Rect> candidates;
	CascadeClassifier();
	~CascadeClassifier();
	void objectDetect(unsigned char* image);	

private:
	QSqlDatabase db;
	struct Data {
		Size origWinSz;
		std::vector<Stage> stages;
		std::vector<DTree> classifiers;
		std::vector<DTreeNode> nodes;
		std::vector<float> leaves;
		std::vector<Stump> stumps;
		std::vector<Feature> features;
	};
	Data data;
	Size sz0;
	Size imgSz;
	Size minObjSz;
	Size maxObjSz;
	std::vector<double> scales;
	std::vector<ScaleData> scaleData;
	std::vector<ImgPlane> imgPyramid;

	std::vector<Stage> readStages(QSqlQuery& query);
	std::vector<DTree> readClassifiers(QSqlQuery& query);
	std::vector<DTreeNode> readNodes(QSqlQuery& query);
	std::vector<float> readLeaves(QSqlQuery& query);
	std::vector<Stump> readStumps(QSqlQuery& query);
	std::vector<Feature> readFeatures(QSqlQuery& query);
	bool loadDataFromDB();

	void setData();
	void calcScales();
	void initScaleData();
	void initImgProc();
	void calcImgPyramid(unsigned char* image);
	void clearImgPyramid();
	double calcNormFactor(int* pSum, int* pSqsum, int x, int y, int width);
	int predictOrderedStump(int* ptr, int width, int height, double varNFact);
	void calcHaarFeature();
	bool compRect(const Rect& r1, const Rect& r2);
	int partition(const std::vector<Rect>& rectList, std::vector<int>& labels);
	void groupRectangles(int threshold, double eps);	
};
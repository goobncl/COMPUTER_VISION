#pragma once

#include "stdafx.h"
#include "util.h"
#include "haarFeature.h"


class CascadeClassifier
{
public:
	CascadeClassifier();
	~CascadeClassifier();
	void calcImgPyramid(unsigned char* image);
	void calcHaarFeature();

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
	std::vector<Rect> faces;

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
	void clearImgPyramid();
	double calcNormFactor(int* pSum, int* pSqsum, int x, int y, int width);
	int predictOrderedStump(int* ptr, int width, int height, double varNFact);
};

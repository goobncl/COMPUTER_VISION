#include "stdafx.h"
#include "CascadeClassifier.h"

CascadeClassifier::CascadeClassifier()
{
	setData();
	initImgProc();
}

CascadeClassifier::~CascadeClassifier()
{
	clearImgPyramid();
}

std::vector<Stage> CascadeClassifier::readStages(QSqlQuery& query)
{
	std::vector<Stage> stages;
	if (query.exec("SELECT FIRST, NTREES, THRESHOLD FROM FRONTALFACE_DEFAULT_STAGE")) {
		while (query.next()) {
			Stage stage;
			stage.first = query.value(0).toInt();
			stage.ntrees = query.value(1).toInt();
			stage.threshold = query.value(2).toFloat();
			stages.push_back(stage);
		}
	}

	return stages;
}

std::vector<DTree> CascadeClassifier::readClassifiers(QSqlQuery& query)
{
	std::vector<DTree> classifiers;

	if (query.exec("SELECT NODECOUNT FROM FRONTALFACE_DEFAULT_DTREE")) {
		while (query.next()) {
			DTree classifier;
			classifier.nodeCount = query.value(0).toInt();
			classifiers.push_back(classifier);
		}
	}

	return classifiers;
}

std::vector<DTreeNode> CascadeClassifier::readNodes(QSqlQuery& query)
{
	std::vector<DTreeNode> nodes;

	if (query.exec("SELECT FEATUREIDX, THRESHOLD, LEFT, RIGHT FROM FRONTALFACE_DEFAULT_DTREENODE")) {
		while (query.next()) {
			DTreeNode node;
			node.featureIdx = query.value(0).toInt();
			node.threshold = query.value(1).toFloat();
			node.left = query.value(2).toInt();
			node.right = query.value(3).toInt();
			nodes.push_back(node);
		}
	}

	return nodes;
}

std::vector<float> CascadeClassifier::readLeaves(QSqlQuery& query)
{
	std::vector<float> leaves;

	if (query.exec("SELECT LEAVE FROM FRONTALFACE_DEFAULT_LEAVES")) {
		while (query.next()) {
			float leave = query.value(0).toFloat();
			leaves.push_back(leave);
		}
	}

	return leaves;
}

std::vector<Stump> CascadeClassifier::readStumps(QSqlQuery& query)
{
	std::vector<Stump> stumps;

	if (query.exec("SELECT FEATUREIDX, THRESHOLD, LEFT, RIGHT FROM FRONTALFACE_DEFAULT_STUMP")) {
		while (query.next()) {
			Stump stump;
			stump.featureIdx = query.value(0).toInt();
			stump.threshold = query.value(1).toFloat();
			stump.left = query.value(2).toFloat();
			stump.right = query.value(3).toFloat();
			stumps.push_back(stump);
		}
	}

	return stumps;
}

std::vector<Feature> CascadeClassifier::readFeatures(QSqlQuery& query)
{
	std::vector<Feature> features;

	QString queryString =
		"SELECT TILTED, "
		"RECT1_X, RECT1_Y, RECT1_WIDTH, RECT1_HEIGHT, RECT1_WEIGHT, "
		"RECT2_X, RECT2_Y, RECT2_WIDTH, RECT2_HEIGHT, RECT2_WEIGHT, "
		"RECT3_X, RECT3_Y, RECT3_WIDTH, RECT3_HEIGHT, RECT3_WEIGHT "
		"FROM FRONTALFACE_DEFAULT_HAAR";

	if (query.exec(queryString)) {
		while (query.next()) {
			Feature feature;
			feature.tilted = query.value(0).toBool();

			feature.rect[0].r.x = query.value(1).toInt();
			feature.rect[0].r.y = query.value(2).toInt();
			feature.rect[0].r.width = query.value(3).toInt();
			feature.rect[0].r.height = query.value(4).toInt();
			feature.rect[0].weight = query.value(5).toFloat();

			feature.rect[1].r.x = query.value(6).toInt();
			feature.rect[1].r.y = query.value(7).toInt();
			feature.rect[1].r.width = query.value(8).toInt();
			feature.rect[1].r.height = query.value(9).toInt();
			feature.rect[1].weight = query.value(10).toFloat();

			feature.rect[2].r.x = query.value(11).toInt();
			feature.rect[2].r.y = query.value(12).toInt();
			feature.rect[2].r.width = query.value(13).toInt();
			feature.rect[2].r.height = query.value(14).toInt();
			feature.rect[2].weight = query.value(15).toFloat();

			features.push_back(feature);
		}
	}
	
	return features;
}

bool CascadeClassifier::loadDataFromDB()
{
	QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE");
	db.setDatabaseName("HaarCascadeFeatures.db");

	if (!db.open()) {
		qDebug() << "Error: connection with database failed";
		return false;
	}

	qDebug() << "Database: connection ok";

	QSqlQuery query;

	data.stages = readStages(query);
	data.classifiers = readClassifiers(query);
	data.nodes = readNodes(query);
	data.leaves = readLeaves(query);
	data.stumps = readStumps(query);
	data.features = readFeatures(query);

	db.close();

	return true;
}

void CascadeClassifier::setData()
{
	data.origWinSz = Size(24, 24);
	if (!loadDataFromDB()) {
		qDebug() << "Error: failed to load data from database";
	}
}

void CascadeClassifier::calcScales()
{
	std::vector<double> allScales;

	for (double factor = 1.f; ; factor *= 1.1f) {
		Size winSz = Size(
			doubleRound(data.origWinSz.width * factor),
			doubleRound(data.origWinSz.height * factor));
		if (winSz.width > imgSz.width || winSz.height > imgSz.height) {
			break;
		}
		allScales.push_back(factor);
	}

	for (size_t idx = 0; idx < allScales.size(); idx++) {
		Size winSz = Size(
			doubleRound(data.origWinSz.width * allScales[idx]),
			doubleRound(data.origWinSz.height * allScales[idx]));
		if (winSz.width > maxObjSz.width || winSz.height > maxObjSz.height) {
			break;
		}
		if (winSz.width < minObjSz.width || winSz.height < minObjSz.height) {
			continue;
		}
		scales.push_back(allScales[idx]);
	}
}

void CascadeClassifier::initScaleData()
{
	size_t nscales = scales.size();
	scaleData.resize(nscales);

	for (size_t i = 0; i < nscales; i++) {
		
		ScaleData& s = scaleData[i];
		double sc = scales[i];
		Size sz;
		
		sz.width = doubleRound(imgSz.width / sc);
		sz.height = doubleRound(imgSz.height / sc);
		s.ystep = (sc >= 2) ? (1) : (2);
		s.scale = sc;
		s.szi = Size(sz.width + 1, sz.height + 1);
	}
}

void CascadeClassifier::initImgProc()
{
	imgSz = Size(640, 480);
	minObjSz = Size(30, 30);
	maxObjSz = imgSz;

	calcScales();
	initScaleData();

	sz0 = scaleData.at(0).szi;
	sz0 = Size((int)alignSize(sz0.width, 16), sz0.height);

	int nscales = scaleData.size();
	imgPyramid.resize(nscales);

	for (size_t i = 0; i < nscales; i++) {
		imgPyramid[i].data = (unsigned char*)malloc(sizeof(unsigned char) * sz0.width * sz0.height);
		imgPyramid[i].sum = (int*)malloc(sizeof(int) * sz0.width * sz0.height);
		imgPyramid[i].sqsum = (int*)malloc(sizeof(int) * sz0.width * sz0.height);
	}
}

void CascadeClassifier::clearImgPyramid()
{
	int n = imgPyramid.size();
	for (size_t i = 0; i < n; i++) {
		free(imgPyramid[i].data);
		free(imgPyramid[i].sum);
		free(imgPyramid[i].sqsum);
	}
	imgPyramid.clear();
}

void CascadeClassifier::calcImgPyramid()
{
	{
		int nscales = scaleData.size();

		for (size_t i = 0; i < nscales; i++) {

			const ScaleData& s = scaleData[i];
			int new_w = s.szi.width;
			int new_h = s.szi.height;

			imgPyramid[i].sz.width = new_w;
			imgPyramid[i].sz.height = new_h;

			downSampling(image, imgPyramid[i].data, new_w, new_h);
			integral(imgPyramid[i].data, imgPyramid[i].sum, new_w, new_h, 0);
			integralSquare(imgPyramid[i].data, imgPyramid[i].sqsum, new_w, new_h, 0);
		}
	}

	//{
	//	std::transform(
	//		std::execution::par,
	//		scaleData.begin(),
	//		scaleData.end(),
	//		imgPyramid.begin(),
	//		[&](const ScaleData& s) {
	//
	//			int new_w = s.szi.width;
	//			int new_h = s.szi.height;
	//			ImgPlane ip;
	//			ip.sz.width = new_w;
	//			ip.sz.height = new_h;
	//
	//			downSampling(image, ip.data, new_w, new_h);
	//			integral(ip.data, ip.sum, new_w, new_h, 0);
	//			integralSquare(ip.data, ip.sqsum, new_w, new_h, 0);
	//
	//			return ip;
	//		}
	//	);
	//}
}

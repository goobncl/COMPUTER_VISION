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

const std::vector<Rect>* CascadeClassifier::objectDetect(unsigned char* image)
{
	calcImgPyramid(image);
	calcHaarFeature();
	groupRectangles(10, 0.2);

	return &candidates;
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

	sz0 = scaleData[0].szi;
	sz0 = Size((int)alignSize(sz0.width, 16), sz0.height);

	int nscales = scaleData.size();
	imgPyramid.resize(nscales);

	for (size_t i = 0; i < nscales; i++) {
		imgPyramid[i].data = (unsigned char*)calloc(sz0.width * sz0.height, sizeof(unsigned char));
		imgPyramid[i].sum = (int*)calloc(sz0.width * sz0.height, sizeof(int));
		imgPyramid[i].sqsum = (int*)calloc(sz0.width * sz0.height, sizeof(int));
	}
}

void CascadeClassifier::calcImgPyramid(unsigned char* image)
{
	{
		size_t nscales = scaleData.size();
	
		for (size_t i = 0; i < nscales; i++) {
			
			const ScaleData& s = scaleData[i];
	
			int new_w = s.szi.width;
			int new_h = s.szi.height;
			imgPyramid[i].sz.width = new_w;
			imgPyramid[i].sz.height = new_h;
	
			downSampling(
				image,
				imgPyramid[i].data,
				(new_w - 1),
				(new_h - 1)
			);

			if (i == 28) {
				cv::Mat temp((new_h - 1), (new_w - 1), CV_8U, imgPyramid[i].data);
				saveMatToCsv(temp, "temp.csv");
				printf("");
			}

			//integral(
			//	imgPyramid[i].data,
			//	imgPyramid[i].sum,
			//	new_w,
			//	new_h,
			//	0
			//);
			//
			//integralSquare(
			//	imgPyramid[i].data,
			//	imgPyramid[i].sqsum,
			//	new_w,
			//	new_h,
			//	0
			//);
		}
	}

	//std::for_each(
	//	std::execution::par,
	//	std::begin(scaleData),
	//	std::end(scaleData),
	//	[&](const ScaleData& s) {
	//
	//		size_t i = &s - &scaleData[0];
	//
	//		int new_w = s.szi.width;
	//		int new_h = s.szi.height;
	//		imgPyramid[i].sz.width = new_w;
	//		imgPyramid[i].sz.height = new_h;
	//
	//		downSampling(
	//			image,
	//			imgPyramid[i].data,
	//			new_w,
	//			new_h
	//		);
	//		integral(
	//			imgPyramid[i].data,
	//			imgPyramid[i].sum,
	//			new_w,
	//			new_h,
	//			0
	//		);
	//		integralSquare(
	//			imgPyramid[i].data,
	//			imgPyramid[i].sqsum,
	//			new_w,
	//			new_h,
	//			0
	//		);
	//	}
	//);
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

double CascadeClassifier::calcNormFactor(int* pSum, int* pSqsum, int x, int y, int width)
{
	int valSum = calcAreaSum(pSum, x, y, width);
	int valSqsum = calcAreaSum(pSqsum, x, y, width);
	double nf = 576.f * valSqsum - (double)valSum * valSum;

	if (nf > 0.f) {
		nf = std::sqrt(nf);
		return (double)(1.f / nf);
	}
	else {
		return 1.f;
	}
}

int CascadeClassifier::predictOrderedStump(int* ptr, int width, int height, double varNFact)
{
	Stump* cascadeStumps = &data.stumps[0];
	Stage* cascadeStages = &data.stages[0];
	Feature* cascadeFeatures = &data.features[0];
	int nstages = data.stages.size();
	double tmp = 0;

	for (size_t stageIdx = 0; stageIdx < nstages; stageIdx++) {
		Stage& stage = cascadeStages[stageIdx];
		int ntrees = stage.ntrees;
		tmp = 0;
		for (size_t stumpIdx = 0; stumpIdx < ntrees; stumpIdx++) {
			Stump& stump = cascadeStumps[stumpIdx];
			Feature& feature = cascadeFeatures[stump.featureIdx];
			double value = feature.calc(ptr, width, height) * varNFact;
			tmp += (value < stump.threshold) ? (stump.left) : (stump.right);
		}

		if (tmp < stage.threshold) {
			return -stageIdx;
		}
		cascadeStumps += ntrees;
	}

	return 1;
}

void CascadeClassifier::calcHaarFeature()
{
	std::mutex mutex;

	std::for_each(
		std::execution::par,
		std::begin(scaleData) + 10,
		std::end(scaleData),
		[&](const ScaleData& s) {

			size_t i = &s - &scaleData[0];

			double scaleFactor = s.scale;
			int* pSum = imgPyramid[i].sum;
			int* pSqsum = imgPyramid[i].sqsum;
			int width = imgPyramid[i].sz.width;
			int height = imgPyramid[i].sz.height;
			int rangeX = s.szi.width - data.origWinSz.width;
			int rangeY = s.szi.height - data.origWinSz.height;
			int step = s.ystep;

			for (int y = 0; y <= rangeY; y += step) {
				for (int x = 0; x <= rangeX; x += step) {
					imgPyramid[i].varNFact = calcNormFactor(
						pSum,
						pSqsum,
						x,
						y,
						width
					);
					int result = predictOrderedStump(
						&pSum[y * width + x],
						width,
						height,
						imgPyramid[i].varNFact
					);

					if (result > 0) {
						std::lock_guard<std::mutex> lock(mutex);
						candidates.push_back(Rect(
							doubleRound(x * scaleFactor),
							doubleRound(y * scaleFactor),
							doubleRound(24 * scaleFactor),
							doubleRound(24 * scaleFactor)
						));
					}
					else if (result == 0) {
						x += step;
					}
				}
			}
		}
	);
}

bool CascadeClassifier::compRect(const Rect& r1, const Rect& r2)
{
	double delta = 0.2 * (intMin(r1.width, r2.width) +
						  intMin(r1.height, r2.height)) * 0.5;
	return intAbs(r1.x - r2.x) <= delta &&
		   intAbs(r1.y - r2.y) <= delta &&
		   intAbs(r1.x + r1.width - r2.x - r2.width) <= delta &&
		   intAbs(r1.y + r1.height - r2.y - r2.height) <= delta;
}

int CascadeClassifier::partition(const std::vector<Rect>& rectList, std::vector<int>& labels)
{
	int N = (int)rectList.size();
	const Rect* pRect = &rectList[0];
	const int PARENT = 0;
	const int RANK = 1;

	QVector<int> nodes(N * 2);
	int(*pNode)[2] = (int(*)[2]) & nodes[0];

	for (size_t i = 0; i < N; i++) {
		pNode[i][PARENT] = -1;
		pNode[i][RANK] = 0;
	}

	for (size_t i = 0; i < N; i++) {
		int root = i;
		while (pNode[root][PARENT] >= 0) {
			root = pNode[root][PARENT];
		}

		for (size_t j = 0; j < N; j++) {

			if (i == j || !compRect(rectList[i], rectList[j])) {
				continue;
			}

			int root2 = j;

			while (pNode[root2][PARENT] >= 0) {
				root2 = pNode[root2][PARENT];
			}

			if (root2 != root) {
				int rank = pNode[root][RANK];
				int rank2 = pNode[root2][RANK];
				if (rank > rank2) {
					pNode[root2][PARENT] = root;
				}
				else {
					pNode[root][PARENT] = root2;
					pNode[root2][RANK] += rank == rank2;
					root = root2;
				}

				int k = j;
				int parent;

				while ((parent = pNode[k][PARENT]) >= 0) {
					pNode[k][PARENT] = root;
					k = parent;
				}

				k = i;
				while ((parent = pNode[k][PARENT]) >= 0) {
					pNode[k][PARENT] = root;
					k = parent;
				}
			}
		}
	}

	labels.resize(N);
	int nclasses = 0;

	for (size_t i = 0; i < N; i++) {
		int root = i;
		while (pNode[root][PARENT] >= 0) {
			root = pNode[root][PARENT];
		}
		if (pNode[root][RANK] >= 0) {
			pNode[root][RANK] = ~nclasses++;
		}
		labels[i] = ~pNode[root][RANK];
	}

	return nclasses;
}

void CascadeClassifier::groupRectangles(int threshold, double eps)
{
	if (candidates.empty()) {
		return;
	}

	std::vector<int> labels;
	int nclasses = partition(candidates, labels);
	std::vector<Rect> rrects(nclasses);
	std::vector<int> rweights(nclasses, 0);
	int nlabels = (int)labels.size();
	size_t i, j;

	for (i = 0; i < nlabels; i++) {
		int cls = labels[i];
		rrects[cls].x += candidates[i].x;
		rrects[cls].y += candidates[i].y;
		rrects[cls].width += candidates[i].width;
		rrects[cls].height += candidates[i].height;
		rweights[cls]++;
	}

	for (i = 0; i < nclasses; i++) {
		Rect r = rrects[i];
		double s = 1.f / rweights[i];
		rrects[i] = Rect(doubleRound(r.x * s),
						 doubleRound(r.y * s),
						 doubleRound(r.width * s),
						 doubleRound(r.height * s));
	}

	candidates.clear();

	for (i = 0; i < nclasses; i++) {
		Rect r1 = rrects[i];
		int n1 = rweights[i];

		if (n1 <= threshold) {
			continue;
		}

		for (j = 0; j < nclasses; j++) {
			int n2 = rweights[j];
			if (j == i || n2 <= threshold) {
				continue;
			}

			Rect r2 = rrects[j];
			int dx = doubleToInt((double)r2.width * eps);
			int dy = doubleToInt((double)r2.height * eps);

			if (i != j &&
				r1.x >= r2.x - dx &&
				r1.y >= r2.y - dy &&
				r1.x + r1.width <= r2.x + r2.width + dx &&
				r1.y + r1.height <= r2.y + r2.height + dy &&
				(n2 > intMax(3, n1)) || (n1 < 3)) {
				break;
			}
		}

		if (j == nclasses) {
			candidates.push_back(r1);
		}
	}	
}

void CascadeClassifier::saveMatToCsv(const cv::Mat& mat, const std::string& filename)
{
	std::ofstream outputFile(filename);
	if (!outputFile) {
		return;
	}
	outputFile << cv::format(mat, cv::Formatter::FMT_CSV);
}

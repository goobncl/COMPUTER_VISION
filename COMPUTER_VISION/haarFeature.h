#pragma once

#ifndef HAAR_FEATURE_H
#define HAAR_FEATURE_H

#define TRUE 1
#define FALSE 0
#define RECT_NUM 3
#define CALC_SUM_OFFSET_(p0, p1, p2, p3, ptr) ((ptr)[p0] - (ptr)[p1] - (ptr)[p2] + (ptr)[p3])
#define CALC_SUM_OFFSET(rect, ptr) CALC_SUM_OFFSET_((rect)[0], (rect)[1], (rect)[2], (rect)[3], ptr)


typedef int Bool;

struct Point {
    int x;
    int y;
    Point() : x(0), y(0) {}
    Point(int x, int y) : x(x), y(y) {}
};

struct Size {
    int width;
    int height;
    Size() : width(0), height(0) {}
    Size(int w, int h) : width(w), height(h) {}
};

struct ScaleData {
    
    ScaleData() : scale(0.f), layer_offset(0), ystep(0) {};
    Size getWorkingSize(Size winSz) const {
        int widthDifference = szi.width - winSz.width;
        int heightDifference = szi.height - winSz.height;
        int workingWidth = widthDifference > 0 ? widthDifference : 0;
        int workingHeight = heightDifference > 0 ? heightDifference : 0;
        return Size(workingWidth, workingHeight);
    }
    double scale;
    Size szi;
    int layer_offset;
    int ystep;
};

struct DTreeNode {
    int featureIdx;
    float threshold;
    int left;
    int right;
};

struct DTree {
    int nodeCount;
};

struct Stage {
    int first;
    int ntrees;
    float threshold;
};

struct Stump {
    int featureIdx;
    float threshold;
    float left;
    float right;
};

struct Data {
    int stageType;
    int featureType;
    int ncategories;
    int minNodesPerTree, maxNodesPerTree;
    Size origWinSize;
    Stage* stages;
    DTree* classifiers;
    DTreeNode* nodes;
    float* leaves;
    int* subsets;
    Stump* stumps;
};

struct Rect {
    int x;
    int y;
    int width;
    int height;
};

struct Feature {
    Bool tilted;
    struct {
        Rect r;
        float weight;
    } rect[RECT_NUM];
};

typedef struct _opt_feature {
    int offset[RECT_NUM][4];
    float weight[4];
} OptFeature;

struct HaarEvaluator {
    Feature* features;
    OptFeature* optfeatures;
    OptFeature* optfeatures_lbuf;
    Bool hasTiltedFeatures;
    int tilted_offset, squares_offset;
    int normalizaed_offset[4];
    Rect normrect;
    const int* pwin;
    OptFeature* optfeaturesPtr;
    float varianceNormFactor;
};

void Feature_init(Feature* feature);
void OptFeature_init(OptFeature* opt_feature);
static float HaarEvaluator_OptFeature_calc(const OptFeature* feature, const int* ptr);

#endif // HAAR_FEATURE_H

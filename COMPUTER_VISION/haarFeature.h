#pragma once

#ifndef HAAR_FEATURE_H
#define HAAR_FEATURE_H

#define TRUE 1
#define FALSE 0
#define RECT_NUM 3
#define CALC_SUM_OFFSET_(p0, p1, p2, p3, ptr) ((ptr)[p0] - (ptr)[p1] - (ptr)[p2] + (ptr)[p3])
#define CALC_SUM_OFFSET(rect, ptr) CALC_SUM_OFFSET_((rect)[0], (rect)[1], (rect)[2], (rect)[3], ptr)


typedef int Bool;

struct Size {
    int width;
    int height;
    Size() : width(0), height(0) {}
    Size(int w, int h) : width(w), height(h) {}
};

typedef struct _dtreenode {
    int featureIdx;
    float threshold;
    int left;
    int right;
} DTreeNode;

typedef struct _dtree {
    int nodeCount;
} DTree;

typedef struct _stage {
    int first;
    int ntrees;
    float threshold;
} Stage;

typedef struct _stump {
    int featureIdx;
    float threshold;
    float left;
    float right;
} Stump;

typedef struct _data {
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
} Data;


typedef struct _rect {
    int x;
    int y;
    int width;
    int height;
} Rect;

typedef struct _feature {
    Bool tilted;
    struct {
        Rect r;
        float weight;
    } rect[RECT_NUM];
} Feature;

typedef struct _opt_feature {
    int offset[RECT_NUM][4];
    float weight[4];
} OptFeature;

typedef struct _haar_evaluator {
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
} HaarEvaluator;

void Feature_init(Feature* feature);
void OptFeature_init(OptFeature* opt_feature);
static float HaarEvaluator_OptFeature_calc(const OptFeature* feature, const int* ptr);

#endif // HAAR_FEATURE_H

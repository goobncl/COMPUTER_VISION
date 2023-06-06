#pragma once

#ifndef HAAR_FEATURE_H
#define HAAR_FEATURE_H

#define TRUE 1
#define FALSE 0
#define RECT_NUM 3
#define CALC_SUM_OFFSET_(p0, p1, p2, p3, ptr) ((ptr)[p0] - (ptr)[p1] - (ptr)[p2] + (ptr)[p3])
#define CALC_SUM_OFFSET(rect, ptr) CALC_SUM_OFFSET_((rect)[0], (rect)[1], (rect)[2], (rect)[3], ptr)
#define SUM_OFS( p0, p1, p2, p3, sum, rect, step )                 \
(p0) = sum + (rect).x + (step) * (rect).y,                            \
(p1) = sum + (rect).x + (rect).width + (step) * (rect).y,             \
(p2) = sum + (rect).x + (step) * ((rect).y + (rect).height),          \
(p3) = sum + (rect).x + (rect).width + (step) * ((rect).y + (rect).height)


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

struct ImgLayer {
    Size sz;
    unsigned char* data;
    int* sum;
    int* sqsum;
    ImgLayer() : sz(Size(0, 0)), data(NULL), sum(NULL), sqsum(NULL) {}
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

struct Rect {
    int x;
    int y;
    int width;
    int height;
    Rect() : x(0), y(0), width(0), height(0) {}
    Rect(int x, int y, int w, int h) : x(x), y(y), width(w), height(h) {}
};

struct Feature {
    Bool tilted;
    struct {
        Rect r;
        float weight;
    } rect[RECT_NUM];

    Feature() {
        tilted = false;
        rect[0].r = rect[1].r = rect[2].r = Rect();
        rect[0].weight = rect[1].weight = rect[2].weight = 0;
    }
};

struct OptFeature {
    int offset[RECT_NUM][4];
    float weight[4];

    OptFeature() {
        weight[0] = weight[1] = weight[2] = 0.f;
        offset[0][0] = offset[0][1] = offset[0][2] = offset[0][3] =
        offset[1][0] = offset[1][1] = offset[1][2] = offset[1][3] =
        offset[2][0] = offset[2][1] = offset[2][2] = offset[2][3] = 0;
    }

    void setOffsets(Feature& _f, int step) {
        weight[0] = _f.rect[0].weight;
        weight[1] = _f.rect[1].weight;
        weight[2] = _f.rect[2].weight;
        SUM_OFS(offset[0][0], offset[0][1], offset[0][2], offset[0][3], 0, _f.rect[0].r, step);
        SUM_OFS(offset[1][0], offset[1][1], offset[1][2], offset[1][3], 0, _f.rect[1].r, step);
        SUM_OFS(offset[2][0], offset[2][1], offset[2][2], offset[2][3], 0, _f.rect[2].r, step);
    }

    double calc(const int* ptr) const {
        double ret = weight[0] * CALC_SUM_OFFSET(offset[0], ptr) + weight[1] * CALC_SUM_OFFSET(offset[1], ptr);
        if (weight[2] != 0.0f) {
            ret += weight[2] * CALC_SUM_OFFSET(offset[2], ptr);
        }
        return ret;
    }
};

struct HaarEvaluator {
    Feature* features;
    OptFeature* optfeatures;
    OptFeature* optfeatures_lbuf;
    Bool hasTiltedFeatures;
    int tilted_offset, sqr_offset;
    int norm_offset[4];
    Rect normrect;
    const int* pwin;
    OptFeature* optfeaturesPtr;
    float varianceNormFactor;
};

#endif // HAAR_FEATURE_H

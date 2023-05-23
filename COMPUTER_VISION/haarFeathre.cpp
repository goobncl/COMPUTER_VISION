#include "stdafx.h"
#include "haarFeature.h"


void Feature_init(Feature* feature) {

    feature->tilted = FALSE;

    for (int i = 0; i < RECT_NUM; i++) {
        feature->rect[i].r.x = 0;
        feature->rect[i].r.y = 0;
        feature->rect[i].r.width = 0;
        feature->rect[i].r.height = 0;
        feature->rect[i].weight = 0.0f;
    }
}

void OptFeature_init(OptFeature* opt_feature) {

    for (int i = 0; i < RECT_NUM; i++) {
        for (int j = 0; j < 4; j++) {
            opt_feature->offset[i][j] = 0;
        }
    }

    for (int i = 0; i < 4; i++) {
        opt_feature->weight[i] = 0.0f;
    }
}

static float HaarEvaluator_OptFeature_calc(const OptFeature* feature, const int* ptr) {

    float ret = feature->weight[0] * CALC_SUM_OFFSET(feature->offset[0], ptr) +
        feature->weight[1] * CALC_SUM_OFFSET(feature->offset[1], ptr);

    if (feature->weight[2] != 0.0f) {
        ret += feature->weight[2] * CALC_SUM_OFFSET(feature->offset[2], ptr);
    }

    return ret;
}


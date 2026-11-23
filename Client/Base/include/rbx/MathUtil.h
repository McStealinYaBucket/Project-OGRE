// Base/include/rbx/MathUtil.h: October 2013 reconstruction (Project OGRE).
// Evidence: iOS 1029 symbols RBX::IsValueOutlier(double, unsigned int, double, double,
// RBX::Confidence) and RBX::GetConfidenceInterval(double, double, RBX::Confidence, double*, double*).
// RunningAverage.h's sanitized statistics use both.
#pragma once

namespace RBX {

// Confidence levels: 90%, 95%, 99%, 99.9%.
enum Confidence { C90, C95, C99, C99p9, ConfidenceMax };

double IsValueOutlier(double value, unsigned count, double average, double std, Confidence conf);
void GetConfidenceInterval(double average, double variance, Confidence conf, double* minV, double* maxV);

}

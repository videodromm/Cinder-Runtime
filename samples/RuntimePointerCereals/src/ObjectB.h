#pragma once

#include "cinder/gl/Batch.h"

class ObjectB {
public:
	ObjectB();
	virtual void draw();
protected:
	float mRotation;
	ci::gl::BatchRef mBatch;
};
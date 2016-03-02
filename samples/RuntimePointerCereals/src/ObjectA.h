#pragma once

#include "cinder/gl/Batch.h"
#include "cinder/params/Params.h"
#include <cereal/archives/binary.hpp>

class ObjectA {
public:
	ObjectA();
	virtual void draw();
	virtual void drawParams();
	
	// !!! Serialization functions can't be modified at runtime
	// but still have to be marked "virtual" !!!
	virtual void save( cereal::BinaryOutputArchive &ar );
	virtual void load( cereal::BinaryInputArchive &ar );
protected:
	virtual void initParams();
	
	bool mIsValid;
	int mSubdivisions;
	float mRotation, mSize;
	ci::params::InterfaceGlRef mParams;
	ci::gl::BatchRef mPlane;
};
#include "ObjectB.h"
#include "cinder/app/App.h"
#include "cinder/gl/Shader.h"
#include "cinder/gl/scoped.h"

using namespace std;
using namespace ci;

ObjectB::ObjectB()
: mRotation( 0.0f )
{
	mBatch = gl::Batch::create( geom::Teapot(), gl::getStockShader( gl::ShaderDef().lambert() ) );
}
void ObjectB::draw()
{
	gl::ScopedModelMatrix scopedModelMatrix;
	gl::ScopedDepth scopedDepth( true );
	gl::rotate( mRotation, vec3( 0.5f, 1.0f, 0.0f ) );
	mBatch->draw();
	
	mRotation += 0.01f;
}

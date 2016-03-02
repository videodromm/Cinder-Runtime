#include "ObjectA.h"
#include "cinder/app/App.h"
#include "cinder/gl/Shader.h"
#include "cinder/gl/scoped.h"

using namespace std;
using namespace ci;

ObjectA::ObjectA()
: mSubdivisions( 10 ), mSize( 10 ), mRotation( 0.0f ), mIsValid( true )
{
	mPlane = gl::Batch::create( geom::Plane().subdivisions( ivec2( mSubdivisions ) ).size( vec2( mSize ) ) >> geom::Lines(), gl::getStockShader( gl::ShaderDef().color() ) );
}

void ObjectA::initParams()
{
	mParams = params::InterfaceGl::create( app::App::get()->getWindow(), "ObjA Params", ivec2( 260, 130 ) );
	mParams->setOptions( "", "valueswidth=170" );
	mParams->addText( "Try to modify ObjA.cpp at runtime" );
	mParams->addSeparator();
	mParams->addParam( "Subdivs", &mSubdivisions ).min(1).max(100).updateFn( [this](){ mIsValid = false; } );
	mParams->addParam( "Size", &mSize ).min(0.1f).max(100).step(0.1f).updateFn( [this](){ mIsValid = false; } );
	mParams->addSeparator();
	static string version = "ObjectA";
	mParams->addParam( "Version", &version );
}
void ObjectA::draw()
{
	// check if the vbo mesh needs to be updated
	if( !mIsValid ) {
		mPlane->replaceVboMesh( gl::VboMesh::create( geom::Plane().subdivisions( ivec2( mSubdivisions ) ).size( vec2( mSize ) ) >> geom::Lines() ) );
		mIsValid = true;
	}
	
	gl::ScopedModelMatrix scopedModelMatrix;
	gl::ScopedDepth scopedDepth( true );
	gl::rotate( mRotation, vec3( 0.5f, 1.0f, 0.0f ) );
	mPlane->draw();
	
	mRotation += 0.01f;
}
void ObjectA::drawParams()
{
	if( !mParams )
		initParams();
	mParams->draw();
}

// !!! Serialization functions can't be modified at runtime
void ObjectA::save( cereal::BinaryOutputArchive &ar )
{
	ar( mRotation, mSize, mSubdivisions );
}
// !!! Serialization functions can't be modified at runtime
void ObjectA::load( cereal::BinaryInputArchive &ar )
{
	ar( mRotation, mSize, mSubdivisions );
	mIsValid = false;
}
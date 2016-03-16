#include "cinder/app/App.h"
#include "cinder/app/RendererGl.h"
#include "cinder/gl/gl.h"
#include "cinder/CameraUi.h"
#include "cinder/Timeline.h"

// uncomment this to disable runtime compilation
//#define DISABLE_RUNTIME_COMPILED_APP
// Define RUNTIME_APP_CEREALIZATION before including runtime_app
#define RUNTIME_APP_CEREALIZATION
#include "runtime_app.h"

#include <cereal/archives/binary.hpp>

using namespace ci;
using namespace ci::app;
using namespace std;

class RuntimeCerealApp : public App {
public:
	void setup() override;
	void draw() override;
	
	virtual void save( cereal::BinaryOutputArchive &ar );
	virtual void load( cereal::BinaryInputArchive &ar );
	
	gl::BatchRef	mPlane, mTeapot;
	CameraPersp		mCamera;
	CameraUi		mCameraUi;
};

void RuntimeCerealApp::setup()
{
	mPlane		= gl::Batch::create( geom::Plane().subdivisions( ivec2( 10 ) ) >> geom::Lines(), gl::getStockShader( gl::ShaderDef().color() ) );
	mTeapot		= gl::Batch::create( geom::Teapot() >> geom::Scale( vec3( 0.35f ) ), gl::getStockShader( gl::ShaderDef().lambert() ) );
	mCamera		= CameraPersp( getWindowWidth(), getWindowHeight(), 60, 0.1, 1000 );
	mCameraUi	= CameraUi( &mCamera, getWindow(), -1 );
	mCamera.lookAt( vec3( 1.0f, 1.0f, 1.0f ), vec3( 0.0f ) );
}

void RuntimeCerealApp::draw()
{
	gl::clear( Color( 0, 0, 0 ) );
	
	gl::setMatrices( mCamera );
	gl::ScopedDepth enableDepth( true );
	
	mPlane->draw();
	gl::rotate( getElapsedSeconds(), vec3( 0.0f, 1.0f, 0.0f ) );
	mTeapot->draw();
}
void RuntimeCerealApp::save( cereal::BinaryOutputArchive &ar )
{
	ar( mCamera.getEyePoint().x, mCamera.getEyePoint().y, mCamera.getEyePoint().z, mCamera.getOrientation().x, mCamera.getOrientation().y, mCamera.getOrientation().z, mCamera.getOrientation().w, mCamera.getPivotDistance() );
}
void RuntimeCerealApp::load( cereal::BinaryInputArchive &ar )
{
	ci::vec3 eye;
	ci::quat orientation;
	float pivotDist;
	ar( eye.x, eye.y, eye.z, orientation.x, orientation.y, orientation.z, orientation.w, pivotDist );
	mCamera.setEyePoint( eye );
	mCamera.setOrientation( orientation );
	mCamera.setPivotDistance( pivotDist );
}

CINDER_RUNTIME_APP( RuntimeCerealApp, RendererGl )

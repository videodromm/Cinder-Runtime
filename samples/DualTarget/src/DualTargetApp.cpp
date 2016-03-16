#include "cinder/app/App.h"
#include "cinder/app/RendererGl.h"
#include "cinder/gl/gl.h"
#include "cinder/CameraUi.h"

#include "runtime_app.h"

using namespace ci;
using namespace ci::app;
using namespace std;

class DualTargetApp : public App {
  public:
	void setup() override;
	void draw() override;
	
	CameraPersp		mCamera;
	CameraUi		mCameraUi;
	
	ci::gl::BatchRef mPlane, mTeapot;
};

void DualTargetApp::setup()
{
	mPlane = gl::Batch::create( geom::Plane().subdivisions( ivec2( 10 ) ).size( vec2( 10 ) ) >> geom::Lines(), gl::getStockShader( gl::ShaderDef().color() ) );
	mTeapot = gl::Batch::create( geom::Teapot(), gl::getStockShader( gl::ShaderDef().lambert() ) );
	mCamera	= CameraPersp( getWindowWidth(), getWindowHeight(), 60, 0.1, 1000 );
	mCameraUi = CameraUi( &mCamera, getWindow(), -1 );
	mCamera.lookAt( vec3( 2.0f, 2.0f, 3.0f ), vec3( 0.0f ) );
}

void DualTargetApp::draw()
{
	gl::clear( Color( 0.5f, 0.5f, 0.5f ) );
	gl::setMatrices( mCamera );
	gl::ScopedDepth scopedDepth( true );
	mPlane->draw();
	mTeapot->draw();
}

CINDER_RUNTIME_APP( DualTargetApp, RendererGl )

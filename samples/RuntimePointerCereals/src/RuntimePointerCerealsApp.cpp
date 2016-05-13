#include "cinder/app/App.h"
#include "cinder/app/RendererGl.h"
#include "cinder/gl/gl.h"
#include "cinder/CameraUi.h"

// Define RUNTIME_PTR_CEREALIZATION before including runtime_ptr
#define RUNTIME_PTR_CEREALIZATION
#include "runtime_ptr.h"

#include "ObjectA.h"
#include "ObjectB.h"

using namespace ci;
using namespace ci::app;
using namespace std;

class RuntimePointerCerealsApp : public App {
public:
	void setup() override;
	void draw() override;
	
	CameraPersp		mCamera;
	CameraUi		mCameraUi;
	
	runtime_ptr<ObjectA> mObjA;
	runtime_ptr<ObjectB> mObjB;
};

void RuntimePointerCerealsApp::setup()
{
#ifndef DISABLE_RUNTIME_COMPILED_PTR
	runtime_class<ObjectA>::initialize( "ObjectA.cpp", runtime_class<ObjectA>::Options()
									   .cinder()
									   // we need to add our "/blocks/Cinder-Cereal/lib" folder so Cling knows where to find <cereal/...>
									  .includePath( "../../../blocks/Cinder-Cereal/lib/cereal/include" ) );
	runtime_class<ObjectB>::initialize( "ObjectB.cpp", runtime_class<ObjectB>::Options().cinder() );
#endif
	
	// create the runtime pointers
	// try to save ObjectA.cpp and ObjectB.cpp at runtime, you'll see that mObjB will restart its rotation at 0.0 everytime because it doesn't support serialization.
	// On the other hand mObjA will restore its previous state every time including the modifications done with its UI
	mObjA = make_runtime<ObjectA>();
	mObjB = make_runtime<ObjectB>();
	
	// create a camera and a camera ui
	mCamera	= CameraPersp( getWindowWidth(), getWindowHeight(), 60, 0.1, 1000 ).calcFraming( Sphere( vec3(0), 2 ) );
	mCameraUi = CameraUi( &mCamera, getWindow(), -1 );
}

void RuntimePointerCerealsApp::draw()
{
	gl::clear( Color::gray( 0.5f ) );
	gl::setMatrices( mCamera );
	
	if( mObjA ) {
		mObjA->draw();
	}
	if( mObjB ) {
		mObjB->draw();
	}
	
	// render mObjA ui
	gl::setMatricesWindow( getWindowSize() );
	if( mObjA ) {
		mObjA->drawParams();
	}
}

CINDER_APP( RuntimePointerCerealsApp, RendererGl )

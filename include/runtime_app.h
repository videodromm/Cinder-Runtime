/*
 Cinder-Runtime
 App
 Copyright (c) 2016, Simon Geilfus, All rights reserved.
 
 Redistribution and use in source and binary forms, with or without modification, are permitted provided that
 the following conditions are met:
 
 * Redistributions of source code must retain the above copyright notice, this list of conditions and
	the following disclaimer.
 * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and
	the following disclaimer in the documentation and/or other materials provided with the distribution.
 
 THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED
 WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A
 PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR
 ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED
 TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 POSSIBILITY OF SUCH DAMAGE.
 */

#pragma once

#include "cinder/app/App.h"
#include "cinder/app/RendererGl.h"

#ifdef RUNTIME_APP_CEREALIZATION
#include <utility>
#include <cereal/archives/binary.hpp>
#endif

#if ! defined( DISABLE_RUNTIME_COMPILATION ) && ! defined( DISABLE_RUNTIME_COMPILED_APP )

#include "cinder/Exception.h"
#include "cinder/Filesystem.h"
#include "cinder/System.h"
#include "cinder/Utilities.h"
#include "cling/Interpreter/Interpreter.h"
#include "SourceWatchdog.h"

class RuntimeApp;

class RuntimeAppWrapper/* : public ci::app::AppBase*/ {
public:
	
	//! Override to perform any application setup after the Renderer has been initialized.
	virtual void	setup() {}
	//! Override to perform any once-per-loop computation.
	virtual void	update() {}
	//! Override to perform any rendering once-per-loop or in response to OS-prompted requests for refreshes.
	virtual void	draw() {}
	
	//! Override to receive mouse-down events.
	virtual void	mouseDown( ci::app::MouseEvent event ) {}
	//! Override to receive mouse-up events.
	virtual void	mouseUp( ci::app::MouseEvent event ) {}
	//! Override to receive mouse-wheel events.
	virtual void	mouseWheel( ci::app::MouseEvent event ) {}
	//! Override to receive mouse-move events.
	virtual void	mouseMove( ci::app::MouseEvent event ) {}
	//! Override to receive mouse-drag events.
	virtual void	mouseDrag( ci::app::MouseEvent event ) {}
	
	//! Override to respond to the beginning of a multitouch sequence
	virtual void	touchesBegan( ci::app::TouchEvent event ) {}
	//! Override to respond to movement (drags) during a multitouch sequence
	virtual void	touchesMoved( ci::app::TouchEvent event ) {}
	//! Override to respond to the end of a multitouch sequence
	virtual void	touchesEnded( ci::app::TouchEvent event ) {}
	
	//! Override to receive key-down events.
	virtual void	keyDown( ci::app::KeyEvent event ) {}
	//! Override to receive key-up events.
	virtual void	keyUp( ci::app::KeyEvent event ) {}
	//! Override to receive window resize events.
	virtual void	resize() {}
	//! Override to receive file-drop events.
	virtual void	fileDrop( ci::app::FileDropEvent event ) {}
	
	//! Override to cleanup any resources before app destruction
	virtual void	cleanup() {}
	
	void launch() {}
	 
	ci::app::WindowRef	createWindow( const ci::app::Window::Format &format = ci::app::Window::Format() )/* override*/;
	void		quit()/* override*/;
	
	float		getFrameRate() const/* override*/;
	void		setFrameRate( float frameRate )/* override*/;
	void		disableFrameRate()/* override*/;
	bool		isFrameRateEnabled() const/* override*/;
	
	ci::app::WindowRef	getWindow() const/* override*/;
	ci::app::WindowRef	getWindowIndex( size_t index ) const/* override*/;
	size_t		getNumWindows() const/* override*/;
	
	ci::app::WindowRef	getForegroundWindow() const/* override*/;
	
	void		hideCursor()/* override*/;
	void		showCursor()/* override*/;
	ci::ivec2		getMousePos() const/* override*/;
	
	
	
	//! Emitted at the start of each application update cycle
	ci::signals::Signal<void()>&	getSignalUpdate();
	
	//! Signal that emits before the app quit process begins. If any slots return false then the app quitting is canceled.
	ci::app::EventSignalShouldQuit&		getSignalShouldQuit();
	//! Emitted prior to the application shutting down
	ci::signals::Signal<void()>&	getSignalCleanup();
	void 						emitCleanup();
	
	ci::signals::Signal<void()>&	getSignalWillResignActive();
	void 						emitWillResignActive();
	ci::signals::Signal<void()>&	getSignalDidBecomeActive();
	void 						emitDidBecomeActive();
	
	//! Emitted when a new display is connected to the system
	ci::signals::Signal<void(const ci::DisplayRef &display)>&	getSignalDisplayConnected();
	//! Emits a signal indicating a new display has connected to the system
	void												emitDisplayConnected( const ci::DisplayRef &display );
	//! Emitted when a display is removed from the system
	ci::signals::Signal<void(const ci::DisplayRef &display)>&	getSignalDisplayDisconnected();
	//! Emits a signal indicating a display has disconnected from the system
	void												emitDisplayDisconnected( const ci::DisplayRef &display );
	//! Emitted when the resolution or some other property of a Display is changed
	ci::signals::Signal<void(const ci::DisplayRef &display)>&	getSignalDisplayChanged() ;
	//! Emits a signal when the resolution or some other property of a Display has changed
	void												emitDisplayChanged( const ci::DisplayRef &display );
	
	const std::vector<ci::app::TouchEvent::Touch>& 	getActiveTouches() const ;
	
	//! Returns the Renderer of the active Window
	ci::app::RendererRef			getRenderer() const;
	//! Returns the Display of the active Window
	ci::DisplayRef			getDisplay() const;
	
	//! a value of true allows screensavers or the system's power management to hide the app. Default value is \c false on desktop, and \c true on mobile
	virtual void	enablePowerManagement( bool powerManagement = true );
	//! is power management enabled, allowing screensavers and the system's power management to hide the application
	virtual bool	isPowerManagementEnabled() const;
	
	
	//! Returns the X coordinate of the top-left corner of the current window measured in points
	int         		getWindowPosX() const;
	//! Returns the Y coordinate of the top-left corner of the current window contents measured in points
	int         		getWindowPosY() const;
	
	//! Returns the average frame-rate attained by the App as measured in frames-per-second
	float				getAverageFps() const;
	//! Returns the sampling rate in seconds for measuring the average frame-per-second as returned by getAverageFps()
	double				getFpsSampleInterval() const;
	//! Sets the sampling rate in seconds for measuring the average frame-per-second as returned by getAverageFps()
	void				setFpsSampleInterval( double sampleInterval );
	
	//! Returns whether the App is in full-screen mode or not.
	virtual bool		isFullScreen() const;
	//! Sets whether the active App is in full-screen mode based on \a fullScreen
	virtual void		setFullScreen( bool fullScreen, const ci::app::FullScreenOptions &options = ci::app::FullScreenOptions() );
	
	
	//! Returns whether the app is registered to receive multiTouch events from the operating system, configurable via Settings at startup. Disabled by default on desktop platforms, enabled on mobile.
	bool				isMultiTouchEnabled() const;
	//! Returns whether Windows created on a high-density (Retina) display will have their resolution doubled, configurable via Settings at startup. Default is \c true on iOS and \c false on other platforms.
	bool				isHighDensityDisplayEnabled() const;
	
	//! Returns a vector of the command line arguments passed to the app when intantiated.
	const std::vector<std::string>&		getCommandLineArgs() const;
	
	//! Returns a reference to the App's boost::asio::io_service()
	asio::io_service&	io_service();
	
	//! Executes a std::function on the App's primary thread ahead of the next update()
	void	dispatchAsync( const std::function<void()> &fn );
	
	template<typename T>
	typename std::result_of<T()>::type dispatchSync( T fn );
	
	//! Returns the default Renderer which will be used when creating a new Window. Set by the app instantiation macro automatically.
	ci::app::RendererRef	getDefaultRenderer() const;
	
#ifdef RUNTIME_APP_CEREALIZATION
	virtual void save( cereal::BinaryOutputArchive &ar ) {}
	virtual void load( cereal::BinaryInputArchive &ar ) {}
#endif
	
	RuntimeApp* mParent;
};

class RuntimeApp : public ci::app::App {
public:
	RuntimeApp(){}
	virtual ~RuntimeApp(){}
	
	//! \cond
	// Called during application instanciation via CINDER_APP_MAC macro
	template<typename AppT>
	static void main( const ci::app::RendererRef &defaultRenderer, const char *title, int argc, char * const argv[], const std::string &file, const SettingsFn &settingsFn = SettingsFn(), const std::function<void(cling::Interpreter *)> &runtimeSettingsFn = std::function<void(cling::Interpreter *)>() );
	//! \endcond
	
	//! Override to perform any application setup after the Renderer has been initialized.
	virtual void	setup() { if( mRuntimeImpl ) mRuntimeImpl->setup(); }
	//! Override to perform any once-per-loop computation.
	virtual void	update() { if( mRuntimeImpl ) mRuntimeImpl->update(); }
	//! Override to perform any rendering once-per-loop or in response to OS-prompted requests for refreshes.
	virtual void	draw() { if( mRuntimeImpl ) mRuntimeImpl->draw(); else ci::gl::clear(); }
	
	//! Override to receive mouse-down events.
	virtual void	mouseDown( ci::app::MouseEvent event ) { if( mRuntimeImpl ) mRuntimeImpl->mouseDown( event ); }
	//! Override to receive mouse-up events.
	virtual void	mouseUp( ci::app::MouseEvent event ) { if( mRuntimeImpl ) mRuntimeImpl->mouseUp( event ); }
	//! Override to receive mouse-wheel events.
	virtual void	mouseWheel( ci::app::MouseEvent event ) { if( mRuntimeImpl ) mRuntimeImpl->mouseWheel( event ); }
	//! Override to receive mouse-move events.
	virtual void	mouseMove( ci::app::MouseEvent event ) { if( mRuntimeImpl ) mRuntimeImpl->mouseMove( event ); }
	//! Override to receive mouse-drag events.
	virtual void	mouseDrag( ci::app::MouseEvent event ) { if( mRuntimeImpl ) mRuntimeImpl->mouseDrag( event ); }
	
	//! Override to respond to the beginning of a multitouch sequence
	virtual void	touchesBegan( ci::app::TouchEvent event ) { if( mRuntimeImpl ) mRuntimeImpl->touchesBegan( event ); }
	//! Override to respond to movement (drags) during a multitouch sequence
	virtual void	touchesMoved( ci::app::TouchEvent event ) { if( mRuntimeImpl ) mRuntimeImpl->touchesMoved( event ); }
	//! Override to respond to the end of a multitouch sequence
	virtual void	touchesEnded( ci::app::TouchEvent event ) { if( mRuntimeImpl ) mRuntimeImpl->touchesEnded( event ); }
	
	//! Override to receive key-down events.
	virtual void	keyDown( ci::app::KeyEvent event ) { if( mRuntimeImpl ) mRuntimeImpl->keyDown( event ); }
	//! Override to receive key-up events.
	virtual void	keyUp( ci::app::KeyEvent event ) { if( mRuntimeImpl ) mRuntimeImpl->keyUp( event ); }
	//! Override to receive window resize events.
	virtual void	resize() { if( mRuntimeImpl ) mRuntimeImpl->resize(); }
	//! Override to receive file-drop events.
	virtual void	fileDrop( ci::app::FileDropEvent event ) { if( mRuntimeImpl ) mRuntimeImpl->fileDrop( event ); }
	
	//! Override to cleanup any resources before app destruction
	virtual void	cleanup() { if( mRuntimeImpl ) mRuntimeImpl->cleanup(); }

protected:

	std::shared_ptr<RuntimeAppWrapper> mRuntimeImpl;
};

ci::app::WindowRef	RuntimeAppWrapper::createWindow( const ci::app::Window::Format &format )
{
	return mParent->createWindow( format );
}
void RuntimeAppWrapper::quit() {
	mParent->quit();
}

float RuntimeAppWrapper::getFrameRate() const
{
	return mParent->getFrameRate();
}
void RuntimeAppWrapper::setFrameRate( float frameRate )
{
	mParent->setFrameRate( frameRate );
}
void RuntimeAppWrapper::disableFrameRate()
{
	mParent->disableFrameRate();
}
bool RuntimeAppWrapper::isFrameRateEnabled() const
{
	return mParent->isFrameRateEnabled();
}

ci::app::WindowRef RuntimeAppWrapper::getWindow() const
{
	return mParent->getWindow();
}
ci::app::WindowRef RuntimeAppWrapper::getWindowIndex( size_t index ) const
{
	return mParent->getWindowIndex( index );
}
size_t RuntimeAppWrapper::getNumWindows() const
{
	return mParent->getNumWindows();
}

ci::app::WindowRef RuntimeAppWrapper::getForegroundWindow() const
{
	return mParent->getForegroundWindow();
}

void RuntimeAppWrapper::hideCursor()
{
	mParent->hideCursor();
}
void RuntimeAppWrapper::showCursor()
{
	mParent->showCursor();
}
ci::ivec2 RuntimeAppWrapper::getMousePos() const
{
	return mParent->getMousePos();
}
ci::signals::Signal<void()>& RuntimeAppWrapper::getSignalUpdate()
{
	return mParent->getSignalUpdate();
}
ci::app::EventSignalShouldQuit& RuntimeAppWrapper::getSignalShouldQuit()
{
	return mParent->getSignalShouldQuit();
}
ci::signals::Signal<void()>& RuntimeAppWrapper::getSignalCleanup()
{
	return mParent->getSignalCleanup();
}
void RuntimeAppWrapper::emitCleanup()
{
	mParent->emitCleanup();
}
ci::signals::Signal<void()>& RuntimeAppWrapper::getSignalWillResignActive()
{
	return mParent->getSignalWillResignActive();
}
void RuntimeAppWrapper::emitWillResignActive()
{
	mParent->emitWillResignActive();
}
ci::signals::Signal<void()>& RuntimeAppWrapper::getSignalDidBecomeActive()
{
	return mParent->getSignalDidBecomeActive();
}
void RuntimeAppWrapper::emitDidBecomeActive()
{
	mParent->emitDidBecomeActive();
}
ci::signals::Signal<void(const ci::DisplayRef &display)>& RuntimeAppWrapper::getSignalDisplayConnected()
{
	return mParent->getSignalDisplayConnected();
}
void RuntimeAppWrapper::emitDisplayConnected( const ci::DisplayRef &display )
{
	mParent->emitDisplayConnected( display );
}
ci::signals::Signal<void(const ci::DisplayRef &display)>& RuntimeAppWrapper::getSignalDisplayDisconnected()
{
	return mParent->getSignalDisplayDisconnected();
}
void RuntimeAppWrapper::emitDisplayDisconnected( const ci::DisplayRef &display )
{
	mParent->emitDisplayDisconnected( display );
}
ci::signals::Signal<void(const ci::DisplayRef &display)>& RuntimeAppWrapper::getSignalDisplayChanged() 
{
	return mParent->getSignalDisplayChanged();
}
void RuntimeAppWrapper::emitDisplayChanged( const ci::DisplayRef &display )
{
	mParent->emitDisplayChanged( display );
}
const std::vector<ci::app::TouchEvent::Touch>& RuntimeAppWrapper::getActiveTouches() const 
{
	return mParent->getActiveTouches();
}
ci::app::RendererRef RuntimeAppWrapper::getRenderer() const
{
	return mParent->getRenderer();
}
ci::DisplayRef RuntimeAppWrapper::getDisplay() const
{
	return mParent->getDisplay();
}
void RuntimeAppWrapper::enablePowerManagement( bool powerManagement )
{
	mParent->enablePowerManagement( powerManagement );
}
bool RuntimeAppWrapper::isPowerManagementEnabled() const
{
	return mParent->isPowerManagementEnabled();
}
int RuntimeAppWrapper::getWindowPosX() const
{
	return mParent->getWindowPosX();
}
int RuntimeAppWrapper::getWindowPosY() const
{
	return mParent->getWindowPosY();
}
float RuntimeAppWrapper::getAverageFps() const
{
	return mParent->getAverageFps();
}
double RuntimeAppWrapper::getFpsSampleInterval() const
{
	return mParent->getFpsSampleInterval();
}
void RuntimeAppWrapper::setFpsSampleInterval( double sampleInterval )
{
	mParent->setFpsSampleInterval( sampleInterval );
}
bool RuntimeAppWrapper::isFullScreen() const
{
	return mParent->isFullScreen();
}
void RuntimeAppWrapper::setFullScreen( bool fullScreen, const ci::app::FullScreenOptions &options )
{
	return mParent->setFullScreen( fullScreen, options );
}
bool RuntimeAppWrapper::isMultiTouchEnabled() const
{
	return mParent->isMultiTouchEnabled();
}
bool RuntimeAppWrapper::isHighDensityDisplayEnabled() const
{
	return mParent->isHighDensityDisplayEnabled();
}
const std::vector<std::string>& RuntimeAppWrapper::getCommandLineArgs() const
{
	return mParent->getCommandLineArgs();
}
asio::io_service& RuntimeAppWrapper::io_service()
{
	return mParent->io_service();
}
void RuntimeAppWrapper::dispatchAsync( const std::function<void()> &fn )
{
	mParent->dispatchAsync( fn );
}


template<typename T>
typename std::result_of<T()>::type RuntimeAppWrapper::dispatchSync( T fn )
{
	return mParent->dispatchSync<T>( fn );
}
ci::app::RendererRef RuntimeAppWrapper::getDefaultRenderer() const
{
	return mParent->getDefaultRenderer();
}

template<typename AppT>
void RuntimeApp::main( const ci::app::RendererRef &defaultRenderer, const char *title, int argc, char * const argv[], const std::string &file, const SettingsFn &settingsFn, const std::function<void(cling::Interpreter *)> &runtimeSettingsFn )
{
	// init interpreter
	// initialize cling interpreter
	const char * args[] = { "-std=c++11", "-Wno-inconsistent-missing-override" };
	auto blockPath = ci::fs::path( __FILE__ ).parent_path().parent_path();
	auto interpreter = new cling::Interpreter( 2, args, ( blockPath.string() + "/lib/" ).c_str() );
	
	// add the parent path to the include paths
	auto path = ci::fs::path( file );
	interpreter->AddIncludePath( path.parent_path().string() );
	interpreter->AddIncludePath( ( path.parent_path().parent_path() / "include" ).string() );
	interpreter->AddIncludePath( ( blockPath / "include" ).string() );
	
	// add cinder
	interpreter->declare( "#define GLM_COMPILER 0" );
	interpreter->AddIncludePath( blockPath.parent_path().parent_path().string() + "/include/" );
#if defined(NDEBUG) || defined(_NDEBUG) || defined(RELEASE) || defined(MASTER) || defined(GOLD)
	interpreter->loadFile( blockPath.parent_path().parent_path().string() + "/lib/libcinder.dylib" );
#else
	interpreter->loadFile( blockPath.parent_path().parent_path().string() + "/lib/libcinder_d.dylib" );
#endif
	
	// add other interpreter options
	if( runtimeSettingsFn ) {
		runtimeSettingsFn( interpreter );
	}
	
	// compile the original class
	// copy the file content to a string
	std::string originalCode;
	std::ifstream infile( path.c_str() );
	std::string line;
	std::vector<std::string> includes;
	while( std::getline( infile, line ) ) {
		if( line.find( "CINDER_RUNTIME_APP" ) != std::string::npos ) {
			break;
		}
		//if( line.find( "#include \"RuntimeApp.h\"" ) == std::string::npos ) {
		if( line.find( "#include" ) == std::string::npos ) {
			originalCode += line + " \n";
		}
		else includes.push_back( line );
	}
	
	// make the App inherit from AppBase instead of App
	size_t pos = 0;
	std::string wrapperApp = "public RuntimeAppWrapper";
	while( (pos = originalCode.find( "public App", pos)) != std::string::npos ) {
		originalCode.replace( pos, std::string( "public App" ).length(), wrapperApp );
		pos += wrapperApp.length();
	}
	// prepend the includes
	std::string includesString;
	for( auto inc : includes ) {
		includesString += inc + "\n";
	}
	
	// wrap original code in its own namespace
	originalCode = includesString + "\n\nnamespace RuntimeBase {\n" + originalCode + "\n};";
	//std::cout << "Original Code " << std::endl << originalCode << std::endl << std::endl;
	
	// process the original code once
	interpreter->enableRawInput();
	interpreter->declare( originalCode );
	interpreter->enableRawInput( false );
	interpreter->declare( "#include <memory>" );
	
	// start the app
	ci::app::AppBase::prepareLaunch();
	
	ci::app::AppMac::Settings settings;
	ci::app::AppBase::initialize( &settings, defaultRenderer, title, argc, argv );
	
	if( settingsFn )
		settingsFn( &settings );
	
	if( settings.getShouldQuit() )
		return;
	
	RuntimeApp *runtimeApp = new RuntimeApp();
	
	// watch cpp
	std::string className = ci::System::demangleTypeName( typeid( AppT ).name() );
	SourceWatchdog::watch( path, [path,className,interpreter,runtimeApp]() {
		std::cout << "Loading " << path << std::endl;
		
		// copy the file content to a string
		std::string code;
		
		std::ifstream infile( path.c_str() );
		std::string line;
		std::vector<std::string> includes;
		while( std::getline( infile, line ) ) {
			if( line.find( "CINDER_RUNTIME_APP" ) != std::string::npos ) {
				break;
			}
			if( line.find( "#include" ) == std::string::npos ) {
				code += line + " \n";
			}
			else includes.push_back( line );
		}
		
		// prepend the includes
		std::string includesString;
		for( auto inc : includes ) {
			includesString += inc + "\n";
		}
		
		// make a unique namespace name
		std::string uniqueNamespace;
		interpreter->createUniqueName( uniqueNamespace );
		uniqueNamespace = className + uniqueNamespace;
		
		// wrap the code in its own unique namespace
		code = includesString + "\n\nnamespace " + uniqueNamespace + " {\n" + code + "\n};";
		
		// Make the class inherit from the original one
		std::string classLineToken = "class " + className + " : public App";
		size_t pos = code.find( classLineToken );
		if( pos != std::string::npos ) {
			code.replace( pos, classLineToken.length(), "class " + className + " : public RuntimeBase::" + className + " " );
		}
		//std::cout << code << std::endl;
		
		// process the new code
		interpreter->enableRawInput();
		interpreter->declare( code );
		interpreter->enableRawInput( false );
		
#ifdef RUNTIME_APP_CEREALIZATION
		bool cerealized = false;
		std::stringstream archiveStream;
#endif
		
		// if the instance already exists override it
		std::string instanceName = "runtime_App";
		std::string scopedClassName = "RuntimeBase::" + className;
		std::string scopedRuntimeClassName = uniqueNamespace + "::" + className;
		if( interpreter->getAddressOfGlobal( instanceName ) ) {
#ifdef RUNTIME_APP_CEREALIZATION
			cereal::BinaryOutputArchive outputArchive( archiveStream );
			if( runtimeApp->mRuntimeImpl ) {
				runtimeApp->mRuntimeImpl->save( outputArchive );
				cerealized = true;
			}
#endif
			interpreter->process( instanceName + " = std::make_shared<" + scopedRuntimeClassName + ">();" );
		}
		// otherwise create it
		else {
			interpreter->process( "std::shared_ptr<" + scopedClassName + "> " + instanceName + " = std::make_shared<" + scopedRuntimeClassName + ">();" );
		}
		
		// grab the new address and update the runtime_ptr instance
		if( auto address = interpreter->getAddressOfGlobal( instanceName ) ) {
			auto newImpl = *reinterpret_cast<std::shared_ptr<RuntimeAppWrapper>*>( address );
			if( newImpl ) {
				runtimeApp->mRuntimeImpl = newImpl;
				runtimeApp->mRuntimeImpl->mParent = runtimeApp;
				runtimeApp->setup();
#ifdef RUNTIME_APP_CEREALIZATION
				if( cerealized ) {
					cereal::BinaryInputArchive inputArchive( archiveStream );
					runtimeApp->mRuntimeImpl->load( inputArchive );
				}
#endif
			}
		}
	} );
	
	runtimeApp->executeLaunch();
	delete runtimeApp;
	
	ci::app::AppBase::cleanupLaunch();
}

#define CINDER_RUNTIME_APP( APP, RENDERER, ... )										\
int main( int argc, char* argv[] )											\
{																					\
cinder::app::RendererRef renderer( new RENDERER );								\
RuntimeApp::main<APP>( renderer, #APP, argc, argv, __FILE__, ##__VA_ARGS__ );	\
return 0;																		\
}

#else
#define CINDER_RUNTIME_APP( APP, RENDERER, ... )	CINDER_APP( APP, RENDERER, ##__VA_ARGS__ )
#endif
/*
 Cinder-Runtime
 Watchdog (This is a smaller version of my Watchdog library re-written for Cinder-Runtime)
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

#include <map>
#include <string>
#include <thread>
#include <memory>
#include <atomic>
#include <mutex>

#include "cinder/Filesystem.h"

class SourceWatchdog {
public:
	static void watch( const ci::fs::path &path, const std::function<void()> &callback )
	{
		watchImpl( path, callback );
	}
	
	//! Recursively searches for the file
	static ci::fs::path findFilePath( const ci::fs::path &path )
	{
		auto appRoot = getAppRoot();
		if( ci::fs::is_directory( appRoot ) ) {
			ci::fs::recursive_directory_iterator dir( appRoot ), endDir;
			for( ; dir != endDir; ++dir ) {
				auto current = (*dir).path();
				if( current.filename() == path ) {
					return ci::fs::canonical( current );
				}
			}
		}
		return ci::fs::path();
	}
	
protected:
	SourceWatchdog()
	: mWatching(false) {}
	
	void close()
	{
		// stop the thread
		mWatching = false;
		if( mThread->joinable() ) mThread->join();
		
		// remove all watchers
		mWatchers.clear();
	}
	
	
	void start()
	{
		mWatching   = true;
		mThread     = std::unique_ptr<std::thread>( new std::thread( [this](){
			// keep watching for modifications every ms milliseconds
			auto ms = std::chrono::milliseconds( 500 );
			while( mWatching ) {
				do {
					// iterate through each watcher and check for modification
					std::lock_guard<std::mutex> lock( mMutex );
					auto end = mWatchers.end();
					for( auto it = mWatchers.begin(); it != end; ++it ) {
						it->second->watch();
					}
					// lock will be released before this thread goes to sleep
				} while( false );
				
				// make this thread sleep for a while
				std::this_thread::sleep_for( ms );
			}
		} ) );
	}
	
	//! Returns the app root folder
	static ci::fs::path getAppRoot()
	{
		return ci::app::getAppPath().parent_path().parent_path().parent_path();
	}
	//! Returns whether the file is a header
	static bool isHeader( const ci::fs::path &path ) { return path.extension() == ".h"; }
	//! Returns whether the file is a cpp file
	static bool isImpl( const ci::fs::path &path ) { return path.extension() == ".cpp"; }
	
	class Watcher {
	public:
		Watcher( const ci::fs::path &path )
		: mPath( path.parent_path() ), mFilename( path.filename() )
		{
			mHasHeader	= ! ( isHeader( mFilename ) ? mFilename : findFilePath( mFilename.stem().string() + ".h" ) ).empty();
			mHasImpl	= ! ( isImpl( mFilename ) ? mFilename : findFilePath( mFilename.stem().string() + ".cpp" ) ).empty();
			
			if( mHasHeader ) {
				mHeaderLastWriteTime = ci::fs::last_write_time( mPath / ( mFilename.stem().string() + ".h" ) );
			}
			if( mHasImpl ) {
				mImplLastWriteTime = ci::fs::last_write_time( mPath / ( mFilename.stem().string() + ".cpp" ) );
			}
		}
		
		//! Checks if the sources have been modified and notify the connections if needed
		void watch()
		{
			if( hasChanged() ) {
				mSignal.emit();
			}
		}
		
		//! Returns whether the header or the impl file has been modified
		bool hasChanged()
		{
			// check if the header has been modified
			if( mHasHeader ) {
				auto time = ci::fs::last_write_time( getHeader() );
				if( std::difftime( time, mHeaderLastWriteTime ) > 0.0 ) {
					mHeaderLastWriteTime = time;
					return true;
				}
			}
			// check if the implementation has been modified
			if( mHasImpl ) {
				auto time = ci::fs::last_write_time( getImplementation() );
				if( std::difftime( time, mImplLastWriteTime ) > 0.0 ) {
					mImplLastWriteTime = time;
					return true;
				}
			}
			return false;
		}
		
		//! Connects to this Watcher signal
		void connect( const ci::signals::Signal<void()>::CallbackFn &callback )
		{
			mSignal.connect( callback );
		}
		
		//! Returns the Watcher header path
		ci::fs::path getHeader() const { return mPath / ( mFilename.stem().string() + ".h" ); }
		//! Returns the Watcher implementation path
		ci::fs::path getImplementation() const { return mPath / ( mFilename.stem().string() + ".cpp" ); }
		
	protected:
		bool						mHasHeader;
		bool						mHasImpl;
		ci::signals::Signal<void()> mSignal;
		ci::fs::path				mPath;
		ci::fs::path				mFilename;
		std::time_t					mHeaderLastWriteTime;
		std::time_t					mImplLastWriteTime;
	};
	
	//! Watches a file and starts the watch thread if needed
	static void watchImpl( const ci::fs::path &path, const std::function<void()> &callback )
	{
		// create the static Watchdog instance
		static SourceWatchdog wd;
		// and start its thread
		if( !wd.mWatching ) {
			wd.start();
			ci::app::App::get()->getSignalCleanup().connect( [&]() {
				wd.close();
			} );
		}
		
		// create a new Watcher if it doesn't exist yet
		if( wd.mWatchers.count( path ) <= 0 ) {
			wd.mWatchers.insert( std::make_pair( path, std::unique_ptr<Watcher>( new Watcher( findFilePath( path.filename() ) ) ) ) );
		}
		
		// find the source watcher
		auto &watcher = wd.mWatchers[path];
		watcher->connect( callback );
		
		// Call the callback once
		callback();
	}
	
	
protected:
	std::mutex                      mMutex;
	std::atomic<bool>               mWatching;
	std::unique_ptr<std::thread>    mThread;
	
	using WatcherRef = std::unique_ptr<Watcher>;
	std::map<ci::fs::path,WatcherRef> mWatchers;
};
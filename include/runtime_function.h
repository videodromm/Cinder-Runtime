/*
 runtime_function
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

#include <functional>

#ifndef DISABLE_RUNTIME_COMPILED_FUNC

#include "cinder/Filesystem.h"
#include "cling/Interpreter/Interpreter.h"
#include "Watchdog.h"

template<class> class runtime_function;

template<class R, class... Args>
class runtime_function<R(Args...)> {
public:
	class Options {
	public:
		Options() : mLoadCinder( false ) {}
		
		Options& includePath( const ci::fs::path &path );
		Options& dynamicLibrary( const ci::fs::path &path );
		Options& cinder();
		Options& declaration( const std::string &declaration );
		
		const std::vector<ci::fs::path>& getIncludePaths() const { return mIncludePaths; }
		const std::vector<ci::fs::path>& getDynamicLibraries() const { return mDynamicLibraries; }
		const std::vector<std::string>& getDeclarations() const { return mDeclarations; }
		bool needsCinder() const { return mLoadCinder; }
		
	protected:
		bool mLoadCinder;
		std::vector<ci::fs::path> mIncludePaths;
		std::vector<ci::fs::path> mDynamicLibraries;
		std::vector<std::string> mDeclarations;
	};
	
	runtime_function() : mIsWatching( false ) {}
	runtime_function( const std::string &function, const ci::fs::path &path, const Options &options = Options() );
	
	R operator()( Args... args );
	explicit operator bool();
protected:
	
	void watch();
	bool mIsWatching;
	ci::fs::path mFunctionPath;
	std::string mFunctionName;
	std::function<R(Args...)> mFunction;
	std::shared_ptr<cling::Interpreter> mInterpreter;
};



template<class R, class... Args>
typename runtime_function<R(Args...)>::Options& runtime_function<R(Args...)>::Options::includePath( const ci::fs::path &path )
{
	mIncludePaths.push_back( path );
	return *this;
}
template<class R, class... Args>
typename runtime_function<R(Args...)>::Options& runtime_function<R(Args...)>::Options::dynamicLibrary( const ci::fs::path &path )
{
	mDynamicLibraries.push_back( path );
	return *this;
}
template<class R, class... Args>
typename runtime_function<R(Args...)>::Options& runtime_function<R(Args...)>::Options::cinder()
{
	mLoadCinder = true;
	return *this;
}
template<class R, class... Args>
typename runtime_function<R(Args...)>::Options& runtime_function<R(Args...)>::Options::declaration( const std::string &declaration )
{
	mDeclarations.push_back( declaration );
	return *this;
}

template<class R, class... Args>
runtime_function<R(Args...)>::runtime_function( const std::string &function, const ci::fs::path &path, const Options &options )
: mFunctionName( function ), mIsWatching( false )
{
	// find the actual path
	ci::fs::path absolutePath;
	if( !path.parent_path().empty() && ci::fs::exists( path ) ) {
		absolutePath = path;
	}
	// check if in src folder
	else if( ci::fs::exists( ci::fs::current_path() / "../../../src" / path ) ) {
		absolutePath = ci::fs::canonical( ci::fs::current_path() / "../../../src" / path );
	}
	// check if in include folder
	else if( ci::fs::exists( ci::fs::current_path() / "../../../include" / path ) ) {
		absolutePath = ci::fs::canonical( ci::fs::current_path() / "../../../include" / path );
	}
	
	// initialize cling interpreter
	const char * args[] = { "", "-std=c++11" };
	auto blockPath = ci::fs::path( __FILE__ ).parent_path().parent_path();
	mInterpreter = std::make_shared<cling::Interpreter>( 2, args, ( blockPath.string() + "/lib/" ).c_str() );
	
	// add the parent path to the include paths
	mInterpreter->AddIncludePath( absolutePath.parent_path().string() );
	
	// process the options
	for( const auto &p : options.getIncludePaths() ) {
		mInterpreter->AddIncludePath( p.string() );
	}
	for( const auto &p : options.getDynamicLibraries() ) {
		mInterpreter->loadFile( p.string() );
	}
	for( const auto &d : options.getDeclarations() ) {
		mInterpreter->declare( d );
	}
	if( options.needsCinder() ) {
		mInterpreter->declare( "#define GLM_COMPILER 0" );
		mInterpreter->AddIncludePath( blockPath.parent_path().parent_path().string() + "/include/" );
		mInterpreter->loadFile( blockPath.parent_path().parent_path().string() + "/lib/libcinder.dylib" );
	}
	
	mFunctionPath = absolutePath;
}

template<class R, class... Args>
R runtime_function<R(Args...)>::operator()( Args... args )
{
	if( ! mIsWatching ) watch();
	return mFunction.operator()( args... );
}
template<class R, class... Args>
runtime_function<R(Args...)>::operator bool()
{
	if( ! mIsWatching ) watch();
	return mFunction.operator bool();
}

template<class R, class... Args>
void runtime_function<R(Args...)>::watch()
{
	if( mInterpreter ) {
		wd::watch( mFunctionPath, [this]( const ci::fs::path &p ) {
			
			// copy the file content to a string
			std::ifstream infile( p.c_str() );
			std::string line, code;
			while( std::getline( infile, line ) ) {
				code += line + " \n";
			}
			
			// replace each occurence of the function name by a new unique name
			std::string uniqueName;
			mInterpreter->createUniqueName( uniqueName );
			uniqueName = mFunctionName + uniqueName;
			size_t pos = 0;
			while( (pos = code.find(mFunctionName, pos)) != std::string::npos ) {
				code.replace(pos, mFunctionName.length(), uniqueName);
				pos += uniqueName.length();
			}
			
			// try to compile the function
			if( auto fn = mInterpreter->compileFunction( uniqueName, code ) ) {
				mFunction = std::function<R(Args...)>( (R(*)(Args...)) fn );
			}
		} );
		mIsWatching = true;
	}
}

#else
template<class R, class... Args>
using runtime_function = std::function<T>;
#endif
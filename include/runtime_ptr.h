/*
 runtime_ptr
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

#include <memory>

#ifndef DISABLE_RUNTIME_COMPILED_PTR

#include <map>

#include "cinder/Exception.h"
#include "cinder/Filesystem.h"
#include "cinder/System.h"
#include "cling/Interpreter/Interpreter.h"
#include "Watchdog.h"

#ifdef RUNTIME_PTR_CEREALIZATION
#include <utility>
#include <cereal/archives/binary.hpp>
#endif

template <class T>
class runtime_class;

template<class T>
class runtime_ptr {
public:
	constexpr runtime_ptr() {}
	
	operator bool() const { return mPtr.operator bool(); }
	T* operator->() const { return mPtr.operator->(); }
	T* get() const { return mPtr.get(); }
	long use_count() const { return mPtr.use_count(); }
	
	runtime_ptr( const runtime_ptr& other );
	runtime_ptr( runtime_ptr&& other );
	runtime_ptr& operator=( const runtime_ptr& other );
	runtime_ptr& operator=( runtime_ptr&& other );
	~runtime_ptr();
	
protected:
	friend class runtime_class<T>;
	std::string getName() const;
	void update( const std::shared_ptr<T> &newInstance );
	
	runtime_ptr( bool runtime );
	template<class U>
	friend runtime_ptr<U> make_runtime();
	
#ifdef RUNTIME_PTR_CEREALIZATION
	template<class U, class R, class = void> struct SupportsCerealSave : std::false_type {};
	template<class U, class R, class... Args>
	struct SupportsCerealSave<U, R(Args...), decltype(std::declval<U>().save( std::declval<Args>()... ), void())> : std::true_type {};
	template<class U, class R, class = void> struct SupportsCerealLoad : std::false_type {};
	template<class U, class R, class... Args>
	struct SupportsCerealLoad<U, R(Args...), decltype(std::declval<U>().load( std::declval<Args>()... ), void())> : std::true_type {};
	
	template<class U, class Enable = void>
	struct Cerealizer {
		template<class Archive> void save( U* ptr, Archive& ){}
		template<class Archive>	void load( U* ptr, Archive& ){}
	};
	template<class U>
	struct Cerealizer<U,typename std::enable_if<SupportsCerealSave<U,void(cereal::BinaryOutputArchive&)>::value && SupportsCerealLoad<U,void(cereal::BinaryInputArchive&)>::value>::type> {
		template<class Archive> void save( U* ptr, Archive &ar ){ ptr->save( ar ); }
		template<class Archive> void load( U* ptr, Archive &ar ){ ptr->load( ar ); }
	protected:
	};
	
	Cerealizer<T> mCerealizer;
#endif
	
	std::shared_ptr<T> mPtr;
};

template<class T>
runtime_ptr<T> make_runtime() {	return runtime_ptr<T>( true ); }

template<class T>
class runtime_class {
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
	
	static std::shared_ptr<cling::Interpreter> initialize( const ci::fs::path &path, const Options &options = Options() );
	
protected:
	
	static void addIncludePath( const ci::fs::path &path );
	static void loadFile( const ci::fs::path &path );
	static void loadCinder();
	static void declare( const std::string &declaration );
	
	static void registerInstance( runtime_ptr<T>* ptr );
	static void unregisterInstance( runtime_ptr<T>* ptr );
	static std::shared_ptr<cling::Interpreter> getInterpreter();
	
	friend class runtime_ptr<T>;
	
	static const std::unique_ptr<runtime_class>& instance() { static std::unique_ptr<runtime_class> inst = std::unique_ptr<runtime_class>( new runtime_class() ); return inst; }
	
	std::shared_ptr<cling::Interpreter> mInterpreter;
	std::map<runtime_ptr<T>*,std::function<void(const std::shared_ptr<T>&)>> mInstances;
};


class MissingInterpreterException : ci::Exception {
public:
	MissingInterpreterException( const std::string &className )
	: ci::Exception( "Missing Interpreter for " + className + ". You need to call RunTimeClass::addInterpreter<" + className + ">( path ) before using this class" ) {}
};

template<class T>
typename runtime_class<T>::Options& runtime_class<T>::Options::includePath( const ci::fs::path &path )
{
	mIncludePaths.push_back( path );
	return *this;
}
template<class T>
typename runtime_class<T>::Options& runtime_class<T>::Options::dynamicLibrary( const ci::fs::path &path )
{
	mDynamicLibraries.push_back( path );
	return *this;
}
template<class T>
typename runtime_class<T>::Options& runtime_class<T>::Options::cinder()
{
	mLoadCinder = true;
	return *this;
}
template<class T>
typename runtime_class<T>::Options& runtime_class<T>::Options::declaration( const std::string &declaration )
{
	mDeclarations.push_back( declaration );
	return *this;
}

template<class T>
std::shared_ptr<cling::Interpreter> runtime_class<T>::initialize( const ci::fs::path &path, const Options &options )
{
	if( !instance()->mInterpreter ) {
		
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
		const char * args[] = { "-std=c++11" };
		auto blockPath = ci::fs::path( __FILE__ ).parent_path().parent_path();
		instance()->mInterpreter = std::make_shared<cling::Interpreter>( 1, args, ( blockPath.string() + "/lib/" ).c_str() );
		
		// add the parent path to the include paths
		getInterpreter()->AddIncludePath( absolutePath.parent_path().string() );
		
		// process the class options
		for( const auto &p : options.getIncludePaths() ) {
			addIncludePath( p );
		}
		for( const auto &p : options.getDynamicLibraries() ) {
			loadFile( p );
		}
		for( const auto &d : options.getDeclarations() ) {
			declare( d );
		}
		if( options.needsCinder() ) {
			loadCinder();
		}
		
		// compile the original class
		bool isCpp = absolutePath.extension() == ".cpp";
		ci::fs::path headerPath = absolutePath.parent_path() / ( absolutePath.stem().string() + ".h" );
		ci::fs::path implPath = absolutePath.parent_path() / ( absolutePath.stem().string() + ".cpp" );
		
		// copy the file content to a string
		std::string originalCode;
		std::vector<std::string> includes;
		
		// if it's a header just copy it like this
		if( !isCpp ) {
			std::ifstream infile( headerPath.c_str() );
			std::string line;
			while( std::getline( infile, line ) ) {
				//originalCode += line + " \n";
				if( line.find( "#include" ) == std::string::npos ) {
					originalCode += line + " \n";
				}
				else includes.push_back( line );
			}
		}
		// otherwise remove the include in the cpp file and prepend the header before the implementation
		else {
			std::string line;
			std::ifstream headerFile( headerPath.c_str() );
			while( std::getline( headerFile, line ) ) {
				//originalCode += line + " \n";
				if( line.find( "#include" ) == std::string::npos ) {
					originalCode += line + " \n";
				}
				else includes.push_back( line );
			}
			std::ifstream implFile( implPath.c_str() );
			while( std::getline( implFile, line ) ) {
				// skip the header include
				if( !( line.find( "#include" ) != std::string::npos && line.find( headerPath.filename().string() ) != std::string::npos ) ) {
					//originalCode += line + " \n";
					if( line.find( "#include" ) == std::string::npos ) {
						originalCode += line + " \n";
					}
					else includes.push_back( line );
				}
			}
		}
		
		// prepend the includes
		std::string includesString;
		for( auto inc : includes ) {
			includesString += inc + "\n";
		}
		
		// wrap original code in its own namespace
		originalCode = includesString + "\n\nnamespace RuntimeBase {\n" + originalCode + "\n};";
		
		// process the original code once
		getInterpreter()->enableRawInput();
		getInterpreter()->declare( originalCode );
		getInterpreter()->enableRawInput( false );
		getInterpreter()->declare( "#include <memory>" );
		
		// start watching file
		std::string className = ci::System::demangleTypeName( typeid( T ).name() );
		wd::watch( absolutePath, [className,isCpp]( const ci::fs::path &p ) {
			// copy the file content to a string
			std::string code;
			std::vector<std::string> includes;
			
			// if it's a header just copy it like this
			if( !isCpp ) {
				std::ifstream infile( p.c_str() );
				std::string line;
				while( std::getline( infile, line ) ) {
					//code += line + " \n";
					if( line.find( "#include" ) == std::string::npos ) {
						code += line + " \n";
					}
					else includes.push_back( line );
				}
			}
			// otherwise remove the include in the cpp file and prepend the header before the implementation
			else {
				std::string stem = p.stem().string();
				ci::fs::path header = p.parent_path() / ( stem + ".h" );
				ci::fs::path impl = p.parent_path() / ( stem + ".cpp" );
				
				std::string line;
				std::ifstream headerFile( header.c_str() );
				while( std::getline( headerFile, line ) ) {
					//code += line + " \n";
					if( line.find( "#include" ) == std::string::npos ) {
						code += line + " \n";
					}
					else includes.push_back( line );
				}
				std::ifstream implFile( impl.c_str() );
				while( std::getline( implFile, line ) ) {
					// skip the header include
					if( !( line.find( "#include" ) != std::string::npos && line.find( header.filename().string() ) != std::string::npos ) ) {
						//code += line + " \n";
						if( line.find( "#include" ) == std::string::npos ) {
							code += line + " \n";
						}
						else includes.push_back( line );
					}
				}
			}
			
			// prepend the includes
			std::string includesString;
			for( auto inc : includes ) {
				includesString += inc + "\n";
			}
			
			// make a unique namespace name
			std::string uniqueNamespace;
			getInterpreter()->createUniqueName( uniqueNamespace );
			uniqueNamespace = className + uniqueNamespace;
			
			// wrap the code in its own unique namespace
			code = includesString + "\n\nnamespace " + uniqueNamespace + " {\n" + code + "\n};";
			
			// Make the class inherit from the original one
			size_t pos = code.find( "class " + className + " { " );
			if( pos != std::string::npos ) {
				code.replace( pos, ( "class " + className + " { " ).length(), "class " + className + " : public RuntimeBase::" + className + " { " );
			}
			else {
				pos = code.find( "class " + className + " : " );
				if( pos != std::string::npos ) {
					code.replace( pos, ( "class " + className + " : " ).length(), "class " + className + " : public RuntimeBase::" + className + ", " );
				}
			}
			
			// process the new code
			getInterpreter()->enableRawInput();
			getInterpreter()->declare( code );
			getInterpreter()->enableRawInput( false );
			
			// update instances with the new implementation
			for( auto instance : instance()->mInstances ) {
				auto instanceName = instance.first->getName();
				
#ifdef RUNTIME_PTR_CEREALIZATION
				bool cerealized = false;
				std::stringstream archiveStream;
#endif

				// if the instance already exists override it
				std::string scopedClassName = "RuntimeBase::" + className;
				std::string scopedRuntimeClassName = uniqueNamespace + "::" + className;
				if( getInterpreter()->getAddressOfGlobal( instanceName ) ) {
#ifdef RUNTIME_PTR_CEREALIZATION
					cereal::BinaryOutputArchive outputArchive( archiveStream );
					instance.first->mCerealizer.save( instance.first->get(), outputArchive );
					cerealized = true;
#endif
					getInterpreter()->process( instanceName + " = std::make_shared<" + scopedRuntimeClassName + ">();" );
				}
				// otherwise create it
				else {
					getInterpreter()->process( "std::shared_ptr<" + scopedClassName + "> " + instanceName + " = std::make_shared<" + scopedRuntimeClassName + ">();" );
				}
				
				// grab the new address and update the runtime_ptr instance
				if( auto address = getInterpreter()->getAddressOfGlobal( instanceName ) ) {
					std::shared_ptr<T> base = *reinterpret_cast<std::shared_ptr<T>*>( address );
					instance.second( *reinterpret_cast<std::shared_ptr<T>*>( address ) );
#ifdef RUNTIME_PTR_CEREALIZATION
					if( cerealized ) {
						cereal::BinaryInputArchive inputArchive( archiveStream );
						instance.first->mCerealizer.load( instance.first->get(), inputArchive );
					}
#endif
				}
			}
		} );
	}
	
	return instance()->mInterpreter;
}

template<class T>
void runtime_class<T>::addIncludePath( const ci::fs::path &path )
{
	getInterpreter()->AddIncludePath( path.string() );
}
template<class T>
void runtime_class<T>::loadFile( const ci::fs::path &path )
{
	getInterpreter()->loadFile( path.string() );
}
template<class T>
void runtime_class<T>::loadCinder()
{
	auto blockPath = ci::fs::path( __FILE__ ).parent_path().parent_path();
	getInterpreter()->declare( "#define GLM_COMPILER 0" );
	getInterpreter()->AddIncludePath( blockPath.parent_path().parent_path().string() + "/include/" );
#if defined(NDEBUG) || defined(_NDEBUG) || defined(RELEASE) || defined(MASTER) || defined(GOLD)
	getInterpreter()->loadFile( blockPath.parent_path().parent_path().string() + "/lib/libcinder.dylib" );
#else
	getInterpreter()->loadFile( blockPath.parent_path().parent_path().string() + "/lib/libcinder_d.dylib" );
#endif
}
template<class T>
void runtime_class<T>::declare( const std::string &declaration )
{
	getInterpreter()->declare( declaration );
}

template<class T>
std::shared_ptr<cling::Interpreter> runtime_class<T>::getInterpreter()
{
	if( !instance()->mInterpreter ) {
		throw MissingInterpreterException( ci::System::demangleTypeName( typeid(T).name() ) );
	}
	return instance()->mInterpreter;
}

template<class T>
void runtime_class<T>::registerInstance( runtime_ptr<T>* ptr )
{
	instance()->mInstances[ptr] = [ptr]( const std::shared_ptr<T> &newInstance ) {
		ptr->update( newInstance );
	};
}
template<class T>
void runtime_class<T>::unregisterInstance( runtime_ptr<T>* ptr )
{
	// make sure to release memory if the object exists in the interpreter
	auto instanceName = ptr->getName();
	if( getInterpreter()->getAddressOfGlobal( instanceName ) ) {
		getInterpreter()->process( instanceName + ".reset();" );
	}
	instance()->mInstances.erase( ptr );
}

template<class T>
runtime_ptr<T>::runtime_ptr( bool runtime )
{
	runtime_class<T>::registerInstance( this );
}

template<class T>
runtime_ptr<T>::runtime_ptr( const runtime_ptr& other )
: mPtr( other.mPtr )
{
	runtime_class<T>::registerInstance( this );
}

template<class T>
runtime_ptr<T>::runtime_ptr( runtime_ptr&& other )
: mPtr( std::move( other.mPtr ) )
{
	// TODO : Do we realy need to unregister in case of a move?
	runtime_class<T>::unregisterInstance( &other );
	runtime_class<T>::registerInstance( this );
}

template<class T>
runtime_ptr<T>& runtime_ptr<T>::operator=( const runtime_ptr& other )
{
	runtime_class<T>::registerInstance( this );
	mPtr = other.mPtr;
	return *this;
}
template<class T>
runtime_ptr<T>& runtime_ptr<T>::operator=( runtime_ptr&& other )
{
	runtime_class<T>::unregisterInstance( &other );
	runtime_class<T>::registerInstance( this );
	mPtr = std::move( other.mPtr );
	return *this;
}

template<class T>
runtime_ptr<T>::~runtime_ptr()
{
	runtime_class<T>::unregisterInstance( this );
}

template<class T>
std::string runtime_ptr<T>::getName() const
{
	const void *instanceAddress = static_cast<const void*>( this );
	std::stringstream ss;
	ss << instanceAddress;
	return "runtimeInstance" + ss.str();
}

template<class T>
void runtime_ptr<T>::update( const std::shared_ptr<T> &newInstance )
{
	mPtr = newInstance;
}

#else
template<class T>
using runtime_ptr = std::shared_ptr<T>;
template<class T>
runtime_ptr<T> make_runtime() { return std::make_shared<T>(); }
#endif
Cinder-Runtime
===================


####Introduction

This block explores what is currently possible to do using Cling, Llvm and Clang. The main code of this block is the ```runtime_ptr``` template. The class wraps a ```shared_ptr``` and pretty much works the same way at the exception that the wrapped class will be reloaded at runtime each time the source file is saved. This can be used to prototype code without having to wait between each compilation. It comes with its own limitations and is certainly not as complete as similar projects but clearly less invasive and much easier to use.

*Make sure to read the ["How it works"](#how-it-works) section to understand what you can and can't do.*


The block only supports [OSX](#osx-build-instructions) at the moment but PRs are most welcome.


####Usage

###### Setup, Options, libraries and include paths

Include the class you want to update at runtime and the runtime_ptr header:
```c++
#include "MyClass.h"
#include "runtime_ptr.h"
```
Declare your instance the same way you would with a std::shared_ptr:
```c++
runtime_ptr<MyClass> mPtr;
```

You need to register the class by using this before trying to instantiate it (if you don't you'll get a MissingInterpreterException):
```c++
#ifndef DISABLE_RUNTIME_COMPILED_PTR
runtime_class<MyClass>::initialize( "MyClass.cpp" )
#endif
```
You can pass Options to the initialize method if your class needs to access anything else
```c++
runtime_class<MyClass>::initialize( "MyClass.cpp", runtime_class<MyClass>::Options().cinder() );
runtime_class<MyClassOtherClass>::initialize( "MyClassOtherClass.cpp", runtime_class<MyClassOtherClass>::Options()
		.cinder()
		.includePath( "../../../blocks/ImGui/include" )
		.includePath( "../../../blocks/ImGui/lib/imgui" )
		.dynamicLibrary( "../../../blocks/ImGui/lib/libCinderImGui.dylib" ) );
```

Once the class is registered, you can create new instances with ```make_runtime<T>()```. From there the ```runtime_ptr``` will be updated with the new implementation each time you save "MyClass.cpp".
```c++
mPtr = make_runtime<MyClass>();
```

As the pointer could become invalid you might want to wrap any call in a if:
```c++
if( mPtr ) {
  mPtr->doSomething();
}
```

###### Class scope restrictions

You're free to include or use any preprocessors in any of the files but you can't define anything outside of the class. Stick to the class scope and you should be good to go.


###### Virtual methods

Any method that is used outside of the class files at compile time and that you want to be able to modify at runtime **needs** to be virtual for the whole hack to work. That also means that you can't change this function signature between two compilation.
```c++
class MyClass {
  virtual void doSomething();
  void doSomethingElse(); // changes to that method at runtime will not work
};
```

###### Disable runtime_ptr

You can easily disable the whole library by using ```DISABLE_RUNTIME_COMPILED_PTR```. As soon as you define ```DISABLE_RUNTIME_COMPILED_PTR``` all the runtime_ptr instances will be replaced by ```std::shared_ptrs``` and all call to ```make_runtime<MyClass>()``` will be transformed to ```std::make_shared<MyClass>()```.


###### Cereal Support

If you want the state of your class to be saved between implementation updates you can define ```RUNTIME_PTR_CEREALIZATION``` before importing runtime_ptr header:
```c++
#define RUNTIME_PTR_CEREALIZATION
#include "runtime_ptr.h"
```

If the save and load functions are detected as member of your class at compile time, they will be used to store the state of each instance. Unfortunately you can't use the templated version of Cereal as the methods need to be virtual for it to work:
```c++
#include <cereal/archives/binary.hpp>

class Object {
public:
	virtual void save( cereal::BinaryOutputArchive &ar );
	virtual void load( cereal::BinaryInputArchive &ar );
	
protected:
	float mValue0, mValue1, mValue2;
};
void Object::save( cereal::BinaryOutputArchive &ar )
{
	ar( mValue0, mValue1, mValue2 );
}
void Object::load( cereal::BinaryInputArchive &ar )
{
	ar( mValue0, mValue1, mValue2 );
}
````

####How it works

To allow to use the fast REPL of Cling, no symbols are unloaded in the interpreter and the code doesn't touch the main app symbols. Instead ```runtime_ptr``` uses a pretty ugly hack based around polymorphism. The ```shared_ptr``` itself will be of the type of the class compiled when building the app, but the actual content will be of a temporary type inheriting from your class. As the REPL system of Cling is made to append code to existing code instead of reloading it, what ```runtime_ptr``` does behind the scene would more or less look like this :

```c++
class Object {};
std::shared_ptr<Object> instance = std::make_shared<Object>();
class Object__cling_Un1Qu34 : public Object {};
instance = std::make_shared<Object__cling_Un1Qu34>();
class Object__cling_Un1Qu35 : public Object {};
instance = std::make_shared<Object__cling_Un1Qu35>();
class Object__cling_Un1Qu36 : public Object {};
instance = std::make_shared<Object__cling_Un1Qu36>();
class Object__cling_Un1Qu37 : public Object {};
instance = std::make_shared<Object__cling_Un1Qu37>();
class Object__cling_Un1Qu38 : public Object {};
instance = std::make_shared<Object__cling_Un1Qu38>();
class Object__cling_Un1Qu39 : public Object {};
instance = std::make_shared<Object__cling_Un1Qu39>();
```

Kind of ugly but it does allow fast reloading of your class while keeping an extremly simple API. Unfortunately it obviously comes with a few downsides explained above. Most of it is currently implemented with a simple string replacement algorithm which explains the scope limitations.


####OSX Build Instructions

###### Install Libraries
Start by running the ```install.sh``` to grab and build Cling, Llvm and Clang. There's a ```cleanup.sh``` if you want to save some disk space, but it will get rid of the binaries as well, so you better do that manually.

###### Build Cinder as a dynamic library
- Right click cinder target / duplicate
- Rename the new target to "cinder_dynamic"
- Then change the following settings:
- Linking / MACH_O_TYPE = mh_dylib / Dynamic Library
- Linking / LINK_WITH_STANDARD_LIBRARIES = YES
- Packaging / EXECUTABLE_EXTENSION = dylib
- Build Phases / Add IOKit, IOSurface, Accelerate, CoreAudio, CoreMedia and AVFoundation frameworks
- Build ```cinder_dynamic.dylib``` and ```cinder_dynyamic_d.dylib```

###### Create new projects with Tinderbox
The block works only in "relative mode" so don't try to copy it!

When creating a new project, you should go to your build settings and remove "libcinder.a"/"libcinder_d.a" from your "Other Linker Flags".

###### Potential errors about ```unistd.h``` or ```XcodeDefault.xctoolchain```
If you run into that kind of errors you might want to try to update your command line dev. tools by running a ```xcode-select —install``` in terminal. If that doesn't work, try to update xcode. Once you have that solved you can remove the folders created by the install script and restart it.

#!/bin/sh
JOBS=$(getconf _NPROCESSORS_CONF)

#clone llvm and apply patches
git clone http://root.cern.ch/git/llvm.git src
cd src
git checkout cling-patches

#clone cling and clang
cd tools
git clone http://root.cern.ch/git/cling.git
git clone http://root.cern.ch/git/clang.git
cd clang
git checkout cling-patches

#build release libraries
cd ../..
mkdir build
cd build
cmake -DCMAKE_INSTALL_PREFIX=../../lib/ -DLLVM_TARGETS_TO_BUILD="CppBackend;PowerPC;X86;" -DCMAKE_BUILD_TYPE=Release --enable-optimized  ..
make -j${JOBS}
make install -j${JOBS}

#package libraries 
cd ../../lib/lib
libtool -static -o libcling.a libclingMetaProcessor.a libclingInterpreter.a libclingUtils.a libclangFrontend.a libclangCodeGen.a libclangParse.a libclangSema.a libclangBasic.a libclangSerialization.a libclangDriver.a libclangEdit.a libclangLex.a libclangTooling.a libclangAnalysis.a libclangAST.a libLLVMAnalysis.a libLLVMAsmParser.a libLLVMAsmPrinter.a libLLVMBitReader.a libLLVMBitWriter.a libLLVMCodeGen.a libLLVMCore.a libLLVMCppBackendCodeGen.a libLLVMCppBackendInfo.a libLLVMDebugInfoDWARF.a libLLVMExecutionEngine.a libLLVMInstCombine.a libLLVMInstrumentation.a libLLVMInterpreter.a libLLVMipa.a libLLVMipo.a libLLVMIRReader.a libLLVMLineEditor.a libLLVMLinker.a libLLVMLTO.a libLLVMMC.a libLLVMMCDisassembler.a libLLVMMCJIT.a libLLVMMCParser.a libLLVMObjCARCOpts.a libLLVMObject.a libLLVMOption.a libLLVMOrcJIT.a libLLVMProfileData.a libLLVMRuntimeDyld.a libLLVMScalarOpts.a libLLVMSelectionDAG.a libLLVMSupport.a libLLVMTableGen.a libLLVMTarget.a libLLVMTransformUtils.a libLLVMVectorize.a libLLVMX86AsmParser.a libLLVMX86AsmPrinter.a libLLVMX86CodeGen.a libLLVMX86Desc.a libLLVMX86Disassembler.a libLLVMX86Info.a libLLVMX86Utils.a
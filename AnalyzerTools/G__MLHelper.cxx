// Do NOT change. Changes will be lost next time file is generated

#define R__DICTIONARY_FILENAME G__MLHelper
#define R__NO_DEPRECATION

/*******************************************************************/
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#define G__DICTIONARY
#include "ROOT/RConfig.hxx"
#include "TClass.h"
#include "TDictAttributeMap.h"
#include "TInterpreter.h"
#include "TROOT.h"
#include "TBuffer.h"
#include "TMemberInspector.h"
#include "TInterpreter.h"
#include "TVirtualMutex.h"
#include "TError.h"

#ifndef G__ROOT
#define G__ROOT
#endif

#include "RtypesImp.h"
#include "TIsAProxy.h"
#include "TFileMergeInfo.h"
#include <algorithm>
#include "TCollectionProxyInfo.h"
/*******************************************************************/

#include "TDataMember.h"

// Header files passed as explicit arguments
#include "include/MLHelper.h"

// Header files passed via #pragma extra_include

// The generated code does not explicitly qualify STL entities
namespace std {} using namespace std;

namespace ROOT {
   static TClass *MLHelper_Dictionary();
   static void MLHelper_TClassManip(TClass*);
   static void delete_MLHelper(void *p);
   static void deleteArray_MLHelper(void *p);
   static void destruct_MLHelper(void *p);

   // Function generating the singleton type initializer
   static TGenericClassInfo *GenerateInitInstanceLocal(const ::MLHelper*)
   {
      ::MLHelper *ptr = nullptr;
      static ::TVirtualIsAProxy* isa_proxy = new ::TIsAProxy(typeid(::MLHelper));
      static ::ROOT::TGenericClassInfo 
         instance("MLHelper", "MLHelper.h", 20,
                  typeid(::MLHelper), ::ROOT::Internal::DefineBehavior(ptr, ptr),
                  &MLHelper_Dictionary, isa_proxy, 4,
                  sizeof(::MLHelper) );
      instance.SetDelete(&delete_MLHelper);
      instance.SetDeleteArray(&deleteArray_MLHelper);
      instance.SetDestructor(&destruct_MLHelper);
      return &instance;
   }
   TGenericClassInfo *GenerateInitInstance(const ::MLHelper*)
   {
      return GenerateInitInstanceLocal(static_cast<::MLHelper*>(nullptr));
   }
   // Static variable to force the class initialization
   static ::ROOT::TGenericClassInfo *_R__UNIQUE_DICT_(Init) = GenerateInitInstanceLocal(static_cast<const ::MLHelper*>(nullptr)); R__UseDummy(_R__UNIQUE_DICT_(Init));

   // Dictionary for non-ClassDef classes
   static TClass *MLHelper_Dictionary() {
      TClass* theClass =::ROOT::GenerateInitInstanceLocal(static_cast<const ::MLHelper*>(nullptr))->GetClass();
      MLHelper_TClassManip(theClass);
   return theClass;
   }

   static void MLHelper_TClassManip(TClass* ){
   }

} // end of namespace ROOT

namespace ROOT {
   // Wrapper around operator delete
   static void delete_MLHelper(void *p) {
      delete (static_cast<::MLHelper*>(p));
   }
   static void deleteArray_MLHelper(void *p) {
      delete [] (static_cast<::MLHelper*>(p));
   }
   static void destruct_MLHelper(void *p) {
      typedef ::MLHelper current_t;
      (static_cast<current_t*>(p))->~current_t();
   }
} // end of namespace ROOT for class ::MLHelper

namespace {
  void TriggerDictionaryInitialization_libMLHelper_Impl() {
    static const char* headers[] = {
"include/MLHelper.h",
nullptr
    };
    static const char* includePaths[] = {
"/data6/Users/achihwan/miniforge3/envs/Nano/include",
"/home/achihwan/SKNanoAnalyzer/AnalyzerTools",
"/home/achihwan/SKNanoAnalyzer/AnalyzerTools/include",
"/data6/Users/achihwan/miniforge3/envs/Nano/include/onnxruntime/core/session",
"/data6/Users/achihwan/miniforge3/envs/Nano/include",
"/home/achihwan/SKNanoAnalyzer/AnalyzerTools/include",
"/data6/Users/achihwan/miniforge3/envs/Nano/include/onnxruntime/core/session",
"/home/achihwan/SKNanoAnalyzer/external/lhapdf/redhat/include",
"/data6/Users/achihwan/miniforge3/envs/Nano/include/",
"/data6/Users/achihwan/SKNanoAnalyzer/AnalyzerTools/",
nullptr
    };
    static const char* fwdDeclCode = R"DICTFWDDCLS(
#line 1 "libMLHelper dictionary forward declarations' payload"
#pragma clang diagnostic ignored "-Wkeyword-compat"
#pragma clang diagnostic ignored "-Wignored-attributes"
#pragma clang diagnostic ignored "-Wreturn-type-c-linkage"
extern int __Cling_AutoLoading_Map;
class __attribute__((annotate("$clingAutoload$include/MLHelper.h")))  MLHelper;
)DICTFWDDCLS";
    static const char* payloadCode = R"DICTPAYLOAD(
#line 1 "libMLHelper dictionary payload"

#ifndef __ROOFIT_NOBANNER
  #define __ROOFIT_NOBANNER 1
#endif
#ifndef __ROOFIT_NOBANNER
  #define __ROOFIT_NOBANNER 1
#endif

#define _BACKWARD_BACKWARD_WARNING_H
// Inline headers
#include "include/MLHelper.h"

#undef  _BACKWARD_BACKWARD_WARNING_H
)DICTPAYLOAD";
    static const char* classesHeaders[] = {
"MLHelper", payloadCode, "@",
nullptr
};
    static bool isInitialized = false;
    if (!isInitialized) {
      TROOT::RegisterModule("libMLHelper",
        headers, includePaths, payloadCode, fwdDeclCode,
        TriggerDictionaryInitialization_libMLHelper_Impl, {}, classesHeaders, /*hasCxxModule*/false);
      isInitialized = true;
    }
  }
  static struct DictInit {
    DictInit() {
      TriggerDictionaryInitialization_libMLHelper_Impl();
    }
  } __TheDictionaryInitializer;
}
void TriggerDictionaryInitialization_libMLHelper() {
  TriggerDictionaryInitialization_libMLHelper_Impl();
}

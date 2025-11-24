// Do NOT change. Changes will be lost next time file is generated

#define R__DICTIONARY_FILENAME G__AnalyzerTools
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
#include "include/AnalyzerParameter.h"
#include "include/ExampleParameter.h"
#include "include/LHAPDFHandler.h"
#include "include/MLHelper.h"
#include "include/MyCorrection.h"
#include "include/NonpromptParameter.h"
#include "include/PDFReweight.h"
#include "include/PhysicalConstants.h"
#include "include/SystematicHelper.h"

// Header files passed via #pragma extra_include

// The generated code does not explicitly qualify STL entities
namespace std {} using namespace std;

namespace ROOT {
   static TClass *AnalyzerParameter_Dictionary();
   static void AnalyzerParameter_TClassManip(TClass*);
   static void *new_AnalyzerParameter(void *p = nullptr);
   static void *newArray_AnalyzerParameter(Long_t size, void *p);
   static void delete_AnalyzerParameter(void *p);
   static void deleteArray_AnalyzerParameter(void *p);
   static void destruct_AnalyzerParameter(void *p);

   // Function generating the singleton type initializer
   static TGenericClassInfo *GenerateInitInstanceLocal(const ::AnalyzerParameter*)
   {
      ::AnalyzerParameter *ptr = nullptr;
      static ::TVirtualIsAProxy* isa_proxy = new ::TIsAProxy(typeid(::AnalyzerParameter));
      static ::ROOT::TGenericClassInfo 
         instance("AnalyzerParameter", "AnalyzerParameter.h", 14,
                  typeid(::AnalyzerParameter), ::ROOT::Internal::DefineBehavior(ptr, ptr),
                  &AnalyzerParameter_Dictionary, isa_proxy, 4,
                  sizeof(::AnalyzerParameter) );
      instance.SetNew(&new_AnalyzerParameter);
      instance.SetNewArray(&newArray_AnalyzerParameter);
      instance.SetDelete(&delete_AnalyzerParameter);
      instance.SetDeleteArray(&deleteArray_AnalyzerParameter);
      instance.SetDestructor(&destruct_AnalyzerParameter);
      return &instance;
   }
   TGenericClassInfo *GenerateInitInstance(const ::AnalyzerParameter*)
   {
      return GenerateInitInstanceLocal(static_cast<::AnalyzerParameter*>(nullptr));
   }
   // Static variable to force the class initialization
   static ::ROOT::TGenericClassInfo *_R__UNIQUE_DICT_(Init) = GenerateInitInstanceLocal(static_cast<const ::AnalyzerParameter*>(nullptr)); R__UseDummy(_R__UNIQUE_DICT_(Init));

   // Dictionary for non-ClassDef classes
   static TClass *AnalyzerParameter_Dictionary() {
      TClass* theClass =::ROOT::GenerateInitInstanceLocal(static_cast<const ::AnalyzerParameter*>(nullptr))->GetClass();
      AnalyzerParameter_TClassManip(theClass);
   return theClass;
   }

   static void AnalyzerParameter_TClassManip(TClass* ){
   }

} // end of namespace ROOT

namespace ROOT {
   static TClass *ExampleParameter_Dictionary();
   static void ExampleParameter_TClassManip(TClass*);
   static void *new_ExampleParameter(void *p = nullptr);
   static void *newArray_ExampleParameter(Long_t size, void *p);
   static void delete_ExampleParameter(void *p);
   static void deleteArray_ExampleParameter(void *p);
   static void destruct_ExampleParameter(void *p);

   // Function generating the singleton type initializer
   static TGenericClassInfo *GenerateInitInstanceLocal(const ::ExampleParameter*)
   {
      ::ExampleParameter *ptr = nullptr;
      static ::TVirtualIsAProxy* isa_proxy = new ::TIsAProxy(typeid(::ExampleParameter));
      static ::ROOT::TGenericClassInfo 
         instance("ExampleParameter", "ExampleParameter.h", 6,
                  typeid(::ExampleParameter), ::ROOT::Internal::DefineBehavior(ptr, ptr),
                  &ExampleParameter_Dictionary, isa_proxy, 4,
                  sizeof(::ExampleParameter) );
      instance.SetNew(&new_ExampleParameter);
      instance.SetNewArray(&newArray_ExampleParameter);
      instance.SetDelete(&delete_ExampleParameter);
      instance.SetDeleteArray(&deleteArray_ExampleParameter);
      instance.SetDestructor(&destruct_ExampleParameter);
      return &instance;
   }
   TGenericClassInfo *GenerateInitInstance(const ::ExampleParameter*)
   {
      return GenerateInitInstanceLocal(static_cast<::ExampleParameter*>(nullptr));
   }
   // Static variable to force the class initialization
   static ::ROOT::TGenericClassInfo *_R__UNIQUE_DICT_(Init) = GenerateInitInstanceLocal(static_cast<const ::ExampleParameter*>(nullptr)); R__UseDummy(_R__UNIQUE_DICT_(Init));

   // Dictionary for non-ClassDef classes
   static TClass *ExampleParameter_Dictionary() {
      TClass* theClass =::ROOT::GenerateInitInstanceLocal(static_cast<const ::ExampleParameter*>(nullptr))->GetClass();
      ExampleParameter_TClassManip(theClass);
   return theClass;
   }

   static void ExampleParameter_TClassManip(TClass* ){
   }

} // end of namespace ROOT

namespace ROOT {
   static TClass *LHAPDFHandler_Dictionary();
   static void LHAPDFHandler_TClassManip(TClass*);
   static void *new_LHAPDFHandler(void *p = nullptr);
   static void *newArray_LHAPDFHandler(Long_t size, void *p);
   static void delete_LHAPDFHandler(void *p);
   static void deleteArray_LHAPDFHandler(void *p);
   static void destruct_LHAPDFHandler(void *p);

   // Function generating the singleton type initializer
   static TGenericClassInfo *GenerateInitInstanceLocal(const ::LHAPDFHandler*)
   {
      ::LHAPDFHandler *ptr = nullptr;
      static ::TVirtualIsAProxy* isa_proxy = new ::TIsAProxy(typeid(::LHAPDFHandler));
      static ::ROOT::TGenericClassInfo 
         instance("LHAPDFHandler", "LHAPDFHandler.h", 16,
                  typeid(::LHAPDFHandler), ::ROOT::Internal::DefineBehavior(ptr, ptr),
                  &LHAPDFHandler_Dictionary, isa_proxy, 4,
                  sizeof(::LHAPDFHandler) );
      instance.SetNew(&new_LHAPDFHandler);
      instance.SetNewArray(&newArray_LHAPDFHandler);
      instance.SetDelete(&delete_LHAPDFHandler);
      instance.SetDeleteArray(&deleteArray_LHAPDFHandler);
      instance.SetDestructor(&destruct_LHAPDFHandler);
      return &instance;
   }
   TGenericClassInfo *GenerateInitInstance(const ::LHAPDFHandler*)
   {
      return GenerateInitInstanceLocal(static_cast<::LHAPDFHandler*>(nullptr));
   }
   // Static variable to force the class initialization
   static ::ROOT::TGenericClassInfo *_R__UNIQUE_DICT_(Init) = GenerateInitInstanceLocal(static_cast<const ::LHAPDFHandler*>(nullptr)); R__UseDummy(_R__UNIQUE_DICT_(Init));

   // Dictionary for non-ClassDef classes
   static TClass *LHAPDFHandler_Dictionary() {
      TClass* theClass =::ROOT::GenerateInitInstanceLocal(static_cast<const ::LHAPDFHandler*>(nullptr))->GetClass();
      LHAPDFHandler_TClassManip(theClass);
   return theClass;
   }

   static void LHAPDFHandler_TClassManip(TClass* ){
   }

} // end of namespace ROOT

namespace ROOT {
   static TClass *MyCorrection_Dictionary();
   static void MyCorrection_TClassManip(TClass*);
   static void *new_MyCorrection(void *p = nullptr);
   static void *newArray_MyCorrection(Long_t size, void *p);
   static void delete_MyCorrection(void *p);
   static void deleteArray_MyCorrection(void *p);
   static void destruct_MyCorrection(void *p);

   // Function generating the singleton type initializer
   static TGenericClassInfo *GenerateInitInstanceLocal(const ::MyCorrection*)
   {
      ::MyCorrection *ptr = nullptr;
      static ::TVirtualIsAProxy* isa_proxy = new ::TIsAProxy(typeid(::MyCorrection));
      static ::ROOT::TGenericClassInfo 
         instance("MyCorrection", "MyCorrection.h", 24,
                  typeid(::MyCorrection), ::ROOT::Internal::DefineBehavior(ptr, ptr),
                  &MyCorrection_Dictionary, isa_proxy, 4,
                  sizeof(::MyCorrection) );
      instance.SetNew(&new_MyCorrection);
      instance.SetNewArray(&newArray_MyCorrection);
      instance.SetDelete(&delete_MyCorrection);
      instance.SetDeleteArray(&deleteArray_MyCorrection);
      instance.SetDestructor(&destruct_MyCorrection);
      return &instance;
   }
   TGenericClassInfo *GenerateInitInstance(const ::MyCorrection*)
   {
      return GenerateInitInstanceLocal(static_cast<::MyCorrection*>(nullptr));
   }
   // Static variable to force the class initialization
   static ::ROOT::TGenericClassInfo *_R__UNIQUE_DICT_(Init) = GenerateInitInstanceLocal(static_cast<const ::MyCorrection*>(nullptr)); R__UseDummy(_R__UNIQUE_DICT_(Init));

   // Dictionary for non-ClassDef classes
   static TClass *MyCorrection_Dictionary() {
      TClass* theClass =::ROOT::GenerateInitInstanceLocal(static_cast<const ::MyCorrection*>(nullptr))->GetClass();
      MyCorrection_TClassManip(theClass);
   return theClass;
   }

   static void MyCorrection_TClassManip(TClass* ){
   }

} // end of namespace ROOT

namespace ROOT {
   static TClass *NonpromptParameter_Dictionary();
   static void NonpromptParameter_TClassManip(TClass*);
   static void *new_NonpromptParameter(void *p = nullptr);
   static void *newArray_NonpromptParameter(Long_t size, void *p);
   static void delete_NonpromptParameter(void *p);
   static void deleteArray_NonpromptParameter(void *p);
   static void destruct_NonpromptParameter(void *p);

   // Function generating the singleton type initializer
   static TGenericClassInfo *GenerateInitInstanceLocal(const ::NonpromptParameter*)
   {
      ::NonpromptParameter *ptr = nullptr;
      static ::TVirtualIsAProxy* isa_proxy = new ::TIsAProxy(typeid(::NonpromptParameter));
      static ::ROOT::TGenericClassInfo 
         instance("NonpromptParameter", "NonpromptParameter.h", 8,
                  typeid(::NonpromptParameter), ::ROOT::Internal::DefineBehavior(ptr, ptr),
                  &NonpromptParameter_Dictionary, isa_proxy, 4,
                  sizeof(::NonpromptParameter) );
      instance.SetNew(&new_NonpromptParameter);
      instance.SetNewArray(&newArray_NonpromptParameter);
      instance.SetDelete(&delete_NonpromptParameter);
      instance.SetDeleteArray(&deleteArray_NonpromptParameter);
      instance.SetDestructor(&destruct_NonpromptParameter);
      return &instance;
   }
   TGenericClassInfo *GenerateInitInstance(const ::NonpromptParameter*)
   {
      return GenerateInitInstanceLocal(static_cast<::NonpromptParameter*>(nullptr));
   }
   // Static variable to force the class initialization
   static ::ROOT::TGenericClassInfo *_R__UNIQUE_DICT_(Init) = GenerateInitInstanceLocal(static_cast<const ::NonpromptParameter*>(nullptr)); R__UseDummy(_R__UNIQUE_DICT_(Init));

   // Dictionary for non-ClassDef classes
   static TClass *NonpromptParameter_Dictionary() {
      TClass* theClass =::ROOT::GenerateInitInstanceLocal(static_cast<const ::NonpromptParameter*>(nullptr))->GetClass();
      NonpromptParameter_TClassManip(theClass);
   return theClass;
   }

   static void NonpromptParameter_TClassManip(TClass* ){
   }

} // end of namespace ROOT

namespace ROOT {
   static TClass *PDFReweight_Dictionary();
   static void PDFReweight_TClassManip(TClass*);
   static void *new_PDFReweight(void *p = nullptr);
   static void *newArray_PDFReweight(Long_t size, void *p);
   static void delete_PDFReweight(void *p);
   static void deleteArray_PDFReweight(void *p);
   static void destruct_PDFReweight(void *p);

   // Function generating the singleton type initializer
   static TGenericClassInfo *GenerateInitInstanceLocal(const ::PDFReweight*)
   {
      ::PDFReweight *ptr = nullptr;
      static ::TVirtualIsAProxy* isa_proxy = new ::TIsAProxy(typeid(::PDFReweight));
      static ::ROOT::TGenericClassInfo 
         instance("PDFReweight", "PDFReweight.h", 12,
                  typeid(::PDFReweight), ::ROOT::Internal::DefineBehavior(ptr, ptr),
                  &PDFReweight_Dictionary, isa_proxy, 4,
                  sizeof(::PDFReweight) );
      instance.SetNew(&new_PDFReweight);
      instance.SetNewArray(&newArray_PDFReweight);
      instance.SetDelete(&delete_PDFReweight);
      instance.SetDeleteArray(&deleteArray_PDFReweight);
      instance.SetDestructor(&destruct_PDFReweight);
      return &instance;
   }
   TGenericClassInfo *GenerateInitInstance(const ::PDFReweight*)
   {
      return GenerateInitInstanceLocal(static_cast<::PDFReweight*>(nullptr));
   }
   // Static variable to force the class initialization
   static ::ROOT::TGenericClassInfo *_R__UNIQUE_DICT_(Init) = GenerateInitInstanceLocal(static_cast<const ::PDFReweight*>(nullptr)); R__UseDummy(_R__UNIQUE_DICT_(Init));

   // Dictionary for non-ClassDef classes
   static TClass *PDFReweight_Dictionary() {
      TClass* theClass =::ROOT::GenerateInitInstanceLocal(static_cast<const ::PDFReweight*>(nullptr))->GetClass();
      PDFReweight_TClassManip(theClass);
   return theClass;
   }

   static void PDFReweight_TClassManip(TClass* ){
   }

} // end of namespace ROOT

namespace ROOT {
   static TClass *SystematicHelper_Dictionary();
   static void SystematicHelper_TClassManip(TClass*);
   static void delete_SystematicHelper(void *p);
   static void deleteArray_SystematicHelper(void *p);
   static void destruct_SystematicHelper(void *p);

   // Function generating the singleton type initializer
   static TGenericClassInfo *GenerateInitInstanceLocal(const ::SystematicHelper*)
   {
      ::SystematicHelper *ptr = nullptr;
      static ::TVirtualIsAProxy* isa_proxy = new ::TIsAProxy(typeid(::SystematicHelper));
      static ::ROOT::TGenericClassInfo 
         instance("SystematicHelper", "SystematicHelper.h", 17,
                  typeid(::SystematicHelper), ::ROOT::Internal::DefineBehavior(ptr, ptr),
                  &SystematicHelper_Dictionary, isa_proxy, 4,
                  sizeof(::SystematicHelper) );
      instance.SetDelete(&delete_SystematicHelper);
      instance.SetDeleteArray(&deleteArray_SystematicHelper);
      instance.SetDestructor(&destruct_SystematicHelper);
      return &instance;
   }
   TGenericClassInfo *GenerateInitInstance(const ::SystematicHelper*)
   {
      return GenerateInitInstanceLocal(static_cast<::SystematicHelper*>(nullptr));
   }
   // Static variable to force the class initialization
   static ::ROOT::TGenericClassInfo *_R__UNIQUE_DICT_(Init) = GenerateInitInstanceLocal(static_cast<const ::SystematicHelper*>(nullptr)); R__UseDummy(_R__UNIQUE_DICT_(Init));

   // Dictionary for non-ClassDef classes
   static TClass *SystematicHelper_Dictionary() {
      TClass* theClass =::ROOT::GenerateInitInstanceLocal(static_cast<const ::SystematicHelper*>(nullptr))->GetClass();
      SystematicHelper_TClassManip(theClass);
   return theClass;
   }

   static void SystematicHelper_TClassManip(TClass* ){
   }

} // end of namespace ROOT

namespace ROOT {
   // Wrappers around operator new
   static void *new_AnalyzerParameter(void *p) {
      return  p ? new(p) ::AnalyzerParameter : new ::AnalyzerParameter;
   }
   static void *newArray_AnalyzerParameter(Long_t nElements, void *p) {
      return p ? new(p) ::AnalyzerParameter[nElements] : new ::AnalyzerParameter[nElements];
   }
   // Wrapper around operator delete
   static void delete_AnalyzerParameter(void *p) {
      delete (static_cast<::AnalyzerParameter*>(p));
   }
   static void deleteArray_AnalyzerParameter(void *p) {
      delete [] (static_cast<::AnalyzerParameter*>(p));
   }
   static void destruct_AnalyzerParameter(void *p) {
      typedef ::AnalyzerParameter current_t;
      (static_cast<current_t*>(p))->~current_t();
   }
} // end of namespace ROOT for class ::AnalyzerParameter

namespace ROOT {
   // Wrappers around operator new
   static void *new_ExampleParameter(void *p) {
      return  p ? new(p) ::ExampleParameter : new ::ExampleParameter;
   }
   static void *newArray_ExampleParameter(Long_t nElements, void *p) {
      return p ? new(p) ::ExampleParameter[nElements] : new ::ExampleParameter[nElements];
   }
   // Wrapper around operator delete
   static void delete_ExampleParameter(void *p) {
      delete (static_cast<::ExampleParameter*>(p));
   }
   static void deleteArray_ExampleParameter(void *p) {
      delete [] (static_cast<::ExampleParameter*>(p));
   }
   static void destruct_ExampleParameter(void *p) {
      typedef ::ExampleParameter current_t;
      (static_cast<current_t*>(p))->~current_t();
   }
} // end of namespace ROOT for class ::ExampleParameter

namespace ROOT {
   // Wrappers around operator new
   static void *new_LHAPDFHandler(void *p) {
      return  p ? new(p) ::LHAPDFHandler : new ::LHAPDFHandler;
   }
   static void *newArray_LHAPDFHandler(Long_t nElements, void *p) {
      return p ? new(p) ::LHAPDFHandler[nElements] : new ::LHAPDFHandler[nElements];
   }
   // Wrapper around operator delete
   static void delete_LHAPDFHandler(void *p) {
      delete (static_cast<::LHAPDFHandler*>(p));
   }
   static void deleteArray_LHAPDFHandler(void *p) {
      delete [] (static_cast<::LHAPDFHandler*>(p));
   }
   static void destruct_LHAPDFHandler(void *p) {
      typedef ::LHAPDFHandler current_t;
      (static_cast<current_t*>(p))->~current_t();
   }
} // end of namespace ROOT for class ::LHAPDFHandler

namespace ROOT {
   // Wrappers around operator new
   static void *new_MyCorrection(void *p) {
      return  p ? new(p) ::MyCorrection : new ::MyCorrection;
   }
   static void *newArray_MyCorrection(Long_t nElements, void *p) {
      return p ? new(p) ::MyCorrection[nElements] : new ::MyCorrection[nElements];
   }
   // Wrapper around operator delete
   static void delete_MyCorrection(void *p) {
      delete (static_cast<::MyCorrection*>(p));
   }
   static void deleteArray_MyCorrection(void *p) {
      delete [] (static_cast<::MyCorrection*>(p));
   }
   static void destruct_MyCorrection(void *p) {
      typedef ::MyCorrection current_t;
      (static_cast<current_t*>(p))->~current_t();
   }
} // end of namespace ROOT for class ::MyCorrection

namespace ROOT {
   // Wrappers around operator new
   static void *new_NonpromptParameter(void *p) {
      return  p ? new(p) ::NonpromptParameter : new ::NonpromptParameter;
   }
   static void *newArray_NonpromptParameter(Long_t nElements, void *p) {
      return p ? new(p) ::NonpromptParameter[nElements] : new ::NonpromptParameter[nElements];
   }
   // Wrapper around operator delete
   static void delete_NonpromptParameter(void *p) {
      delete (static_cast<::NonpromptParameter*>(p));
   }
   static void deleteArray_NonpromptParameter(void *p) {
      delete [] (static_cast<::NonpromptParameter*>(p));
   }
   static void destruct_NonpromptParameter(void *p) {
      typedef ::NonpromptParameter current_t;
      (static_cast<current_t*>(p))->~current_t();
   }
} // end of namespace ROOT for class ::NonpromptParameter

namespace ROOT {
   // Wrappers around operator new
   static void *new_PDFReweight(void *p) {
      return  p ? new(p) ::PDFReweight : new ::PDFReweight;
   }
   static void *newArray_PDFReweight(Long_t nElements, void *p) {
      return p ? new(p) ::PDFReweight[nElements] : new ::PDFReweight[nElements];
   }
   // Wrapper around operator delete
   static void delete_PDFReweight(void *p) {
      delete (static_cast<::PDFReweight*>(p));
   }
   static void deleteArray_PDFReweight(void *p) {
      delete [] (static_cast<::PDFReweight*>(p));
   }
   static void destruct_PDFReweight(void *p) {
      typedef ::PDFReweight current_t;
      (static_cast<current_t*>(p))->~current_t();
   }
} // end of namespace ROOT for class ::PDFReweight

namespace ROOT {
   // Wrapper around operator delete
   static void delete_SystematicHelper(void *p) {
      delete (static_cast<::SystematicHelper*>(p));
   }
   static void deleteArray_SystematicHelper(void *p) {
      delete [] (static_cast<::SystematicHelper*>(p));
   }
   static void destruct_SystematicHelper(void *p) {
      typedef ::SystematicHelper current_t;
      (static_cast<current_t*>(p))->~current_t();
   }
} // end of namespace ROOT for class ::SystematicHelper

namespace ROOT {
   static TClass *vectorlEstringgR_Dictionary();
   static void vectorlEstringgR_TClassManip(TClass*);
   static void *new_vectorlEstringgR(void *p = nullptr);
   static void *newArray_vectorlEstringgR(Long_t size, void *p);
   static void delete_vectorlEstringgR(void *p);
   static void deleteArray_vectorlEstringgR(void *p);
   static void destruct_vectorlEstringgR(void *p);

   // Function generating the singleton type initializer
   static TGenericClassInfo *GenerateInitInstanceLocal(const vector<string>*)
   {
      vector<string> *ptr = nullptr;
      static ::TVirtualIsAProxy* isa_proxy = new ::TIsAProxy(typeid(vector<string>));
      static ::ROOT::TGenericClassInfo 
         instance("vector<string>", -2, "vector", 428,
                  typeid(vector<string>), ::ROOT::Internal::DefineBehavior(ptr, ptr),
                  &vectorlEstringgR_Dictionary, isa_proxy, 0,
                  sizeof(vector<string>) );
      instance.SetNew(&new_vectorlEstringgR);
      instance.SetNewArray(&newArray_vectorlEstringgR);
      instance.SetDelete(&delete_vectorlEstringgR);
      instance.SetDeleteArray(&deleteArray_vectorlEstringgR);
      instance.SetDestructor(&destruct_vectorlEstringgR);
      instance.AdoptCollectionProxyInfo(TCollectionProxyInfo::Generate(TCollectionProxyInfo::Pushback< vector<string> >()));

      instance.AdoptAlternate(::ROOT::AddClassAlternate("vector<string>","std::vector<std::__cxx11::basic_string<char, std::char_traits<char>, std::allocator<char> >, std::allocator<std::__cxx11::basic_string<char, std::char_traits<char>, std::allocator<char> > > >"));
      return &instance;
   }
   // Static variable to force the class initialization
   static ::ROOT::TGenericClassInfo *_R__UNIQUE_DICT_(Init) = GenerateInitInstanceLocal(static_cast<const vector<string>*>(nullptr)); R__UseDummy(_R__UNIQUE_DICT_(Init));

   // Dictionary for non-ClassDef classes
   static TClass *vectorlEstringgR_Dictionary() {
      TClass* theClass =::ROOT::GenerateInitInstanceLocal(static_cast<const vector<string>*>(nullptr))->GetClass();
      vectorlEstringgR_TClassManip(theClass);
   return theClass;
   }

   static void vectorlEstringgR_TClassManip(TClass* ){
   }

} // end of namespace ROOT

namespace ROOT {
   // Wrappers around operator new
   static void *new_vectorlEstringgR(void *p) {
      return  p ? ::new(static_cast<::ROOT::Internal::TOperatorNewHelper*>(p)) vector<string> : new vector<string>;
   }
   static void *newArray_vectorlEstringgR(Long_t nElements, void *p) {
      return p ? ::new(static_cast<::ROOT::Internal::TOperatorNewHelper*>(p)) vector<string>[nElements] : new vector<string>[nElements];
   }
   // Wrapper around operator delete
   static void delete_vectorlEstringgR(void *p) {
      delete (static_cast<vector<string>*>(p));
   }
   static void deleteArray_vectorlEstringgR(void *p) {
      delete [] (static_cast<vector<string>*>(p));
   }
   static void destruct_vectorlEstringgR(void *p) {
      typedef vector<string> current_t;
      (static_cast<current_t*>(p))->~current_t();
   }
} // end of namespace ROOT for class vector<string>

namespace ROOT {
   static TClass *vectorlESystematicHelpercLcLSYSTgR_Dictionary();
   static void vectorlESystematicHelpercLcLSYSTgR_TClassManip(TClass*);
   static void *new_vectorlESystematicHelpercLcLSYSTgR(void *p = nullptr);
   static void *newArray_vectorlESystematicHelpercLcLSYSTgR(Long_t size, void *p);
   static void delete_vectorlESystematicHelpercLcLSYSTgR(void *p);
   static void deleteArray_vectorlESystematicHelpercLcLSYSTgR(void *p);
   static void destruct_vectorlESystematicHelpercLcLSYSTgR(void *p);

   // Function generating the singleton type initializer
   static TGenericClassInfo *GenerateInitInstanceLocal(const vector<SystematicHelper::SYST>*)
   {
      vector<SystematicHelper::SYST> *ptr = nullptr;
      static ::TVirtualIsAProxy* isa_proxy = new ::TIsAProxy(typeid(vector<SystematicHelper::SYST>));
      static ::ROOT::TGenericClassInfo 
         instance("vector<SystematicHelper::SYST>", -2, "vector", 428,
                  typeid(vector<SystematicHelper::SYST>), ::ROOT::Internal::DefineBehavior(ptr, ptr),
                  &vectorlESystematicHelpercLcLSYSTgR_Dictionary, isa_proxy, 0,
                  sizeof(vector<SystematicHelper::SYST>) );
      instance.SetNew(&new_vectorlESystematicHelpercLcLSYSTgR);
      instance.SetNewArray(&newArray_vectorlESystematicHelpercLcLSYSTgR);
      instance.SetDelete(&delete_vectorlESystematicHelpercLcLSYSTgR);
      instance.SetDeleteArray(&deleteArray_vectorlESystematicHelpercLcLSYSTgR);
      instance.SetDestructor(&destruct_vectorlESystematicHelpercLcLSYSTgR);
      instance.AdoptCollectionProxyInfo(TCollectionProxyInfo::Generate(TCollectionProxyInfo::Pushback< vector<SystematicHelper::SYST> >()));

      instance.AdoptAlternate(::ROOT::AddClassAlternate("vector<SystematicHelper::SYST>","std::vector<SystematicHelper::SYST, std::allocator<SystematicHelper::SYST> >"));
      return &instance;
   }
   // Static variable to force the class initialization
   static ::ROOT::TGenericClassInfo *_R__UNIQUE_DICT_(Init) = GenerateInitInstanceLocal(static_cast<const vector<SystematicHelper::SYST>*>(nullptr)); R__UseDummy(_R__UNIQUE_DICT_(Init));

   // Dictionary for non-ClassDef classes
   static TClass *vectorlESystematicHelpercLcLSYSTgR_Dictionary() {
      TClass* theClass =::ROOT::GenerateInitInstanceLocal(static_cast<const vector<SystematicHelper::SYST>*>(nullptr))->GetClass();
      vectorlESystematicHelpercLcLSYSTgR_TClassManip(theClass);
   return theClass;
   }

   static void vectorlESystematicHelpercLcLSYSTgR_TClassManip(TClass* ){
   }

} // end of namespace ROOT

namespace ROOT {
   // Wrappers around operator new
   static void *new_vectorlESystematicHelpercLcLSYSTgR(void *p) {
      return  p ? ::new(static_cast<::ROOT::Internal::TOperatorNewHelper*>(p)) vector<SystematicHelper::SYST> : new vector<SystematicHelper::SYST>;
   }
   static void *newArray_vectorlESystematicHelpercLcLSYSTgR(Long_t nElements, void *p) {
      return p ? ::new(static_cast<::ROOT::Internal::TOperatorNewHelper*>(p)) vector<SystematicHelper::SYST>[nElements] : new vector<SystematicHelper::SYST>[nElements];
   }
   // Wrapper around operator delete
   static void delete_vectorlESystematicHelpercLcLSYSTgR(void *p) {
      delete (static_cast<vector<SystematicHelper::SYST>*>(p));
   }
   static void deleteArray_vectorlESystematicHelpercLcLSYSTgR(void *p) {
      delete [] (static_cast<vector<SystematicHelper::SYST>*>(p));
   }
   static void destruct_vectorlESystematicHelpercLcLSYSTgR(void *p) {
      typedef vector<SystematicHelper::SYST> current_t;
      (static_cast<current_t*>(p))->~current_t();
   }
} // end of namespace ROOT for class vector<SystematicHelper::SYST>

namespace ROOT {
   static TClass *vectorlESystematicHelpercLcLIter_objgR_Dictionary();
   static void vectorlESystematicHelpercLcLIter_objgR_TClassManip(TClass*);
   static void *new_vectorlESystematicHelpercLcLIter_objgR(void *p = nullptr);
   static void *newArray_vectorlESystematicHelpercLcLIter_objgR(Long_t size, void *p);
   static void delete_vectorlESystematicHelpercLcLIter_objgR(void *p);
   static void deleteArray_vectorlESystematicHelpercLcLIter_objgR(void *p);
   static void destruct_vectorlESystematicHelpercLcLIter_objgR(void *p);

   // Function generating the singleton type initializer
   static TGenericClassInfo *GenerateInitInstanceLocal(const vector<SystematicHelper::Iter_obj>*)
   {
      vector<SystematicHelper::Iter_obj> *ptr = nullptr;
      static ::TVirtualIsAProxy* isa_proxy = new ::TIsAProxy(typeid(vector<SystematicHelper::Iter_obj>));
      static ::ROOT::TGenericClassInfo 
         instance("vector<SystematicHelper::Iter_obj>", -2, "vector", 428,
                  typeid(vector<SystematicHelper::Iter_obj>), ::ROOT::Internal::DefineBehavior(ptr, ptr),
                  &vectorlESystematicHelpercLcLIter_objgR_Dictionary, isa_proxy, 0,
                  sizeof(vector<SystematicHelper::Iter_obj>) );
      instance.SetNew(&new_vectorlESystematicHelpercLcLIter_objgR);
      instance.SetNewArray(&newArray_vectorlESystematicHelpercLcLIter_objgR);
      instance.SetDelete(&delete_vectorlESystematicHelpercLcLIter_objgR);
      instance.SetDeleteArray(&deleteArray_vectorlESystematicHelpercLcLIter_objgR);
      instance.SetDestructor(&destruct_vectorlESystematicHelpercLcLIter_objgR);
      instance.AdoptCollectionProxyInfo(TCollectionProxyInfo::Generate(TCollectionProxyInfo::Pushback< vector<SystematicHelper::Iter_obj> >()));

      instance.AdoptAlternate(::ROOT::AddClassAlternate("vector<SystematicHelper::Iter_obj>","std::vector<SystematicHelper::Iter_obj, std::allocator<SystematicHelper::Iter_obj> >"));
      return &instance;
   }
   // Static variable to force the class initialization
   static ::ROOT::TGenericClassInfo *_R__UNIQUE_DICT_(Init) = GenerateInitInstanceLocal(static_cast<const vector<SystematicHelper::Iter_obj>*>(nullptr)); R__UseDummy(_R__UNIQUE_DICT_(Init));

   // Dictionary for non-ClassDef classes
   static TClass *vectorlESystematicHelpercLcLIter_objgR_Dictionary() {
      TClass* theClass =::ROOT::GenerateInitInstanceLocal(static_cast<const vector<SystematicHelper::Iter_obj>*>(nullptr))->GetClass();
      vectorlESystematicHelpercLcLIter_objgR_TClassManip(theClass);
   return theClass;
   }

   static void vectorlESystematicHelpercLcLIter_objgR_TClassManip(TClass* ){
   }

} // end of namespace ROOT

namespace ROOT {
   // Wrappers around operator new
   static void *new_vectorlESystematicHelpercLcLIter_objgR(void *p) {
      return  p ? ::new(static_cast<::ROOT::Internal::TOperatorNewHelper*>(p)) vector<SystematicHelper::Iter_obj> : new vector<SystematicHelper::Iter_obj>;
   }
   static void *newArray_vectorlESystematicHelpercLcLIter_objgR(Long_t nElements, void *p) {
      return p ? ::new(static_cast<::ROOT::Internal::TOperatorNewHelper*>(p)) vector<SystematicHelper::Iter_obj>[nElements] : new vector<SystematicHelper::Iter_obj>[nElements];
   }
   // Wrapper around operator delete
   static void delete_vectorlESystematicHelpercLcLIter_objgR(void *p) {
      delete (static_cast<vector<SystematicHelper::Iter_obj>*>(p));
   }
   static void deleteArray_vectorlESystematicHelpercLcLIter_objgR(void *p) {
      delete [] (static_cast<vector<SystematicHelper::Iter_obj>*>(p));
   }
   static void destruct_vectorlESystematicHelpercLcLIter_objgR(void *p) {
      typedef vector<SystematicHelper::Iter_obj> current_t;
      (static_cast<current_t*>(p))->~current_t();
   }
} // end of namespace ROOT for class vector<SystematicHelper::Iter_obj>

namespace ROOT {
   static TClass *unordered_maplEstringcOstringgR_Dictionary();
   static void unordered_maplEstringcOstringgR_TClassManip(TClass*);
   static void *new_unordered_maplEstringcOstringgR(void *p = nullptr);
   static void *newArray_unordered_maplEstringcOstringgR(Long_t size, void *p);
   static void delete_unordered_maplEstringcOstringgR(void *p);
   static void deleteArray_unordered_maplEstringcOstringgR(void *p);
   static void destruct_unordered_maplEstringcOstringgR(void *p);

   // Function generating the singleton type initializer
   static TGenericClassInfo *GenerateInitInstanceLocal(const unordered_map<string,string>*)
   {
      unordered_map<string,string> *ptr = nullptr;
      static ::TVirtualIsAProxy* isa_proxy = new ::TIsAProxy(typeid(unordered_map<string,string>));
      static ::ROOT::TGenericClassInfo 
         instance("unordered_map<string,string>", -2, "unordered_map", 109,
                  typeid(unordered_map<string,string>), ::ROOT::Internal::DefineBehavior(ptr, ptr),
                  &unordered_maplEstringcOstringgR_Dictionary, isa_proxy, 0,
                  sizeof(unordered_map<string,string>) );
      instance.SetNew(&new_unordered_maplEstringcOstringgR);
      instance.SetNewArray(&newArray_unordered_maplEstringcOstringgR);
      instance.SetDelete(&delete_unordered_maplEstringcOstringgR);
      instance.SetDeleteArray(&deleteArray_unordered_maplEstringcOstringgR);
      instance.SetDestructor(&destruct_unordered_maplEstringcOstringgR);
      instance.AdoptCollectionProxyInfo(TCollectionProxyInfo::Generate(TCollectionProxyInfo::MapInsert< unordered_map<string,string> >()));

      instance.AdoptAlternate(::ROOT::AddClassAlternate("unordered_map<string,string>","std::unordered_map<std::__cxx11::basic_string<char, std::char_traits<char>, std::allocator<char> >, std::__cxx11::basic_string<char, std::char_traits<char>, std::allocator<char> >, std::hash<std::__cxx11::basic_string<char, std::char_traits<char>, std::allocator<char> > >, std::equal_to<std::__cxx11::basic_string<char, std::char_traits<char>, std::allocator<char> > >, std::allocator<std::pair<std::__cxx11::basic_string<char, std::char_traits<char>, std::allocator<char> > const, std::__cxx11::basic_string<char, std::char_traits<char>, std::allocator<char> > > > >"));
      return &instance;
   }
   // Static variable to force the class initialization
   static ::ROOT::TGenericClassInfo *_R__UNIQUE_DICT_(Init) = GenerateInitInstanceLocal(static_cast<const unordered_map<string,string>*>(nullptr)); R__UseDummy(_R__UNIQUE_DICT_(Init));

   // Dictionary for non-ClassDef classes
   static TClass *unordered_maplEstringcOstringgR_Dictionary() {
      TClass* theClass =::ROOT::GenerateInitInstanceLocal(static_cast<const unordered_map<string,string>*>(nullptr))->GetClass();
      unordered_maplEstringcOstringgR_TClassManip(theClass);
   return theClass;
   }

   static void unordered_maplEstringcOstringgR_TClassManip(TClass* ){
   }

} // end of namespace ROOT

namespace ROOT {
   // Wrappers around operator new
   static void *new_unordered_maplEstringcOstringgR(void *p) {
      return  p ? ::new(static_cast<::ROOT::Internal::TOperatorNewHelper*>(p)) unordered_map<string,string> : new unordered_map<string,string>;
   }
   static void *newArray_unordered_maplEstringcOstringgR(Long_t nElements, void *p) {
      return p ? ::new(static_cast<::ROOT::Internal::TOperatorNewHelper*>(p)) unordered_map<string,string>[nElements] : new unordered_map<string,string>[nElements];
   }
   // Wrapper around operator delete
   static void delete_unordered_maplEstringcOstringgR(void *p) {
      delete (static_cast<unordered_map<string,string>*>(p));
   }
   static void deleteArray_unordered_maplEstringcOstringgR(void *p) {
      delete [] (static_cast<unordered_map<string,string>*>(p));
   }
   static void destruct_unordered_maplEstringcOstringgR(void *p) {
      typedef unordered_map<string,string> current_t;
      (static_cast<current_t*>(p))->~current_t();
   }
} // end of namespace ROOT for class unordered_map<string,string>

namespace ROOT {
   static TClass *unordered_maplEstringcOfunctionlEfloatoPMyCorrectioncLcLvariationcOTStringcPgRsPgR_Dictionary();
   static void unordered_maplEstringcOfunctionlEfloatoPMyCorrectioncLcLvariationcOTStringcPgRsPgR_TClassManip(TClass*);
   static void *new_unordered_maplEstringcOfunctionlEfloatoPMyCorrectioncLcLvariationcOTStringcPgRsPgR(void *p = nullptr);
   static void *newArray_unordered_maplEstringcOfunctionlEfloatoPMyCorrectioncLcLvariationcOTStringcPgRsPgR(Long_t size, void *p);
   static void delete_unordered_maplEstringcOfunctionlEfloatoPMyCorrectioncLcLvariationcOTStringcPgRsPgR(void *p);
   static void deleteArray_unordered_maplEstringcOfunctionlEfloatoPMyCorrectioncLcLvariationcOTStringcPgRsPgR(void *p);
   static void destruct_unordered_maplEstringcOfunctionlEfloatoPMyCorrectioncLcLvariationcOTStringcPgRsPgR(void *p);

   // Function generating the singleton type initializer
   static TGenericClassInfo *GenerateInitInstanceLocal(const unordered_map<string,function<float(MyCorrection::variation,TString)> >*)
   {
      unordered_map<string,function<float(MyCorrection::variation,TString)> > *ptr = nullptr;
      static ::TVirtualIsAProxy* isa_proxy = new ::TIsAProxy(typeid(unordered_map<string,function<float(MyCorrection::variation,TString)> >));
      static ::ROOT::TGenericClassInfo 
         instance("unordered_map<string,function<float(MyCorrection::variation,TString)> >", -2, "unordered_map", 109,
                  typeid(unordered_map<string,function<float(MyCorrection::variation,TString)> >), ::ROOT::Internal::DefineBehavior(ptr, ptr),
                  &unordered_maplEstringcOfunctionlEfloatoPMyCorrectioncLcLvariationcOTStringcPgRsPgR_Dictionary, isa_proxy, 0,
                  sizeof(unordered_map<string,function<float(MyCorrection::variation,TString)> >) );
      instance.SetNew(&new_unordered_maplEstringcOfunctionlEfloatoPMyCorrectioncLcLvariationcOTStringcPgRsPgR);
      instance.SetNewArray(&newArray_unordered_maplEstringcOfunctionlEfloatoPMyCorrectioncLcLvariationcOTStringcPgRsPgR);
      instance.SetDelete(&delete_unordered_maplEstringcOfunctionlEfloatoPMyCorrectioncLcLvariationcOTStringcPgRsPgR);
      instance.SetDeleteArray(&deleteArray_unordered_maplEstringcOfunctionlEfloatoPMyCorrectioncLcLvariationcOTStringcPgRsPgR);
      instance.SetDestructor(&destruct_unordered_maplEstringcOfunctionlEfloatoPMyCorrectioncLcLvariationcOTStringcPgRsPgR);
      instance.AdoptCollectionProxyInfo(TCollectionProxyInfo::Generate(TCollectionProxyInfo::MapInsert< unordered_map<string,function<float(MyCorrection::variation,TString)> > >()));

      instance.AdoptAlternate(::ROOT::AddClassAlternate("unordered_map<string,function<float(MyCorrection::variation,TString)> >","std::unordered_map<std::__cxx11::basic_string<char, std::char_traits<char>, std::allocator<char> >, std::function<float (MyCorrection::variation, TString)>, std::hash<std::__cxx11::basic_string<char, std::char_traits<char>, std::allocator<char> > >, std::equal_to<std::__cxx11::basic_string<char, std::char_traits<char>, std::allocator<char> > >, std::allocator<std::pair<std::__cxx11::basic_string<char, std::char_traits<char>, std::allocator<char> > const, std::function<float (MyCorrection::variation, TString)> > > >"));
      return &instance;
   }
   // Static variable to force the class initialization
   static ::ROOT::TGenericClassInfo *_R__UNIQUE_DICT_(Init) = GenerateInitInstanceLocal(static_cast<const unordered_map<string,function<float(MyCorrection::variation,TString)> >*>(nullptr)); R__UseDummy(_R__UNIQUE_DICT_(Init));

   // Dictionary for non-ClassDef classes
   static TClass *unordered_maplEstringcOfunctionlEfloatoPMyCorrectioncLcLvariationcOTStringcPgRsPgR_Dictionary() {
      TClass* theClass =::ROOT::GenerateInitInstanceLocal(static_cast<const unordered_map<string,function<float(MyCorrection::variation,TString)> >*>(nullptr))->GetClass();
      unordered_maplEstringcOfunctionlEfloatoPMyCorrectioncLcLvariationcOTStringcPgRsPgR_TClassManip(theClass);
   return theClass;
   }

   static void unordered_maplEstringcOfunctionlEfloatoPMyCorrectioncLcLvariationcOTStringcPgRsPgR_TClassManip(TClass* ){
   }

} // end of namespace ROOT

namespace ROOT {
   // Wrappers around operator new
   static void *new_unordered_maplEstringcOfunctionlEfloatoPMyCorrectioncLcLvariationcOTStringcPgRsPgR(void *p) {
      return  p ? ::new(static_cast<::ROOT::Internal::TOperatorNewHelper*>(p)) unordered_map<string,function<float(MyCorrection::variation,TString)> > : new unordered_map<string,function<float(MyCorrection::variation,TString)> >;
   }
   static void *newArray_unordered_maplEstringcOfunctionlEfloatoPMyCorrectioncLcLvariationcOTStringcPgRsPgR(Long_t nElements, void *p) {
      return p ? ::new(static_cast<::ROOT::Internal::TOperatorNewHelper*>(p)) unordered_map<string,function<float(MyCorrection::variation,TString)> >[nElements] : new unordered_map<string,function<float(MyCorrection::variation,TString)> >[nElements];
   }
   // Wrapper around operator delete
   static void delete_unordered_maplEstringcOfunctionlEfloatoPMyCorrectioncLcLvariationcOTStringcPgRsPgR(void *p) {
      delete (static_cast<unordered_map<string,function<float(MyCorrection::variation,TString)> >*>(p));
   }
   static void deleteArray_unordered_maplEstringcOfunctionlEfloatoPMyCorrectioncLcLvariationcOTStringcPgRsPgR(void *p) {
      delete [] (static_cast<unordered_map<string,function<float(MyCorrection::variation,TString)> >*>(p));
   }
   static void destruct_unordered_maplEstringcOfunctionlEfloatoPMyCorrectioncLcLvariationcOTStringcPgRsPgR(void *p) {
      typedef unordered_map<string,function<float(MyCorrection::variation,TString)> > current_t;
      (static_cast<current_t*>(p))->~current_t();
   }
} // end of namespace ROOT for class unordered_map<string,function<float(MyCorrection::variation,TString)> >

namespace ROOT {
   static TClass *unordered_maplEstringcOfunctionlEfloatoPcPgRsPgR_Dictionary();
   static void unordered_maplEstringcOfunctionlEfloatoPcPgRsPgR_TClassManip(TClass*);
   static void *new_unordered_maplEstringcOfunctionlEfloatoPcPgRsPgR(void *p = nullptr);
   static void *newArray_unordered_maplEstringcOfunctionlEfloatoPcPgRsPgR(Long_t size, void *p);
   static void delete_unordered_maplEstringcOfunctionlEfloatoPcPgRsPgR(void *p);
   static void deleteArray_unordered_maplEstringcOfunctionlEfloatoPcPgRsPgR(void *p);
   static void destruct_unordered_maplEstringcOfunctionlEfloatoPcPgRsPgR(void *p);

   // Function generating the singleton type initializer
   static TGenericClassInfo *GenerateInitInstanceLocal(const unordered_map<string,function<float()> >*)
   {
      unordered_map<string,function<float()> > *ptr = nullptr;
      static ::TVirtualIsAProxy* isa_proxy = new ::TIsAProxy(typeid(unordered_map<string,function<float()> >));
      static ::ROOT::TGenericClassInfo 
         instance("unordered_map<string,function<float()> >", -2, "unordered_map", 109,
                  typeid(unordered_map<string,function<float()> >), ::ROOT::Internal::DefineBehavior(ptr, ptr),
                  &unordered_maplEstringcOfunctionlEfloatoPcPgRsPgR_Dictionary, isa_proxy, 0,
                  sizeof(unordered_map<string,function<float()> >) );
      instance.SetNew(&new_unordered_maplEstringcOfunctionlEfloatoPcPgRsPgR);
      instance.SetNewArray(&newArray_unordered_maplEstringcOfunctionlEfloatoPcPgRsPgR);
      instance.SetDelete(&delete_unordered_maplEstringcOfunctionlEfloatoPcPgRsPgR);
      instance.SetDeleteArray(&deleteArray_unordered_maplEstringcOfunctionlEfloatoPcPgRsPgR);
      instance.SetDestructor(&destruct_unordered_maplEstringcOfunctionlEfloatoPcPgRsPgR);
      instance.AdoptCollectionProxyInfo(TCollectionProxyInfo::Generate(TCollectionProxyInfo::MapInsert< unordered_map<string,function<float()> > >()));

      instance.AdoptAlternate(::ROOT::AddClassAlternate("unordered_map<string,function<float()> >","std::unordered_map<std::__cxx11::basic_string<char, std::char_traits<char>, std::allocator<char> >, std::function<float ()>, std::hash<std::__cxx11::basic_string<char, std::char_traits<char>, std::allocator<char> > >, std::equal_to<std::__cxx11::basic_string<char, std::char_traits<char>, std::allocator<char> > >, std::allocator<std::pair<std::__cxx11::basic_string<char, std::char_traits<char>, std::allocator<char> > const, std::function<float ()> > > >"));
      return &instance;
   }
   // Static variable to force the class initialization
   static ::ROOT::TGenericClassInfo *_R__UNIQUE_DICT_(Init) = GenerateInitInstanceLocal(static_cast<const unordered_map<string,function<float()> >*>(nullptr)); R__UseDummy(_R__UNIQUE_DICT_(Init));

   // Dictionary for non-ClassDef classes
   static TClass *unordered_maplEstringcOfunctionlEfloatoPcPgRsPgR_Dictionary() {
      TClass* theClass =::ROOT::GenerateInitInstanceLocal(static_cast<const unordered_map<string,function<float()> >*>(nullptr))->GetClass();
      unordered_maplEstringcOfunctionlEfloatoPcPgRsPgR_TClassManip(theClass);
   return theClass;
   }

   static void unordered_maplEstringcOfunctionlEfloatoPcPgRsPgR_TClassManip(TClass* ){
   }

} // end of namespace ROOT

namespace ROOT {
   // Wrappers around operator new
   static void *new_unordered_maplEstringcOfunctionlEfloatoPcPgRsPgR(void *p) {
      return  p ? ::new(static_cast<::ROOT::Internal::TOperatorNewHelper*>(p)) unordered_map<string,function<float()> > : new unordered_map<string,function<float()> >;
   }
   static void *newArray_unordered_maplEstringcOfunctionlEfloatoPcPgRsPgR(Long_t nElements, void *p) {
      return p ? ::new(static_cast<::ROOT::Internal::TOperatorNewHelper*>(p)) unordered_map<string,function<float()> >[nElements] : new unordered_map<string,function<float()> >[nElements];
   }
   // Wrapper around operator delete
   static void delete_unordered_maplEstringcOfunctionlEfloatoPcPgRsPgR(void *p) {
      delete (static_cast<unordered_map<string,function<float()> >*>(p));
   }
   static void deleteArray_unordered_maplEstringcOfunctionlEfloatoPcPgRsPgR(void *p) {
      delete [] (static_cast<unordered_map<string,function<float()> >*>(p));
   }
   static void destruct_unordered_maplEstringcOfunctionlEfloatoPcPgRsPgR(void *p) {
      typedef unordered_map<string,function<float()> > current_t;
      (static_cast<current_t*>(p))->~current_t();
   }
} // end of namespace ROOT for class unordered_map<string,function<float()> >

namespace ROOT {
   static TClass *unordered_maplEstringcOfloatgR_Dictionary();
   static void unordered_maplEstringcOfloatgR_TClassManip(TClass*);
   static void *new_unordered_maplEstringcOfloatgR(void *p = nullptr);
   static void *newArray_unordered_maplEstringcOfloatgR(Long_t size, void *p);
   static void delete_unordered_maplEstringcOfloatgR(void *p);
   static void deleteArray_unordered_maplEstringcOfloatgR(void *p);
   static void destruct_unordered_maplEstringcOfloatgR(void *p);

   // Function generating the singleton type initializer
   static TGenericClassInfo *GenerateInitInstanceLocal(const unordered_map<string,float>*)
   {
      unordered_map<string,float> *ptr = nullptr;
      static ::TVirtualIsAProxy* isa_proxy = new ::TIsAProxy(typeid(unordered_map<string,float>));
      static ::ROOT::TGenericClassInfo 
         instance("unordered_map<string,float>", -2, "unordered_map", 109,
                  typeid(unordered_map<string,float>), ::ROOT::Internal::DefineBehavior(ptr, ptr),
                  &unordered_maplEstringcOfloatgR_Dictionary, isa_proxy, 0,
                  sizeof(unordered_map<string,float>) );
      instance.SetNew(&new_unordered_maplEstringcOfloatgR);
      instance.SetNewArray(&newArray_unordered_maplEstringcOfloatgR);
      instance.SetDelete(&delete_unordered_maplEstringcOfloatgR);
      instance.SetDeleteArray(&deleteArray_unordered_maplEstringcOfloatgR);
      instance.SetDestructor(&destruct_unordered_maplEstringcOfloatgR);
      instance.AdoptCollectionProxyInfo(TCollectionProxyInfo::Generate(TCollectionProxyInfo::MapInsert< unordered_map<string,float> >()));

      instance.AdoptAlternate(::ROOT::AddClassAlternate("unordered_map<string,float>","std::unordered_map<std::__cxx11::basic_string<char, std::char_traits<char>, std::allocator<char> >, float, std::hash<std::__cxx11::basic_string<char, std::char_traits<char>, std::allocator<char> > >, std::equal_to<std::__cxx11::basic_string<char, std::char_traits<char>, std::allocator<char> > >, std::allocator<std::pair<std::__cxx11::basic_string<char, std::char_traits<char>, std::allocator<char> > const, float> > >"));
      return &instance;
   }
   // Static variable to force the class initialization
   static ::ROOT::TGenericClassInfo *_R__UNIQUE_DICT_(Init) = GenerateInitInstanceLocal(static_cast<const unordered_map<string,float>*>(nullptr)); R__UseDummy(_R__UNIQUE_DICT_(Init));

   // Dictionary for non-ClassDef classes
   static TClass *unordered_maplEstringcOfloatgR_Dictionary() {
      TClass* theClass =::ROOT::GenerateInitInstanceLocal(static_cast<const unordered_map<string,float>*>(nullptr))->GetClass();
      unordered_maplEstringcOfloatgR_TClassManip(theClass);
   return theClass;
   }

   static void unordered_maplEstringcOfloatgR_TClassManip(TClass* ){
   }

} // end of namespace ROOT

namespace ROOT {
   // Wrappers around operator new
   static void *new_unordered_maplEstringcOfloatgR(void *p) {
      return  p ? ::new(static_cast<::ROOT::Internal::TOperatorNewHelper*>(p)) unordered_map<string,float> : new unordered_map<string,float>;
   }
   static void *newArray_unordered_maplEstringcOfloatgR(Long_t nElements, void *p) {
      return p ? ::new(static_cast<::ROOT::Internal::TOperatorNewHelper*>(p)) unordered_map<string,float>[nElements] : new unordered_map<string,float>[nElements];
   }
   // Wrapper around operator delete
   static void delete_unordered_maplEstringcOfloatgR(void *p) {
      delete (static_cast<unordered_map<string,float>*>(p));
   }
   static void deleteArray_unordered_maplEstringcOfloatgR(void *p) {
      delete [] (static_cast<unordered_map<string,float>*>(p));
   }
   static void destruct_unordered_maplEstringcOfloatgR(void *p) {
      typedef unordered_map<string,float> current_t;
      (static_cast<current_t*>(p))->~current_t();
   }
} // end of namespace ROOT for class unordered_map<string,float>

namespace ROOT {
   static TClass *unordered_maplEstringcOSystematicHelpercLcLCORRELATIONgR_Dictionary();
   static void unordered_maplEstringcOSystematicHelpercLcLCORRELATIONgR_TClassManip(TClass*);
   static void *new_unordered_maplEstringcOSystematicHelpercLcLCORRELATIONgR(void *p = nullptr);
   static void *newArray_unordered_maplEstringcOSystematicHelpercLcLCORRELATIONgR(Long_t size, void *p);
   static void delete_unordered_maplEstringcOSystematicHelpercLcLCORRELATIONgR(void *p);
   static void deleteArray_unordered_maplEstringcOSystematicHelpercLcLCORRELATIONgR(void *p);
   static void destruct_unordered_maplEstringcOSystematicHelpercLcLCORRELATIONgR(void *p);

   // Function generating the singleton type initializer
   static TGenericClassInfo *GenerateInitInstanceLocal(const unordered_map<string,SystematicHelper::CORRELATION>*)
   {
      unordered_map<string,SystematicHelper::CORRELATION> *ptr = nullptr;
      static ::TVirtualIsAProxy* isa_proxy = new ::TIsAProxy(typeid(unordered_map<string,SystematicHelper::CORRELATION>));
      static ::ROOT::TGenericClassInfo 
         instance("unordered_map<string,SystematicHelper::CORRELATION>", -2, "unordered_map", 109,
                  typeid(unordered_map<string,SystematicHelper::CORRELATION>), ::ROOT::Internal::DefineBehavior(ptr, ptr),
                  &unordered_maplEstringcOSystematicHelpercLcLCORRELATIONgR_Dictionary, isa_proxy, 0,
                  sizeof(unordered_map<string,SystematicHelper::CORRELATION>) );
      instance.SetNew(&new_unordered_maplEstringcOSystematicHelpercLcLCORRELATIONgR);
      instance.SetNewArray(&newArray_unordered_maplEstringcOSystematicHelpercLcLCORRELATIONgR);
      instance.SetDelete(&delete_unordered_maplEstringcOSystematicHelpercLcLCORRELATIONgR);
      instance.SetDeleteArray(&deleteArray_unordered_maplEstringcOSystematicHelpercLcLCORRELATIONgR);
      instance.SetDestructor(&destruct_unordered_maplEstringcOSystematicHelpercLcLCORRELATIONgR);
      instance.AdoptCollectionProxyInfo(TCollectionProxyInfo::Generate(TCollectionProxyInfo::MapInsert< unordered_map<string,SystematicHelper::CORRELATION> >()));

      instance.AdoptAlternate(::ROOT::AddClassAlternate("unordered_map<string,SystematicHelper::CORRELATION>","std::unordered_map<std::__cxx11::basic_string<char, std::char_traits<char>, std::allocator<char> >, SystematicHelper::CORRELATION, std::hash<std::__cxx11::basic_string<char, std::char_traits<char>, std::allocator<char> > >, std::equal_to<std::__cxx11::basic_string<char, std::char_traits<char>, std::allocator<char> > >, std::allocator<std::pair<std::__cxx11::basic_string<char, std::char_traits<char>, std::allocator<char> > const, SystematicHelper::CORRELATION> > >"));
      return &instance;
   }
   // Static variable to force the class initialization
   static ::ROOT::TGenericClassInfo *_R__UNIQUE_DICT_(Init) = GenerateInitInstanceLocal(static_cast<const unordered_map<string,SystematicHelper::CORRELATION>*>(nullptr)); R__UseDummy(_R__UNIQUE_DICT_(Init));

   // Dictionary for non-ClassDef classes
   static TClass *unordered_maplEstringcOSystematicHelpercLcLCORRELATIONgR_Dictionary() {
      TClass* theClass =::ROOT::GenerateInitInstanceLocal(static_cast<const unordered_map<string,SystematicHelper::CORRELATION>*>(nullptr))->GetClass();
      unordered_maplEstringcOSystematicHelpercLcLCORRELATIONgR_TClassManip(theClass);
   return theClass;
   }

   static void unordered_maplEstringcOSystematicHelpercLcLCORRELATIONgR_TClassManip(TClass* ){
   }

} // end of namespace ROOT

namespace ROOT {
   // Wrappers around operator new
   static void *new_unordered_maplEstringcOSystematicHelpercLcLCORRELATIONgR(void *p) {
      return  p ? ::new(static_cast<::ROOT::Internal::TOperatorNewHelper*>(p)) unordered_map<string,SystematicHelper::CORRELATION> : new unordered_map<string,SystematicHelper::CORRELATION>;
   }
   static void *newArray_unordered_maplEstringcOSystematicHelpercLcLCORRELATIONgR(Long_t nElements, void *p) {
      return p ? ::new(static_cast<::ROOT::Internal::TOperatorNewHelper*>(p)) unordered_map<string,SystematicHelper::CORRELATION>[nElements] : new unordered_map<string,SystematicHelper::CORRELATION>[nElements];
   }
   // Wrapper around operator delete
   static void delete_unordered_maplEstringcOSystematicHelpercLcLCORRELATIONgR(void *p) {
      delete (static_cast<unordered_map<string,SystematicHelper::CORRELATION>*>(p));
   }
   static void deleteArray_unordered_maplEstringcOSystematicHelpercLcLCORRELATIONgR(void *p) {
      delete [] (static_cast<unordered_map<string,SystematicHelper::CORRELATION>*>(p));
   }
   static void destruct_unordered_maplEstringcOSystematicHelpercLcLCORRELATIONgR(void *p) {
      typedef unordered_map<string,SystematicHelper::CORRELATION> current_t;
      (static_cast<current_t*>(p))->~current_t();
   }
} // end of namespace ROOT for class unordered_map<string,SystematicHelper::CORRELATION>

namespace ROOT {
   static TClass *unordered_maplEMyCorrectioncLcLvariationcOstringgR_Dictionary();
   static void unordered_maplEMyCorrectioncLcLvariationcOstringgR_TClassManip(TClass*);
   static void *new_unordered_maplEMyCorrectioncLcLvariationcOstringgR(void *p = nullptr);
   static void *newArray_unordered_maplEMyCorrectioncLcLvariationcOstringgR(Long_t size, void *p);
   static void delete_unordered_maplEMyCorrectioncLcLvariationcOstringgR(void *p);
   static void deleteArray_unordered_maplEMyCorrectioncLcLvariationcOstringgR(void *p);
   static void destruct_unordered_maplEMyCorrectioncLcLvariationcOstringgR(void *p);

   // Function generating the singleton type initializer
   static TGenericClassInfo *GenerateInitInstanceLocal(const unordered_map<MyCorrection::variation,string>*)
   {
      unordered_map<MyCorrection::variation,string> *ptr = nullptr;
      static ::TVirtualIsAProxy* isa_proxy = new ::TIsAProxy(typeid(unordered_map<MyCorrection::variation,string>));
      static ::ROOT::TGenericClassInfo 
         instance("unordered_map<MyCorrection::variation,string>", -2, "unordered_map", 109,
                  typeid(unordered_map<MyCorrection::variation,string>), ::ROOT::Internal::DefineBehavior(ptr, ptr),
                  &unordered_maplEMyCorrectioncLcLvariationcOstringgR_Dictionary, isa_proxy, 0,
                  sizeof(unordered_map<MyCorrection::variation,string>) );
      instance.SetNew(&new_unordered_maplEMyCorrectioncLcLvariationcOstringgR);
      instance.SetNewArray(&newArray_unordered_maplEMyCorrectioncLcLvariationcOstringgR);
      instance.SetDelete(&delete_unordered_maplEMyCorrectioncLcLvariationcOstringgR);
      instance.SetDeleteArray(&deleteArray_unordered_maplEMyCorrectioncLcLvariationcOstringgR);
      instance.SetDestructor(&destruct_unordered_maplEMyCorrectioncLcLvariationcOstringgR);
      instance.AdoptCollectionProxyInfo(TCollectionProxyInfo::Generate(TCollectionProxyInfo::MapInsert< unordered_map<MyCorrection::variation,string> >()));

      instance.AdoptAlternate(::ROOT::AddClassAlternate("unordered_map<MyCorrection::variation,string>","std::unordered_map<MyCorrection::variation, std::__cxx11::basic_string<char, std::char_traits<char>, std::allocator<char> >, std::hash<MyCorrection::variation>, std::equal_to<MyCorrection::variation>, std::allocator<std::pair<MyCorrection::variation const, std::__cxx11::basic_string<char, std::char_traits<char>, std::allocator<char> > > > >"));
      return &instance;
   }
   // Static variable to force the class initialization
   static ::ROOT::TGenericClassInfo *_R__UNIQUE_DICT_(Init) = GenerateInitInstanceLocal(static_cast<const unordered_map<MyCorrection::variation,string>*>(nullptr)); R__UseDummy(_R__UNIQUE_DICT_(Init));

   // Dictionary for non-ClassDef classes
   static TClass *unordered_maplEMyCorrectioncLcLvariationcOstringgR_Dictionary() {
      TClass* theClass =::ROOT::GenerateInitInstanceLocal(static_cast<const unordered_map<MyCorrection::variation,string>*>(nullptr))->GetClass();
      unordered_maplEMyCorrectioncLcLvariationcOstringgR_TClassManip(theClass);
   return theClass;
   }

   static void unordered_maplEMyCorrectioncLcLvariationcOstringgR_TClassManip(TClass* ){
   }

} // end of namespace ROOT

namespace ROOT {
   // Wrappers around operator new
   static void *new_unordered_maplEMyCorrectioncLcLvariationcOstringgR(void *p) {
      return  p ? ::new(static_cast<::ROOT::Internal::TOperatorNewHelper*>(p)) unordered_map<MyCorrection::variation,string> : new unordered_map<MyCorrection::variation,string>;
   }
   static void *newArray_unordered_maplEMyCorrectioncLcLvariationcOstringgR(Long_t nElements, void *p) {
      return p ? ::new(static_cast<::ROOT::Internal::TOperatorNewHelper*>(p)) unordered_map<MyCorrection::variation,string>[nElements] : new unordered_map<MyCorrection::variation,string>[nElements];
   }
   // Wrapper around operator delete
   static void delete_unordered_maplEMyCorrectioncLcLvariationcOstringgR(void *p) {
      delete (static_cast<unordered_map<MyCorrection::variation,string>*>(p));
   }
   static void deleteArray_unordered_maplEMyCorrectioncLcLvariationcOstringgR(void *p) {
      delete [] (static_cast<unordered_map<MyCorrection::variation,string>*>(p));
   }
   static void destruct_unordered_maplEMyCorrectioncLcLvariationcOstringgR(void *p) {
      typedef unordered_map<MyCorrection::variation,string> current_t;
      (static_cast<current_t*>(p))->~current_t();
   }
} // end of namespace ROOT for class unordered_map<MyCorrection::variation,string>

namespace ROOT {
   static TClass *ROOTcLcLVecOpscLcLRVeclELHAPDFcLcLPDFmUgR_Dictionary();
   static void ROOTcLcLVecOpscLcLRVeclELHAPDFcLcLPDFmUgR_TClassManip(TClass*);
   static void *new_ROOTcLcLVecOpscLcLRVeclELHAPDFcLcLPDFmUgR(void *p = nullptr);
   static void *newArray_ROOTcLcLVecOpscLcLRVeclELHAPDFcLcLPDFmUgR(Long_t size, void *p);
   static void delete_ROOTcLcLVecOpscLcLRVeclELHAPDFcLcLPDFmUgR(void *p);
   static void deleteArray_ROOTcLcLVecOpscLcLRVeclELHAPDFcLcLPDFmUgR(void *p);
   static void destruct_ROOTcLcLVecOpscLcLRVeclELHAPDFcLcLPDFmUgR(void *p);

   // Function generating the singleton type initializer
   static TGenericClassInfo *GenerateInitInstanceLocal(const ROOT::VecOps::RVec<LHAPDF::PDF*>*)
   {
      ROOT::VecOps::RVec<LHAPDF::PDF*> *ptr = nullptr;
      static ::TVirtualIsAProxy* isa_proxy = new ::TIsAProxy(typeid(ROOT::VecOps::RVec<LHAPDF::PDF*>));
      static ::ROOT::TGenericClassInfo 
         instance("ROOT::VecOps::RVec<LHAPDF::PDF*>", -2, "ROOT/RVec.hxx", 1529,
                  typeid(ROOT::VecOps::RVec<LHAPDF::PDF*>), ::ROOT::Internal::DefineBehavior(ptr, ptr),
                  &ROOTcLcLVecOpscLcLRVeclELHAPDFcLcLPDFmUgR_Dictionary, isa_proxy, 0,
                  sizeof(ROOT::VecOps::RVec<LHAPDF::PDF*>) );
      instance.SetNew(&new_ROOTcLcLVecOpscLcLRVeclELHAPDFcLcLPDFmUgR);
      instance.SetNewArray(&newArray_ROOTcLcLVecOpscLcLRVeclELHAPDFcLcLPDFmUgR);
      instance.SetDelete(&delete_ROOTcLcLVecOpscLcLRVeclELHAPDFcLcLPDFmUgR);
      instance.SetDeleteArray(&deleteArray_ROOTcLcLVecOpscLcLRVeclELHAPDFcLcLPDFmUgR);
      instance.SetDestructor(&destruct_ROOTcLcLVecOpscLcLRVeclELHAPDFcLcLPDFmUgR);
      instance.AdoptCollectionProxyInfo(TCollectionProxyInfo::Generate(TCollectionProxyInfo::Pushback< ROOT::VecOps::RVec<LHAPDF::PDF*> >()));
      return &instance;
   }
   TGenericClassInfo *GenerateInitInstance(const ROOT::VecOps::RVec<LHAPDF::PDF*>*)
   {
      return GenerateInitInstanceLocal(static_cast<ROOT::VecOps::RVec<LHAPDF::PDF*>*>(nullptr));
   }
   // Static variable to force the class initialization
   static ::ROOT::TGenericClassInfo *_R__UNIQUE_DICT_(Init) = GenerateInitInstanceLocal(static_cast<const ROOT::VecOps::RVec<LHAPDF::PDF*>*>(nullptr)); R__UseDummy(_R__UNIQUE_DICT_(Init));

   // Dictionary for non-ClassDef classes
   static TClass *ROOTcLcLVecOpscLcLRVeclELHAPDFcLcLPDFmUgR_Dictionary() {
      TClass* theClass =::ROOT::GenerateInitInstanceLocal(static_cast<const ROOT::VecOps::RVec<LHAPDF::PDF*>*>(nullptr))->GetClass();
      ROOTcLcLVecOpscLcLRVeclELHAPDFcLcLPDFmUgR_TClassManip(theClass);
   return theClass;
   }

   static void ROOTcLcLVecOpscLcLRVeclELHAPDFcLcLPDFmUgR_TClassManip(TClass* ){
   }

} // end of namespace ROOT

namespace ROOT {
   // Wrappers around operator new
   static void *new_ROOTcLcLVecOpscLcLRVeclELHAPDFcLcLPDFmUgR(void *p) {
      return  p ? ::new(static_cast<::ROOT::Internal::TOperatorNewHelper*>(p)) ROOT::VecOps::RVec<LHAPDF::PDF*> : new ROOT::VecOps::RVec<LHAPDF::PDF*>;
   }
   static void *newArray_ROOTcLcLVecOpscLcLRVeclELHAPDFcLcLPDFmUgR(Long_t nElements, void *p) {
      return p ? ::new(static_cast<::ROOT::Internal::TOperatorNewHelper*>(p)) ROOT::VecOps::RVec<LHAPDF::PDF*>[nElements] : new ROOT::VecOps::RVec<LHAPDF::PDF*>[nElements];
   }
   // Wrapper around operator delete
   static void delete_ROOTcLcLVecOpscLcLRVeclELHAPDFcLcLPDFmUgR(void *p) {
      delete (static_cast<ROOT::VecOps::RVec<LHAPDF::PDF*>*>(p));
   }
   static void deleteArray_ROOTcLcLVecOpscLcLRVeclELHAPDFcLcLPDFmUgR(void *p) {
      delete [] (static_cast<ROOT::VecOps::RVec<LHAPDF::PDF*>*>(p));
   }
   static void destruct_ROOTcLcLVecOpscLcLRVeclELHAPDFcLcLPDFmUgR(void *p) {
      typedef ROOT::VecOps::RVec<LHAPDF::PDF*> current_t;
      (static_cast<current_t*>(p))->~current_t();
   }
} // end of namespace ROOT for class ROOT::VecOps::RVec<LHAPDF::PDF*>

namespace {
  void TriggerDictionaryInitialization_libAnalyzerTools_Impl() {
    static const char* headers[] = {
"include/AnalyzerParameter.h",
"include/ExampleParameter.h",
"include/LHAPDFHandler.h",
"include/MLHelper.h",
"include/MyCorrection.h",
"include/NonpromptParameter.h",
"include/PDFReweight.h",
"include/PhysicalConstants.h",
"include/SystematicHelper.h",
nullptr
    };
    static const char* includePaths[] = {
"/data6/Users/achihwan/miniforge3/envs/Nano/include",
"/home/achihwan/SKNanoAnalyzer/external/lhapdf/redhat/include",
"/home/achihwan/SKNanoAnalyzer/AnalyzerTools",
"/home/achihwan/SKNanoAnalyzer/AnalyzerTools/include",
"/home/achihwan/SKNanoAnalyzer/AnalyzerTools/../DataFormats/include",
"/data6/Users/achihwan/miniforge3/envs/Nano/lib/python3.12/site-packages/correctionlib/include",
"/home/achihwan/SKNanoAnalyzer/external/RoccoR",
"/data6/Users/achihwan/miniforge3/envs/Nano/include",
"/home/achihwan/SKNanoAnalyzer/AnalyzerTools/include",
"/home/achihwan/SKNanoAnalyzer/AnalyzerTools/../DataFormats/include",
"/home/achihwan/SKNanoAnalyzer/external/lhapdf/redhat/include",
"/data6/Users/achihwan/miniforge3/envs/Nano/lib/python3.12/site-packages/correctionlib/include",
"/home/achihwan/SKNanoAnalyzer/external/RoccoR",
"/home/achihwan/SKNanoAnalyzer/_deps/yaml-cpp-src/include",
"/data6/Users/achihwan/miniforge3/envs/Nano/include/onnxruntime/core/session",
"/data6/Users/achihwan/miniforge3/envs/Nano/include/",
"/data6/Users/achihwan/SKNanoAnalyzer/AnalyzerTools/",
nullptr
    };
    static const char* fwdDeclCode = R"DICTFWDDCLS(
#line 1 "libAnalyzerTools dictionary forward declarations' payload"
#pragma clang diagnostic ignored "-Wkeyword-compat"
#pragma clang diagnostic ignored "-Wignored-attributes"
#pragma clang diagnostic ignored "-Wreturn-type-c-linkage"
extern int __Cling_AutoLoading_Map;
class __attribute__((annotate("$clingAutoload$include/AnalyzerParameter.h")))  AnalyzerParameter;
class __attribute__((annotate("$clingAutoload$include/ExampleParameter.h")))  ExampleParameter;
class __attribute__((annotate("$clingAutoload$include/LHAPDFHandler.h")))  LHAPDFHandler;
class __attribute__((annotate("$clingAutoload$include/MyCorrection.h")))  MyCorrection;
class __attribute__((annotate("$clingAutoload$include/NonpromptParameter.h")))  NonpromptParameter;
class __attribute__((annotate("$clingAutoload$include/PDFReweight.h")))  PDFReweight;
class __attribute__((annotate("$clingAutoload$include/SystematicHelper.h")))  SystematicHelper;
)DICTFWDDCLS";
    static const char* payloadCode = R"DICTPAYLOAD(
#line 1 "libAnalyzerTools dictionary payload"

#ifndef __ROOFIT_NOBANNER
  #define __ROOFIT_NOBANNER 1
#endif
#ifndef __ROOFIT_NOBANNER
  #define __ROOFIT_NOBANNER 1
#endif
#ifndef YAML_CPP_STATIC_DEFINE
  #define YAML_CPP_STATIC_DEFINE 1
#endif

#define _BACKWARD_BACKWARD_WARNING_H
// Inline headers
#include "include/AnalyzerParameter.h"
#include "include/ExampleParameter.h"
#include "include/LHAPDFHandler.h"
#include "include/MLHelper.h"
#include "include/MyCorrection.h"
#include "include/NonpromptParameter.h"
#include "include/PDFReweight.h"
#include "include/PhysicalConstants.h"
#include "include/SystematicHelper.h"

#undef  _BACKWARD_BACKWARD_WARNING_H
)DICTPAYLOAD";
    static const char* classesHeaders[] = {
"AnalyzerParameter", payloadCode, "@",
"ExampleParameter", payloadCode, "@",
"LHAPDFHandler", payloadCode, "@",
"MyCorrection", payloadCode, "@",
"NonpromptParameter", payloadCode, "@",
"PDFReweight", payloadCode, "@",
"SystematicHelper", payloadCode, "@",
nullptr
};
    static bool isInitialized = false;
    if (!isInitialized) {
      TROOT::RegisterModule("libAnalyzerTools",
        headers, includePaths, payloadCode, fwdDeclCode,
        TriggerDictionaryInitialization_libAnalyzerTools_Impl, {}, classesHeaders, /*hasCxxModule*/false);
      isInitialized = true;
    }
  }
  static struct DictInit {
    DictInit() {
      TriggerDictionaryInitialization_libAnalyzerTools_Impl();
    }
  } __TheDictionaryInitializer;
}
void TriggerDictionaryInitialization_libAnalyzerTools() {
  TriggerDictionaryInitialization_libAnalyzerTools_Impl();
}

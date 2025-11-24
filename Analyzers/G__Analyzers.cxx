// Do NOT change. Changes will be lost next time file is generated

#define R__DICTIONARY_FILENAME G__Analyzers
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
#include "include/AnalyzerCore.h"
#include "include/DY.h"
#include "include/DiLepton.h"
#include "include/DiLeptonBase.h"
#include "include/FullHadronicTriggerTnP.h"
#include "include/GeneratorBase.h"
#include "include/GetEffLumi.h"
#include "include/LRSM_TBChannel.h"
#include "include/LRSM_TBChannel_notusingbjet.h"
#include "include/LRSM_TBChannel_wtagging.h"
#include "include/MeasTrigEff.h"
#include "include/MeasureJetTaggingEff.h"
#include "include/MuonTnPProducer.h"
#include "include/ParseEleIDVariables.h"
#include "include/ParseMuIDVariables.h"
#include "include/Reproduce20_002.h"
#include "include/SKNanoLoader.h"
#include "include/TTbar_test.h"
#include "include/TestGetLeptonType.h"
#include "include/ttbar_hadronic.h"

// Header files passed via #pragma extra_include

// The generated code does not explicitly qualify STL entities
namespace std {} using namespace std;

namespace ROOT {
   static TClass *SKNanoLoader_Dictionary();
   static void SKNanoLoader_TClassManip(TClass*);
   static void *new_SKNanoLoader(void *p = nullptr);
   static void *newArray_SKNanoLoader(Long_t size, void *p);
   static void delete_SKNanoLoader(void *p);
   static void deleteArray_SKNanoLoader(void *p);
   static void destruct_SKNanoLoader(void *p);

   // Function generating the singleton type initializer
   static TGenericClassInfo *GenerateInitInstanceLocal(const ::SKNanoLoader*)
   {
      ::SKNanoLoader *ptr = nullptr;
      static ::TVirtualIsAProxy* isa_proxy = new ::TIsAProxy(typeid(::SKNanoLoader));
      static ::ROOT::TGenericClassInfo 
         instance("SKNanoLoader", "SKNanoLoader.h", 21,
                  typeid(::SKNanoLoader), ::ROOT::Internal::DefineBehavior(ptr, ptr),
                  &SKNanoLoader_Dictionary, isa_proxy, 4,
                  sizeof(::SKNanoLoader) );
      instance.SetNew(&new_SKNanoLoader);
      instance.SetNewArray(&newArray_SKNanoLoader);
      instance.SetDelete(&delete_SKNanoLoader);
      instance.SetDeleteArray(&deleteArray_SKNanoLoader);
      instance.SetDestructor(&destruct_SKNanoLoader);
      return &instance;
   }
   TGenericClassInfo *GenerateInitInstance(const ::SKNanoLoader*)
   {
      return GenerateInitInstanceLocal(static_cast<::SKNanoLoader*>(nullptr));
   }
   // Static variable to force the class initialization
   static ::ROOT::TGenericClassInfo *_R__UNIQUE_DICT_(Init) = GenerateInitInstanceLocal(static_cast<const ::SKNanoLoader*>(nullptr)); R__UseDummy(_R__UNIQUE_DICT_(Init));

   // Dictionary for non-ClassDef classes
   static TClass *SKNanoLoader_Dictionary() {
      TClass* theClass =::ROOT::GenerateInitInstanceLocal(static_cast<const ::SKNanoLoader*>(nullptr))->GetClass();
      SKNanoLoader_TClassManip(theClass);
   return theClass;
   }

   static void SKNanoLoader_TClassManip(TClass* ){
   }

} // end of namespace ROOT

namespace ROOT {
   static TClass *AnalyzerCore_Dictionary();
   static void AnalyzerCore_TClassManip(TClass*);
   static void *new_AnalyzerCore(void *p = nullptr);
   static void *newArray_AnalyzerCore(Long_t size, void *p);
   static void delete_AnalyzerCore(void *p);
   static void deleteArray_AnalyzerCore(void *p);
   static void destruct_AnalyzerCore(void *p);

   // Function generating the singleton type initializer
   static TGenericClassInfo *GenerateInitInstanceLocal(const ::AnalyzerCore*)
   {
      ::AnalyzerCore *ptr = nullptr;
      static ::TVirtualIsAProxy* isa_proxy = new ::TIsAProxy(typeid(::AnalyzerCore));
      static ::ROOT::TGenericClassInfo 
         instance("AnalyzerCore", "AnalyzerCore.h", 62,
                  typeid(::AnalyzerCore), ::ROOT::Internal::DefineBehavior(ptr, ptr),
                  &AnalyzerCore_Dictionary, isa_proxy, 4,
                  sizeof(::AnalyzerCore) );
      instance.SetNew(&new_AnalyzerCore);
      instance.SetNewArray(&newArray_AnalyzerCore);
      instance.SetDelete(&delete_AnalyzerCore);
      instance.SetDeleteArray(&deleteArray_AnalyzerCore);
      instance.SetDestructor(&destruct_AnalyzerCore);
      return &instance;
   }
   TGenericClassInfo *GenerateInitInstance(const ::AnalyzerCore*)
   {
      return GenerateInitInstanceLocal(static_cast<::AnalyzerCore*>(nullptr));
   }
   // Static variable to force the class initialization
   static ::ROOT::TGenericClassInfo *_R__UNIQUE_DICT_(Init) = GenerateInitInstanceLocal(static_cast<const ::AnalyzerCore*>(nullptr)); R__UseDummy(_R__UNIQUE_DICT_(Init));

   // Dictionary for non-ClassDef classes
   static TClass *AnalyzerCore_Dictionary() {
      TClass* theClass =::ROOT::GenerateInitInstanceLocal(static_cast<const ::AnalyzerCore*>(nullptr))->GetClass();
      AnalyzerCore_TClassManip(theClass);
   return theClass;
   }

   static void AnalyzerCore_TClassManip(TClass* ){
   }

} // end of namespace ROOT

namespace ROOT {
   static TClass *DY_Dictionary();
   static void DY_TClassManip(TClass*);
   static void *new_DY(void *p = nullptr);
   static void *newArray_DY(Long_t size, void *p);
   static void delete_DY(void *p);
   static void deleteArray_DY(void *p);
   static void destruct_DY(void *p);

   // Function generating the singleton type initializer
   static TGenericClassInfo *GenerateInitInstanceLocal(const ::DY*)
   {
      ::DY *ptr = nullptr;
      static ::TVirtualIsAProxy* isa_proxy = new ::TIsAProxy(typeid(::DY));
      static ::ROOT::TGenericClassInfo 
         instance("DY", "DY.h", 7,
                  typeid(::DY), ::ROOT::Internal::DefineBehavior(ptr, ptr),
                  &DY_Dictionary, isa_proxy, 4,
                  sizeof(::DY) );
      instance.SetNew(&new_DY);
      instance.SetNewArray(&newArray_DY);
      instance.SetDelete(&delete_DY);
      instance.SetDeleteArray(&deleteArray_DY);
      instance.SetDestructor(&destruct_DY);
      return &instance;
   }
   TGenericClassInfo *GenerateInitInstance(const ::DY*)
   {
      return GenerateInitInstanceLocal(static_cast<::DY*>(nullptr));
   }
   // Static variable to force the class initialization
   static ::ROOT::TGenericClassInfo *_R__UNIQUE_DICT_(Init) = GenerateInitInstanceLocal(static_cast<const ::DY*>(nullptr)); R__UseDummy(_R__UNIQUE_DICT_(Init));

   // Dictionary for non-ClassDef classes
   static TClass *DY_Dictionary() {
      TClass* theClass =::ROOT::GenerateInitInstanceLocal(static_cast<const ::DY*>(nullptr))->GetClass();
      DY_TClassManip(theClass);
   return theClass;
   }

   static void DY_TClassManip(TClass* ){
   }

} // end of namespace ROOT

namespace ROOT {
   static TClass *DiLeptonBase_Dictionary();
   static void DiLeptonBase_TClassManip(TClass*);
   static void *new_DiLeptonBase(void *p = nullptr);
   static void *newArray_DiLeptonBase(Long_t size, void *p);
   static void delete_DiLeptonBase(void *p);
   static void deleteArray_DiLeptonBase(void *p);
   static void destruct_DiLeptonBase(void *p);

   // Function generating the singleton type initializer
   static TGenericClassInfo *GenerateInitInstanceLocal(const ::DiLeptonBase*)
   {
      ::DiLeptonBase *ptr = nullptr;
      static ::TVirtualIsAProxy* isa_proxy = new ::TIsAProxy(typeid(::DiLeptonBase));
      static ::ROOT::TGenericClassInfo 
         instance("DiLeptonBase", "DiLeptonBase.h", 7,
                  typeid(::DiLeptonBase), ::ROOT::Internal::DefineBehavior(ptr, ptr),
                  &DiLeptonBase_Dictionary, isa_proxy, 4,
                  sizeof(::DiLeptonBase) );
      instance.SetNew(&new_DiLeptonBase);
      instance.SetNewArray(&newArray_DiLeptonBase);
      instance.SetDelete(&delete_DiLeptonBase);
      instance.SetDeleteArray(&deleteArray_DiLeptonBase);
      instance.SetDestructor(&destruct_DiLeptonBase);
      return &instance;
   }
   TGenericClassInfo *GenerateInitInstance(const ::DiLeptonBase*)
   {
      return GenerateInitInstanceLocal(static_cast<::DiLeptonBase*>(nullptr));
   }
   // Static variable to force the class initialization
   static ::ROOT::TGenericClassInfo *_R__UNIQUE_DICT_(Init) = GenerateInitInstanceLocal(static_cast<const ::DiLeptonBase*>(nullptr)); R__UseDummy(_R__UNIQUE_DICT_(Init));

   // Dictionary for non-ClassDef classes
   static TClass *DiLeptonBase_Dictionary() {
      TClass* theClass =::ROOT::GenerateInitInstanceLocal(static_cast<const ::DiLeptonBase*>(nullptr))->GetClass();
      DiLeptonBase_TClassManip(theClass);
   return theClass;
   }

   static void DiLeptonBase_TClassManip(TClass* ){
   }

} // end of namespace ROOT

namespace ROOT {
   static TClass *FullHadronicTriggerTnP_Dictionary();
   static void FullHadronicTriggerTnP_TClassManip(TClass*);
   static void *new_FullHadronicTriggerTnP(void *p = nullptr);
   static void *newArray_FullHadronicTriggerTnP(Long_t size, void *p);
   static void delete_FullHadronicTriggerTnP(void *p);
   static void deleteArray_FullHadronicTriggerTnP(void *p);
   static void destruct_FullHadronicTriggerTnP(void *p);

   // Function generating the singleton type initializer
   static TGenericClassInfo *GenerateInitInstanceLocal(const ::FullHadronicTriggerTnP*)
   {
      ::FullHadronicTriggerTnP *ptr = nullptr;
      static ::TVirtualIsAProxy* isa_proxy = new ::TIsAProxy(typeid(::FullHadronicTriggerTnP));
      static ::ROOT::TGenericClassInfo 
         instance("FullHadronicTriggerTnP", "FullHadronicTriggerTnP.h", 16,
                  typeid(::FullHadronicTriggerTnP), ::ROOT::Internal::DefineBehavior(ptr, ptr),
                  &FullHadronicTriggerTnP_Dictionary, isa_proxy, 4,
                  sizeof(::FullHadronicTriggerTnP) );
      instance.SetNew(&new_FullHadronicTriggerTnP);
      instance.SetNewArray(&newArray_FullHadronicTriggerTnP);
      instance.SetDelete(&delete_FullHadronicTriggerTnP);
      instance.SetDeleteArray(&deleteArray_FullHadronicTriggerTnP);
      instance.SetDestructor(&destruct_FullHadronicTriggerTnP);
      return &instance;
   }
   TGenericClassInfo *GenerateInitInstance(const ::FullHadronicTriggerTnP*)
   {
      return GenerateInitInstanceLocal(static_cast<::FullHadronicTriggerTnP*>(nullptr));
   }
   // Static variable to force the class initialization
   static ::ROOT::TGenericClassInfo *_R__UNIQUE_DICT_(Init) = GenerateInitInstanceLocal(static_cast<const ::FullHadronicTriggerTnP*>(nullptr)); R__UseDummy(_R__UNIQUE_DICT_(Init));

   // Dictionary for non-ClassDef classes
   static TClass *FullHadronicTriggerTnP_Dictionary() {
      TClass* theClass =::ROOT::GenerateInitInstanceLocal(static_cast<const ::FullHadronicTriggerTnP*>(nullptr))->GetClass();
      FullHadronicTriggerTnP_TClassManip(theClass);
   return theClass;
   }

   static void FullHadronicTriggerTnP_TClassManip(TClass* ){
   }

} // end of namespace ROOT

namespace ROOT {
   static TClass *GeneratorBase_Dictionary();
   static void GeneratorBase_TClassManip(TClass*);
   static void *new_GeneratorBase(void *p = nullptr);
   static void *newArray_GeneratorBase(Long_t size, void *p);
   static void delete_GeneratorBase(void *p);
   static void deleteArray_GeneratorBase(void *p);
   static void destruct_GeneratorBase(void *p);

   // Function generating the singleton type initializer
   static TGenericClassInfo *GenerateInitInstanceLocal(const ::GeneratorBase*)
   {
      ::GeneratorBase *ptr = nullptr;
      static ::TVirtualIsAProxy* isa_proxy = new ::TIsAProxy(typeid(::GeneratorBase));
      static ::ROOT::TGenericClassInfo 
         instance("GeneratorBase", "GeneratorBase.h", 6,
                  typeid(::GeneratorBase), ::ROOT::Internal::DefineBehavior(ptr, ptr),
                  &GeneratorBase_Dictionary, isa_proxy, 4,
                  sizeof(::GeneratorBase) );
      instance.SetNew(&new_GeneratorBase);
      instance.SetNewArray(&newArray_GeneratorBase);
      instance.SetDelete(&delete_GeneratorBase);
      instance.SetDeleteArray(&deleteArray_GeneratorBase);
      instance.SetDestructor(&destruct_GeneratorBase);
      return &instance;
   }
   TGenericClassInfo *GenerateInitInstance(const ::GeneratorBase*)
   {
      return GenerateInitInstanceLocal(static_cast<::GeneratorBase*>(nullptr));
   }
   // Static variable to force the class initialization
   static ::ROOT::TGenericClassInfo *_R__UNIQUE_DICT_(Init) = GenerateInitInstanceLocal(static_cast<const ::GeneratorBase*>(nullptr)); R__UseDummy(_R__UNIQUE_DICT_(Init));

   // Dictionary for non-ClassDef classes
   static TClass *GeneratorBase_Dictionary() {
      TClass* theClass =::ROOT::GenerateInitInstanceLocal(static_cast<const ::GeneratorBase*>(nullptr))->GetClass();
      GeneratorBase_TClassManip(theClass);
   return theClass;
   }

   static void GeneratorBase_TClassManip(TClass* ){
   }

} // end of namespace ROOT

namespace ROOT {
   static TClass *GetEffLumi_Dictionary();
   static void GetEffLumi_TClassManip(TClass*);
   static void *new_GetEffLumi(void *p = nullptr);
   static void *newArray_GetEffLumi(Long_t size, void *p);
   static void delete_GetEffLumi(void *p);
   static void deleteArray_GetEffLumi(void *p);
   static void destruct_GetEffLumi(void *p);

   // Function generating the singleton type initializer
   static TGenericClassInfo *GenerateInitInstanceLocal(const ::GetEffLumi*)
   {
      ::GetEffLumi *ptr = nullptr;
      static ::TVirtualIsAProxy* isa_proxy = new ::TIsAProxy(typeid(::GetEffLumi));
      static ::ROOT::TGenericClassInfo 
         instance("GetEffLumi", "GetEffLumi.h", 8,
                  typeid(::GetEffLumi), ::ROOT::Internal::DefineBehavior(ptr, ptr),
                  &GetEffLumi_Dictionary, isa_proxy, 4,
                  sizeof(::GetEffLumi) );
      instance.SetNew(&new_GetEffLumi);
      instance.SetNewArray(&newArray_GetEffLumi);
      instance.SetDelete(&delete_GetEffLumi);
      instance.SetDeleteArray(&deleteArray_GetEffLumi);
      instance.SetDestructor(&destruct_GetEffLumi);
      return &instance;
   }
   TGenericClassInfo *GenerateInitInstance(const ::GetEffLumi*)
   {
      return GenerateInitInstanceLocal(static_cast<::GetEffLumi*>(nullptr));
   }
   // Static variable to force the class initialization
   static ::ROOT::TGenericClassInfo *_R__UNIQUE_DICT_(Init) = GenerateInitInstanceLocal(static_cast<const ::GetEffLumi*>(nullptr)); R__UseDummy(_R__UNIQUE_DICT_(Init));

   // Dictionary for non-ClassDef classes
   static TClass *GetEffLumi_Dictionary() {
      TClass* theClass =::ROOT::GenerateInitInstanceLocal(static_cast<const ::GetEffLumi*>(nullptr))->GetClass();
      GetEffLumi_TClassManip(theClass);
   return theClass;
   }

   static void GetEffLumi_TClassManip(TClass* ){
   }

} // end of namespace ROOT

namespace ROOT {
   static TClass *LRSM_TBChannel_Dictionary();
   static void LRSM_TBChannel_TClassManip(TClass*);
   static void *new_LRSM_TBChannel(void *p = nullptr);
   static void *newArray_LRSM_TBChannel(Long_t size, void *p);
   static void delete_LRSM_TBChannel(void *p);
   static void deleteArray_LRSM_TBChannel(void *p);
   static void destruct_LRSM_TBChannel(void *p);

   // Function generating the singleton type initializer
   static TGenericClassInfo *GenerateInitInstanceLocal(const ::LRSM_TBChannel*)
   {
      ::LRSM_TBChannel *ptr = nullptr;
      static ::TVirtualIsAProxy* isa_proxy = new ::TIsAProxy(typeid(::LRSM_TBChannel));
      static ::ROOT::TGenericClassInfo 
         instance("LRSM_TBChannel", "LRSM_TBChannel.h", 7,
                  typeid(::LRSM_TBChannel), ::ROOT::Internal::DefineBehavior(ptr, ptr),
                  &LRSM_TBChannel_Dictionary, isa_proxy, 4,
                  sizeof(::LRSM_TBChannel) );
      instance.SetNew(&new_LRSM_TBChannel);
      instance.SetNewArray(&newArray_LRSM_TBChannel);
      instance.SetDelete(&delete_LRSM_TBChannel);
      instance.SetDeleteArray(&deleteArray_LRSM_TBChannel);
      instance.SetDestructor(&destruct_LRSM_TBChannel);
      return &instance;
   }
   TGenericClassInfo *GenerateInitInstance(const ::LRSM_TBChannel*)
   {
      return GenerateInitInstanceLocal(static_cast<::LRSM_TBChannel*>(nullptr));
   }
   // Static variable to force the class initialization
   static ::ROOT::TGenericClassInfo *_R__UNIQUE_DICT_(Init) = GenerateInitInstanceLocal(static_cast<const ::LRSM_TBChannel*>(nullptr)); R__UseDummy(_R__UNIQUE_DICT_(Init));

   // Dictionary for non-ClassDef classes
   static TClass *LRSM_TBChannel_Dictionary() {
      TClass* theClass =::ROOT::GenerateInitInstanceLocal(static_cast<const ::LRSM_TBChannel*>(nullptr))->GetClass();
      LRSM_TBChannel_TClassManip(theClass);
   return theClass;
   }

   static void LRSM_TBChannel_TClassManip(TClass* ){
   }

} // end of namespace ROOT

namespace ROOT {
   static TClass *LRSM_TBChannel_notusingbjet_Dictionary();
   static void LRSM_TBChannel_notusingbjet_TClassManip(TClass*);
   static void *new_LRSM_TBChannel_notusingbjet(void *p = nullptr);
   static void *newArray_LRSM_TBChannel_notusingbjet(Long_t size, void *p);
   static void delete_LRSM_TBChannel_notusingbjet(void *p);
   static void deleteArray_LRSM_TBChannel_notusingbjet(void *p);
   static void destruct_LRSM_TBChannel_notusingbjet(void *p);

   // Function generating the singleton type initializer
   static TGenericClassInfo *GenerateInitInstanceLocal(const ::LRSM_TBChannel_notusingbjet*)
   {
      ::LRSM_TBChannel_notusingbjet *ptr = nullptr;
      static ::TVirtualIsAProxy* isa_proxy = new ::TIsAProxy(typeid(::LRSM_TBChannel_notusingbjet));
      static ::ROOT::TGenericClassInfo 
         instance("LRSM_TBChannel_notusingbjet", "LRSM_TBChannel_notusingbjet.h", 7,
                  typeid(::LRSM_TBChannel_notusingbjet), ::ROOT::Internal::DefineBehavior(ptr, ptr),
                  &LRSM_TBChannel_notusingbjet_Dictionary, isa_proxy, 4,
                  sizeof(::LRSM_TBChannel_notusingbjet) );
      instance.SetNew(&new_LRSM_TBChannel_notusingbjet);
      instance.SetNewArray(&newArray_LRSM_TBChannel_notusingbjet);
      instance.SetDelete(&delete_LRSM_TBChannel_notusingbjet);
      instance.SetDeleteArray(&deleteArray_LRSM_TBChannel_notusingbjet);
      instance.SetDestructor(&destruct_LRSM_TBChannel_notusingbjet);
      return &instance;
   }
   TGenericClassInfo *GenerateInitInstance(const ::LRSM_TBChannel_notusingbjet*)
   {
      return GenerateInitInstanceLocal(static_cast<::LRSM_TBChannel_notusingbjet*>(nullptr));
   }
   // Static variable to force the class initialization
   static ::ROOT::TGenericClassInfo *_R__UNIQUE_DICT_(Init) = GenerateInitInstanceLocal(static_cast<const ::LRSM_TBChannel_notusingbjet*>(nullptr)); R__UseDummy(_R__UNIQUE_DICT_(Init));

   // Dictionary for non-ClassDef classes
   static TClass *LRSM_TBChannel_notusingbjet_Dictionary() {
      TClass* theClass =::ROOT::GenerateInitInstanceLocal(static_cast<const ::LRSM_TBChannel_notusingbjet*>(nullptr))->GetClass();
      LRSM_TBChannel_notusingbjet_TClassManip(theClass);
   return theClass;
   }

   static void LRSM_TBChannel_notusingbjet_TClassManip(TClass* ){
   }

} // end of namespace ROOT

namespace ROOT {
   static TClass *LRSM_TBChannel_wtagging_Dictionary();
   static void LRSM_TBChannel_wtagging_TClassManip(TClass*);
   static void *new_LRSM_TBChannel_wtagging(void *p = nullptr);
   static void *newArray_LRSM_TBChannel_wtagging(Long_t size, void *p);
   static void delete_LRSM_TBChannel_wtagging(void *p);
   static void deleteArray_LRSM_TBChannel_wtagging(void *p);
   static void destruct_LRSM_TBChannel_wtagging(void *p);

   // Function generating the singleton type initializer
   static TGenericClassInfo *GenerateInitInstanceLocal(const ::LRSM_TBChannel_wtagging*)
   {
      ::LRSM_TBChannel_wtagging *ptr = nullptr;
      static ::TVirtualIsAProxy* isa_proxy = new ::TIsAProxy(typeid(::LRSM_TBChannel_wtagging));
      static ::ROOT::TGenericClassInfo 
         instance("LRSM_TBChannel_wtagging", "LRSM_TBChannel_wtagging.h", 7,
                  typeid(::LRSM_TBChannel_wtagging), ::ROOT::Internal::DefineBehavior(ptr, ptr),
                  &LRSM_TBChannel_wtagging_Dictionary, isa_proxy, 4,
                  sizeof(::LRSM_TBChannel_wtagging) );
      instance.SetNew(&new_LRSM_TBChannel_wtagging);
      instance.SetNewArray(&newArray_LRSM_TBChannel_wtagging);
      instance.SetDelete(&delete_LRSM_TBChannel_wtagging);
      instance.SetDeleteArray(&deleteArray_LRSM_TBChannel_wtagging);
      instance.SetDestructor(&destruct_LRSM_TBChannel_wtagging);
      return &instance;
   }
   TGenericClassInfo *GenerateInitInstance(const ::LRSM_TBChannel_wtagging*)
   {
      return GenerateInitInstanceLocal(static_cast<::LRSM_TBChannel_wtagging*>(nullptr));
   }
   // Static variable to force the class initialization
   static ::ROOT::TGenericClassInfo *_R__UNIQUE_DICT_(Init) = GenerateInitInstanceLocal(static_cast<const ::LRSM_TBChannel_wtagging*>(nullptr)); R__UseDummy(_R__UNIQUE_DICT_(Init));

   // Dictionary for non-ClassDef classes
   static TClass *LRSM_TBChannel_wtagging_Dictionary() {
      TClass* theClass =::ROOT::GenerateInitInstanceLocal(static_cast<const ::LRSM_TBChannel_wtagging*>(nullptr))->GetClass();
      LRSM_TBChannel_wtagging_TClassManip(theClass);
   return theClass;
   }

   static void LRSM_TBChannel_wtagging_TClassManip(TClass* ){
   }

} // end of namespace ROOT

namespace ROOT {
   static TClass *MeasureJetTaggingEff_Dictionary();
   static void MeasureJetTaggingEff_TClassManip(TClass*);
   static void *new_MeasureJetTaggingEff(void *p = nullptr);
   static void *newArray_MeasureJetTaggingEff(Long_t size, void *p);
   static void delete_MeasureJetTaggingEff(void *p);
   static void deleteArray_MeasureJetTaggingEff(void *p);
   static void destruct_MeasureJetTaggingEff(void *p);

   // Function generating the singleton type initializer
   static TGenericClassInfo *GenerateInitInstanceLocal(const ::MeasureJetTaggingEff*)
   {
      ::MeasureJetTaggingEff *ptr = nullptr;
      static ::TVirtualIsAProxy* isa_proxy = new ::TIsAProxy(typeid(::MeasureJetTaggingEff));
      static ::ROOT::TGenericClassInfo 
         instance("MeasureJetTaggingEff", "MeasureJetTaggingEff.h", 6,
                  typeid(::MeasureJetTaggingEff), ::ROOT::Internal::DefineBehavior(ptr, ptr),
                  &MeasureJetTaggingEff_Dictionary, isa_proxy, 4,
                  sizeof(::MeasureJetTaggingEff) );
      instance.SetNew(&new_MeasureJetTaggingEff);
      instance.SetNewArray(&newArray_MeasureJetTaggingEff);
      instance.SetDelete(&delete_MeasureJetTaggingEff);
      instance.SetDeleteArray(&deleteArray_MeasureJetTaggingEff);
      instance.SetDestructor(&destruct_MeasureJetTaggingEff);
      return &instance;
   }
   TGenericClassInfo *GenerateInitInstance(const ::MeasureJetTaggingEff*)
   {
      return GenerateInitInstanceLocal(static_cast<::MeasureJetTaggingEff*>(nullptr));
   }
   // Static variable to force the class initialization
   static ::ROOT::TGenericClassInfo *_R__UNIQUE_DICT_(Init) = GenerateInitInstanceLocal(static_cast<const ::MeasureJetTaggingEff*>(nullptr)); R__UseDummy(_R__UNIQUE_DICT_(Init));

   // Dictionary for non-ClassDef classes
   static TClass *MeasureJetTaggingEff_Dictionary() {
      TClass* theClass =::ROOT::GenerateInitInstanceLocal(static_cast<const ::MeasureJetTaggingEff*>(nullptr))->GetClass();
      MeasureJetTaggingEff_TClassManip(theClass);
   return theClass;
   }

   static void MeasureJetTaggingEff_TClassManip(TClass* ){
   }

} // end of namespace ROOT

namespace ROOT {
   static TClass *ParseEleIDVariables_Dictionary();
   static void ParseEleIDVariables_TClassManip(TClass*);
   static void *new_ParseEleIDVariables(void *p = nullptr);
   static void *newArray_ParseEleIDVariables(Long_t size, void *p);
   static void delete_ParseEleIDVariables(void *p);
   static void deleteArray_ParseEleIDVariables(void *p);
   static void destruct_ParseEleIDVariables(void *p);

   // Function generating the singleton type initializer
   static TGenericClassInfo *GenerateInitInstanceLocal(const ::ParseEleIDVariables*)
   {
      ::ParseEleIDVariables *ptr = nullptr;
      static ::TVirtualIsAProxy* isa_proxy = new ::TIsAProxy(typeid(::ParseEleIDVariables));
      static ::ROOT::TGenericClassInfo 
         instance("ParseEleIDVariables", "ParseEleIDVariables.h", 6,
                  typeid(::ParseEleIDVariables), ::ROOT::Internal::DefineBehavior(ptr, ptr),
                  &ParseEleIDVariables_Dictionary, isa_proxy, 4,
                  sizeof(::ParseEleIDVariables) );
      instance.SetNew(&new_ParseEleIDVariables);
      instance.SetNewArray(&newArray_ParseEleIDVariables);
      instance.SetDelete(&delete_ParseEleIDVariables);
      instance.SetDeleteArray(&deleteArray_ParseEleIDVariables);
      instance.SetDestructor(&destruct_ParseEleIDVariables);
      return &instance;
   }
   TGenericClassInfo *GenerateInitInstance(const ::ParseEleIDVariables*)
   {
      return GenerateInitInstanceLocal(static_cast<::ParseEleIDVariables*>(nullptr));
   }
   // Static variable to force the class initialization
   static ::ROOT::TGenericClassInfo *_R__UNIQUE_DICT_(Init) = GenerateInitInstanceLocal(static_cast<const ::ParseEleIDVariables*>(nullptr)); R__UseDummy(_R__UNIQUE_DICT_(Init));

   // Dictionary for non-ClassDef classes
   static TClass *ParseEleIDVariables_Dictionary() {
      TClass* theClass =::ROOT::GenerateInitInstanceLocal(static_cast<const ::ParseEleIDVariables*>(nullptr))->GetClass();
      ParseEleIDVariables_TClassManip(theClass);
   return theClass;
   }

   static void ParseEleIDVariables_TClassManip(TClass* ){
   }

} // end of namespace ROOT

namespace ROOT {
   static TClass *ParseMuIDVariables_Dictionary();
   static void ParseMuIDVariables_TClassManip(TClass*);
   static void *new_ParseMuIDVariables(void *p = nullptr);
   static void *newArray_ParseMuIDVariables(Long_t size, void *p);
   static void delete_ParseMuIDVariables(void *p);
   static void deleteArray_ParseMuIDVariables(void *p);
   static void destruct_ParseMuIDVariables(void *p);

   // Function generating the singleton type initializer
   static TGenericClassInfo *GenerateInitInstanceLocal(const ::ParseMuIDVariables*)
   {
      ::ParseMuIDVariables *ptr = nullptr;
      static ::TVirtualIsAProxy* isa_proxy = new ::TIsAProxy(typeid(::ParseMuIDVariables));
      static ::ROOT::TGenericClassInfo 
         instance("ParseMuIDVariables", "ParseMuIDVariables.h", 6,
                  typeid(::ParseMuIDVariables), ::ROOT::Internal::DefineBehavior(ptr, ptr),
                  &ParseMuIDVariables_Dictionary, isa_proxy, 4,
                  sizeof(::ParseMuIDVariables) );
      instance.SetNew(&new_ParseMuIDVariables);
      instance.SetNewArray(&newArray_ParseMuIDVariables);
      instance.SetDelete(&delete_ParseMuIDVariables);
      instance.SetDeleteArray(&deleteArray_ParseMuIDVariables);
      instance.SetDestructor(&destruct_ParseMuIDVariables);
      return &instance;
   }
   TGenericClassInfo *GenerateInitInstance(const ::ParseMuIDVariables*)
   {
      return GenerateInitInstanceLocal(static_cast<::ParseMuIDVariables*>(nullptr));
   }
   // Static variable to force the class initialization
   static ::ROOT::TGenericClassInfo *_R__UNIQUE_DICT_(Init) = GenerateInitInstanceLocal(static_cast<const ::ParseMuIDVariables*>(nullptr)); R__UseDummy(_R__UNIQUE_DICT_(Init));

   // Dictionary for non-ClassDef classes
   static TClass *ParseMuIDVariables_Dictionary() {
      TClass* theClass =::ROOT::GenerateInitInstanceLocal(static_cast<const ::ParseMuIDVariables*>(nullptr))->GetClass();
      ParseMuIDVariables_TClassManip(theClass);
   return theClass;
   }

   static void ParseMuIDVariables_TClassManip(TClass* ){
   }

} // end of namespace ROOT

namespace ROOT {
   static TClass *Reproduce20_002_Dictionary();
   static void Reproduce20_002_TClassManip(TClass*);
   static void *new_Reproduce20_002(void *p = nullptr);
   static void *newArray_Reproduce20_002(Long_t size, void *p);
   static void delete_Reproduce20_002(void *p);
   static void deleteArray_Reproduce20_002(void *p);
   static void destruct_Reproduce20_002(void *p);

   // Function generating the singleton type initializer
   static TGenericClassInfo *GenerateInitInstanceLocal(const ::Reproduce20_002*)
   {
      ::Reproduce20_002 *ptr = nullptr;
      static ::TVirtualIsAProxy* isa_proxy = new ::TIsAProxy(typeid(::Reproduce20_002));
      static ::ROOT::TGenericClassInfo 
         instance("Reproduce20_002", "Reproduce20_002.h", 8,
                  typeid(::Reproduce20_002), ::ROOT::Internal::DefineBehavior(ptr, ptr),
                  &Reproduce20_002_Dictionary, isa_proxy, 4,
                  sizeof(::Reproduce20_002) );
      instance.SetNew(&new_Reproduce20_002);
      instance.SetNewArray(&newArray_Reproduce20_002);
      instance.SetDelete(&delete_Reproduce20_002);
      instance.SetDeleteArray(&deleteArray_Reproduce20_002);
      instance.SetDestructor(&destruct_Reproduce20_002);
      return &instance;
   }
   TGenericClassInfo *GenerateInitInstance(const ::Reproduce20_002*)
   {
      return GenerateInitInstanceLocal(static_cast<::Reproduce20_002*>(nullptr));
   }
   // Static variable to force the class initialization
   static ::ROOT::TGenericClassInfo *_R__UNIQUE_DICT_(Init) = GenerateInitInstanceLocal(static_cast<const ::Reproduce20_002*>(nullptr)); R__UseDummy(_R__UNIQUE_DICT_(Init));

   // Dictionary for non-ClassDef classes
   static TClass *Reproduce20_002_Dictionary() {
      TClass* theClass =::ROOT::GenerateInitInstanceLocal(static_cast<const ::Reproduce20_002*>(nullptr))->GetClass();
      Reproduce20_002_TClassManip(theClass);
   return theClass;
   }

   static void Reproduce20_002_TClassManip(TClass* ){
   }

} // end of namespace ROOT

namespace ROOT {
   static TClass *TTbar_test_Dictionary();
   static void TTbar_test_TClassManip(TClass*);
   static void *new_TTbar_test(void *p = nullptr);
   static void *newArray_TTbar_test(Long_t size, void *p);
   static void delete_TTbar_test(void *p);
   static void deleteArray_TTbar_test(void *p);
   static void destruct_TTbar_test(void *p);

   // Function generating the singleton type initializer
   static TGenericClassInfo *GenerateInitInstanceLocal(const ::TTbar_test*)
   {
      ::TTbar_test *ptr = nullptr;
      static ::TVirtualIsAProxy* isa_proxy = new ::TIsAProxy(typeid(::TTbar_test));
      static ::ROOT::TGenericClassInfo 
         instance("TTbar_test", "TTbar_test.h", 7,
                  typeid(::TTbar_test), ::ROOT::Internal::DefineBehavior(ptr, ptr),
                  &TTbar_test_Dictionary, isa_proxy, 4,
                  sizeof(::TTbar_test) );
      instance.SetNew(&new_TTbar_test);
      instance.SetNewArray(&newArray_TTbar_test);
      instance.SetDelete(&delete_TTbar_test);
      instance.SetDeleteArray(&deleteArray_TTbar_test);
      instance.SetDestructor(&destruct_TTbar_test);
      return &instance;
   }
   TGenericClassInfo *GenerateInitInstance(const ::TTbar_test*)
   {
      return GenerateInitInstanceLocal(static_cast<::TTbar_test*>(nullptr));
   }
   // Static variable to force the class initialization
   static ::ROOT::TGenericClassInfo *_R__UNIQUE_DICT_(Init) = GenerateInitInstanceLocal(static_cast<const ::TTbar_test*>(nullptr)); R__UseDummy(_R__UNIQUE_DICT_(Init));

   // Dictionary for non-ClassDef classes
   static TClass *TTbar_test_Dictionary() {
      TClass* theClass =::ROOT::GenerateInitInstanceLocal(static_cast<const ::TTbar_test*>(nullptr))->GetClass();
      TTbar_test_TClassManip(theClass);
   return theClass;
   }

   static void TTbar_test_TClassManip(TClass* ){
   }

} // end of namespace ROOT

namespace ROOT {
   static TClass *TestGetLeptonType_Dictionary();
   static void TestGetLeptonType_TClassManip(TClass*);
   static void *new_TestGetLeptonType(void *p = nullptr);
   static void *newArray_TestGetLeptonType(Long_t size, void *p);
   static void delete_TestGetLeptonType(void *p);
   static void deleteArray_TestGetLeptonType(void *p);
   static void destruct_TestGetLeptonType(void *p);

   // Function generating the singleton type initializer
   static TGenericClassInfo *GenerateInitInstanceLocal(const ::TestGetLeptonType*)
   {
      ::TestGetLeptonType *ptr = nullptr;
      static ::TVirtualIsAProxy* isa_proxy = new ::TIsAProxy(typeid(::TestGetLeptonType));
      static ::ROOT::TGenericClassInfo 
         instance("TestGetLeptonType", "TestGetLeptonType.h", 6,
                  typeid(::TestGetLeptonType), ::ROOT::Internal::DefineBehavior(ptr, ptr),
                  &TestGetLeptonType_Dictionary, isa_proxy, 4,
                  sizeof(::TestGetLeptonType) );
      instance.SetNew(&new_TestGetLeptonType);
      instance.SetNewArray(&newArray_TestGetLeptonType);
      instance.SetDelete(&delete_TestGetLeptonType);
      instance.SetDeleteArray(&deleteArray_TestGetLeptonType);
      instance.SetDestructor(&destruct_TestGetLeptonType);
      return &instance;
   }
   TGenericClassInfo *GenerateInitInstance(const ::TestGetLeptonType*)
   {
      return GenerateInitInstanceLocal(static_cast<::TestGetLeptonType*>(nullptr));
   }
   // Static variable to force the class initialization
   static ::ROOT::TGenericClassInfo *_R__UNIQUE_DICT_(Init) = GenerateInitInstanceLocal(static_cast<const ::TestGetLeptonType*>(nullptr)); R__UseDummy(_R__UNIQUE_DICT_(Init));

   // Dictionary for non-ClassDef classes
   static TClass *TestGetLeptonType_Dictionary() {
      TClass* theClass =::ROOT::GenerateInitInstanceLocal(static_cast<const ::TestGetLeptonType*>(nullptr))->GetClass();
      TestGetLeptonType_TClassManip(theClass);
   return theClass;
   }

   static void TestGetLeptonType_TClassManip(TClass* ){
   }

} // end of namespace ROOT

namespace ROOT {
   static TClass *ttbar_hadronic_Dictionary();
   static void ttbar_hadronic_TClassManip(TClass*);
   static void *new_ttbar_hadronic(void *p = nullptr);
   static void *newArray_ttbar_hadronic(Long_t size, void *p);
   static void delete_ttbar_hadronic(void *p);
   static void deleteArray_ttbar_hadronic(void *p);
   static void destruct_ttbar_hadronic(void *p);

   // Function generating the singleton type initializer
   static TGenericClassInfo *GenerateInitInstanceLocal(const ::ttbar_hadronic*)
   {
      ::ttbar_hadronic *ptr = nullptr;
      static ::TVirtualIsAProxy* isa_proxy = new ::TIsAProxy(typeid(::ttbar_hadronic));
      static ::ROOT::TGenericClassInfo 
         instance("ttbar_hadronic", "ttbar_hadronic.h", 7,
                  typeid(::ttbar_hadronic), ::ROOT::Internal::DefineBehavior(ptr, ptr),
                  &ttbar_hadronic_Dictionary, isa_proxy, 4,
                  sizeof(::ttbar_hadronic) );
      instance.SetNew(&new_ttbar_hadronic);
      instance.SetNewArray(&newArray_ttbar_hadronic);
      instance.SetDelete(&delete_ttbar_hadronic);
      instance.SetDeleteArray(&deleteArray_ttbar_hadronic);
      instance.SetDestructor(&destruct_ttbar_hadronic);
      return &instance;
   }
   TGenericClassInfo *GenerateInitInstance(const ::ttbar_hadronic*)
   {
      return GenerateInitInstanceLocal(static_cast<::ttbar_hadronic*>(nullptr));
   }
   // Static variable to force the class initialization
   static ::ROOT::TGenericClassInfo *_R__UNIQUE_DICT_(Init) = GenerateInitInstanceLocal(static_cast<const ::ttbar_hadronic*>(nullptr)); R__UseDummy(_R__UNIQUE_DICT_(Init));

   // Dictionary for non-ClassDef classes
   static TClass *ttbar_hadronic_Dictionary() {
      TClass* theClass =::ROOT::GenerateInitInstanceLocal(static_cast<const ::ttbar_hadronic*>(nullptr))->GetClass();
      ttbar_hadronic_TClassManip(theClass);
   return theClass;
   }

   static void ttbar_hadronic_TClassManip(TClass* ){
   }

} // end of namespace ROOT

namespace ROOT {
   // Wrappers around operator new
   static void *new_SKNanoLoader(void *p) {
      return  p ? new(p) ::SKNanoLoader : new ::SKNanoLoader;
   }
   static void *newArray_SKNanoLoader(Long_t nElements, void *p) {
      return p ? new(p) ::SKNanoLoader[nElements] : new ::SKNanoLoader[nElements];
   }
   // Wrapper around operator delete
   static void delete_SKNanoLoader(void *p) {
      delete (static_cast<::SKNanoLoader*>(p));
   }
   static void deleteArray_SKNanoLoader(void *p) {
      delete [] (static_cast<::SKNanoLoader*>(p));
   }
   static void destruct_SKNanoLoader(void *p) {
      typedef ::SKNanoLoader current_t;
      (static_cast<current_t*>(p))->~current_t();
   }
} // end of namespace ROOT for class ::SKNanoLoader

namespace ROOT {
   // Wrappers around operator new
   static void *new_AnalyzerCore(void *p) {
      return  p ? new(p) ::AnalyzerCore : new ::AnalyzerCore;
   }
   static void *newArray_AnalyzerCore(Long_t nElements, void *p) {
      return p ? new(p) ::AnalyzerCore[nElements] : new ::AnalyzerCore[nElements];
   }
   // Wrapper around operator delete
   static void delete_AnalyzerCore(void *p) {
      delete (static_cast<::AnalyzerCore*>(p));
   }
   static void deleteArray_AnalyzerCore(void *p) {
      delete [] (static_cast<::AnalyzerCore*>(p));
   }
   static void destruct_AnalyzerCore(void *p) {
      typedef ::AnalyzerCore current_t;
      (static_cast<current_t*>(p))->~current_t();
   }
} // end of namespace ROOT for class ::AnalyzerCore

namespace ROOT {
   // Wrappers around operator new
   static void *new_DY(void *p) {
      return  p ? new(p) ::DY : new ::DY;
   }
   static void *newArray_DY(Long_t nElements, void *p) {
      return p ? new(p) ::DY[nElements] : new ::DY[nElements];
   }
   // Wrapper around operator delete
   static void delete_DY(void *p) {
      delete (static_cast<::DY*>(p));
   }
   static void deleteArray_DY(void *p) {
      delete [] (static_cast<::DY*>(p));
   }
   static void destruct_DY(void *p) {
      typedef ::DY current_t;
      (static_cast<current_t*>(p))->~current_t();
   }
} // end of namespace ROOT for class ::DY

namespace ROOT {
   // Wrappers around operator new
   static void *new_DiLeptonBase(void *p) {
      return  p ? new(p) ::DiLeptonBase : new ::DiLeptonBase;
   }
   static void *newArray_DiLeptonBase(Long_t nElements, void *p) {
      return p ? new(p) ::DiLeptonBase[nElements] : new ::DiLeptonBase[nElements];
   }
   // Wrapper around operator delete
   static void delete_DiLeptonBase(void *p) {
      delete (static_cast<::DiLeptonBase*>(p));
   }
   static void deleteArray_DiLeptonBase(void *p) {
      delete [] (static_cast<::DiLeptonBase*>(p));
   }
   static void destruct_DiLeptonBase(void *p) {
      typedef ::DiLeptonBase current_t;
      (static_cast<current_t*>(p))->~current_t();
   }
} // end of namespace ROOT for class ::DiLeptonBase

namespace ROOT {
   // Wrappers around operator new
   static void *new_FullHadronicTriggerTnP(void *p) {
      return  p ? new(p) ::FullHadronicTriggerTnP : new ::FullHadronicTriggerTnP;
   }
   static void *newArray_FullHadronicTriggerTnP(Long_t nElements, void *p) {
      return p ? new(p) ::FullHadronicTriggerTnP[nElements] : new ::FullHadronicTriggerTnP[nElements];
   }
   // Wrapper around operator delete
   static void delete_FullHadronicTriggerTnP(void *p) {
      delete (static_cast<::FullHadronicTriggerTnP*>(p));
   }
   static void deleteArray_FullHadronicTriggerTnP(void *p) {
      delete [] (static_cast<::FullHadronicTriggerTnP*>(p));
   }
   static void destruct_FullHadronicTriggerTnP(void *p) {
      typedef ::FullHadronicTriggerTnP current_t;
      (static_cast<current_t*>(p))->~current_t();
   }
} // end of namespace ROOT for class ::FullHadronicTriggerTnP

namespace ROOT {
   // Wrappers around operator new
   static void *new_GeneratorBase(void *p) {
      return  p ? new(p) ::GeneratorBase : new ::GeneratorBase;
   }
   static void *newArray_GeneratorBase(Long_t nElements, void *p) {
      return p ? new(p) ::GeneratorBase[nElements] : new ::GeneratorBase[nElements];
   }
   // Wrapper around operator delete
   static void delete_GeneratorBase(void *p) {
      delete (static_cast<::GeneratorBase*>(p));
   }
   static void deleteArray_GeneratorBase(void *p) {
      delete [] (static_cast<::GeneratorBase*>(p));
   }
   static void destruct_GeneratorBase(void *p) {
      typedef ::GeneratorBase current_t;
      (static_cast<current_t*>(p))->~current_t();
   }
} // end of namespace ROOT for class ::GeneratorBase

namespace ROOT {
   // Wrappers around operator new
   static void *new_GetEffLumi(void *p) {
      return  p ? new(p) ::GetEffLumi : new ::GetEffLumi;
   }
   static void *newArray_GetEffLumi(Long_t nElements, void *p) {
      return p ? new(p) ::GetEffLumi[nElements] : new ::GetEffLumi[nElements];
   }
   // Wrapper around operator delete
   static void delete_GetEffLumi(void *p) {
      delete (static_cast<::GetEffLumi*>(p));
   }
   static void deleteArray_GetEffLumi(void *p) {
      delete [] (static_cast<::GetEffLumi*>(p));
   }
   static void destruct_GetEffLumi(void *p) {
      typedef ::GetEffLumi current_t;
      (static_cast<current_t*>(p))->~current_t();
   }
} // end of namespace ROOT for class ::GetEffLumi

namespace ROOT {
   // Wrappers around operator new
   static void *new_LRSM_TBChannel(void *p) {
      return  p ? new(p) ::LRSM_TBChannel : new ::LRSM_TBChannel;
   }
   static void *newArray_LRSM_TBChannel(Long_t nElements, void *p) {
      return p ? new(p) ::LRSM_TBChannel[nElements] : new ::LRSM_TBChannel[nElements];
   }
   // Wrapper around operator delete
   static void delete_LRSM_TBChannel(void *p) {
      delete (static_cast<::LRSM_TBChannel*>(p));
   }
   static void deleteArray_LRSM_TBChannel(void *p) {
      delete [] (static_cast<::LRSM_TBChannel*>(p));
   }
   static void destruct_LRSM_TBChannel(void *p) {
      typedef ::LRSM_TBChannel current_t;
      (static_cast<current_t*>(p))->~current_t();
   }
} // end of namespace ROOT for class ::LRSM_TBChannel

namespace ROOT {
   // Wrappers around operator new
   static void *new_LRSM_TBChannel_notusingbjet(void *p) {
      return  p ? new(p) ::LRSM_TBChannel_notusingbjet : new ::LRSM_TBChannel_notusingbjet;
   }
   static void *newArray_LRSM_TBChannel_notusingbjet(Long_t nElements, void *p) {
      return p ? new(p) ::LRSM_TBChannel_notusingbjet[nElements] : new ::LRSM_TBChannel_notusingbjet[nElements];
   }
   // Wrapper around operator delete
   static void delete_LRSM_TBChannel_notusingbjet(void *p) {
      delete (static_cast<::LRSM_TBChannel_notusingbjet*>(p));
   }
   static void deleteArray_LRSM_TBChannel_notusingbjet(void *p) {
      delete [] (static_cast<::LRSM_TBChannel_notusingbjet*>(p));
   }
   static void destruct_LRSM_TBChannel_notusingbjet(void *p) {
      typedef ::LRSM_TBChannel_notusingbjet current_t;
      (static_cast<current_t*>(p))->~current_t();
   }
} // end of namespace ROOT for class ::LRSM_TBChannel_notusingbjet

namespace ROOT {
   // Wrappers around operator new
   static void *new_LRSM_TBChannel_wtagging(void *p) {
      return  p ? new(p) ::LRSM_TBChannel_wtagging : new ::LRSM_TBChannel_wtagging;
   }
   static void *newArray_LRSM_TBChannel_wtagging(Long_t nElements, void *p) {
      return p ? new(p) ::LRSM_TBChannel_wtagging[nElements] : new ::LRSM_TBChannel_wtagging[nElements];
   }
   // Wrapper around operator delete
   static void delete_LRSM_TBChannel_wtagging(void *p) {
      delete (static_cast<::LRSM_TBChannel_wtagging*>(p));
   }
   static void deleteArray_LRSM_TBChannel_wtagging(void *p) {
      delete [] (static_cast<::LRSM_TBChannel_wtagging*>(p));
   }
   static void destruct_LRSM_TBChannel_wtagging(void *p) {
      typedef ::LRSM_TBChannel_wtagging current_t;
      (static_cast<current_t*>(p))->~current_t();
   }
} // end of namespace ROOT for class ::LRSM_TBChannel_wtagging

namespace ROOT {
   // Wrappers around operator new
   static void *new_MeasureJetTaggingEff(void *p) {
      return  p ? new(p) ::MeasureJetTaggingEff : new ::MeasureJetTaggingEff;
   }
   static void *newArray_MeasureJetTaggingEff(Long_t nElements, void *p) {
      return p ? new(p) ::MeasureJetTaggingEff[nElements] : new ::MeasureJetTaggingEff[nElements];
   }
   // Wrapper around operator delete
   static void delete_MeasureJetTaggingEff(void *p) {
      delete (static_cast<::MeasureJetTaggingEff*>(p));
   }
   static void deleteArray_MeasureJetTaggingEff(void *p) {
      delete [] (static_cast<::MeasureJetTaggingEff*>(p));
   }
   static void destruct_MeasureJetTaggingEff(void *p) {
      typedef ::MeasureJetTaggingEff current_t;
      (static_cast<current_t*>(p))->~current_t();
   }
} // end of namespace ROOT for class ::MeasureJetTaggingEff

namespace ROOT {
   // Wrappers around operator new
   static void *new_ParseEleIDVariables(void *p) {
      return  p ? new(p) ::ParseEleIDVariables : new ::ParseEleIDVariables;
   }
   static void *newArray_ParseEleIDVariables(Long_t nElements, void *p) {
      return p ? new(p) ::ParseEleIDVariables[nElements] : new ::ParseEleIDVariables[nElements];
   }
   // Wrapper around operator delete
   static void delete_ParseEleIDVariables(void *p) {
      delete (static_cast<::ParseEleIDVariables*>(p));
   }
   static void deleteArray_ParseEleIDVariables(void *p) {
      delete [] (static_cast<::ParseEleIDVariables*>(p));
   }
   static void destruct_ParseEleIDVariables(void *p) {
      typedef ::ParseEleIDVariables current_t;
      (static_cast<current_t*>(p))->~current_t();
   }
} // end of namespace ROOT for class ::ParseEleIDVariables

namespace ROOT {
   // Wrappers around operator new
   static void *new_ParseMuIDVariables(void *p) {
      return  p ? new(p) ::ParseMuIDVariables : new ::ParseMuIDVariables;
   }
   static void *newArray_ParseMuIDVariables(Long_t nElements, void *p) {
      return p ? new(p) ::ParseMuIDVariables[nElements] : new ::ParseMuIDVariables[nElements];
   }
   // Wrapper around operator delete
   static void delete_ParseMuIDVariables(void *p) {
      delete (static_cast<::ParseMuIDVariables*>(p));
   }
   static void deleteArray_ParseMuIDVariables(void *p) {
      delete [] (static_cast<::ParseMuIDVariables*>(p));
   }
   static void destruct_ParseMuIDVariables(void *p) {
      typedef ::ParseMuIDVariables current_t;
      (static_cast<current_t*>(p))->~current_t();
   }
} // end of namespace ROOT for class ::ParseMuIDVariables

namespace ROOT {
   // Wrappers around operator new
   static void *new_Reproduce20_002(void *p) {
      return  p ? new(p) ::Reproduce20_002 : new ::Reproduce20_002;
   }
   static void *newArray_Reproduce20_002(Long_t nElements, void *p) {
      return p ? new(p) ::Reproduce20_002[nElements] : new ::Reproduce20_002[nElements];
   }
   // Wrapper around operator delete
   static void delete_Reproduce20_002(void *p) {
      delete (static_cast<::Reproduce20_002*>(p));
   }
   static void deleteArray_Reproduce20_002(void *p) {
      delete [] (static_cast<::Reproduce20_002*>(p));
   }
   static void destruct_Reproduce20_002(void *p) {
      typedef ::Reproduce20_002 current_t;
      (static_cast<current_t*>(p))->~current_t();
   }
} // end of namespace ROOT for class ::Reproduce20_002

namespace ROOT {
   // Wrappers around operator new
   static void *new_TTbar_test(void *p) {
      return  p ? new(p) ::TTbar_test : new ::TTbar_test;
   }
   static void *newArray_TTbar_test(Long_t nElements, void *p) {
      return p ? new(p) ::TTbar_test[nElements] : new ::TTbar_test[nElements];
   }
   // Wrapper around operator delete
   static void delete_TTbar_test(void *p) {
      delete (static_cast<::TTbar_test*>(p));
   }
   static void deleteArray_TTbar_test(void *p) {
      delete [] (static_cast<::TTbar_test*>(p));
   }
   static void destruct_TTbar_test(void *p) {
      typedef ::TTbar_test current_t;
      (static_cast<current_t*>(p))->~current_t();
   }
} // end of namespace ROOT for class ::TTbar_test

namespace ROOT {
   // Wrappers around operator new
   static void *new_TestGetLeptonType(void *p) {
      return  p ? new(p) ::TestGetLeptonType : new ::TestGetLeptonType;
   }
   static void *newArray_TestGetLeptonType(Long_t nElements, void *p) {
      return p ? new(p) ::TestGetLeptonType[nElements] : new ::TestGetLeptonType[nElements];
   }
   // Wrapper around operator delete
   static void delete_TestGetLeptonType(void *p) {
      delete (static_cast<::TestGetLeptonType*>(p));
   }
   static void deleteArray_TestGetLeptonType(void *p) {
      delete [] (static_cast<::TestGetLeptonType*>(p));
   }
   static void destruct_TestGetLeptonType(void *p) {
      typedef ::TestGetLeptonType current_t;
      (static_cast<current_t*>(p))->~current_t();
   }
} // end of namespace ROOT for class ::TestGetLeptonType

namespace ROOT {
   // Wrappers around operator new
   static void *new_ttbar_hadronic(void *p) {
      return  p ? new(p) ::ttbar_hadronic : new ::ttbar_hadronic;
   }
   static void *newArray_ttbar_hadronic(Long_t nElements, void *p) {
      return p ? new(p) ::ttbar_hadronic[nElements] : new ::ttbar_hadronic[nElements];
   }
   // Wrapper around operator delete
   static void delete_ttbar_hadronic(void *p) {
      delete (static_cast<::ttbar_hadronic*>(p));
   }
   static void deleteArray_ttbar_hadronic(void *p) {
      delete [] (static_cast<::ttbar_hadronic*>(p));
   }
   static void destruct_ttbar_hadronic(void *p) {
      typedef ::ttbar_hadronic current_t;
      (static_cast<current_t*>(p))->~current_t();
   }
} // end of namespace ROOT for class ::ttbar_hadronic

namespace ROOT {
   static TClass *vectorlEfloatgR_Dictionary();
   static void vectorlEfloatgR_TClassManip(TClass*);
   static void *new_vectorlEfloatgR(void *p = nullptr);
   static void *newArray_vectorlEfloatgR(Long_t size, void *p);
   static void delete_vectorlEfloatgR(void *p);
   static void deleteArray_vectorlEfloatgR(void *p);
   static void destruct_vectorlEfloatgR(void *p);

   // Function generating the singleton type initializer
   static TGenericClassInfo *GenerateInitInstanceLocal(const vector<float>*)
   {
      vector<float> *ptr = nullptr;
      static ::TVirtualIsAProxy* isa_proxy = new ::TIsAProxy(typeid(vector<float>));
      static ::ROOT::TGenericClassInfo 
         instance("vector<float>", -2, "vector", 428,
                  typeid(vector<float>), ::ROOT::Internal::DefineBehavior(ptr, ptr),
                  &vectorlEfloatgR_Dictionary, isa_proxy, 0,
                  sizeof(vector<float>) );
      instance.SetNew(&new_vectorlEfloatgR);
      instance.SetNewArray(&newArray_vectorlEfloatgR);
      instance.SetDelete(&delete_vectorlEfloatgR);
      instance.SetDeleteArray(&deleteArray_vectorlEfloatgR);
      instance.SetDestructor(&destruct_vectorlEfloatgR);
      instance.AdoptCollectionProxyInfo(TCollectionProxyInfo::Generate(TCollectionProxyInfo::Pushback< vector<float> >()));

      instance.AdoptAlternate(::ROOT::AddClassAlternate("vector<float>","std::vector<float, std::allocator<float> >"));
      return &instance;
   }
   // Static variable to force the class initialization
   static ::ROOT::TGenericClassInfo *_R__UNIQUE_DICT_(Init) = GenerateInitInstanceLocal(static_cast<const vector<float>*>(nullptr)); R__UseDummy(_R__UNIQUE_DICT_(Init));

   // Dictionary for non-ClassDef classes
   static TClass *vectorlEfloatgR_Dictionary() {
      TClass* theClass =::ROOT::GenerateInitInstanceLocal(static_cast<const vector<float>*>(nullptr))->GetClass();
      vectorlEfloatgR_TClassManip(theClass);
   return theClass;
   }

   static void vectorlEfloatgR_TClassManip(TClass* ){
   }

} // end of namespace ROOT

namespace ROOT {
   // Wrappers around operator new
   static void *new_vectorlEfloatgR(void *p) {
      return  p ? ::new(static_cast<::ROOT::Internal::TOperatorNewHelper*>(p)) vector<float> : new vector<float>;
   }
   static void *newArray_vectorlEfloatgR(Long_t nElements, void *p) {
      return p ? ::new(static_cast<::ROOT::Internal::TOperatorNewHelper*>(p)) vector<float>[nElements] : new vector<float>[nElements];
   }
   // Wrapper around operator delete
   static void delete_vectorlEfloatgR(void *p) {
      delete (static_cast<vector<float>*>(p));
   }
   static void deleteArray_vectorlEfloatgR(void *p) {
      delete [] (static_cast<vector<float>*>(p));
   }
   static void destruct_vectorlEfloatgR(void *p) {
      typedef vector<float> current_t;
      (static_cast<current_t*>(p))->~current_t();
   }
} // end of namespace ROOT for class vector<float>

namespace ROOT {
   static TClass *unordered_maplEstringcOTTreemUgR_Dictionary();
   static void unordered_maplEstringcOTTreemUgR_TClassManip(TClass*);
   static void *new_unordered_maplEstringcOTTreemUgR(void *p = nullptr);
   static void *newArray_unordered_maplEstringcOTTreemUgR(Long_t size, void *p);
   static void delete_unordered_maplEstringcOTTreemUgR(void *p);
   static void deleteArray_unordered_maplEstringcOTTreemUgR(void *p);
   static void destruct_unordered_maplEstringcOTTreemUgR(void *p);

   // Function generating the singleton type initializer
   static TGenericClassInfo *GenerateInitInstanceLocal(const unordered_map<string,TTree*>*)
   {
      unordered_map<string,TTree*> *ptr = nullptr;
      static ::TVirtualIsAProxy* isa_proxy = new ::TIsAProxy(typeid(unordered_map<string,TTree*>));
      static ::ROOT::TGenericClassInfo 
         instance("unordered_map<string,TTree*>", -2, "unordered_map", 109,
                  typeid(unordered_map<string,TTree*>), ::ROOT::Internal::DefineBehavior(ptr, ptr),
                  &unordered_maplEstringcOTTreemUgR_Dictionary, isa_proxy, 0,
                  sizeof(unordered_map<string,TTree*>) );
      instance.SetNew(&new_unordered_maplEstringcOTTreemUgR);
      instance.SetNewArray(&newArray_unordered_maplEstringcOTTreemUgR);
      instance.SetDelete(&delete_unordered_maplEstringcOTTreemUgR);
      instance.SetDeleteArray(&deleteArray_unordered_maplEstringcOTTreemUgR);
      instance.SetDestructor(&destruct_unordered_maplEstringcOTTreemUgR);
      instance.AdoptCollectionProxyInfo(TCollectionProxyInfo::Generate(TCollectionProxyInfo::MapInsert< unordered_map<string,TTree*> >()));

      instance.AdoptAlternate(::ROOT::AddClassAlternate("unordered_map<string,TTree*>","std::unordered_map<std::__cxx11::basic_string<char, std::char_traits<char>, std::allocator<char> >, TTree*, std::hash<std::__cxx11::basic_string<char, std::char_traits<char>, std::allocator<char> > >, std::equal_to<std::__cxx11::basic_string<char, std::char_traits<char>, std::allocator<char> > >, std::allocator<std::pair<std::__cxx11::basic_string<char, std::char_traits<char>, std::allocator<char> > const, TTree*> > >"));
      return &instance;
   }
   // Static variable to force the class initialization
   static ::ROOT::TGenericClassInfo *_R__UNIQUE_DICT_(Init) = GenerateInitInstanceLocal(static_cast<const unordered_map<string,TTree*>*>(nullptr)); R__UseDummy(_R__UNIQUE_DICT_(Init));

   // Dictionary for non-ClassDef classes
   static TClass *unordered_maplEstringcOTTreemUgR_Dictionary() {
      TClass* theClass =::ROOT::GenerateInitInstanceLocal(static_cast<const unordered_map<string,TTree*>*>(nullptr))->GetClass();
      unordered_maplEstringcOTTreemUgR_TClassManip(theClass);
   return theClass;
   }

   static void unordered_maplEstringcOTTreemUgR_TClassManip(TClass* ){
   }

} // end of namespace ROOT

namespace ROOT {
   // Wrappers around operator new
   static void *new_unordered_maplEstringcOTTreemUgR(void *p) {
      return  p ? ::new(static_cast<::ROOT::Internal::TOperatorNewHelper*>(p)) unordered_map<string,TTree*> : new unordered_map<string,TTree*>;
   }
   static void *newArray_unordered_maplEstringcOTTreemUgR(Long_t nElements, void *p) {
      return p ? ::new(static_cast<::ROOT::Internal::TOperatorNewHelper*>(p)) unordered_map<string,TTree*>[nElements] : new unordered_map<string,TTree*>[nElements];
   }
   // Wrapper around operator delete
   static void delete_unordered_maplEstringcOTTreemUgR(void *p) {
      delete (static_cast<unordered_map<string,TTree*>*>(p));
   }
   static void deleteArray_unordered_maplEstringcOTTreemUgR(void *p) {
      delete [] (static_cast<unordered_map<string,TTree*>*>(p));
   }
   static void destruct_unordered_maplEstringcOTTreemUgR(void *p) {
      typedef unordered_map<string,TTree*> current_t;
      (static_cast<current_t*>(p))->~current_t();
   }
} // end of namespace ROOT for class unordered_map<string,TTree*>

namespace ROOT {
   static TClass *unordered_maplEstringcOTH3mUgR_Dictionary();
   static void unordered_maplEstringcOTH3mUgR_TClassManip(TClass*);
   static void *new_unordered_maplEstringcOTH3mUgR(void *p = nullptr);
   static void *newArray_unordered_maplEstringcOTH3mUgR(Long_t size, void *p);
   static void delete_unordered_maplEstringcOTH3mUgR(void *p);
   static void deleteArray_unordered_maplEstringcOTH3mUgR(void *p);
   static void destruct_unordered_maplEstringcOTH3mUgR(void *p);

   // Function generating the singleton type initializer
   static TGenericClassInfo *GenerateInitInstanceLocal(const unordered_map<string,TH3*>*)
   {
      unordered_map<string,TH3*> *ptr = nullptr;
      static ::TVirtualIsAProxy* isa_proxy = new ::TIsAProxy(typeid(unordered_map<string,TH3*>));
      static ::ROOT::TGenericClassInfo 
         instance("unordered_map<string,TH3*>", -2, "unordered_map", 109,
                  typeid(unordered_map<string,TH3*>), ::ROOT::Internal::DefineBehavior(ptr, ptr),
                  &unordered_maplEstringcOTH3mUgR_Dictionary, isa_proxy, 0,
                  sizeof(unordered_map<string,TH3*>) );
      instance.SetNew(&new_unordered_maplEstringcOTH3mUgR);
      instance.SetNewArray(&newArray_unordered_maplEstringcOTH3mUgR);
      instance.SetDelete(&delete_unordered_maplEstringcOTH3mUgR);
      instance.SetDeleteArray(&deleteArray_unordered_maplEstringcOTH3mUgR);
      instance.SetDestructor(&destruct_unordered_maplEstringcOTH3mUgR);
      instance.AdoptCollectionProxyInfo(TCollectionProxyInfo::Generate(TCollectionProxyInfo::MapInsert< unordered_map<string,TH3*> >()));

      instance.AdoptAlternate(::ROOT::AddClassAlternate("unordered_map<string,TH3*>","std::unordered_map<std::__cxx11::basic_string<char, std::char_traits<char>, std::allocator<char> >, TH3*, std::hash<std::__cxx11::basic_string<char, std::char_traits<char>, std::allocator<char> > >, std::equal_to<std::__cxx11::basic_string<char, std::char_traits<char>, std::allocator<char> > >, std::allocator<std::pair<std::__cxx11::basic_string<char, std::char_traits<char>, std::allocator<char> > const, TH3*> > >"));
      return &instance;
   }
   // Static variable to force the class initialization
   static ::ROOT::TGenericClassInfo *_R__UNIQUE_DICT_(Init) = GenerateInitInstanceLocal(static_cast<const unordered_map<string,TH3*>*>(nullptr)); R__UseDummy(_R__UNIQUE_DICT_(Init));

   // Dictionary for non-ClassDef classes
   static TClass *unordered_maplEstringcOTH3mUgR_Dictionary() {
      TClass* theClass =::ROOT::GenerateInitInstanceLocal(static_cast<const unordered_map<string,TH3*>*>(nullptr))->GetClass();
      unordered_maplEstringcOTH3mUgR_TClassManip(theClass);
   return theClass;
   }

   static void unordered_maplEstringcOTH3mUgR_TClassManip(TClass* ){
   }

} // end of namespace ROOT

namespace ROOT {
   // Wrappers around operator new
   static void *new_unordered_maplEstringcOTH3mUgR(void *p) {
      return  p ? ::new(static_cast<::ROOT::Internal::TOperatorNewHelper*>(p)) unordered_map<string,TH3*> : new unordered_map<string,TH3*>;
   }
   static void *newArray_unordered_maplEstringcOTH3mUgR(Long_t nElements, void *p) {
      return p ? ::new(static_cast<::ROOT::Internal::TOperatorNewHelper*>(p)) unordered_map<string,TH3*>[nElements] : new unordered_map<string,TH3*>[nElements];
   }
   // Wrapper around operator delete
   static void delete_unordered_maplEstringcOTH3mUgR(void *p) {
      delete (static_cast<unordered_map<string,TH3*>*>(p));
   }
   static void deleteArray_unordered_maplEstringcOTH3mUgR(void *p) {
      delete [] (static_cast<unordered_map<string,TH3*>*>(p));
   }
   static void destruct_unordered_maplEstringcOTH3mUgR(void *p) {
      typedef unordered_map<string,TH3*> current_t;
      (static_cast<current_t*>(p))->~current_t();
   }
} // end of namespace ROOT for class unordered_map<string,TH3*>

namespace ROOT {
   static TClass *unordered_maplEstringcOTH2mUgR_Dictionary();
   static void unordered_maplEstringcOTH2mUgR_TClassManip(TClass*);
   static void *new_unordered_maplEstringcOTH2mUgR(void *p = nullptr);
   static void *newArray_unordered_maplEstringcOTH2mUgR(Long_t size, void *p);
   static void delete_unordered_maplEstringcOTH2mUgR(void *p);
   static void deleteArray_unordered_maplEstringcOTH2mUgR(void *p);
   static void destruct_unordered_maplEstringcOTH2mUgR(void *p);

   // Function generating the singleton type initializer
   static TGenericClassInfo *GenerateInitInstanceLocal(const unordered_map<string,TH2*>*)
   {
      unordered_map<string,TH2*> *ptr = nullptr;
      static ::TVirtualIsAProxy* isa_proxy = new ::TIsAProxy(typeid(unordered_map<string,TH2*>));
      static ::ROOT::TGenericClassInfo 
         instance("unordered_map<string,TH2*>", -2, "unordered_map", 109,
                  typeid(unordered_map<string,TH2*>), ::ROOT::Internal::DefineBehavior(ptr, ptr),
                  &unordered_maplEstringcOTH2mUgR_Dictionary, isa_proxy, 0,
                  sizeof(unordered_map<string,TH2*>) );
      instance.SetNew(&new_unordered_maplEstringcOTH2mUgR);
      instance.SetNewArray(&newArray_unordered_maplEstringcOTH2mUgR);
      instance.SetDelete(&delete_unordered_maplEstringcOTH2mUgR);
      instance.SetDeleteArray(&deleteArray_unordered_maplEstringcOTH2mUgR);
      instance.SetDestructor(&destruct_unordered_maplEstringcOTH2mUgR);
      instance.AdoptCollectionProxyInfo(TCollectionProxyInfo::Generate(TCollectionProxyInfo::MapInsert< unordered_map<string,TH2*> >()));

      instance.AdoptAlternate(::ROOT::AddClassAlternate("unordered_map<string,TH2*>","std::unordered_map<std::__cxx11::basic_string<char, std::char_traits<char>, std::allocator<char> >, TH2*, std::hash<std::__cxx11::basic_string<char, std::char_traits<char>, std::allocator<char> > >, std::equal_to<std::__cxx11::basic_string<char, std::char_traits<char>, std::allocator<char> > >, std::allocator<std::pair<std::__cxx11::basic_string<char, std::char_traits<char>, std::allocator<char> > const, TH2*> > >"));
      return &instance;
   }
   // Static variable to force the class initialization
   static ::ROOT::TGenericClassInfo *_R__UNIQUE_DICT_(Init) = GenerateInitInstanceLocal(static_cast<const unordered_map<string,TH2*>*>(nullptr)); R__UseDummy(_R__UNIQUE_DICT_(Init));

   // Dictionary for non-ClassDef classes
   static TClass *unordered_maplEstringcOTH2mUgR_Dictionary() {
      TClass* theClass =::ROOT::GenerateInitInstanceLocal(static_cast<const unordered_map<string,TH2*>*>(nullptr))->GetClass();
      unordered_maplEstringcOTH2mUgR_TClassManip(theClass);
   return theClass;
   }

   static void unordered_maplEstringcOTH2mUgR_TClassManip(TClass* ){
   }

} // end of namespace ROOT

namespace ROOT {
   // Wrappers around operator new
   static void *new_unordered_maplEstringcOTH2mUgR(void *p) {
      return  p ? ::new(static_cast<::ROOT::Internal::TOperatorNewHelper*>(p)) unordered_map<string,TH2*> : new unordered_map<string,TH2*>;
   }
   static void *newArray_unordered_maplEstringcOTH2mUgR(Long_t nElements, void *p) {
      return p ? ::new(static_cast<::ROOT::Internal::TOperatorNewHelper*>(p)) unordered_map<string,TH2*>[nElements] : new unordered_map<string,TH2*>[nElements];
   }
   // Wrapper around operator delete
   static void delete_unordered_maplEstringcOTH2mUgR(void *p) {
      delete (static_cast<unordered_map<string,TH2*>*>(p));
   }
   static void deleteArray_unordered_maplEstringcOTH2mUgR(void *p) {
      delete [] (static_cast<unordered_map<string,TH2*>*>(p));
   }
   static void destruct_unordered_maplEstringcOTH2mUgR(void *p) {
      typedef unordered_map<string,TH2*> current_t;
      (static_cast<current_t*>(p))->~current_t();
   }
} // end of namespace ROOT for class unordered_map<string,TH2*>

namespace ROOT {
   static TClass *unordered_maplEstringcOTH1mUgR_Dictionary();
   static void unordered_maplEstringcOTH1mUgR_TClassManip(TClass*);
   static void *new_unordered_maplEstringcOTH1mUgR(void *p = nullptr);
   static void *newArray_unordered_maplEstringcOTH1mUgR(Long_t size, void *p);
   static void delete_unordered_maplEstringcOTH1mUgR(void *p);
   static void deleteArray_unordered_maplEstringcOTH1mUgR(void *p);
   static void destruct_unordered_maplEstringcOTH1mUgR(void *p);

   // Function generating the singleton type initializer
   static TGenericClassInfo *GenerateInitInstanceLocal(const unordered_map<string,TH1*>*)
   {
      unordered_map<string,TH1*> *ptr = nullptr;
      static ::TVirtualIsAProxy* isa_proxy = new ::TIsAProxy(typeid(unordered_map<string,TH1*>));
      static ::ROOT::TGenericClassInfo 
         instance("unordered_map<string,TH1*>", -2, "unordered_map", 109,
                  typeid(unordered_map<string,TH1*>), ::ROOT::Internal::DefineBehavior(ptr, ptr),
                  &unordered_maplEstringcOTH1mUgR_Dictionary, isa_proxy, 0,
                  sizeof(unordered_map<string,TH1*>) );
      instance.SetNew(&new_unordered_maplEstringcOTH1mUgR);
      instance.SetNewArray(&newArray_unordered_maplEstringcOTH1mUgR);
      instance.SetDelete(&delete_unordered_maplEstringcOTH1mUgR);
      instance.SetDeleteArray(&deleteArray_unordered_maplEstringcOTH1mUgR);
      instance.SetDestructor(&destruct_unordered_maplEstringcOTH1mUgR);
      instance.AdoptCollectionProxyInfo(TCollectionProxyInfo::Generate(TCollectionProxyInfo::MapInsert< unordered_map<string,TH1*> >()));

      instance.AdoptAlternate(::ROOT::AddClassAlternate("unordered_map<string,TH1*>","std::unordered_map<std::__cxx11::basic_string<char, std::char_traits<char>, std::allocator<char> >, TH1*, std::hash<std::__cxx11::basic_string<char, std::char_traits<char>, std::allocator<char> > >, std::equal_to<std::__cxx11::basic_string<char, std::char_traits<char>, std::allocator<char> > >, std::allocator<std::pair<std::__cxx11::basic_string<char, std::char_traits<char>, std::allocator<char> > const, TH1*> > >"));
      return &instance;
   }
   // Static variable to force the class initialization
   static ::ROOT::TGenericClassInfo *_R__UNIQUE_DICT_(Init) = GenerateInitInstanceLocal(static_cast<const unordered_map<string,TH1*>*>(nullptr)); R__UseDummy(_R__UNIQUE_DICT_(Init));

   // Dictionary for non-ClassDef classes
   static TClass *unordered_maplEstringcOTH1mUgR_Dictionary() {
      TClass* theClass =::ROOT::GenerateInitInstanceLocal(static_cast<const unordered_map<string,TH1*>*>(nullptr))->GetClass();
      unordered_maplEstringcOTH1mUgR_TClassManip(theClass);
   return theClass;
   }

   static void unordered_maplEstringcOTH1mUgR_TClassManip(TClass* ){
   }

} // end of namespace ROOT

namespace ROOT {
   // Wrappers around operator new
   static void *new_unordered_maplEstringcOTH1mUgR(void *p) {
      return  p ? ::new(static_cast<::ROOT::Internal::TOperatorNewHelper*>(p)) unordered_map<string,TH1*> : new unordered_map<string,TH1*>;
   }
   static void *newArray_unordered_maplEstringcOTH1mUgR(Long_t nElements, void *p) {
      return p ? ::new(static_cast<::ROOT::Internal::TOperatorNewHelper*>(p)) unordered_map<string,TH1*>[nElements] : new unordered_map<string,TH1*>[nElements];
   }
   // Wrapper around operator delete
   static void delete_unordered_maplEstringcOTH1mUgR(void *p) {
      delete (static_cast<unordered_map<string,TH1*>*>(p));
   }
   static void deleteArray_unordered_maplEstringcOTH1mUgR(void *p) {
      delete [] (static_cast<unordered_map<string,TH1*>*>(p));
   }
   static void destruct_unordered_maplEstringcOTH1mUgR(void *p) {
      typedef unordered_map<string,TH1*> current_t;
      (static_cast<current_t*>(p))->~current_t();
   }
} // end of namespace ROOT for class unordered_map<string,TH1*>

namespace ROOT {
   static TClass *unordered_maplEstringcOTBranchmUgR_Dictionary();
   static void unordered_maplEstringcOTBranchmUgR_TClassManip(TClass*);
   static void *new_unordered_maplEstringcOTBranchmUgR(void *p = nullptr);
   static void *newArray_unordered_maplEstringcOTBranchmUgR(Long_t size, void *p);
   static void delete_unordered_maplEstringcOTBranchmUgR(void *p);
   static void deleteArray_unordered_maplEstringcOTBranchmUgR(void *p);
   static void destruct_unordered_maplEstringcOTBranchmUgR(void *p);

   // Function generating the singleton type initializer
   static TGenericClassInfo *GenerateInitInstanceLocal(const unordered_map<string,TBranch*>*)
   {
      unordered_map<string,TBranch*> *ptr = nullptr;
      static ::TVirtualIsAProxy* isa_proxy = new ::TIsAProxy(typeid(unordered_map<string,TBranch*>));
      static ::ROOT::TGenericClassInfo 
         instance("unordered_map<string,TBranch*>", -2, "unordered_map", 109,
                  typeid(unordered_map<string,TBranch*>), ::ROOT::Internal::DefineBehavior(ptr, ptr),
                  &unordered_maplEstringcOTBranchmUgR_Dictionary, isa_proxy, 0,
                  sizeof(unordered_map<string,TBranch*>) );
      instance.SetNew(&new_unordered_maplEstringcOTBranchmUgR);
      instance.SetNewArray(&newArray_unordered_maplEstringcOTBranchmUgR);
      instance.SetDelete(&delete_unordered_maplEstringcOTBranchmUgR);
      instance.SetDeleteArray(&deleteArray_unordered_maplEstringcOTBranchmUgR);
      instance.SetDestructor(&destruct_unordered_maplEstringcOTBranchmUgR);
      instance.AdoptCollectionProxyInfo(TCollectionProxyInfo::Generate(TCollectionProxyInfo::MapInsert< unordered_map<string,TBranch*> >()));

      instance.AdoptAlternate(::ROOT::AddClassAlternate("unordered_map<string,TBranch*>","std::unordered_map<std::__cxx11::basic_string<char, std::char_traits<char>, std::allocator<char> >, TBranch*, std::hash<std::__cxx11::basic_string<char, std::char_traits<char>, std::allocator<char> > >, std::equal_to<std::__cxx11::basic_string<char, std::char_traits<char>, std::allocator<char> > >, std::allocator<std::pair<std::__cxx11::basic_string<char, std::char_traits<char>, std::allocator<char> > const, TBranch*> > >"));
      return &instance;
   }
   // Static variable to force the class initialization
   static ::ROOT::TGenericClassInfo *_R__UNIQUE_DICT_(Init) = GenerateInitInstanceLocal(static_cast<const unordered_map<string,TBranch*>*>(nullptr)); R__UseDummy(_R__UNIQUE_DICT_(Init));

   // Dictionary for non-ClassDef classes
   static TClass *unordered_maplEstringcOTBranchmUgR_Dictionary() {
      TClass* theClass =::ROOT::GenerateInitInstanceLocal(static_cast<const unordered_map<string,TBranch*>*>(nullptr))->GetClass();
      unordered_maplEstringcOTBranchmUgR_TClassManip(theClass);
   return theClass;
   }

   static void unordered_maplEstringcOTBranchmUgR_TClassManip(TClass* ){
   }

} // end of namespace ROOT

namespace ROOT {
   // Wrappers around operator new
   static void *new_unordered_maplEstringcOTBranchmUgR(void *p) {
      return  p ? ::new(static_cast<::ROOT::Internal::TOperatorNewHelper*>(p)) unordered_map<string,TBranch*> : new unordered_map<string,TBranch*>;
   }
   static void *newArray_unordered_maplEstringcOTBranchmUgR(Long_t nElements, void *p) {
      return p ? ::new(static_cast<::ROOT::Internal::TOperatorNewHelper*>(p)) unordered_map<string,TBranch*>[nElements] : new unordered_map<string,TBranch*>[nElements];
   }
   // Wrapper around operator delete
   static void delete_unordered_maplEstringcOTBranchmUgR(void *p) {
      delete (static_cast<unordered_map<string,TBranch*>*>(p));
   }
   static void deleteArray_unordered_maplEstringcOTBranchmUgR(void *p) {
      delete [] (static_cast<unordered_map<string,TBranch*>*>(p));
   }
   static void destruct_unordered_maplEstringcOTBranchmUgR(void *p) {
      typedef unordered_map<string,TBranch*> current_t;
      (static_cast<current_t*>(p))->~current_t();
   }
} // end of namespace ROOT for class unordered_map<string,TBranch*>

namespace ROOT {
   static TClass *unordered_maplETTreemUcOunordered_maplEstringcOTBranchmUgRsPgR_Dictionary();
   static void unordered_maplETTreemUcOunordered_maplEstringcOTBranchmUgRsPgR_TClassManip(TClass*);
   static void *new_unordered_maplETTreemUcOunordered_maplEstringcOTBranchmUgRsPgR(void *p = nullptr);
   static void *newArray_unordered_maplETTreemUcOunordered_maplEstringcOTBranchmUgRsPgR(Long_t size, void *p);
   static void delete_unordered_maplETTreemUcOunordered_maplEstringcOTBranchmUgRsPgR(void *p);
   static void deleteArray_unordered_maplETTreemUcOunordered_maplEstringcOTBranchmUgRsPgR(void *p);
   static void destruct_unordered_maplETTreemUcOunordered_maplEstringcOTBranchmUgRsPgR(void *p);

   // Function generating the singleton type initializer
   static TGenericClassInfo *GenerateInitInstanceLocal(const unordered_map<TTree*,unordered_map<string,TBranch*> >*)
   {
      unordered_map<TTree*,unordered_map<string,TBranch*> > *ptr = nullptr;
      static ::TVirtualIsAProxy* isa_proxy = new ::TIsAProxy(typeid(unordered_map<TTree*,unordered_map<string,TBranch*> >));
      static ::ROOT::TGenericClassInfo 
         instance("unordered_map<TTree*,unordered_map<string,TBranch*> >", -2, "unordered_map", 109,
                  typeid(unordered_map<TTree*,unordered_map<string,TBranch*> >), ::ROOT::Internal::DefineBehavior(ptr, ptr),
                  &unordered_maplETTreemUcOunordered_maplEstringcOTBranchmUgRsPgR_Dictionary, isa_proxy, 0,
                  sizeof(unordered_map<TTree*,unordered_map<string,TBranch*> >) );
      instance.SetNew(&new_unordered_maplETTreemUcOunordered_maplEstringcOTBranchmUgRsPgR);
      instance.SetNewArray(&newArray_unordered_maplETTreemUcOunordered_maplEstringcOTBranchmUgRsPgR);
      instance.SetDelete(&delete_unordered_maplETTreemUcOunordered_maplEstringcOTBranchmUgRsPgR);
      instance.SetDeleteArray(&deleteArray_unordered_maplETTreemUcOunordered_maplEstringcOTBranchmUgRsPgR);
      instance.SetDestructor(&destruct_unordered_maplETTreemUcOunordered_maplEstringcOTBranchmUgRsPgR);
      instance.AdoptCollectionProxyInfo(TCollectionProxyInfo::Generate(TCollectionProxyInfo::MapInsert< unordered_map<TTree*,unordered_map<string,TBranch*> > >()));

      instance.AdoptAlternate(::ROOT::AddClassAlternate("unordered_map<TTree*,unordered_map<string,TBranch*> >","std::unordered_map<TTree*, std::unordered_map<std::__cxx11::basic_string<char, std::char_traits<char>, std::allocator<char> >, TBranch*, std::hash<std::__cxx11::basic_string<char, std::char_traits<char>, std::allocator<char> > >, std::equal_to<std::__cxx11::basic_string<char, std::char_traits<char>, std::allocator<char> > >, std::allocator<std::pair<std::__cxx11::basic_string<char, std::char_traits<char>, std::allocator<char> > const, TBranch*> > >, std::hash<TTree*>, std::equal_to<TTree*>, std::allocator<std::pair<TTree* const, std::unordered_map<std::__cxx11::basic_string<char, std::char_traits<char>, std::allocator<char> >, TBranch*, std::hash<std::__cxx11::basic_string<char, std::char_traits<char>, std::allocator<char> > >, std::equal_to<std::__cxx11::basic_string<char, std::char_traits<char>, std::allocator<char> > >, std::allocator<std::pair<std::__cxx11::basic_string<char, std::char_traits<char>, std::allocator<char> > const, TBranch*> > > > > >"));
      return &instance;
   }
   // Static variable to force the class initialization
   static ::ROOT::TGenericClassInfo *_R__UNIQUE_DICT_(Init) = GenerateInitInstanceLocal(static_cast<const unordered_map<TTree*,unordered_map<string,TBranch*> >*>(nullptr)); R__UseDummy(_R__UNIQUE_DICT_(Init));

   // Dictionary for non-ClassDef classes
   static TClass *unordered_maplETTreemUcOunordered_maplEstringcOTBranchmUgRsPgR_Dictionary() {
      TClass* theClass =::ROOT::GenerateInitInstanceLocal(static_cast<const unordered_map<TTree*,unordered_map<string,TBranch*> >*>(nullptr))->GetClass();
      unordered_maplETTreemUcOunordered_maplEstringcOTBranchmUgRsPgR_TClassManip(theClass);
   return theClass;
   }

   static void unordered_maplETTreemUcOunordered_maplEstringcOTBranchmUgRsPgR_TClassManip(TClass* ){
   }

} // end of namespace ROOT

namespace ROOT {
   // Wrappers around operator new
   static void *new_unordered_maplETTreemUcOunordered_maplEstringcOTBranchmUgRsPgR(void *p) {
      return  p ? ::new(static_cast<::ROOT::Internal::TOperatorNewHelper*>(p)) unordered_map<TTree*,unordered_map<string,TBranch*> > : new unordered_map<TTree*,unordered_map<string,TBranch*> >;
   }
   static void *newArray_unordered_maplETTreemUcOunordered_maplEstringcOTBranchmUgRsPgR(Long_t nElements, void *p) {
      return p ? ::new(static_cast<::ROOT::Internal::TOperatorNewHelper*>(p)) unordered_map<TTree*,unordered_map<string,TBranch*> >[nElements] : new unordered_map<TTree*,unordered_map<string,TBranch*> >[nElements];
   }
   // Wrapper around operator delete
   static void delete_unordered_maplETTreemUcOunordered_maplEstringcOTBranchmUgRsPgR(void *p) {
      delete (static_cast<unordered_map<TTree*,unordered_map<string,TBranch*> >*>(p));
   }
   static void deleteArray_unordered_maplETTreemUcOunordered_maplEstringcOTBranchmUgRsPgR(void *p) {
      delete [] (static_cast<unordered_map<TTree*,unordered_map<string,TBranch*> >*>(p));
   }
   static void destruct_unordered_maplETTreemUcOunordered_maplEstringcOTBranchmUgRsPgR(void *p) {
      typedef unordered_map<TTree*,unordered_map<string,TBranch*> > current_t;
      (static_cast<current_t*>(p))->~current_t();
   }
} // end of namespace ROOT for class unordered_map<TTree*,unordered_map<string,TBranch*> >

namespace ROOT {
   static TClass *maplETStringcOpairlEboolmUcOfloatgRsPgR_Dictionary();
   static void maplETStringcOpairlEboolmUcOfloatgRsPgR_TClassManip(TClass*);
   static void *new_maplETStringcOpairlEboolmUcOfloatgRsPgR(void *p = nullptr);
   static void *newArray_maplETStringcOpairlEboolmUcOfloatgRsPgR(Long_t size, void *p);
   static void delete_maplETStringcOpairlEboolmUcOfloatgRsPgR(void *p);
   static void deleteArray_maplETStringcOpairlEboolmUcOfloatgRsPgR(void *p);
   static void destruct_maplETStringcOpairlEboolmUcOfloatgRsPgR(void *p);

   // Function generating the singleton type initializer
   static TGenericClassInfo *GenerateInitInstanceLocal(const map<TString,pair<bool*,float> >*)
   {
      map<TString,pair<bool*,float> > *ptr = nullptr;
      static ::TVirtualIsAProxy* isa_proxy = new ::TIsAProxy(typeid(map<TString,pair<bool*,float> >));
      static ::ROOT::TGenericClassInfo 
         instance("map<TString,pair<bool*,float> >", -2, "map", 102,
                  typeid(map<TString,pair<bool*,float> >), ::ROOT::Internal::DefineBehavior(ptr, ptr),
                  &maplETStringcOpairlEboolmUcOfloatgRsPgR_Dictionary, isa_proxy, 0,
                  sizeof(map<TString,pair<bool*,float> >) );
      instance.SetNew(&new_maplETStringcOpairlEboolmUcOfloatgRsPgR);
      instance.SetNewArray(&newArray_maplETStringcOpairlEboolmUcOfloatgRsPgR);
      instance.SetDelete(&delete_maplETStringcOpairlEboolmUcOfloatgRsPgR);
      instance.SetDeleteArray(&deleteArray_maplETStringcOpairlEboolmUcOfloatgRsPgR);
      instance.SetDestructor(&destruct_maplETStringcOpairlEboolmUcOfloatgRsPgR);
      instance.AdoptCollectionProxyInfo(TCollectionProxyInfo::Generate(TCollectionProxyInfo::MapInsert< map<TString,pair<bool*,float> > >()));

      instance.AdoptAlternate(::ROOT::AddClassAlternate("map<TString,pair<bool*,float> >","std::map<TString, std::pair<bool*, float>, std::less<TString>, std::allocator<std::pair<TString const, std::pair<bool*, float> > > >"));
      return &instance;
   }
   // Static variable to force the class initialization
   static ::ROOT::TGenericClassInfo *_R__UNIQUE_DICT_(Init) = GenerateInitInstanceLocal(static_cast<const map<TString,pair<bool*,float> >*>(nullptr)); R__UseDummy(_R__UNIQUE_DICT_(Init));

   // Dictionary for non-ClassDef classes
   static TClass *maplETStringcOpairlEboolmUcOfloatgRsPgR_Dictionary() {
      TClass* theClass =::ROOT::GenerateInitInstanceLocal(static_cast<const map<TString,pair<bool*,float> >*>(nullptr))->GetClass();
      maplETStringcOpairlEboolmUcOfloatgRsPgR_TClassManip(theClass);
   return theClass;
   }

   static void maplETStringcOpairlEboolmUcOfloatgRsPgR_TClassManip(TClass* ){
   }

} // end of namespace ROOT

namespace ROOT {
   // Wrappers around operator new
   static void *new_maplETStringcOpairlEboolmUcOfloatgRsPgR(void *p) {
      return  p ? ::new(static_cast<::ROOT::Internal::TOperatorNewHelper*>(p)) map<TString,pair<bool*,float> > : new map<TString,pair<bool*,float> >;
   }
   static void *newArray_maplETStringcOpairlEboolmUcOfloatgRsPgR(Long_t nElements, void *p) {
      return p ? ::new(static_cast<::ROOT::Internal::TOperatorNewHelper*>(p)) map<TString,pair<bool*,float> >[nElements] : new map<TString,pair<bool*,float> >[nElements];
   }
   // Wrapper around operator delete
   static void delete_maplETStringcOpairlEboolmUcOfloatgRsPgR(void *p) {
      delete (static_cast<map<TString,pair<bool*,float> >*>(p));
   }
   static void deleteArray_maplETStringcOpairlEboolmUcOfloatgRsPgR(void *p) {
      delete [] (static_cast<map<TString,pair<bool*,float> >*>(p));
   }
   static void destruct_maplETStringcOpairlEboolmUcOfloatgRsPgR(void *p) {
      typedef map<TString,pair<bool*,float> > current_t;
      (static_cast<current_t*>(p))->~current_t();
   }
} // end of namespace ROOT for class map<TString,pair<bool*,float> >

namespace ROOT {
   static TClass *maplEJetTaggingcLcLJetFlavTaggerWPcOboolgR_Dictionary();
   static void maplEJetTaggingcLcLJetFlavTaggerWPcOboolgR_TClassManip(TClass*);
   static void *new_maplEJetTaggingcLcLJetFlavTaggerWPcOboolgR(void *p = nullptr);
   static void *newArray_maplEJetTaggingcLcLJetFlavTaggerWPcOboolgR(Long_t size, void *p);
   static void delete_maplEJetTaggingcLcLJetFlavTaggerWPcOboolgR(void *p);
   static void deleteArray_maplEJetTaggingcLcLJetFlavTaggerWPcOboolgR(void *p);
   static void destruct_maplEJetTaggingcLcLJetFlavTaggerWPcOboolgR(void *p);

   // Function generating the singleton type initializer
   static TGenericClassInfo *GenerateInitInstanceLocal(const map<JetTagging::JetFlavTaggerWP,bool>*)
   {
      map<JetTagging::JetFlavTaggerWP,bool> *ptr = nullptr;
      static ::TVirtualIsAProxy* isa_proxy = new ::TIsAProxy(typeid(map<JetTagging::JetFlavTaggerWP,bool>));
      static ::ROOT::TGenericClassInfo 
         instance("map<JetTagging::JetFlavTaggerWP,bool>", -2, "map", 102,
                  typeid(map<JetTagging::JetFlavTaggerWP,bool>), ::ROOT::Internal::DefineBehavior(ptr, ptr),
                  &maplEJetTaggingcLcLJetFlavTaggerWPcOboolgR_Dictionary, isa_proxy, 0,
                  sizeof(map<JetTagging::JetFlavTaggerWP,bool>) );
      instance.SetNew(&new_maplEJetTaggingcLcLJetFlavTaggerWPcOboolgR);
      instance.SetNewArray(&newArray_maplEJetTaggingcLcLJetFlavTaggerWPcOboolgR);
      instance.SetDelete(&delete_maplEJetTaggingcLcLJetFlavTaggerWPcOboolgR);
      instance.SetDeleteArray(&deleteArray_maplEJetTaggingcLcLJetFlavTaggerWPcOboolgR);
      instance.SetDestructor(&destruct_maplEJetTaggingcLcLJetFlavTaggerWPcOboolgR);
      instance.AdoptCollectionProxyInfo(TCollectionProxyInfo::Generate(TCollectionProxyInfo::MapInsert< map<JetTagging::JetFlavTaggerWP,bool> >()));

      instance.AdoptAlternate(::ROOT::AddClassAlternate("map<JetTagging::JetFlavTaggerWP,bool>","std::map<JetTagging::JetFlavTaggerWP, bool, std::less<JetTagging::JetFlavTaggerWP>, std::allocator<std::pair<JetTagging::JetFlavTaggerWP const, bool> > >"));
      return &instance;
   }
   // Static variable to force the class initialization
   static ::ROOT::TGenericClassInfo *_R__UNIQUE_DICT_(Init) = GenerateInitInstanceLocal(static_cast<const map<JetTagging::JetFlavTaggerWP,bool>*>(nullptr)); R__UseDummy(_R__UNIQUE_DICT_(Init));

   // Dictionary for non-ClassDef classes
   static TClass *maplEJetTaggingcLcLJetFlavTaggerWPcOboolgR_Dictionary() {
      TClass* theClass =::ROOT::GenerateInitInstanceLocal(static_cast<const map<JetTagging::JetFlavTaggerWP,bool>*>(nullptr))->GetClass();
      maplEJetTaggingcLcLJetFlavTaggerWPcOboolgR_TClassManip(theClass);
   return theClass;
   }

   static void maplEJetTaggingcLcLJetFlavTaggerWPcOboolgR_TClassManip(TClass* ){
   }

} // end of namespace ROOT

namespace ROOT {
   // Wrappers around operator new
   static void *new_maplEJetTaggingcLcLJetFlavTaggerWPcOboolgR(void *p) {
      return  p ? ::new(static_cast<::ROOT::Internal::TOperatorNewHelper*>(p)) map<JetTagging::JetFlavTaggerWP,bool> : new map<JetTagging::JetFlavTaggerWP,bool>;
   }
   static void *newArray_maplEJetTaggingcLcLJetFlavTaggerWPcOboolgR(Long_t nElements, void *p) {
      return p ? ::new(static_cast<::ROOT::Internal::TOperatorNewHelper*>(p)) map<JetTagging::JetFlavTaggerWP,bool>[nElements] : new map<JetTagging::JetFlavTaggerWP,bool>[nElements];
   }
   // Wrapper around operator delete
   static void delete_maplEJetTaggingcLcLJetFlavTaggerWPcOboolgR(void *p) {
      delete (static_cast<map<JetTagging::JetFlavTaggerWP,bool>*>(p));
   }
   static void deleteArray_maplEJetTaggingcLcLJetFlavTaggerWPcOboolgR(void *p) {
      delete [] (static_cast<map<JetTagging::JetFlavTaggerWP,bool>*>(p));
   }
   static void destruct_maplEJetTaggingcLcLJetFlavTaggerWPcOboolgR(void *p) {
      typedef map<JetTagging::JetFlavTaggerWP,bool> current_t;
      (static_cast<current_t*>(p))->~current_t();
   }
} // end of namespace ROOT for class map<JetTagging::JetFlavTaggerWP,bool>

namespace ROOT {
   static TClass *dequelEintgR_Dictionary();
   static void dequelEintgR_TClassManip(TClass*);
   static void *new_dequelEintgR(void *p = nullptr);
   static void *newArray_dequelEintgR(Long_t size, void *p);
   static void delete_dequelEintgR(void *p);
   static void deleteArray_dequelEintgR(void *p);
   static void destruct_dequelEintgR(void *p);

   // Function generating the singleton type initializer
   static TGenericClassInfo *GenerateInitInstanceLocal(const deque<int>*)
   {
      deque<int> *ptr = nullptr;
      static ::TVirtualIsAProxy* isa_proxy = new ::TIsAProxy(typeid(deque<int>));
      static ::ROOT::TGenericClassInfo 
         instance("deque<int>", -2, "deque", 788,
                  typeid(deque<int>), ::ROOT::Internal::DefineBehavior(ptr, ptr),
                  &dequelEintgR_Dictionary, isa_proxy, 0,
                  sizeof(deque<int>) );
      instance.SetNew(&new_dequelEintgR);
      instance.SetNewArray(&newArray_dequelEintgR);
      instance.SetDelete(&delete_dequelEintgR);
      instance.SetDeleteArray(&deleteArray_dequelEintgR);
      instance.SetDestructor(&destruct_dequelEintgR);
      instance.AdoptCollectionProxyInfo(TCollectionProxyInfo::Generate(TCollectionProxyInfo::Pushback< deque<int> >()));

      instance.AdoptAlternate(::ROOT::AddClassAlternate("deque<int>","std::deque<int, std::allocator<int> >"));
      return &instance;
   }
   // Static variable to force the class initialization
   static ::ROOT::TGenericClassInfo *_R__UNIQUE_DICT_(Init) = GenerateInitInstanceLocal(static_cast<const deque<int>*>(nullptr)); R__UseDummy(_R__UNIQUE_DICT_(Init));

   // Dictionary for non-ClassDef classes
   static TClass *dequelEintgR_Dictionary() {
      TClass* theClass =::ROOT::GenerateInitInstanceLocal(static_cast<const deque<int>*>(nullptr))->GetClass();
      dequelEintgR_TClassManip(theClass);
   return theClass;
   }

   static void dequelEintgR_TClassManip(TClass* ){
   }

} // end of namespace ROOT

namespace ROOT {
   // Wrappers around operator new
   static void *new_dequelEintgR(void *p) {
      return  p ? ::new(static_cast<::ROOT::Internal::TOperatorNewHelper*>(p)) deque<int> : new deque<int>;
   }
   static void *newArray_dequelEintgR(Long_t nElements, void *p) {
      return p ? ::new(static_cast<::ROOT::Internal::TOperatorNewHelper*>(p)) deque<int>[nElements] : new deque<int>[nElements];
   }
   // Wrapper around operator delete
   static void delete_dequelEintgR(void *p) {
      delete (static_cast<deque<int>*>(p));
   }
   static void deleteArray_dequelEintgR(void *p) {
      delete [] (static_cast<deque<int>*>(p));
   }
   static void destruct_dequelEintgR(void *p) {
      typedef deque<int> current_t;
      (static_cast<current_t*>(p))->~current_t();
   }
} // end of namespace ROOT for class deque<int>

namespace ROOT {
   static TClass *dequelEfloatgR_Dictionary();
   static void dequelEfloatgR_TClassManip(TClass*);
   static void *new_dequelEfloatgR(void *p = nullptr);
   static void *newArray_dequelEfloatgR(Long_t size, void *p);
   static void delete_dequelEfloatgR(void *p);
   static void deleteArray_dequelEfloatgR(void *p);
   static void destruct_dequelEfloatgR(void *p);

   // Function generating the singleton type initializer
   static TGenericClassInfo *GenerateInitInstanceLocal(const deque<float>*)
   {
      deque<float> *ptr = nullptr;
      static ::TVirtualIsAProxy* isa_proxy = new ::TIsAProxy(typeid(deque<float>));
      static ::ROOT::TGenericClassInfo 
         instance("deque<float>", -2, "deque", 788,
                  typeid(deque<float>), ::ROOT::Internal::DefineBehavior(ptr, ptr),
                  &dequelEfloatgR_Dictionary, isa_proxy, 0,
                  sizeof(deque<float>) );
      instance.SetNew(&new_dequelEfloatgR);
      instance.SetNewArray(&newArray_dequelEfloatgR);
      instance.SetDelete(&delete_dequelEfloatgR);
      instance.SetDeleteArray(&deleteArray_dequelEfloatgR);
      instance.SetDestructor(&destruct_dequelEfloatgR);
      instance.AdoptCollectionProxyInfo(TCollectionProxyInfo::Generate(TCollectionProxyInfo::Pushback< deque<float> >()));

      instance.AdoptAlternate(::ROOT::AddClassAlternate("deque<float>","std::deque<float, std::allocator<float> >"));
      return &instance;
   }
   // Static variable to force the class initialization
   static ::ROOT::TGenericClassInfo *_R__UNIQUE_DICT_(Init) = GenerateInitInstanceLocal(static_cast<const deque<float>*>(nullptr)); R__UseDummy(_R__UNIQUE_DICT_(Init));

   // Dictionary for non-ClassDef classes
   static TClass *dequelEfloatgR_Dictionary() {
      TClass* theClass =::ROOT::GenerateInitInstanceLocal(static_cast<const deque<float>*>(nullptr))->GetClass();
      dequelEfloatgR_TClassManip(theClass);
   return theClass;
   }

   static void dequelEfloatgR_TClassManip(TClass* ){
   }

} // end of namespace ROOT

namespace ROOT {
   // Wrappers around operator new
   static void *new_dequelEfloatgR(void *p) {
      return  p ? ::new(static_cast<::ROOT::Internal::TOperatorNewHelper*>(p)) deque<float> : new deque<float>;
   }
   static void *newArray_dequelEfloatgR(Long_t nElements, void *p) {
      return p ? ::new(static_cast<::ROOT::Internal::TOperatorNewHelper*>(p)) deque<float>[nElements] : new deque<float>[nElements];
   }
   // Wrapper around operator delete
   static void delete_dequelEfloatgR(void *p) {
      delete (static_cast<deque<float>*>(p));
   }
   static void deleteArray_dequelEfloatgR(void *p) {
      delete [] (static_cast<deque<float>*>(p));
   }
   static void destruct_dequelEfloatgR(void *p) {
      typedef deque<float> current_t;
      (static_cast<current_t*>(p))->~current_t();
   }
} // end of namespace ROOT for class deque<float>

namespace ROOT {
   static TClass *dequelEchargR_Dictionary();
   static void dequelEchargR_TClassManip(TClass*);
   static void *new_dequelEchargR(void *p = nullptr);
   static void *newArray_dequelEchargR(Long_t size, void *p);
   static void delete_dequelEchargR(void *p);
   static void deleteArray_dequelEchargR(void *p);
   static void destruct_dequelEchargR(void *p);

   // Function generating the singleton type initializer
   static TGenericClassInfo *GenerateInitInstanceLocal(const deque<char>*)
   {
      deque<char> *ptr = nullptr;
      static ::TVirtualIsAProxy* isa_proxy = new ::TIsAProxy(typeid(deque<char>));
      static ::ROOT::TGenericClassInfo 
         instance("deque<char>", -2, "deque", 788,
                  typeid(deque<char>), ::ROOT::Internal::DefineBehavior(ptr, ptr),
                  &dequelEchargR_Dictionary, isa_proxy, 0,
                  sizeof(deque<char>) );
      instance.SetNew(&new_dequelEchargR);
      instance.SetNewArray(&newArray_dequelEchargR);
      instance.SetDelete(&delete_dequelEchargR);
      instance.SetDeleteArray(&deleteArray_dequelEchargR);
      instance.SetDestructor(&destruct_dequelEchargR);
      instance.AdoptCollectionProxyInfo(TCollectionProxyInfo::Generate(TCollectionProxyInfo::Pushback< deque<char> >()));

      instance.AdoptAlternate(::ROOT::AddClassAlternate("deque<char>","std::deque<char, std::allocator<char> >"));
      return &instance;
   }
   // Static variable to force the class initialization
   static ::ROOT::TGenericClassInfo *_R__UNIQUE_DICT_(Init) = GenerateInitInstanceLocal(static_cast<const deque<char>*>(nullptr)); R__UseDummy(_R__UNIQUE_DICT_(Init));

   // Dictionary for non-ClassDef classes
   static TClass *dequelEchargR_Dictionary() {
      TClass* theClass =::ROOT::GenerateInitInstanceLocal(static_cast<const deque<char>*>(nullptr))->GetClass();
      dequelEchargR_TClassManip(theClass);
   return theClass;
   }

   static void dequelEchargR_TClassManip(TClass* ){
   }

} // end of namespace ROOT

namespace ROOT {
   // Wrappers around operator new
   static void *new_dequelEchargR(void *p) {
      return  p ? ::new(static_cast<::ROOT::Internal::TOperatorNewHelper*>(p)) deque<char> : new deque<char>;
   }
   static void *newArray_dequelEchargR(Long_t nElements, void *p) {
      return p ? ::new(static_cast<::ROOT::Internal::TOperatorNewHelper*>(p)) deque<char>[nElements] : new deque<char>[nElements];
   }
   // Wrapper around operator delete
   static void delete_dequelEchargR(void *p) {
      delete (static_cast<deque<char>*>(p));
   }
   static void deleteArray_dequelEchargR(void *p) {
      delete [] (static_cast<deque<char>*>(p));
   }
   static void destruct_dequelEchargR(void *p) {
      typedef deque<char> current_t;
      (static_cast<current_t*>(p))->~current_t();
   }
} // end of namespace ROOT for class deque<char>

namespace ROOT {
   static TClass *ROOTcLcLVecOpscLcLRVeclEunsignedsPshortgR_Dictionary();
   static void ROOTcLcLVecOpscLcLRVeclEunsignedsPshortgR_TClassManip(TClass*);
   static void *new_ROOTcLcLVecOpscLcLRVeclEunsignedsPshortgR(void *p = nullptr);
   static void *newArray_ROOTcLcLVecOpscLcLRVeclEunsignedsPshortgR(Long_t size, void *p);
   static void delete_ROOTcLcLVecOpscLcLRVeclEunsignedsPshortgR(void *p);
   static void deleteArray_ROOTcLcLVecOpscLcLRVeclEunsignedsPshortgR(void *p);
   static void destruct_ROOTcLcLVecOpscLcLRVeclEunsignedsPshortgR(void *p);

   // Function generating the singleton type initializer
   static TGenericClassInfo *GenerateInitInstanceLocal(const ROOT::VecOps::RVec<unsigned short>*)
   {
      ROOT::VecOps::RVec<unsigned short> *ptr = nullptr;
      static ::TVirtualIsAProxy* isa_proxy = new ::TIsAProxy(typeid(ROOT::VecOps::RVec<unsigned short>));
      static ::ROOT::TGenericClassInfo 
         instance("ROOT::VecOps::RVec<unsigned short>", -2, "ROOT/RVec.hxx", 3418,
                  typeid(ROOT::VecOps::RVec<unsigned short>), ::ROOT::Internal::DefineBehavior(ptr, ptr),
                  &ROOTcLcLVecOpscLcLRVeclEunsignedsPshortgR_Dictionary, isa_proxy, 0,
                  sizeof(ROOT::VecOps::RVec<unsigned short>) );
      instance.SetNew(&new_ROOTcLcLVecOpscLcLRVeclEunsignedsPshortgR);
      instance.SetNewArray(&newArray_ROOTcLcLVecOpscLcLRVeclEunsignedsPshortgR);
      instance.SetDelete(&delete_ROOTcLcLVecOpscLcLRVeclEunsignedsPshortgR);
      instance.SetDeleteArray(&deleteArray_ROOTcLcLVecOpscLcLRVeclEunsignedsPshortgR);
      instance.SetDestructor(&destruct_ROOTcLcLVecOpscLcLRVeclEunsignedsPshortgR);
      instance.AdoptCollectionProxyInfo(TCollectionProxyInfo::Generate(TCollectionProxyInfo::Pushback< ROOT::VecOps::RVec<unsigned short> >()));
      return &instance;
   }
   TGenericClassInfo *GenerateInitInstance(const ROOT::VecOps::RVec<unsigned short>*)
   {
      return GenerateInitInstanceLocal(static_cast<ROOT::VecOps::RVec<unsigned short>*>(nullptr));
   }
   // Static variable to force the class initialization
   static ::ROOT::TGenericClassInfo *_R__UNIQUE_DICT_(Init) = GenerateInitInstanceLocal(static_cast<const ROOT::VecOps::RVec<unsigned short>*>(nullptr)); R__UseDummy(_R__UNIQUE_DICT_(Init));

   // Dictionary for non-ClassDef classes
   static TClass *ROOTcLcLVecOpscLcLRVeclEunsignedsPshortgR_Dictionary() {
      TClass* theClass =::ROOT::GenerateInitInstanceLocal(static_cast<const ROOT::VecOps::RVec<unsigned short>*>(nullptr))->GetClass();
      ROOTcLcLVecOpscLcLRVeclEunsignedsPshortgR_TClassManip(theClass);
   return theClass;
   }

   static void ROOTcLcLVecOpscLcLRVeclEunsignedsPshortgR_TClassManip(TClass* ){
   }

} // end of namespace ROOT

namespace ROOT {
   // Wrappers around operator new
   static void *new_ROOTcLcLVecOpscLcLRVeclEunsignedsPshortgR(void *p) {
      return  p ? ::new(static_cast<::ROOT::Internal::TOperatorNewHelper*>(p)) ROOT::VecOps::RVec<unsigned short> : new ROOT::VecOps::RVec<unsigned short>;
   }
   static void *newArray_ROOTcLcLVecOpscLcLRVeclEunsignedsPshortgR(Long_t nElements, void *p) {
      return p ? ::new(static_cast<::ROOT::Internal::TOperatorNewHelper*>(p)) ROOT::VecOps::RVec<unsigned short>[nElements] : new ROOT::VecOps::RVec<unsigned short>[nElements];
   }
   // Wrapper around operator delete
   static void delete_ROOTcLcLVecOpscLcLRVeclEunsignedsPshortgR(void *p) {
      delete (static_cast<ROOT::VecOps::RVec<unsigned short>*>(p));
   }
   static void deleteArray_ROOTcLcLVecOpscLcLRVeclEunsignedsPshortgR(void *p) {
      delete [] (static_cast<ROOT::VecOps::RVec<unsigned short>*>(p));
   }
   static void destruct_ROOTcLcLVecOpscLcLRVeclEunsignedsPshortgR(void *p) {
      typedef ROOT::VecOps::RVec<unsigned short> current_t;
      (static_cast<current_t*>(p))->~current_t();
   }
} // end of namespace ROOT for class ROOT::VecOps::RVec<unsigned short>

namespace ROOT {
   static TClass *ROOTcLcLVecOpscLcLRVeclEunsignedsPintgR_Dictionary();
   static void ROOTcLcLVecOpscLcLRVeclEunsignedsPintgR_TClassManip(TClass*);
   static void *new_ROOTcLcLVecOpscLcLRVeclEunsignedsPintgR(void *p = nullptr);
   static void *newArray_ROOTcLcLVecOpscLcLRVeclEunsignedsPintgR(Long_t size, void *p);
   static void delete_ROOTcLcLVecOpscLcLRVeclEunsignedsPintgR(void *p);
   static void deleteArray_ROOTcLcLVecOpscLcLRVeclEunsignedsPintgR(void *p);
   static void destruct_ROOTcLcLVecOpscLcLRVeclEunsignedsPintgR(void *p);

   // Function generating the singleton type initializer
   static TGenericClassInfo *GenerateInitInstanceLocal(const ROOT::VecOps::RVec<unsigned int>*)
   {
      ROOT::VecOps::RVec<unsigned int> *ptr = nullptr;
      static ::TVirtualIsAProxy* isa_proxy = new ::TIsAProxy(typeid(ROOT::VecOps::RVec<unsigned int>));
      static ::ROOT::TGenericClassInfo 
         instance("ROOT::VecOps::RVec<unsigned int>", -2, "ROOT/RVec.hxx", 3419,
                  typeid(ROOT::VecOps::RVec<unsigned int>), ::ROOT::Internal::DefineBehavior(ptr, ptr),
                  &ROOTcLcLVecOpscLcLRVeclEunsignedsPintgR_Dictionary, isa_proxy, 0,
                  sizeof(ROOT::VecOps::RVec<unsigned int>) );
      instance.SetNew(&new_ROOTcLcLVecOpscLcLRVeclEunsignedsPintgR);
      instance.SetNewArray(&newArray_ROOTcLcLVecOpscLcLRVeclEunsignedsPintgR);
      instance.SetDelete(&delete_ROOTcLcLVecOpscLcLRVeclEunsignedsPintgR);
      instance.SetDeleteArray(&deleteArray_ROOTcLcLVecOpscLcLRVeclEunsignedsPintgR);
      instance.SetDestructor(&destruct_ROOTcLcLVecOpscLcLRVeclEunsignedsPintgR);
      instance.AdoptCollectionProxyInfo(TCollectionProxyInfo::Generate(TCollectionProxyInfo::Pushback< ROOT::VecOps::RVec<unsigned int> >()));
      return &instance;
   }
   TGenericClassInfo *GenerateInitInstance(const ROOT::VecOps::RVec<unsigned int>*)
   {
      return GenerateInitInstanceLocal(static_cast<ROOT::VecOps::RVec<unsigned int>*>(nullptr));
   }
   // Static variable to force the class initialization
   static ::ROOT::TGenericClassInfo *_R__UNIQUE_DICT_(Init) = GenerateInitInstanceLocal(static_cast<const ROOT::VecOps::RVec<unsigned int>*>(nullptr)); R__UseDummy(_R__UNIQUE_DICT_(Init));

   // Dictionary for non-ClassDef classes
   static TClass *ROOTcLcLVecOpscLcLRVeclEunsignedsPintgR_Dictionary() {
      TClass* theClass =::ROOT::GenerateInitInstanceLocal(static_cast<const ROOT::VecOps::RVec<unsigned int>*>(nullptr))->GetClass();
      ROOTcLcLVecOpscLcLRVeclEunsignedsPintgR_TClassManip(theClass);
   return theClass;
   }

   static void ROOTcLcLVecOpscLcLRVeclEunsignedsPintgR_TClassManip(TClass* ){
   }

} // end of namespace ROOT

namespace ROOT {
   // Wrappers around operator new
   static void *new_ROOTcLcLVecOpscLcLRVeclEunsignedsPintgR(void *p) {
      return  p ? ::new(static_cast<::ROOT::Internal::TOperatorNewHelper*>(p)) ROOT::VecOps::RVec<unsigned int> : new ROOT::VecOps::RVec<unsigned int>;
   }
   static void *newArray_ROOTcLcLVecOpscLcLRVeclEunsignedsPintgR(Long_t nElements, void *p) {
      return p ? ::new(static_cast<::ROOT::Internal::TOperatorNewHelper*>(p)) ROOT::VecOps::RVec<unsigned int>[nElements] : new ROOT::VecOps::RVec<unsigned int>[nElements];
   }
   // Wrapper around operator delete
   static void delete_ROOTcLcLVecOpscLcLRVeclEunsignedsPintgR(void *p) {
      delete (static_cast<ROOT::VecOps::RVec<unsigned int>*>(p));
   }
   static void deleteArray_ROOTcLcLVecOpscLcLRVeclEunsignedsPintgR(void *p) {
      delete [] (static_cast<ROOT::VecOps::RVec<unsigned int>*>(p));
   }
   static void destruct_ROOTcLcLVecOpscLcLRVeclEunsignedsPintgR(void *p) {
      typedef ROOT::VecOps::RVec<unsigned int> current_t;
      (static_cast<current_t*>(p))->~current_t();
   }
} // end of namespace ROOT for class ROOT::VecOps::RVec<unsigned int>

namespace ROOT {
   static TClass *ROOTcLcLVecOpscLcLRVeclEunsignedsPchargR_Dictionary();
   static void ROOTcLcLVecOpscLcLRVeclEunsignedsPchargR_TClassManip(TClass*);
   static void *new_ROOTcLcLVecOpscLcLRVeclEunsignedsPchargR(void *p = nullptr);
   static void *newArray_ROOTcLcLVecOpscLcLRVeclEunsignedsPchargR(Long_t size, void *p);
   static void delete_ROOTcLcLVecOpscLcLRVeclEunsignedsPchargR(void *p);
   static void deleteArray_ROOTcLcLVecOpscLcLRVeclEunsignedsPchargR(void *p);
   static void destruct_ROOTcLcLVecOpscLcLRVeclEunsignedsPchargR(void *p);

   // Function generating the singleton type initializer
   static TGenericClassInfo *GenerateInitInstanceLocal(const ROOT::VecOps::RVec<unsigned char>*)
   {
      ROOT::VecOps::RVec<unsigned char> *ptr = nullptr;
      static ::TVirtualIsAProxy* isa_proxy = new ::TIsAProxy(typeid(ROOT::VecOps::RVec<unsigned char>));
      static ::ROOT::TGenericClassInfo 
         instance("ROOT::VecOps::RVec<unsigned char>", -2, "ROOT/RVec.hxx", 3417,
                  typeid(ROOT::VecOps::RVec<unsigned char>), ::ROOT::Internal::DefineBehavior(ptr, ptr),
                  &ROOTcLcLVecOpscLcLRVeclEunsignedsPchargR_Dictionary, isa_proxy, 0,
                  sizeof(ROOT::VecOps::RVec<unsigned char>) );
      instance.SetNew(&new_ROOTcLcLVecOpscLcLRVeclEunsignedsPchargR);
      instance.SetNewArray(&newArray_ROOTcLcLVecOpscLcLRVeclEunsignedsPchargR);
      instance.SetDelete(&delete_ROOTcLcLVecOpscLcLRVeclEunsignedsPchargR);
      instance.SetDeleteArray(&deleteArray_ROOTcLcLVecOpscLcLRVeclEunsignedsPchargR);
      instance.SetDestructor(&destruct_ROOTcLcLVecOpscLcLRVeclEunsignedsPchargR);
      instance.AdoptCollectionProxyInfo(TCollectionProxyInfo::Generate(TCollectionProxyInfo::Pushback< ROOT::VecOps::RVec<unsigned char> >()));
      return &instance;
   }
   TGenericClassInfo *GenerateInitInstance(const ROOT::VecOps::RVec<unsigned char>*)
   {
      return GenerateInitInstanceLocal(static_cast<ROOT::VecOps::RVec<unsigned char>*>(nullptr));
   }
   // Static variable to force the class initialization
   static ::ROOT::TGenericClassInfo *_R__UNIQUE_DICT_(Init) = GenerateInitInstanceLocal(static_cast<const ROOT::VecOps::RVec<unsigned char>*>(nullptr)); R__UseDummy(_R__UNIQUE_DICT_(Init));

   // Dictionary for non-ClassDef classes
   static TClass *ROOTcLcLVecOpscLcLRVeclEunsignedsPchargR_Dictionary() {
      TClass* theClass =::ROOT::GenerateInitInstanceLocal(static_cast<const ROOT::VecOps::RVec<unsigned char>*>(nullptr))->GetClass();
      ROOTcLcLVecOpscLcLRVeclEunsignedsPchargR_TClassManip(theClass);
   return theClass;
   }

   static void ROOTcLcLVecOpscLcLRVeclEunsignedsPchargR_TClassManip(TClass* ){
   }

} // end of namespace ROOT

namespace ROOT {
   // Wrappers around operator new
   static void *new_ROOTcLcLVecOpscLcLRVeclEunsignedsPchargR(void *p) {
      return  p ? ::new(static_cast<::ROOT::Internal::TOperatorNewHelper*>(p)) ROOT::VecOps::RVec<unsigned char> : new ROOT::VecOps::RVec<unsigned char>;
   }
   static void *newArray_ROOTcLcLVecOpscLcLRVeclEunsignedsPchargR(Long_t nElements, void *p) {
      return p ? ::new(static_cast<::ROOT::Internal::TOperatorNewHelper*>(p)) ROOT::VecOps::RVec<unsigned char>[nElements] : new ROOT::VecOps::RVec<unsigned char>[nElements];
   }
   // Wrapper around operator delete
   static void delete_ROOTcLcLVecOpscLcLRVeclEunsignedsPchargR(void *p) {
      delete (static_cast<ROOT::VecOps::RVec<unsigned char>*>(p));
   }
   static void deleteArray_ROOTcLcLVecOpscLcLRVeclEunsignedsPchargR(void *p) {
      delete [] (static_cast<ROOT::VecOps::RVec<unsigned char>*>(p));
   }
   static void destruct_ROOTcLcLVecOpscLcLRVeclEunsignedsPchargR(void *p) {
      typedef ROOT::VecOps::RVec<unsigned char> current_t;
      (static_cast<current_t*>(p))->~current_t();
   }
} // end of namespace ROOT for class ROOT::VecOps::RVec<unsigned char>

namespace ROOT {
   static TClass *ROOTcLcLVecOpscLcLRVeclEshortgR_Dictionary();
   static void ROOTcLcLVecOpscLcLRVeclEshortgR_TClassManip(TClass*);
   static void *new_ROOTcLcLVecOpscLcLRVeclEshortgR(void *p = nullptr);
   static void *newArray_ROOTcLcLVecOpscLcLRVeclEshortgR(Long_t size, void *p);
   static void delete_ROOTcLcLVecOpscLcLRVeclEshortgR(void *p);
   static void deleteArray_ROOTcLcLVecOpscLcLRVeclEshortgR(void *p);
   static void destruct_ROOTcLcLVecOpscLcLRVeclEshortgR(void *p);

   // Function generating the singleton type initializer
   static TGenericClassInfo *GenerateInitInstanceLocal(const ROOT::VecOps::RVec<short>*)
   {
      ROOT::VecOps::RVec<short> *ptr = nullptr;
      static ::TVirtualIsAProxy* isa_proxy = new ::TIsAProxy(typeid(ROOT::VecOps::RVec<short>));
      static ::ROOT::TGenericClassInfo 
         instance("ROOT::VecOps::RVec<short>", -2, "ROOT/RVec.hxx", 3412,
                  typeid(ROOT::VecOps::RVec<short>), ::ROOT::Internal::DefineBehavior(ptr, ptr),
                  &ROOTcLcLVecOpscLcLRVeclEshortgR_Dictionary, isa_proxy, 0,
                  sizeof(ROOT::VecOps::RVec<short>) );
      instance.SetNew(&new_ROOTcLcLVecOpscLcLRVeclEshortgR);
      instance.SetNewArray(&newArray_ROOTcLcLVecOpscLcLRVeclEshortgR);
      instance.SetDelete(&delete_ROOTcLcLVecOpscLcLRVeclEshortgR);
      instance.SetDeleteArray(&deleteArray_ROOTcLcLVecOpscLcLRVeclEshortgR);
      instance.SetDestructor(&destruct_ROOTcLcLVecOpscLcLRVeclEshortgR);
      instance.AdoptCollectionProxyInfo(TCollectionProxyInfo::Generate(TCollectionProxyInfo::Pushback< ROOT::VecOps::RVec<short> >()));
      return &instance;
   }
   TGenericClassInfo *GenerateInitInstance(const ROOT::VecOps::RVec<short>*)
   {
      return GenerateInitInstanceLocal(static_cast<ROOT::VecOps::RVec<short>*>(nullptr));
   }
   // Static variable to force the class initialization
   static ::ROOT::TGenericClassInfo *_R__UNIQUE_DICT_(Init) = GenerateInitInstanceLocal(static_cast<const ROOT::VecOps::RVec<short>*>(nullptr)); R__UseDummy(_R__UNIQUE_DICT_(Init));

   // Dictionary for non-ClassDef classes
   static TClass *ROOTcLcLVecOpscLcLRVeclEshortgR_Dictionary() {
      TClass* theClass =::ROOT::GenerateInitInstanceLocal(static_cast<const ROOT::VecOps::RVec<short>*>(nullptr))->GetClass();
      ROOTcLcLVecOpscLcLRVeclEshortgR_TClassManip(theClass);
   return theClass;
   }

   static void ROOTcLcLVecOpscLcLRVeclEshortgR_TClassManip(TClass* ){
   }

} // end of namespace ROOT

namespace ROOT {
   // Wrappers around operator new
   static void *new_ROOTcLcLVecOpscLcLRVeclEshortgR(void *p) {
      return  p ? ::new(static_cast<::ROOT::Internal::TOperatorNewHelper*>(p)) ROOT::VecOps::RVec<short> : new ROOT::VecOps::RVec<short>;
   }
   static void *newArray_ROOTcLcLVecOpscLcLRVeclEshortgR(Long_t nElements, void *p) {
      return p ? ::new(static_cast<::ROOT::Internal::TOperatorNewHelper*>(p)) ROOT::VecOps::RVec<short>[nElements] : new ROOT::VecOps::RVec<short>[nElements];
   }
   // Wrapper around operator delete
   static void delete_ROOTcLcLVecOpscLcLRVeclEshortgR(void *p) {
      delete (static_cast<ROOT::VecOps::RVec<short>*>(p));
   }
   static void deleteArray_ROOTcLcLVecOpscLcLRVeclEshortgR(void *p) {
      delete [] (static_cast<ROOT::VecOps::RVec<short>*>(p));
   }
   static void destruct_ROOTcLcLVecOpscLcLRVeclEshortgR(void *p) {
      typedef ROOT::VecOps::RVec<short> current_t;
      (static_cast<current_t*>(p))->~current_t();
   }
} // end of namespace ROOT for class ROOT::VecOps::RVec<short>

namespace ROOT {
   static TClass *ROOTcLcLVecOpscLcLRVeclEintgR_Dictionary();
   static void ROOTcLcLVecOpscLcLRVeclEintgR_TClassManip(TClass*);
   static void *new_ROOTcLcLVecOpscLcLRVeclEintgR(void *p = nullptr);
   static void *newArray_ROOTcLcLVecOpscLcLRVeclEintgR(Long_t size, void *p);
   static void delete_ROOTcLcLVecOpscLcLRVeclEintgR(void *p);
   static void deleteArray_ROOTcLcLVecOpscLcLRVeclEintgR(void *p);
   static void destruct_ROOTcLcLVecOpscLcLRVeclEintgR(void *p);

   // Function generating the singleton type initializer
   static TGenericClassInfo *GenerateInitInstanceLocal(const ROOT::VecOps::RVec<int>*)
   {
      ROOT::VecOps::RVec<int> *ptr = nullptr;
      static ::TVirtualIsAProxy* isa_proxy = new ::TIsAProxy(typeid(ROOT::VecOps::RVec<int>));
      static ::ROOT::TGenericClassInfo 
         instance("ROOT::VecOps::RVec<int>", -2, "ROOT/RVec.hxx", 3413,
                  typeid(ROOT::VecOps::RVec<int>), ::ROOT::Internal::DefineBehavior(ptr, ptr),
                  &ROOTcLcLVecOpscLcLRVeclEintgR_Dictionary, isa_proxy, 0,
                  sizeof(ROOT::VecOps::RVec<int>) );
      instance.SetNew(&new_ROOTcLcLVecOpscLcLRVeclEintgR);
      instance.SetNewArray(&newArray_ROOTcLcLVecOpscLcLRVeclEintgR);
      instance.SetDelete(&delete_ROOTcLcLVecOpscLcLRVeclEintgR);
      instance.SetDeleteArray(&deleteArray_ROOTcLcLVecOpscLcLRVeclEintgR);
      instance.SetDestructor(&destruct_ROOTcLcLVecOpscLcLRVeclEintgR);
      instance.AdoptCollectionProxyInfo(TCollectionProxyInfo::Generate(TCollectionProxyInfo::Pushback< ROOT::VecOps::RVec<int> >()));
      return &instance;
   }
   TGenericClassInfo *GenerateInitInstance(const ROOT::VecOps::RVec<int>*)
   {
      return GenerateInitInstanceLocal(static_cast<ROOT::VecOps::RVec<int>*>(nullptr));
   }
   // Static variable to force the class initialization
   static ::ROOT::TGenericClassInfo *_R__UNIQUE_DICT_(Init) = GenerateInitInstanceLocal(static_cast<const ROOT::VecOps::RVec<int>*>(nullptr)); R__UseDummy(_R__UNIQUE_DICT_(Init));

   // Dictionary for non-ClassDef classes
   static TClass *ROOTcLcLVecOpscLcLRVeclEintgR_Dictionary() {
      TClass* theClass =::ROOT::GenerateInitInstanceLocal(static_cast<const ROOT::VecOps::RVec<int>*>(nullptr))->GetClass();
      ROOTcLcLVecOpscLcLRVeclEintgR_TClassManip(theClass);
   return theClass;
   }

   static void ROOTcLcLVecOpscLcLRVeclEintgR_TClassManip(TClass* ){
   }

} // end of namespace ROOT

namespace ROOT {
   // Wrappers around operator new
   static void *new_ROOTcLcLVecOpscLcLRVeclEintgR(void *p) {
      return  p ? ::new(static_cast<::ROOT::Internal::TOperatorNewHelper*>(p)) ROOT::VecOps::RVec<int> : new ROOT::VecOps::RVec<int>;
   }
   static void *newArray_ROOTcLcLVecOpscLcLRVeclEintgR(Long_t nElements, void *p) {
      return p ? ::new(static_cast<::ROOT::Internal::TOperatorNewHelper*>(p)) ROOT::VecOps::RVec<int>[nElements] : new ROOT::VecOps::RVec<int>[nElements];
   }
   // Wrapper around operator delete
   static void delete_ROOTcLcLVecOpscLcLRVeclEintgR(void *p) {
      delete (static_cast<ROOT::VecOps::RVec<int>*>(p));
   }
   static void deleteArray_ROOTcLcLVecOpscLcLRVeclEintgR(void *p) {
      delete [] (static_cast<ROOT::VecOps::RVec<int>*>(p));
   }
   static void destruct_ROOTcLcLVecOpscLcLRVeclEintgR(void *p) {
      typedef ROOT::VecOps::RVec<int> current_t;
      (static_cast<current_t*>(p))->~current_t();
   }
} // end of namespace ROOT for class ROOT::VecOps::RVec<int>

namespace ROOT {
   static TClass *ROOTcLcLVecOpscLcLRVeclEfloatgR_Dictionary();
   static void ROOTcLcLVecOpscLcLRVeclEfloatgR_TClassManip(TClass*);
   static void *new_ROOTcLcLVecOpscLcLRVeclEfloatgR(void *p = nullptr);
   static void *newArray_ROOTcLcLVecOpscLcLRVeclEfloatgR(Long_t size, void *p);
   static void delete_ROOTcLcLVecOpscLcLRVeclEfloatgR(void *p);
   static void deleteArray_ROOTcLcLVecOpscLcLRVeclEfloatgR(void *p);
   static void destruct_ROOTcLcLVecOpscLcLRVeclEfloatgR(void *p);

   // Function generating the singleton type initializer
   static TGenericClassInfo *GenerateInitInstanceLocal(const ROOT::VecOps::RVec<float>*)
   {
      ROOT::VecOps::RVec<float> *ptr = nullptr;
      static ::TVirtualIsAProxy* isa_proxy = new ::TIsAProxy(typeid(ROOT::VecOps::RVec<float>));
      static ::ROOT::TGenericClassInfo 
         instance("ROOT::VecOps::RVec<float>", -2, "ROOT/RVec.hxx", 3423,
                  typeid(ROOT::VecOps::RVec<float>), ::ROOT::Internal::DefineBehavior(ptr, ptr),
                  &ROOTcLcLVecOpscLcLRVeclEfloatgR_Dictionary, isa_proxy, 0,
                  sizeof(ROOT::VecOps::RVec<float>) );
      instance.SetNew(&new_ROOTcLcLVecOpscLcLRVeclEfloatgR);
      instance.SetNewArray(&newArray_ROOTcLcLVecOpscLcLRVeclEfloatgR);
      instance.SetDelete(&delete_ROOTcLcLVecOpscLcLRVeclEfloatgR);
      instance.SetDeleteArray(&deleteArray_ROOTcLcLVecOpscLcLRVeclEfloatgR);
      instance.SetDestructor(&destruct_ROOTcLcLVecOpscLcLRVeclEfloatgR);
      instance.AdoptCollectionProxyInfo(TCollectionProxyInfo::Generate(TCollectionProxyInfo::Pushback< ROOT::VecOps::RVec<float> >()));
      return &instance;
   }
   TGenericClassInfo *GenerateInitInstance(const ROOT::VecOps::RVec<float>*)
   {
      return GenerateInitInstanceLocal(static_cast<ROOT::VecOps::RVec<float>*>(nullptr));
   }
   // Static variable to force the class initialization
   static ::ROOT::TGenericClassInfo *_R__UNIQUE_DICT_(Init) = GenerateInitInstanceLocal(static_cast<const ROOT::VecOps::RVec<float>*>(nullptr)); R__UseDummy(_R__UNIQUE_DICT_(Init));

   // Dictionary for non-ClassDef classes
   static TClass *ROOTcLcLVecOpscLcLRVeclEfloatgR_Dictionary() {
      TClass* theClass =::ROOT::GenerateInitInstanceLocal(static_cast<const ROOT::VecOps::RVec<float>*>(nullptr))->GetClass();
      ROOTcLcLVecOpscLcLRVeclEfloatgR_TClassManip(theClass);
   return theClass;
   }

   static void ROOTcLcLVecOpscLcLRVeclEfloatgR_TClassManip(TClass* ){
   }

} // end of namespace ROOT

namespace ROOT {
   // Wrappers around operator new
   static void *new_ROOTcLcLVecOpscLcLRVeclEfloatgR(void *p) {
      return  p ? ::new(static_cast<::ROOT::Internal::TOperatorNewHelper*>(p)) ROOT::VecOps::RVec<float> : new ROOT::VecOps::RVec<float>;
   }
   static void *newArray_ROOTcLcLVecOpscLcLRVeclEfloatgR(Long_t nElements, void *p) {
      return p ? ::new(static_cast<::ROOT::Internal::TOperatorNewHelper*>(p)) ROOT::VecOps::RVec<float>[nElements] : new ROOT::VecOps::RVec<float>[nElements];
   }
   // Wrapper around operator delete
   static void delete_ROOTcLcLVecOpscLcLRVeclEfloatgR(void *p) {
      delete (static_cast<ROOT::VecOps::RVec<float>*>(p));
   }
   static void deleteArray_ROOTcLcLVecOpscLcLRVeclEfloatgR(void *p) {
      delete [] (static_cast<ROOT::VecOps::RVec<float>*>(p));
   }
   static void destruct_ROOTcLcLVecOpscLcLRVeclEfloatgR(void *p) {
      typedef ROOT::VecOps::RVec<float> current_t;
      (static_cast<current_t*>(p))->~current_t();
   }
} // end of namespace ROOT for class ROOT::VecOps::RVec<float>

namespace ROOT {
   static TClass *ROOTcLcLVecOpscLcLRVeclEchargR_Dictionary();
   static void ROOTcLcLVecOpscLcLRVeclEchargR_TClassManip(TClass*);
   static void *new_ROOTcLcLVecOpscLcLRVeclEchargR(void *p = nullptr);
   static void *newArray_ROOTcLcLVecOpscLcLRVeclEchargR(Long_t size, void *p);
   static void delete_ROOTcLcLVecOpscLcLRVeclEchargR(void *p);
   static void deleteArray_ROOTcLcLVecOpscLcLRVeclEchargR(void *p);
   static void destruct_ROOTcLcLVecOpscLcLRVeclEchargR(void *p);

   // Function generating the singleton type initializer
   static TGenericClassInfo *GenerateInitInstanceLocal(const ROOT::VecOps::RVec<char>*)
   {
      ROOT::VecOps::RVec<char> *ptr = nullptr;
      static ::TVirtualIsAProxy* isa_proxy = new ::TIsAProxy(typeid(ROOT::VecOps::RVec<char>));
      static ::ROOT::TGenericClassInfo 
         instance("ROOT::VecOps::RVec<char>", -2, "ROOT/RVec.hxx", 3411,
                  typeid(ROOT::VecOps::RVec<char>), ::ROOT::Internal::DefineBehavior(ptr, ptr),
                  &ROOTcLcLVecOpscLcLRVeclEchargR_Dictionary, isa_proxy, 0,
                  sizeof(ROOT::VecOps::RVec<char>) );
      instance.SetNew(&new_ROOTcLcLVecOpscLcLRVeclEchargR);
      instance.SetNewArray(&newArray_ROOTcLcLVecOpscLcLRVeclEchargR);
      instance.SetDelete(&delete_ROOTcLcLVecOpscLcLRVeclEchargR);
      instance.SetDeleteArray(&deleteArray_ROOTcLcLVecOpscLcLRVeclEchargR);
      instance.SetDestructor(&destruct_ROOTcLcLVecOpscLcLRVeclEchargR);
      instance.AdoptCollectionProxyInfo(TCollectionProxyInfo::Generate(TCollectionProxyInfo::Pushback< ROOT::VecOps::RVec<char> >()));
      return &instance;
   }
   TGenericClassInfo *GenerateInitInstance(const ROOT::VecOps::RVec<char>*)
   {
      return GenerateInitInstanceLocal(static_cast<ROOT::VecOps::RVec<char>*>(nullptr));
   }
   // Static variable to force the class initialization
   static ::ROOT::TGenericClassInfo *_R__UNIQUE_DICT_(Init) = GenerateInitInstanceLocal(static_cast<const ROOT::VecOps::RVec<char>*>(nullptr)); R__UseDummy(_R__UNIQUE_DICT_(Init));

   // Dictionary for non-ClassDef classes
   static TClass *ROOTcLcLVecOpscLcLRVeclEchargR_Dictionary() {
      TClass* theClass =::ROOT::GenerateInitInstanceLocal(static_cast<const ROOT::VecOps::RVec<char>*>(nullptr))->GetClass();
      ROOTcLcLVecOpscLcLRVeclEchargR_TClassManip(theClass);
   return theClass;
   }

   static void ROOTcLcLVecOpscLcLRVeclEchargR_TClassManip(TClass* ){
   }

} // end of namespace ROOT

namespace ROOT {
   // Wrappers around operator new
   static void *new_ROOTcLcLVecOpscLcLRVeclEchargR(void *p) {
      return  p ? ::new(static_cast<::ROOT::Internal::TOperatorNewHelper*>(p)) ROOT::VecOps::RVec<char> : new ROOT::VecOps::RVec<char>;
   }
   static void *newArray_ROOTcLcLVecOpscLcLRVeclEchargR(Long_t nElements, void *p) {
      return p ? ::new(static_cast<::ROOT::Internal::TOperatorNewHelper*>(p)) ROOT::VecOps::RVec<char>[nElements] : new ROOT::VecOps::RVec<char>[nElements];
   }
   // Wrapper around operator delete
   static void delete_ROOTcLcLVecOpscLcLRVeclEchargR(void *p) {
      delete (static_cast<ROOT::VecOps::RVec<char>*>(p));
   }
   static void deleteArray_ROOTcLcLVecOpscLcLRVeclEchargR(void *p) {
      delete [] (static_cast<ROOT::VecOps::RVec<char>*>(p));
   }
   static void destruct_ROOTcLcLVecOpscLcLRVeclEchargR(void *p) {
      typedef ROOT::VecOps::RVec<char> current_t;
      (static_cast<current_t*>(p))->~current_t();
   }
} // end of namespace ROOT for class ROOT::VecOps::RVec<char>

namespace ROOT {
   static TClass *ROOTcLcLVecOpscLcLRVeclEboolgR_Dictionary();
   static void ROOTcLcLVecOpscLcLRVeclEboolgR_TClassManip(TClass*);
   static void *new_ROOTcLcLVecOpscLcLRVeclEboolgR(void *p = nullptr);
   static void *newArray_ROOTcLcLVecOpscLcLRVeclEboolgR(Long_t size, void *p);
   static void delete_ROOTcLcLVecOpscLcLRVeclEboolgR(void *p);
   static void deleteArray_ROOTcLcLVecOpscLcLRVeclEboolgR(void *p);
   static void destruct_ROOTcLcLVecOpscLcLRVeclEboolgR(void *p);

   // Function generating the singleton type initializer
   static TGenericClassInfo *GenerateInitInstanceLocal(const ROOT::VecOps::RVec<bool>*)
   {
      ROOT::VecOps::RVec<bool> *ptr = nullptr;
      static ::TVirtualIsAProxy* isa_proxy = new ::TIsAProxy(typeid(ROOT::VecOps::RVec<bool>));
      static ::ROOT::TGenericClassInfo 
         instance("ROOT::VecOps::RVec<bool>", -2, "ROOT/RVec.hxx", 1529,
                  typeid(ROOT::VecOps::RVec<bool>), ::ROOT::Internal::DefineBehavior(ptr, ptr),
                  &ROOTcLcLVecOpscLcLRVeclEboolgR_Dictionary, isa_proxy, 0,
                  sizeof(ROOT::VecOps::RVec<bool>) );
      instance.SetNew(&new_ROOTcLcLVecOpscLcLRVeclEboolgR);
      instance.SetNewArray(&newArray_ROOTcLcLVecOpscLcLRVeclEboolgR);
      instance.SetDelete(&delete_ROOTcLcLVecOpscLcLRVeclEboolgR);
      instance.SetDeleteArray(&deleteArray_ROOTcLcLVecOpscLcLRVeclEboolgR);
      instance.SetDestructor(&destruct_ROOTcLcLVecOpscLcLRVeclEboolgR);
      instance.AdoptCollectionProxyInfo(TCollectionProxyInfo::Generate(TCollectionProxyInfo::Pushback< ROOT::VecOps::RVec<bool> >()));
      return &instance;
   }
   TGenericClassInfo *GenerateInitInstance(const ROOT::VecOps::RVec<bool>*)
   {
      return GenerateInitInstanceLocal(static_cast<ROOT::VecOps::RVec<bool>*>(nullptr));
   }
   // Static variable to force the class initialization
   static ::ROOT::TGenericClassInfo *_R__UNIQUE_DICT_(Init) = GenerateInitInstanceLocal(static_cast<const ROOT::VecOps::RVec<bool>*>(nullptr)); R__UseDummy(_R__UNIQUE_DICT_(Init));

   // Dictionary for non-ClassDef classes
   static TClass *ROOTcLcLVecOpscLcLRVeclEboolgR_Dictionary() {
      TClass* theClass =::ROOT::GenerateInitInstanceLocal(static_cast<const ROOT::VecOps::RVec<bool>*>(nullptr))->GetClass();
      ROOTcLcLVecOpscLcLRVeclEboolgR_TClassManip(theClass);
   return theClass;
   }

   static void ROOTcLcLVecOpscLcLRVeclEboolgR_TClassManip(TClass* ){
   }

} // end of namespace ROOT

namespace ROOT {
   // Wrappers around operator new
   static void *new_ROOTcLcLVecOpscLcLRVeclEboolgR(void *p) {
      return  p ? ::new(static_cast<::ROOT::Internal::TOperatorNewHelper*>(p)) ROOT::VecOps::RVec<bool> : new ROOT::VecOps::RVec<bool>;
   }
   static void *newArray_ROOTcLcLVecOpscLcLRVeclEboolgR(Long_t nElements, void *p) {
      return p ? ::new(static_cast<::ROOT::Internal::TOperatorNewHelper*>(p)) ROOT::VecOps::RVec<bool>[nElements] : new ROOT::VecOps::RVec<bool>[nElements];
   }
   // Wrapper around operator delete
   static void delete_ROOTcLcLVecOpscLcLRVeclEboolgR(void *p) {
      delete (static_cast<ROOT::VecOps::RVec<bool>*>(p));
   }
   static void deleteArray_ROOTcLcLVecOpscLcLRVeclEboolgR(void *p) {
      delete [] (static_cast<ROOT::VecOps::RVec<bool>*>(p));
   }
   static void destruct_ROOTcLcLVecOpscLcLRVeclEboolgR(void *p) {
      typedef ROOT::VecOps::RVec<bool> current_t;
      (static_cast<current_t*>(p))->~current_t();
   }
} // end of namespace ROOT for class ROOT::VecOps::RVec<bool>

namespace ROOT {
   static TClass *ROOTcLcLVecOpscLcLRVeclETStringgR_Dictionary();
   static void ROOTcLcLVecOpscLcLRVeclETStringgR_TClassManip(TClass*);
   static void *new_ROOTcLcLVecOpscLcLRVeclETStringgR(void *p = nullptr);
   static void *newArray_ROOTcLcLVecOpscLcLRVeclETStringgR(Long_t size, void *p);
   static void delete_ROOTcLcLVecOpscLcLRVeclETStringgR(void *p);
   static void deleteArray_ROOTcLcLVecOpscLcLRVeclETStringgR(void *p);
   static void destruct_ROOTcLcLVecOpscLcLRVeclETStringgR(void *p);

   // Function generating the singleton type initializer
   static TGenericClassInfo *GenerateInitInstanceLocal(const ROOT::VecOps::RVec<TString>*)
   {
      ROOT::VecOps::RVec<TString> *ptr = nullptr;
      static ::TVirtualIsAProxy* isa_proxy = new ::TIsAProxy(typeid(ROOT::VecOps::RVec<TString>));
      static ::ROOT::TGenericClassInfo 
         instance("ROOT::VecOps::RVec<TString>", -2, "ROOT/RVec.hxx", 1529,
                  typeid(ROOT::VecOps::RVec<TString>), ::ROOT::Internal::DefineBehavior(ptr, ptr),
                  &ROOTcLcLVecOpscLcLRVeclETStringgR_Dictionary, isa_proxy, 0,
                  sizeof(ROOT::VecOps::RVec<TString>) );
      instance.SetNew(&new_ROOTcLcLVecOpscLcLRVeclETStringgR);
      instance.SetNewArray(&newArray_ROOTcLcLVecOpscLcLRVeclETStringgR);
      instance.SetDelete(&delete_ROOTcLcLVecOpscLcLRVeclETStringgR);
      instance.SetDeleteArray(&deleteArray_ROOTcLcLVecOpscLcLRVeclETStringgR);
      instance.SetDestructor(&destruct_ROOTcLcLVecOpscLcLRVeclETStringgR);
      instance.AdoptCollectionProxyInfo(TCollectionProxyInfo::Generate(TCollectionProxyInfo::Pushback< ROOT::VecOps::RVec<TString> >()));
      return &instance;
   }
   TGenericClassInfo *GenerateInitInstance(const ROOT::VecOps::RVec<TString>*)
   {
      return GenerateInitInstanceLocal(static_cast<ROOT::VecOps::RVec<TString>*>(nullptr));
   }
   // Static variable to force the class initialization
   static ::ROOT::TGenericClassInfo *_R__UNIQUE_DICT_(Init) = GenerateInitInstanceLocal(static_cast<const ROOT::VecOps::RVec<TString>*>(nullptr)); R__UseDummy(_R__UNIQUE_DICT_(Init));

   // Dictionary for non-ClassDef classes
   static TClass *ROOTcLcLVecOpscLcLRVeclETStringgR_Dictionary() {
      TClass* theClass =::ROOT::GenerateInitInstanceLocal(static_cast<const ROOT::VecOps::RVec<TString>*>(nullptr))->GetClass();
      ROOTcLcLVecOpscLcLRVeclETStringgR_TClassManip(theClass);
   return theClass;
   }

   static void ROOTcLcLVecOpscLcLRVeclETStringgR_TClassManip(TClass* ){
   }

} // end of namespace ROOT

namespace ROOT {
   // Wrappers around operator new
   static void *new_ROOTcLcLVecOpscLcLRVeclETStringgR(void *p) {
      return  p ? ::new(static_cast<::ROOT::Internal::TOperatorNewHelper*>(p)) ROOT::VecOps::RVec<TString> : new ROOT::VecOps::RVec<TString>;
   }
   static void *newArray_ROOTcLcLVecOpscLcLRVeclETStringgR(Long_t nElements, void *p) {
      return p ? ::new(static_cast<::ROOT::Internal::TOperatorNewHelper*>(p)) ROOT::VecOps::RVec<TString>[nElements] : new ROOT::VecOps::RVec<TString>[nElements];
   }
   // Wrapper around operator delete
   static void delete_ROOTcLcLVecOpscLcLRVeclETStringgR(void *p) {
      delete (static_cast<ROOT::VecOps::RVec<TString>*>(p));
   }
   static void deleteArray_ROOTcLcLVecOpscLcLRVeclETStringgR(void *p) {
      delete [] (static_cast<ROOT::VecOps::RVec<TString>*>(p));
   }
   static void destruct_ROOTcLcLVecOpscLcLRVeclETStringgR(void *p) {
      typedef ROOT::VecOps::RVec<TString> current_t;
      (static_cast<current_t*>(p))->~current_t();
   }
} // end of namespace ROOT for class ROOT::VecOps::RVec<TString>

namespace ROOT {
   static TClass *ROOTcLcLVecOpscLcLRVeclEMuongR_Dictionary();
   static void ROOTcLcLVecOpscLcLRVeclEMuongR_TClassManip(TClass*);
   static void *new_ROOTcLcLVecOpscLcLRVeclEMuongR(void *p = nullptr);
   static void *newArray_ROOTcLcLVecOpscLcLRVeclEMuongR(Long_t size, void *p);
   static void delete_ROOTcLcLVecOpscLcLRVeclEMuongR(void *p);
   static void deleteArray_ROOTcLcLVecOpscLcLRVeclEMuongR(void *p);
   static void destruct_ROOTcLcLVecOpscLcLRVeclEMuongR(void *p);

   // Function generating the singleton type initializer
   static TGenericClassInfo *GenerateInitInstanceLocal(const ROOT::VecOps::RVec<Muon>*)
   {
      ROOT::VecOps::RVec<Muon> *ptr = nullptr;
      static ::TVirtualIsAProxy* isa_proxy = new ::TIsAProxy(typeid(ROOT::VecOps::RVec<Muon>));
      static ::ROOT::TGenericClassInfo 
         instance("ROOT::VecOps::RVec<Muon>", -2, "ROOT/RVec.hxx", 1529,
                  typeid(ROOT::VecOps::RVec<Muon>), ::ROOT::Internal::DefineBehavior(ptr, ptr),
                  &ROOTcLcLVecOpscLcLRVeclEMuongR_Dictionary, isa_proxy, 0,
                  sizeof(ROOT::VecOps::RVec<Muon>) );
      instance.SetNew(&new_ROOTcLcLVecOpscLcLRVeclEMuongR);
      instance.SetNewArray(&newArray_ROOTcLcLVecOpscLcLRVeclEMuongR);
      instance.SetDelete(&delete_ROOTcLcLVecOpscLcLRVeclEMuongR);
      instance.SetDeleteArray(&deleteArray_ROOTcLcLVecOpscLcLRVeclEMuongR);
      instance.SetDestructor(&destruct_ROOTcLcLVecOpscLcLRVeclEMuongR);
      instance.AdoptCollectionProxyInfo(TCollectionProxyInfo::Generate(TCollectionProxyInfo::Pushback< ROOT::VecOps::RVec<Muon> >()));
      return &instance;
   }
   TGenericClassInfo *GenerateInitInstance(const ROOT::VecOps::RVec<Muon>*)
   {
      return GenerateInitInstanceLocal(static_cast<ROOT::VecOps::RVec<Muon>*>(nullptr));
   }
   // Static variable to force the class initialization
   static ::ROOT::TGenericClassInfo *_R__UNIQUE_DICT_(Init) = GenerateInitInstanceLocal(static_cast<const ROOT::VecOps::RVec<Muon>*>(nullptr)); R__UseDummy(_R__UNIQUE_DICT_(Init));

   // Dictionary for non-ClassDef classes
   static TClass *ROOTcLcLVecOpscLcLRVeclEMuongR_Dictionary() {
      TClass* theClass =::ROOT::GenerateInitInstanceLocal(static_cast<const ROOT::VecOps::RVec<Muon>*>(nullptr))->GetClass();
      ROOTcLcLVecOpscLcLRVeclEMuongR_TClassManip(theClass);
   return theClass;
   }

   static void ROOTcLcLVecOpscLcLRVeclEMuongR_TClassManip(TClass* ){
   }

} // end of namespace ROOT

namespace ROOT {
   // Wrappers around operator new
   static void *new_ROOTcLcLVecOpscLcLRVeclEMuongR(void *p) {
      return  p ? ::new(static_cast<::ROOT::Internal::TOperatorNewHelper*>(p)) ROOT::VecOps::RVec<Muon> : new ROOT::VecOps::RVec<Muon>;
   }
   static void *newArray_ROOTcLcLVecOpscLcLRVeclEMuongR(Long_t nElements, void *p) {
      return p ? ::new(static_cast<::ROOT::Internal::TOperatorNewHelper*>(p)) ROOT::VecOps::RVec<Muon>[nElements] : new ROOT::VecOps::RVec<Muon>[nElements];
   }
   // Wrapper around operator delete
   static void delete_ROOTcLcLVecOpscLcLRVeclEMuongR(void *p) {
      delete (static_cast<ROOT::VecOps::RVec<Muon>*>(p));
   }
   static void deleteArray_ROOTcLcLVecOpscLcLRVeclEMuongR(void *p) {
      delete [] (static_cast<ROOT::VecOps::RVec<Muon>*>(p));
   }
   static void destruct_ROOTcLcLVecOpscLcLRVeclEMuongR(void *p) {
      typedef ROOT::VecOps::RVec<Muon> current_t;
      (static_cast<current_t*>(p))->~current_t();
   }
} // end of namespace ROOT for class ROOT::VecOps::RVec<Muon>

namespace ROOT {
   static TClass *ROOTcLcLVecOpscLcLRVeclEMuoncLcLMuonIDgR_Dictionary();
   static void ROOTcLcLVecOpscLcLRVeclEMuoncLcLMuonIDgR_TClassManip(TClass*);
   static void *new_ROOTcLcLVecOpscLcLRVeclEMuoncLcLMuonIDgR(void *p = nullptr);
   static void *newArray_ROOTcLcLVecOpscLcLRVeclEMuoncLcLMuonIDgR(Long_t size, void *p);
   static void delete_ROOTcLcLVecOpscLcLRVeclEMuoncLcLMuonIDgR(void *p);
   static void deleteArray_ROOTcLcLVecOpscLcLRVeclEMuoncLcLMuonIDgR(void *p);
   static void destruct_ROOTcLcLVecOpscLcLRVeclEMuoncLcLMuonIDgR(void *p);

   // Function generating the singleton type initializer
   static TGenericClassInfo *GenerateInitInstanceLocal(const ROOT::VecOps::RVec<Muon::MuonID>*)
   {
      ROOT::VecOps::RVec<Muon::MuonID> *ptr = nullptr;
      static ::TVirtualIsAProxy* isa_proxy = new ::TIsAProxy(typeid(ROOT::VecOps::RVec<Muon::MuonID>));
      static ::ROOT::TGenericClassInfo 
         instance("ROOT::VecOps::RVec<Muon::MuonID>", -2, "ROOT/RVec.hxx", 1529,
                  typeid(ROOT::VecOps::RVec<Muon::MuonID>), ::ROOT::Internal::DefineBehavior(ptr, ptr),
                  &ROOTcLcLVecOpscLcLRVeclEMuoncLcLMuonIDgR_Dictionary, isa_proxy, 0,
                  sizeof(ROOT::VecOps::RVec<Muon::MuonID>) );
      instance.SetNew(&new_ROOTcLcLVecOpscLcLRVeclEMuoncLcLMuonIDgR);
      instance.SetNewArray(&newArray_ROOTcLcLVecOpscLcLRVeclEMuoncLcLMuonIDgR);
      instance.SetDelete(&delete_ROOTcLcLVecOpscLcLRVeclEMuoncLcLMuonIDgR);
      instance.SetDeleteArray(&deleteArray_ROOTcLcLVecOpscLcLRVeclEMuoncLcLMuonIDgR);
      instance.SetDestructor(&destruct_ROOTcLcLVecOpscLcLRVeclEMuoncLcLMuonIDgR);
      instance.AdoptCollectionProxyInfo(TCollectionProxyInfo::Generate(TCollectionProxyInfo::Pushback< ROOT::VecOps::RVec<Muon::MuonID> >()));
      return &instance;
   }
   TGenericClassInfo *GenerateInitInstance(const ROOT::VecOps::RVec<Muon::MuonID>*)
   {
      return GenerateInitInstanceLocal(static_cast<ROOT::VecOps::RVec<Muon::MuonID>*>(nullptr));
   }
   // Static variable to force the class initialization
   static ::ROOT::TGenericClassInfo *_R__UNIQUE_DICT_(Init) = GenerateInitInstanceLocal(static_cast<const ROOT::VecOps::RVec<Muon::MuonID>*>(nullptr)); R__UseDummy(_R__UNIQUE_DICT_(Init));

   // Dictionary for non-ClassDef classes
   static TClass *ROOTcLcLVecOpscLcLRVeclEMuoncLcLMuonIDgR_Dictionary() {
      TClass* theClass =::ROOT::GenerateInitInstanceLocal(static_cast<const ROOT::VecOps::RVec<Muon::MuonID>*>(nullptr))->GetClass();
      ROOTcLcLVecOpscLcLRVeclEMuoncLcLMuonIDgR_TClassManip(theClass);
   return theClass;
   }

   static void ROOTcLcLVecOpscLcLRVeclEMuoncLcLMuonIDgR_TClassManip(TClass* ){
   }

} // end of namespace ROOT

namespace ROOT {
   // Wrappers around operator new
   static void *new_ROOTcLcLVecOpscLcLRVeclEMuoncLcLMuonIDgR(void *p) {
      return  p ? ::new(static_cast<::ROOT::Internal::TOperatorNewHelper*>(p)) ROOT::VecOps::RVec<Muon::MuonID> : new ROOT::VecOps::RVec<Muon::MuonID>;
   }
   static void *newArray_ROOTcLcLVecOpscLcLRVeclEMuoncLcLMuonIDgR(Long_t nElements, void *p) {
      return p ? ::new(static_cast<::ROOT::Internal::TOperatorNewHelper*>(p)) ROOT::VecOps::RVec<Muon::MuonID>[nElements] : new ROOT::VecOps::RVec<Muon::MuonID>[nElements];
   }
   // Wrapper around operator delete
   static void delete_ROOTcLcLVecOpscLcLRVeclEMuoncLcLMuonIDgR(void *p) {
      delete (static_cast<ROOT::VecOps::RVec<Muon::MuonID>*>(p));
   }
   static void deleteArray_ROOTcLcLVecOpscLcLRVeclEMuoncLcLMuonIDgR(void *p) {
      delete [] (static_cast<ROOT::VecOps::RVec<Muon::MuonID>*>(p));
   }
   static void destruct_ROOTcLcLVecOpscLcLRVeclEMuoncLcLMuonIDgR(void *p) {
      typedef ROOT::VecOps::RVec<Muon::MuonID> current_t;
      (static_cast<current_t*>(p))->~current_t();
   }
} // end of namespace ROOT for class ROOT::VecOps::RVec<Muon::MuonID>

namespace ROOT {
   static TClass *ROOTcLcLVecOpscLcLRVeclELHEgR_Dictionary();
   static void ROOTcLcLVecOpscLcLRVeclELHEgR_TClassManip(TClass*);
   static void *new_ROOTcLcLVecOpscLcLRVeclELHEgR(void *p = nullptr);
   static void *newArray_ROOTcLcLVecOpscLcLRVeclELHEgR(Long_t size, void *p);
   static void delete_ROOTcLcLVecOpscLcLRVeclELHEgR(void *p);
   static void deleteArray_ROOTcLcLVecOpscLcLRVeclELHEgR(void *p);
   static void destruct_ROOTcLcLVecOpscLcLRVeclELHEgR(void *p);

   // Function generating the singleton type initializer
   static TGenericClassInfo *GenerateInitInstanceLocal(const ROOT::VecOps::RVec<LHE>*)
   {
      ROOT::VecOps::RVec<LHE> *ptr = nullptr;
      static ::TVirtualIsAProxy* isa_proxy = new ::TIsAProxy(typeid(ROOT::VecOps::RVec<LHE>));
      static ::ROOT::TGenericClassInfo 
         instance("ROOT::VecOps::RVec<LHE>", -2, "ROOT/RVec.hxx", 1529,
                  typeid(ROOT::VecOps::RVec<LHE>), ::ROOT::Internal::DefineBehavior(ptr, ptr),
                  &ROOTcLcLVecOpscLcLRVeclELHEgR_Dictionary, isa_proxy, 0,
                  sizeof(ROOT::VecOps::RVec<LHE>) );
      instance.SetNew(&new_ROOTcLcLVecOpscLcLRVeclELHEgR);
      instance.SetNewArray(&newArray_ROOTcLcLVecOpscLcLRVeclELHEgR);
      instance.SetDelete(&delete_ROOTcLcLVecOpscLcLRVeclELHEgR);
      instance.SetDeleteArray(&deleteArray_ROOTcLcLVecOpscLcLRVeclELHEgR);
      instance.SetDestructor(&destruct_ROOTcLcLVecOpscLcLRVeclELHEgR);
      instance.AdoptCollectionProxyInfo(TCollectionProxyInfo::Generate(TCollectionProxyInfo::Pushback< ROOT::VecOps::RVec<LHE> >()));
      return &instance;
   }
   TGenericClassInfo *GenerateInitInstance(const ROOT::VecOps::RVec<LHE>*)
   {
      return GenerateInitInstanceLocal(static_cast<ROOT::VecOps::RVec<LHE>*>(nullptr));
   }
   // Static variable to force the class initialization
   static ::ROOT::TGenericClassInfo *_R__UNIQUE_DICT_(Init) = GenerateInitInstanceLocal(static_cast<const ROOT::VecOps::RVec<LHE>*>(nullptr)); R__UseDummy(_R__UNIQUE_DICT_(Init));

   // Dictionary for non-ClassDef classes
   static TClass *ROOTcLcLVecOpscLcLRVeclELHEgR_Dictionary() {
      TClass* theClass =::ROOT::GenerateInitInstanceLocal(static_cast<const ROOT::VecOps::RVec<LHE>*>(nullptr))->GetClass();
      ROOTcLcLVecOpscLcLRVeclELHEgR_TClassManip(theClass);
   return theClass;
   }

   static void ROOTcLcLVecOpscLcLRVeclELHEgR_TClassManip(TClass* ){
   }

} // end of namespace ROOT

namespace ROOT {
   // Wrappers around operator new
   static void *new_ROOTcLcLVecOpscLcLRVeclELHEgR(void *p) {
      return  p ? ::new(static_cast<::ROOT::Internal::TOperatorNewHelper*>(p)) ROOT::VecOps::RVec<LHE> : new ROOT::VecOps::RVec<LHE>;
   }
   static void *newArray_ROOTcLcLVecOpscLcLRVeclELHEgR(Long_t nElements, void *p) {
      return p ? ::new(static_cast<::ROOT::Internal::TOperatorNewHelper*>(p)) ROOT::VecOps::RVec<LHE>[nElements] : new ROOT::VecOps::RVec<LHE>[nElements];
   }
   // Wrapper around operator delete
   static void delete_ROOTcLcLVecOpscLcLRVeclELHEgR(void *p) {
      delete (static_cast<ROOT::VecOps::RVec<LHE>*>(p));
   }
   static void deleteArray_ROOTcLcLVecOpscLcLRVeclELHEgR(void *p) {
      delete [] (static_cast<ROOT::VecOps::RVec<LHE>*>(p));
   }
   static void destruct_ROOTcLcLVecOpscLcLRVeclELHEgR(void *p) {
      typedef ROOT::VecOps::RVec<LHE> current_t;
      (static_cast<current_t*>(p))->~current_t();
   }
} // end of namespace ROOT for class ROOT::VecOps::RVec<LHE>

namespace ROOT {
   static TClass *ROOTcLcLVecOpscLcLRVeclEJetTaggingcLcLJetFlavTaggerWPgR_Dictionary();
   static void ROOTcLcLVecOpscLcLRVeclEJetTaggingcLcLJetFlavTaggerWPgR_TClassManip(TClass*);
   static void *new_ROOTcLcLVecOpscLcLRVeclEJetTaggingcLcLJetFlavTaggerWPgR(void *p = nullptr);
   static void *newArray_ROOTcLcLVecOpscLcLRVeclEJetTaggingcLcLJetFlavTaggerWPgR(Long_t size, void *p);
   static void delete_ROOTcLcLVecOpscLcLRVeclEJetTaggingcLcLJetFlavTaggerWPgR(void *p);
   static void deleteArray_ROOTcLcLVecOpscLcLRVeclEJetTaggingcLcLJetFlavTaggerWPgR(void *p);
   static void destruct_ROOTcLcLVecOpscLcLRVeclEJetTaggingcLcLJetFlavTaggerWPgR(void *p);

   // Function generating the singleton type initializer
   static TGenericClassInfo *GenerateInitInstanceLocal(const ROOT::VecOps::RVec<JetTagging::JetFlavTaggerWP>*)
   {
      ROOT::VecOps::RVec<JetTagging::JetFlavTaggerWP> *ptr = nullptr;
      static ::TVirtualIsAProxy* isa_proxy = new ::TIsAProxy(typeid(ROOT::VecOps::RVec<JetTagging::JetFlavTaggerWP>));
      static ::ROOT::TGenericClassInfo 
         instance("ROOT::VecOps::RVec<JetTagging::JetFlavTaggerWP>", -2, "ROOT/RVec.hxx", 1529,
                  typeid(ROOT::VecOps::RVec<JetTagging::JetFlavTaggerWP>), ::ROOT::Internal::DefineBehavior(ptr, ptr),
                  &ROOTcLcLVecOpscLcLRVeclEJetTaggingcLcLJetFlavTaggerWPgR_Dictionary, isa_proxy, 0,
                  sizeof(ROOT::VecOps::RVec<JetTagging::JetFlavTaggerWP>) );
      instance.SetNew(&new_ROOTcLcLVecOpscLcLRVeclEJetTaggingcLcLJetFlavTaggerWPgR);
      instance.SetNewArray(&newArray_ROOTcLcLVecOpscLcLRVeclEJetTaggingcLcLJetFlavTaggerWPgR);
      instance.SetDelete(&delete_ROOTcLcLVecOpscLcLRVeclEJetTaggingcLcLJetFlavTaggerWPgR);
      instance.SetDeleteArray(&deleteArray_ROOTcLcLVecOpscLcLRVeclEJetTaggingcLcLJetFlavTaggerWPgR);
      instance.SetDestructor(&destruct_ROOTcLcLVecOpscLcLRVeclEJetTaggingcLcLJetFlavTaggerWPgR);
      instance.AdoptCollectionProxyInfo(TCollectionProxyInfo::Generate(TCollectionProxyInfo::Pushback< ROOT::VecOps::RVec<JetTagging::JetFlavTaggerWP> >()));
      return &instance;
   }
   TGenericClassInfo *GenerateInitInstance(const ROOT::VecOps::RVec<JetTagging::JetFlavTaggerWP>*)
   {
      return GenerateInitInstanceLocal(static_cast<ROOT::VecOps::RVec<JetTagging::JetFlavTaggerWP>*>(nullptr));
   }
   // Static variable to force the class initialization
   static ::ROOT::TGenericClassInfo *_R__UNIQUE_DICT_(Init) = GenerateInitInstanceLocal(static_cast<const ROOT::VecOps::RVec<JetTagging::JetFlavTaggerWP>*>(nullptr)); R__UseDummy(_R__UNIQUE_DICT_(Init));

   // Dictionary for non-ClassDef classes
   static TClass *ROOTcLcLVecOpscLcLRVeclEJetTaggingcLcLJetFlavTaggerWPgR_Dictionary() {
      TClass* theClass =::ROOT::GenerateInitInstanceLocal(static_cast<const ROOT::VecOps::RVec<JetTagging::JetFlavTaggerWP>*>(nullptr))->GetClass();
      ROOTcLcLVecOpscLcLRVeclEJetTaggingcLcLJetFlavTaggerWPgR_TClassManip(theClass);
   return theClass;
   }

   static void ROOTcLcLVecOpscLcLRVeclEJetTaggingcLcLJetFlavTaggerWPgR_TClassManip(TClass* ){
   }

} // end of namespace ROOT

namespace ROOT {
   // Wrappers around operator new
   static void *new_ROOTcLcLVecOpscLcLRVeclEJetTaggingcLcLJetFlavTaggerWPgR(void *p) {
      return  p ? ::new(static_cast<::ROOT::Internal::TOperatorNewHelper*>(p)) ROOT::VecOps::RVec<JetTagging::JetFlavTaggerWP> : new ROOT::VecOps::RVec<JetTagging::JetFlavTaggerWP>;
   }
   static void *newArray_ROOTcLcLVecOpscLcLRVeclEJetTaggingcLcLJetFlavTaggerWPgR(Long_t nElements, void *p) {
      return p ? ::new(static_cast<::ROOT::Internal::TOperatorNewHelper*>(p)) ROOT::VecOps::RVec<JetTagging::JetFlavTaggerWP>[nElements] : new ROOT::VecOps::RVec<JetTagging::JetFlavTaggerWP>[nElements];
   }
   // Wrapper around operator delete
   static void delete_ROOTcLcLVecOpscLcLRVeclEJetTaggingcLcLJetFlavTaggerWPgR(void *p) {
      delete (static_cast<ROOT::VecOps::RVec<JetTagging::JetFlavTaggerWP>*>(p));
   }
   static void deleteArray_ROOTcLcLVecOpscLcLRVeclEJetTaggingcLcLJetFlavTaggerWPgR(void *p) {
      delete [] (static_cast<ROOT::VecOps::RVec<JetTagging::JetFlavTaggerWP>*>(p));
   }
   static void destruct_ROOTcLcLVecOpscLcLRVeclEJetTaggingcLcLJetFlavTaggerWPgR(void *p) {
      typedef ROOT::VecOps::RVec<JetTagging::JetFlavTaggerWP> current_t;
      (static_cast<current_t*>(p))->~current_t();
   }
} // end of namespace ROOT for class ROOT::VecOps::RVec<JetTagging::JetFlavTaggerWP>

namespace ROOT {
   static TClass *ROOTcLcLVecOpscLcLRVeclEJetTaggingcLcLJetFlavTaggergR_Dictionary();
   static void ROOTcLcLVecOpscLcLRVeclEJetTaggingcLcLJetFlavTaggergR_TClassManip(TClass*);
   static void *new_ROOTcLcLVecOpscLcLRVeclEJetTaggingcLcLJetFlavTaggergR(void *p = nullptr);
   static void *newArray_ROOTcLcLVecOpscLcLRVeclEJetTaggingcLcLJetFlavTaggergR(Long_t size, void *p);
   static void delete_ROOTcLcLVecOpscLcLRVeclEJetTaggingcLcLJetFlavTaggergR(void *p);
   static void deleteArray_ROOTcLcLVecOpscLcLRVeclEJetTaggingcLcLJetFlavTaggergR(void *p);
   static void destruct_ROOTcLcLVecOpscLcLRVeclEJetTaggingcLcLJetFlavTaggergR(void *p);

   // Function generating the singleton type initializer
   static TGenericClassInfo *GenerateInitInstanceLocal(const ROOT::VecOps::RVec<JetTagging::JetFlavTagger>*)
   {
      ROOT::VecOps::RVec<JetTagging::JetFlavTagger> *ptr = nullptr;
      static ::TVirtualIsAProxy* isa_proxy = new ::TIsAProxy(typeid(ROOT::VecOps::RVec<JetTagging::JetFlavTagger>));
      static ::ROOT::TGenericClassInfo 
         instance("ROOT::VecOps::RVec<JetTagging::JetFlavTagger>", -2, "ROOT/RVec.hxx", 1529,
                  typeid(ROOT::VecOps::RVec<JetTagging::JetFlavTagger>), ::ROOT::Internal::DefineBehavior(ptr, ptr),
                  &ROOTcLcLVecOpscLcLRVeclEJetTaggingcLcLJetFlavTaggergR_Dictionary, isa_proxy, 0,
                  sizeof(ROOT::VecOps::RVec<JetTagging::JetFlavTagger>) );
      instance.SetNew(&new_ROOTcLcLVecOpscLcLRVeclEJetTaggingcLcLJetFlavTaggergR);
      instance.SetNewArray(&newArray_ROOTcLcLVecOpscLcLRVeclEJetTaggingcLcLJetFlavTaggergR);
      instance.SetDelete(&delete_ROOTcLcLVecOpscLcLRVeclEJetTaggingcLcLJetFlavTaggergR);
      instance.SetDeleteArray(&deleteArray_ROOTcLcLVecOpscLcLRVeclEJetTaggingcLcLJetFlavTaggergR);
      instance.SetDestructor(&destruct_ROOTcLcLVecOpscLcLRVeclEJetTaggingcLcLJetFlavTaggergR);
      instance.AdoptCollectionProxyInfo(TCollectionProxyInfo::Generate(TCollectionProxyInfo::Pushback< ROOT::VecOps::RVec<JetTagging::JetFlavTagger> >()));
      return &instance;
   }
   TGenericClassInfo *GenerateInitInstance(const ROOT::VecOps::RVec<JetTagging::JetFlavTagger>*)
   {
      return GenerateInitInstanceLocal(static_cast<ROOT::VecOps::RVec<JetTagging::JetFlavTagger>*>(nullptr));
   }
   // Static variable to force the class initialization
   static ::ROOT::TGenericClassInfo *_R__UNIQUE_DICT_(Init) = GenerateInitInstanceLocal(static_cast<const ROOT::VecOps::RVec<JetTagging::JetFlavTagger>*>(nullptr)); R__UseDummy(_R__UNIQUE_DICT_(Init));

   // Dictionary for non-ClassDef classes
   static TClass *ROOTcLcLVecOpscLcLRVeclEJetTaggingcLcLJetFlavTaggergR_Dictionary() {
      TClass* theClass =::ROOT::GenerateInitInstanceLocal(static_cast<const ROOT::VecOps::RVec<JetTagging::JetFlavTagger>*>(nullptr))->GetClass();
      ROOTcLcLVecOpscLcLRVeclEJetTaggingcLcLJetFlavTaggergR_TClassManip(theClass);
   return theClass;
   }

   static void ROOTcLcLVecOpscLcLRVeclEJetTaggingcLcLJetFlavTaggergR_TClassManip(TClass* ){
   }

} // end of namespace ROOT

namespace ROOT {
   // Wrappers around operator new
   static void *new_ROOTcLcLVecOpscLcLRVeclEJetTaggingcLcLJetFlavTaggergR(void *p) {
      return  p ? ::new(static_cast<::ROOT::Internal::TOperatorNewHelper*>(p)) ROOT::VecOps::RVec<JetTagging::JetFlavTagger> : new ROOT::VecOps::RVec<JetTagging::JetFlavTagger>;
   }
   static void *newArray_ROOTcLcLVecOpscLcLRVeclEJetTaggingcLcLJetFlavTaggergR(Long_t nElements, void *p) {
      return p ? ::new(static_cast<::ROOT::Internal::TOperatorNewHelper*>(p)) ROOT::VecOps::RVec<JetTagging::JetFlavTagger>[nElements] : new ROOT::VecOps::RVec<JetTagging::JetFlavTagger>[nElements];
   }
   // Wrapper around operator delete
   static void delete_ROOTcLcLVecOpscLcLRVeclEJetTaggingcLcLJetFlavTaggergR(void *p) {
      delete (static_cast<ROOT::VecOps::RVec<JetTagging::JetFlavTagger>*>(p));
   }
   static void deleteArray_ROOTcLcLVecOpscLcLRVeclEJetTaggingcLcLJetFlavTaggergR(void *p) {
      delete [] (static_cast<ROOT::VecOps::RVec<JetTagging::JetFlavTagger>*>(p));
   }
   static void destruct_ROOTcLcLVecOpscLcLRVeclEJetTaggingcLcLJetFlavTaggergR(void *p) {
      typedef ROOT::VecOps::RVec<JetTagging::JetFlavTagger> current_t;
      (static_cast<current_t*>(p))->~current_t();
   }
} // end of namespace ROOT for class ROOT::VecOps::RVec<JetTagging::JetFlavTagger>

namespace ROOT {
   static TClass *ROOTcLcLVecOpscLcLRVeclEJetgR_Dictionary();
   static void ROOTcLcLVecOpscLcLRVeclEJetgR_TClassManip(TClass*);
   static void *new_ROOTcLcLVecOpscLcLRVeclEJetgR(void *p = nullptr);
   static void *newArray_ROOTcLcLVecOpscLcLRVeclEJetgR(Long_t size, void *p);
   static void delete_ROOTcLcLVecOpscLcLRVeclEJetgR(void *p);
   static void deleteArray_ROOTcLcLVecOpscLcLRVeclEJetgR(void *p);
   static void destruct_ROOTcLcLVecOpscLcLRVeclEJetgR(void *p);

   // Function generating the singleton type initializer
   static TGenericClassInfo *GenerateInitInstanceLocal(const ROOT::VecOps::RVec<Jet>*)
   {
      ROOT::VecOps::RVec<Jet> *ptr = nullptr;
      static ::TVirtualIsAProxy* isa_proxy = new ::TIsAProxy(typeid(ROOT::VecOps::RVec<Jet>));
      static ::ROOT::TGenericClassInfo 
         instance("ROOT::VecOps::RVec<Jet>", -2, "ROOT/RVec.hxx", 1529,
                  typeid(ROOT::VecOps::RVec<Jet>), ::ROOT::Internal::DefineBehavior(ptr, ptr),
                  &ROOTcLcLVecOpscLcLRVeclEJetgR_Dictionary, isa_proxy, 0,
                  sizeof(ROOT::VecOps::RVec<Jet>) );
      instance.SetNew(&new_ROOTcLcLVecOpscLcLRVeclEJetgR);
      instance.SetNewArray(&newArray_ROOTcLcLVecOpscLcLRVeclEJetgR);
      instance.SetDelete(&delete_ROOTcLcLVecOpscLcLRVeclEJetgR);
      instance.SetDeleteArray(&deleteArray_ROOTcLcLVecOpscLcLRVeclEJetgR);
      instance.SetDestructor(&destruct_ROOTcLcLVecOpscLcLRVeclEJetgR);
      instance.AdoptCollectionProxyInfo(TCollectionProxyInfo::Generate(TCollectionProxyInfo::Pushback< ROOT::VecOps::RVec<Jet> >()));
      return &instance;
   }
   TGenericClassInfo *GenerateInitInstance(const ROOT::VecOps::RVec<Jet>*)
   {
      return GenerateInitInstanceLocal(static_cast<ROOT::VecOps::RVec<Jet>*>(nullptr));
   }
   // Static variable to force the class initialization
   static ::ROOT::TGenericClassInfo *_R__UNIQUE_DICT_(Init) = GenerateInitInstanceLocal(static_cast<const ROOT::VecOps::RVec<Jet>*>(nullptr)); R__UseDummy(_R__UNIQUE_DICT_(Init));

   // Dictionary for non-ClassDef classes
   static TClass *ROOTcLcLVecOpscLcLRVeclEJetgR_Dictionary() {
      TClass* theClass =::ROOT::GenerateInitInstanceLocal(static_cast<const ROOT::VecOps::RVec<Jet>*>(nullptr))->GetClass();
      ROOTcLcLVecOpscLcLRVeclEJetgR_TClassManip(theClass);
   return theClass;
   }

   static void ROOTcLcLVecOpscLcLRVeclEJetgR_TClassManip(TClass* ){
   }

} // end of namespace ROOT

namespace ROOT {
   // Wrappers around operator new
   static void *new_ROOTcLcLVecOpscLcLRVeclEJetgR(void *p) {
      return  p ? ::new(static_cast<::ROOT::Internal::TOperatorNewHelper*>(p)) ROOT::VecOps::RVec<Jet> : new ROOT::VecOps::RVec<Jet>;
   }
   static void *newArray_ROOTcLcLVecOpscLcLRVeclEJetgR(Long_t nElements, void *p) {
      return p ? ::new(static_cast<::ROOT::Internal::TOperatorNewHelper*>(p)) ROOT::VecOps::RVec<Jet>[nElements] : new ROOT::VecOps::RVec<Jet>[nElements];
   }
   // Wrapper around operator delete
   static void delete_ROOTcLcLVecOpscLcLRVeclEJetgR(void *p) {
      delete (static_cast<ROOT::VecOps::RVec<Jet>*>(p));
   }
   static void deleteArray_ROOTcLcLVecOpscLcLRVeclEJetgR(void *p) {
      delete [] (static_cast<ROOT::VecOps::RVec<Jet>*>(p));
   }
   static void destruct_ROOTcLcLVecOpscLcLRVeclEJetgR(void *p) {
      typedef ROOT::VecOps::RVec<Jet> current_t;
      (static_cast<current_t*>(p))->~current_t();
   }
} // end of namespace ROOT for class ROOT::VecOps::RVec<Jet>

namespace ROOT {
   static TClass *ROOTcLcLVecOpscLcLRVeclEJetcLcLJetIDgR_Dictionary();
   static void ROOTcLcLVecOpscLcLRVeclEJetcLcLJetIDgR_TClassManip(TClass*);
   static void *new_ROOTcLcLVecOpscLcLRVeclEJetcLcLJetIDgR(void *p = nullptr);
   static void *newArray_ROOTcLcLVecOpscLcLRVeclEJetcLcLJetIDgR(Long_t size, void *p);
   static void delete_ROOTcLcLVecOpscLcLRVeclEJetcLcLJetIDgR(void *p);
   static void deleteArray_ROOTcLcLVecOpscLcLRVeclEJetcLcLJetIDgR(void *p);
   static void destruct_ROOTcLcLVecOpscLcLRVeclEJetcLcLJetIDgR(void *p);

   // Function generating the singleton type initializer
   static TGenericClassInfo *GenerateInitInstanceLocal(const ROOT::VecOps::RVec<Jet::JetID>*)
   {
      ROOT::VecOps::RVec<Jet::JetID> *ptr = nullptr;
      static ::TVirtualIsAProxy* isa_proxy = new ::TIsAProxy(typeid(ROOT::VecOps::RVec<Jet::JetID>));
      static ::ROOT::TGenericClassInfo 
         instance("ROOT::VecOps::RVec<Jet::JetID>", -2, "ROOT/RVec.hxx", 1529,
                  typeid(ROOT::VecOps::RVec<Jet::JetID>), ::ROOT::Internal::DefineBehavior(ptr, ptr),
                  &ROOTcLcLVecOpscLcLRVeclEJetcLcLJetIDgR_Dictionary, isa_proxy, 0,
                  sizeof(ROOT::VecOps::RVec<Jet::JetID>) );
      instance.SetNew(&new_ROOTcLcLVecOpscLcLRVeclEJetcLcLJetIDgR);
      instance.SetNewArray(&newArray_ROOTcLcLVecOpscLcLRVeclEJetcLcLJetIDgR);
      instance.SetDelete(&delete_ROOTcLcLVecOpscLcLRVeclEJetcLcLJetIDgR);
      instance.SetDeleteArray(&deleteArray_ROOTcLcLVecOpscLcLRVeclEJetcLcLJetIDgR);
      instance.SetDestructor(&destruct_ROOTcLcLVecOpscLcLRVeclEJetcLcLJetIDgR);
      instance.AdoptCollectionProxyInfo(TCollectionProxyInfo::Generate(TCollectionProxyInfo::Pushback< ROOT::VecOps::RVec<Jet::JetID> >()));
      return &instance;
   }
   TGenericClassInfo *GenerateInitInstance(const ROOT::VecOps::RVec<Jet::JetID>*)
   {
      return GenerateInitInstanceLocal(static_cast<ROOT::VecOps::RVec<Jet::JetID>*>(nullptr));
   }
   // Static variable to force the class initialization
   static ::ROOT::TGenericClassInfo *_R__UNIQUE_DICT_(Init) = GenerateInitInstanceLocal(static_cast<const ROOT::VecOps::RVec<Jet::JetID>*>(nullptr)); R__UseDummy(_R__UNIQUE_DICT_(Init));

   // Dictionary for non-ClassDef classes
   static TClass *ROOTcLcLVecOpscLcLRVeclEJetcLcLJetIDgR_Dictionary() {
      TClass* theClass =::ROOT::GenerateInitInstanceLocal(static_cast<const ROOT::VecOps::RVec<Jet::JetID>*>(nullptr))->GetClass();
      ROOTcLcLVecOpscLcLRVeclEJetcLcLJetIDgR_TClassManip(theClass);
   return theClass;
   }

   static void ROOTcLcLVecOpscLcLRVeclEJetcLcLJetIDgR_TClassManip(TClass* ){
   }

} // end of namespace ROOT

namespace ROOT {
   // Wrappers around operator new
   static void *new_ROOTcLcLVecOpscLcLRVeclEJetcLcLJetIDgR(void *p) {
      return  p ? ::new(static_cast<::ROOT::Internal::TOperatorNewHelper*>(p)) ROOT::VecOps::RVec<Jet::JetID> : new ROOT::VecOps::RVec<Jet::JetID>;
   }
   static void *newArray_ROOTcLcLVecOpscLcLRVeclEJetcLcLJetIDgR(Long_t nElements, void *p) {
      return p ? ::new(static_cast<::ROOT::Internal::TOperatorNewHelper*>(p)) ROOT::VecOps::RVec<Jet::JetID>[nElements] : new ROOT::VecOps::RVec<Jet::JetID>[nElements];
   }
   // Wrapper around operator delete
   static void delete_ROOTcLcLVecOpscLcLRVeclEJetcLcLJetIDgR(void *p) {
      delete (static_cast<ROOT::VecOps::RVec<Jet::JetID>*>(p));
   }
   static void deleteArray_ROOTcLcLVecOpscLcLRVeclEJetcLcLJetIDgR(void *p) {
      delete [] (static_cast<ROOT::VecOps::RVec<Jet::JetID>*>(p));
   }
   static void destruct_ROOTcLcLVecOpscLcLRVeclEJetcLcLJetIDgR(void *p) {
      typedef ROOT::VecOps::RVec<Jet::JetID> current_t;
      (static_cast<current_t*>(p))->~current_t();
   }
} // end of namespace ROOT for class ROOT::VecOps::RVec<Jet::JetID>

namespace ROOT {
   static TClass *ROOTcLcLVecOpscLcLRVeclEGenVisTaugR_Dictionary();
   static void ROOTcLcLVecOpscLcLRVeclEGenVisTaugR_TClassManip(TClass*);
   static void *new_ROOTcLcLVecOpscLcLRVeclEGenVisTaugR(void *p = nullptr);
   static void *newArray_ROOTcLcLVecOpscLcLRVeclEGenVisTaugR(Long_t size, void *p);
   static void delete_ROOTcLcLVecOpscLcLRVeclEGenVisTaugR(void *p);
   static void deleteArray_ROOTcLcLVecOpscLcLRVeclEGenVisTaugR(void *p);
   static void destruct_ROOTcLcLVecOpscLcLRVeclEGenVisTaugR(void *p);

   // Function generating the singleton type initializer
   static TGenericClassInfo *GenerateInitInstanceLocal(const ROOT::VecOps::RVec<GenVisTau>*)
   {
      ROOT::VecOps::RVec<GenVisTau> *ptr = nullptr;
      static ::TVirtualIsAProxy* isa_proxy = new ::TIsAProxy(typeid(ROOT::VecOps::RVec<GenVisTau>));
      static ::ROOT::TGenericClassInfo 
         instance("ROOT::VecOps::RVec<GenVisTau>", -2, "ROOT/RVec.hxx", 1529,
                  typeid(ROOT::VecOps::RVec<GenVisTau>), ::ROOT::Internal::DefineBehavior(ptr, ptr),
                  &ROOTcLcLVecOpscLcLRVeclEGenVisTaugR_Dictionary, isa_proxy, 0,
                  sizeof(ROOT::VecOps::RVec<GenVisTau>) );
      instance.SetNew(&new_ROOTcLcLVecOpscLcLRVeclEGenVisTaugR);
      instance.SetNewArray(&newArray_ROOTcLcLVecOpscLcLRVeclEGenVisTaugR);
      instance.SetDelete(&delete_ROOTcLcLVecOpscLcLRVeclEGenVisTaugR);
      instance.SetDeleteArray(&deleteArray_ROOTcLcLVecOpscLcLRVeclEGenVisTaugR);
      instance.SetDestructor(&destruct_ROOTcLcLVecOpscLcLRVeclEGenVisTaugR);
      instance.AdoptCollectionProxyInfo(TCollectionProxyInfo::Generate(TCollectionProxyInfo::Pushback< ROOT::VecOps::RVec<GenVisTau> >()));
      return &instance;
   }
   TGenericClassInfo *GenerateInitInstance(const ROOT::VecOps::RVec<GenVisTau>*)
   {
      return GenerateInitInstanceLocal(static_cast<ROOT::VecOps::RVec<GenVisTau>*>(nullptr));
   }
   // Static variable to force the class initialization
   static ::ROOT::TGenericClassInfo *_R__UNIQUE_DICT_(Init) = GenerateInitInstanceLocal(static_cast<const ROOT::VecOps::RVec<GenVisTau>*>(nullptr)); R__UseDummy(_R__UNIQUE_DICT_(Init));

   // Dictionary for non-ClassDef classes
   static TClass *ROOTcLcLVecOpscLcLRVeclEGenVisTaugR_Dictionary() {
      TClass* theClass =::ROOT::GenerateInitInstanceLocal(static_cast<const ROOT::VecOps::RVec<GenVisTau>*>(nullptr))->GetClass();
      ROOTcLcLVecOpscLcLRVeclEGenVisTaugR_TClassManip(theClass);
   return theClass;
   }

   static void ROOTcLcLVecOpscLcLRVeclEGenVisTaugR_TClassManip(TClass* ){
   }

} // end of namespace ROOT

namespace ROOT {
   // Wrappers around operator new
   static void *new_ROOTcLcLVecOpscLcLRVeclEGenVisTaugR(void *p) {
      return  p ? ::new(static_cast<::ROOT::Internal::TOperatorNewHelper*>(p)) ROOT::VecOps::RVec<GenVisTau> : new ROOT::VecOps::RVec<GenVisTau>;
   }
   static void *newArray_ROOTcLcLVecOpscLcLRVeclEGenVisTaugR(Long_t nElements, void *p) {
      return p ? ::new(static_cast<::ROOT::Internal::TOperatorNewHelper*>(p)) ROOT::VecOps::RVec<GenVisTau>[nElements] : new ROOT::VecOps::RVec<GenVisTau>[nElements];
   }
   // Wrapper around operator delete
   static void delete_ROOTcLcLVecOpscLcLRVeclEGenVisTaugR(void *p) {
      delete (static_cast<ROOT::VecOps::RVec<GenVisTau>*>(p));
   }
   static void deleteArray_ROOTcLcLVecOpscLcLRVeclEGenVisTaugR(void *p) {
      delete [] (static_cast<ROOT::VecOps::RVec<GenVisTau>*>(p));
   }
   static void destruct_ROOTcLcLVecOpscLcLRVeclEGenVisTaugR(void *p) {
      typedef ROOT::VecOps::RVec<GenVisTau> current_t;
      (static_cast<current_t*>(p))->~current_t();
   }
} // end of namespace ROOT for class ROOT::VecOps::RVec<GenVisTau>

namespace ROOT {
   static TClass *ROOTcLcLVecOpscLcLRVeclEGenJetgR_Dictionary();
   static void ROOTcLcLVecOpscLcLRVeclEGenJetgR_TClassManip(TClass*);
   static void *new_ROOTcLcLVecOpscLcLRVeclEGenJetgR(void *p = nullptr);
   static void *newArray_ROOTcLcLVecOpscLcLRVeclEGenJetgR(Long_t size, void *p);
   static void delete_ROOTcLcLVecOpscLcLRVeclEGenJetgR(void *p);
   static void deleteArray_ROOTcLcLVecOpscLcLRVeclEGenJetgR(void *p);
   static void destruct_ROOTcLcLVecOpscLcLRVeclEGenJetgR(void *p);

   // Function generating the singleton type initializer
   static TGenericClassInfo *GenerateInitInstanceLocal(const ROOT::VecOps::RVec<GenJet>*)
   {
      ROOT::VecOps::RVec<GenJet> *ptr = nullptr;
      static ::TVirtualIsAProxy* isa_proxy = new ::TIsAProxy(typeid(ROOT::VecOps::RVec<GenJet>));
      static ::ROOT::TGenericClassInfo 
         instance("ROOT::VecOps::RVec<GenJet>", -2, "ROOT/RVec.hxx", 1529,
                  typeid(ROOT::VecOps::RVec<GenJet>), ::ROOT::Internal::DefineBehavior(ptr, ptr),
                  &ROOTcLcLVecOpscLcLRVeclEGenJetgR_Dictionary, isa_proxy, 0,
                  sizeof(ROOT::VecOps::RVec<GenJet>) );
      instance.SetNew(&new_ROOTcLcLVecOpscLcLRVeclEGenJetgR);
      instance.SetNewArray(&newArray_ROOTcLcLVecOpscLcLRVeclEGenJetgR);
      instance.SetDelete(&delete_ROOTcLcLVecOpscLcLRVeclEGenJetgR);
      instance.SetDeleteArray(&deleteArray_ROOTcLcLVecOpscLcLRVeclEGenJetgR);
      instance.SetDestructor(&destruct_ROOTcLcLVecOpscLcLRVeclEGenJetgR);
      instance.AdoptCollectionProxyInfo(TCollectionProxyInfo::Generate(TCollectionProxyInfo::Pushback< ROOT::VecOps::RVec<GenJet> >()));
      return &instance;
   }
   TGenericClassInfo *GenerateInitInstance(const ROOT::VecOps::RVec<GenJet>*)
   {
      return GenerateInitInstanceLocal(static_cast<ROOT::VecOps::RVec<GenJet>*>(nullptr));
   }
   // Static variable to force the class initialization
   static ::ROOT::TGenericClassInfo *_R__UNIQUE_DICT_(Init) = GenerateInitInstanceLocal(static_cast<const ROOT::VecOps::RVec<GenJet>*>(nullptr)); R__UseDummy(_R__UNIQUE_DICT_(Init));

   // Dictionary for non-ClassDef classes
   static TClass *ROOTcLcLVecOpscLcLRVeclEGenJetgR_Dictionary() {
      TClass* theClass =::ROOT::GenerateInitInstanceLocal(static_cast<const ROOT::VecOps::RVec<GenJet>*>(nullptr))->GetClass();
      ROOTcLcLVecOpscLcLRVeclEGenJetgR_TClassManip(theClass);
   return theClass;
   }

   static void ROOTcLcLVecOpscLcLRVeclEGenJetgR_TClassManip(TClass* ){
   }

} // end of namespace ROOT

namespace ROOT {
   // Wrappers around operator new
   static void *new_ROOTcLcLVecOpscLcLRVeclEGenJetgR(void *p) {
      return  p ? ::new(static_cast<::ROOT::Internal::TOperatorNewHelper*>(p)) ROOT::VecOps::RVec<GenJet> : new ROOT::VecOps::RVec<GenJet>;
   }
   static void *newArray_ROOTcLcLVecOpscLcLRVeclEGenJetgR(Long_t nElements, void *p) {
      return p ? ::new(static_cast<::ROOT::Internal::TOperatorNewHelper*>(p)) ROOT::VecOps::RVec<GenJet>[nElements] : new ROOT::VecOps::RVec<GenJet>[nElements];
   }
   // Wrapper around operator delete
   static void delete_ROOTcLcLVecOpscLcLRVeclEGenJetgR(void *p) {
      delete (static_cast<ROOT::VecOps::RVec<GenJet>*>(p));
   }
   static void deleteArray_ROOTcLcLVecOpscLcLRVeclEGenJetgR(void *p) {
      delete [] (static_cast<ROOT::VecOps::RVec<GenJet>*>(p));
   }
   static void destruct_ROOTcLcLVecOpscLcLRVeclEGenJetgR(void *p) {
      typedef ROOT::VecOps::RVec<GenJet> current_t;
      (static_cast<current_t*>(p))->~current_t();
   }
} // end of namespace ROOT for class ROOT::VecOps::RVec<GenJet>

namespace ROOT {
   static TClass *ROOTcLcLVecOpscLcLRVeclEGenIsolatedPhotongR_Dictionary();
   static void ROOTcLcLVecOpscLcLRVeclEGenIsolatedPhotongR_TClassManip(TClass*);
   static void *new_ROOTcLcLVecOpscLcLRVeclEGenIsolatedPhotongR(void *p = nullptr);
   static void *newArray_ROOTcLcLVecOpscLcLRVeclEGenIsolatedPhotongR(Long_t size, void *p);
   static void delete_ROOTcLcLVecOpscLcLRVeclEGenIsolatedPhotongR(void *p);
   static void deleteArray_ROOTcLcLVecOpscLcLRVeclEGenIsolatedPhotongR(void *p);
   static void destruct_ROOTcLcLVecOpscLcLRVeclEGenIsolatedPhotongR(void *p);

   // Function generating the singleton type initializer
   static TGenericClassInfo *GenerateInitInstanceLocal(const ROOT::VecOps::RVec<GenIsolatedPhoton>*)
   {
      ROOT::VecOps::RVec<GenIsolatedPhoton> *ptr = nullptr;
      static ::TVirtualIsAProxy* isa_proxy = new ::TIsAProxy(typeid(ROOT::VecOps::RVec<GenIsolatedPhoton>));
      static ::ROOT::TGenericClassInfo 
         instance("ROOT::VecOps::RVec<GenIsolatedPhoton>", -2, "ROOT/RVec.hxx", 1529,
                  typeid(ROOT::VecOps::RVec<GenIsolatedPhoton>), ::ROOT::Internal::DefineBehavior(ptr, ptr),
                  &ROOTcLcLVecOpscLcLRVeclEGenIsolatedPhotongR_Dictionary, isa_proxy, 0,
                  sizeof(ROOT::VecOps::RVec<GenIsolatedPhoton>) );
      instance.SetNew(&new_ROOTcLcLVecOpscLcLRVeclEGenIsolatedPhotongR);
      instance.SetNewArray(&newArray_ROOTcLcLVecOpscLcLRVeclEGenIsolatedPhotongR);
      instance.SetDelete(&delete_ROOTcLcLVecOpscLcLRVeclEGenIsolatedPhotongR);
      instance.SetDeleteArray(&deleteArray_ROOTcLcLVecOpscLcLRVeclEGenIsolatedPhotongR);
      instance.SetDestructor(&destruct_ROOTcLcLVecOpscLcLRVeclEGenIsolatedPhotongR);
      instance.AdoptCollectionProxyInfo(TCollectionProxyInfo::Generate(TCollectionProxyInfo::Pushback< ROOT::VecOps::RVec<GenIsolatedPhoton> >()));
      return &instance;
   }
   TGenericClassInfo *GenerateInitInstance(const ROOT::VecOps::RVec<GenIsolatedPhoton>*)
   {
      return GenerateInitInstanceLocal(static_cast<ROOT::VecOps::RVec<GenIsolatedPhoton>*>(nullptr));
   }
   // Static variable to force the class initialization
   static ::ROOT::TGenericClassInfo *_R__UNIQUE_DICT_(Init) = GenerateInitInstanceLocal(static_cast<const ROOT::VecOps::RVec<GenIsolatedPhoton>*>(nullptr)); R__UseDummy(_R__UNIQUE_DICT_(Init));

   // Dictionary for non-ClassDef classes
   static TClass *ROOTcLcLVecOpscLcLRVeclEGenIsolatedPhotongR_Dictionary() {
      TClass* theClass =::ROOT::GenerateInitInstanceLocal(static_cast<const ROOT::VecOps::RVec<GenIsolatedPhoton>*>(nullptr))->GetClass();
      ROOTcLcLVecOpscLcLRVeclEGenIsolatedPhotongR_TClassManip(theClass);
   return theClass;
   }

   static void ROOTcLcLVecOpscLcLRVeclEGenIsolatedPhotongR_TClassManip(TClass* ){
   }

} // end of namespace ROOT

namespace ROOT {
   // Wrappers around operator new
   static void *new_ROOTcLcLVecOpscLcLRVeclEGenIsolatedPhotongR(void *p) {
      return  p ? ::new(static_cast<::ROOT::Internal::TOperatorNewHelper*>(p)) ROOT::VecOps::RVec<GenIsolatedPhoton> : new ROOT::VecOps::RVec<GenIsolatedPhoton>;
   }
   static void *newArray_ROOTcLcLVecOpscLcLRVeclEGenIsolatedPhotongR(Long_t nElements, void *p) {
      return p ? ::new(static_cast<::ROOT::Internal::TOperatorNewHelper*>(p)) ROOT::VecOps::RVec<GenIsolatedPhoton>[nElements] : new ROOT::VecOps::RVec<GenIsolatedPhoton>[nElements];
   }
   // Wrapper around operator delete
   static void delete_ROOTcLcLVecOpscLcLRVeclEGenIsolatedPhotongR(void *p) {
      delete (static_cast<ROOT::VecOps::RVec<GenIsolatedPhoton>*>(p));
   }
   static void deleteArray_ROOTcLcLVecOpscLcLRVeclEGenIsolatedPhotongR(void *p) {
      delete [] (static_cast<ROOT::VecOps::RVec<GenIsolatedPhoton>*>(p));
   }
   static void destruct_ROOTcLcLVecOpscLcLRVeclEGenIsolatedPhotongR(void *p) {
      typedef ROOT::VecOps::RVec<GenIsolatedPhoton> current_t;
      (static_cast<current_t*>(p))->~current_t();
   }
} // end of namespace ROOT for class ROOT::VecOps::RVec<GenIsolatedPhoton>

namespace ROOT {
   static TClass *ROOTcLcLVecOpscLcLRVeclEGenDressedLeptongR_Dictionary();
   static void ROOTcLcLVecOpscLcLRVeclEGenDressedLeptongR_TClassManip(TClass*);
   static void *new_ROOTcLcLVecOpscLcLRVeclEGenDressedLeptongR(void *p = nullptr);
   static void *newArray_ROOTcLcLVecOpscLcLRVeclEGenDressedLeptongR(Long_t size, void *p);
   static void delete_ROOTcLcLVecOpscLcLRVeclEGenDressedLeptongR(void *p);
   static void deleteArray_ROOTcLcLVecOpscLcLRVeclEGenDressedLeptongR(void *p);
   static void destruct_ROOTcLcLVecOpscLcLRVeclEGenDressedLeptongR(void *p);

   // Function generating the singleton type initializer
   static TGenericClassInfo *GenerateInitInstanceLocal(const ROOT::VecOps::RVec<GenDressedLepton>*)
   {
      ROOT::VecOps::RVec<GenDressedLepton> *ptr = nullptr;
      static ::TVirtualIsAProxy* isa_proxy = new ::TIsAProxy(typeid(ROOT::VecOps::RVec<GenDressedLepton>));
      static ::ROOT::TGenericClassInfo 
         instance("ROOT::VecOps::RVec<GenDressedLepton>", -2, "ROOT/RVec.hxx", 1529,
                  typeid(ROOT::VecOps::RVec<GenDressedLepton>), ::ROOT::Internal::DefineBehavior(ptr, ptr),
                  &ROOTcLcLVecOpscLcLRVeclEGenDressedLeptongR_Dictionary, isa_proxy, 0,
                  sizeof(ROOT::VecOps::RVec<GenDressedLepton>) );
      instance.SetNew(&new_ROOTcLcLVecOpscLcLRVeclEGenDressedLeptongR);
      instance.SetNewArray(&newArray_ROOTcLcLVecOpscLcLRVeclEGenDressedLeptongR);
      instance.SetDelete(&delete_ROOTcLcLVecOpscLcLRVeclEGenDressedLeptongR);
      instance.SetDeleteArray(&deleteArray_ROOTcLcLVecOpscLcLRVeclEGenDressedLeptongR);
      instance.SetDestructor(&destruct_ROOTcLcLVecOpscLcLRVeclEGenDressedLeptongR);
      instance.AdoptCollectionProxyInfo(TCollectionProxyInfo::Generate(TCollectionProxyInfo::Pushback< ROOT::VecOps::RVec<GenDressedLepton> >()));
      return &instance;
   }
   TGenericClassInfo *GenerateInitInstance(const ROOT::VecOps::RVec<GenDressedLepton>*)
   {
      return GenerateInitInstanceLocal(static_cast<ROOT::VecOps::RVec<GenDressedLepton>*>(nullptr));
   }
   // Static variable to force the class initialization
   static ::ROOT::TGenericClassInfo *_R__UNIQUE_DICT_(Init) = GenerateInitInstanceLocal(static_cast<const ROOT::VecOps::RVec<GenDressedLepton>*>(nullptr)); R__UseDummy(_R__UNIQUE_DICT_(Init));

   // Dictionary for non-ClassDef classes
   static TClass *ROOTcLcLVecOpscLcLRVeclEGenDressedLeptongR_Dictionary() {
      TClass* theClass =::ROOT::GenerateInitInstanceLocal(static_cast<const ROOT::VecOps::RVec<GenDressedLepton>*>(nullptr))->GetClass();
      ROOTcLcLVecOpscLcLRVeclEGenDressedLeptongR_TClassManip(theClass);
   return theClass;
   }

   static void ROOTcLcLVecOpscLcLRVeclEGenDressedLeptongR_TClassManip(TClass* ){
   }

} // end of namespace ROOT

namespace ROOT {
   // Wrappers around operator new
   static void *new_ROOTcLcLVecOpscLcLRVeclEGenDressedLeptongR(void *p) {
      return  p ? ::new(static_cast<::ROOT::Internal::TOperatorNewHelper*>(p)) ROOT::VecOps::RVec<GenDressedLepton> : new ROOT::VecOps::RVec<GenDressedLepton>;
   }
   static void *newArray_ROOTcLcLVecOpscLcLRVeclEGenDressedLeptongR(Long_t nElements, void *p) {
      return p ? ::new(static_cast<::ROOT::Internal::TOperatorNewHelper*>(p)) ROOT::VecOps::RVec<GenDressedLepton>[nElements] : new ROOT::VecOps::RVec<GenDressedLepton>[nElements];
   }
   // Wrapper around operator delete
   static void delete_ROOTcLcLVecOpscLcLRVeclEGenDressedLeptongR(void *p) {
      delete (static_cast<ROOT::VecOps::RVec<GenDressedLepton>*>(p));
   }
   static void deleteArray_ROOTcLcLVecOpscLcLRVeclEGenDressedLeptongR(void *p) {
      delete [] (static_cast<ROOT::VecOps::RVec<GenDressedLepton>*>(p));
   }
   static void destruct_ROOTcLcLVecOpscLcLRVeclEGenDressedLeptongR(void *p) {
      typedef ROOT::VecOps::RVec<GenDressedLepton> current_t;
      (static_cast<current_t*>(p))->~current_t();
   }
} // end of namespace ROOT for class ROOT::VecOps::RVec<GenDressedLepton>

namespace ROOT {
   static TClass *ROOTcLcLVecOpscLcLRVeclEGengR_Dictionary();
   static void ROOTcLcLVecOpscLcLRVeclEGengR_TClassManip(TClass*);
   static void *new_ROOTcLcLVecOpscLcLRVeclEGengR(void *p = nullptr);
   static void *newArray_ROOTcLcLVecOpscLcLRVeclEGengR(Long_t size, void *p);
   static void delete_ROOTcLcLVecOpscLcLRVeclEGengR(void *p);
   static void deleteArray_ROOTcLcLVecOpscLcLRVeclEGengR(void *p);
   static void destruct_ROOTcLcLVecOpscLcLRVeclEGengR(void *p);

   // Function generating the singleton type initializer
   static TGenericClassInfo *GenerateInitInstanceLocal(const ROOT::VecOps::RVec<Gen>*)
   {
      ROOT::VecOps::RVec<Gen> *ptr = nullptr;
      static ::TVirtualIsAProxy* isa_proxy = new ::TIsAProxy(typeid(ROOT::VecOps::RVec<Gen>));
      static ::ROOT::TGenericClassInfo 
         instance("ROOT::VecOps::RVec<Gen>", -2, "ROOT/RVec.hxx", 1529,
                  typeid(ROOT::VecOps::RVec<Gen>), ::ROOT::Internal::DefineBehavior(ptr, ptr),
                  &ROOTcLcLVecOpscLcLRVeclEGengR_Dictionary, isa_proxy, 0,
                  sizeof(ROOT::VecOps::RVec<Gen>) );
      instance.SetNew(&new_ROOTcLcLVecOpscLcLRVeclEGengR);
      instance.SetNewArray(&newArray_ROOTcLcLVecOpscLcLRVeclEGengR);
      instance.SetDelete(&delete_ROOTcLcLVecOpscLcLRVeclEGengR);
      instance.SetDeleteArray(&deleteArray_ROOTcLcLVecOpscLcLRVeclEGengR);
      instance.SetDestructor(&destruct_ROOTcLcLVecOpscLcLRVeclEGengR);
      instance.AdoptCollectionProxyInfo(TCollectionProxyInfo::Generate(TCollectionProxyInfo::Pushback< ROOT::VecOps::RVec<Gen> >()));
      return &instance;
   }
   TGenericClassInfo *GenerateInitInstance(const ROOT::VecOps::RVec<Gen>*)
   {
      return GenerateInitInstanceLocal(static_cast<ROOT::VecOps::RVec<Gen>*>(nullptr));
   }
   // Static variable to force the class initialization
   static ::ROOT::TGenericClassInfo *_R__UNIQUE_DICT_(Init) = GenerateInitInstanceLocal(static_cast<const ROOT::VecOps::RVec<Gen>*>(nullptr)); R__UseDummy(_R__UNIQUE_DICT_(Init));

   // Dictionary for non-ClassDef classes
   static TClass *ROOTcLcLVecOpscLcLRVeclEGengR_Dictionary() {
      TClass* theClass =::ROOT::GenerateInitInstanceLocal(static_cast<const ROOT::VecOps::RVec<Gen>*>(nullptr))->GetClass();
      ROOTcLcLVecOpscLcLRVeclEGengR_TClassManip(theClass);
   return theClass;
   }

   static void ROOTcLcLVecOpscLcLRVeclEGengR_TClassManip(TClass* ){
   }

} // end of namespace ROOT

namespace ROOT {
   // Wrappers around operator new
   static void *new_ROOTcLcLVecOpscLcLRVeclEGengR(void *p) {
      return  p ? ::new(static_cast<::ROOT::Internal::TOperatorNewHelper*>(p)) ROOT::VecOps::RVec<Gen> : new ROOT::VecOps::RVec<Gen>;
   }
   static void *newArray_ROOTcLcLVecOpscLcLRVeclEGengR(Long_t nElements, void *p) {
      return p ? ::new(static_cast<::ROOT::Internal::TOperatorNewHelper*>(p)) ROOT::VecOps::RVec<Gen>[nElements] : new ROOT::VecOps::RVec<Gen>[nElements];
   }
   // Wrapper around operator delete
   static void delete_ROOTcLcLVecOpscLcLRVeclEGengR(void *p) {
      delete (static_cast<ROOT::VecOps::RVec<Gen>*>(p));
   }
   static void deleteArray_ROOTcLcLVecOpscLcLRVeclEGengR(void *p) {
      delete [] (static_cast<ROOT::VecOps::RVec<Gen>*>(p));
   }
   static void destruct_ROOTcLcLVecOpscLcLRVeclEGengR(void *p) {
      typedef ROOT::VecOps::RVec<Gen> current_t;
      (static_cast<current_t*>(p))->~current_t();
   }
} // end of namespace ROOT for class ROOT::VecOps::RVec<Gen>

namespace ROOT {
   static TClass *ROOTcLcLVecOpscLcLRVeclEFatJetgR_Dictionary();
   static void ROOTcLcLVecOpscLcLRVeclEFatJetgR_TClassManip(TClass*);
   static void *new_ROOTcLcLVecOpscLcLRVeclEFatJetgR(void *p = nullptr);
   static void *newArray_ROOTcLcLVecOpscLcLRVeclEFatJetgR(Long_t size, void *p);
   static void delete_ROOTcLcLVecOpscLcLRVeclEFatJetgR(void *p);
   static void deleteArray_ROOTcLcLVecOpscLcLRVeclEFatJetgR(void *p);
   static void destruct_ROOTcLcLVecOpscLcLRVeclEFatJetgR(void *p);

   // Function generating the singleton type initializer
   static TGenericClassInfo *GenerateInitInstanceLocal(const ROOT::VecOps::RVec<FatJet>*)
   {
      ROOT::VecOps::RVec<FatJet> *ptr = nullptr;
      static ::TVirtualIsAProxy* isa_proxy = new ::TIsAProxy(typeid(ROOT::VecOps::RVec<FatJet>));
      static ::ROOT::TGenericClassInfo 
         instance("ROOT::VecOps::RVec<FatJet>", -2, "ROOT/RVec.hxx", 1529,
                  typeid(ROOT::VecOps::RVec<FatJet>), ::ROOT::Internal::DefineBehavior(ptr, ptr),
                  &ROOTcLcLVecOpscLcLRVeclEFatJetgR_Dictionary, isa_proxy, 0,
                  sizeof(ROOT::VecOps::RVec<FatJet>) );
      instance.SetNew(&new_ROOTcLcLVecOpscLcLRVeclEFatJetgR);
      instance.SetNewArray(&newArray_ROOTcLcLVecOpscLcLRVeclEFatJetgR);
      instance.SetDelete(&delete_ROOTcLcLVecOpscLcLRVeclEFatJetgR);
      instance.SetDeleteArray(&deleteArray_ROOTcLcLVecOpscLcLRVeclEFatJetgR);
      instance.SetDestructor(&destruct_ROOTcLcLVecOpscLcLRVeclEFatJetgR);
      instance.AdoptCollectionProxyInfo(TCollectionProxyInfo::Generate(TCollectionProxyInfo::Pushback< ROOT::VecOps::RVec<FatJet> >()));
      return &instance;
   }
   TGenericClassInfo *GenerateInitInstance(const ROOT::VecOps::RVec<FatJet>*)
   {
      return GenerateInitInstanceLocal(static_cast<ROOT::VecOps::RVec<FatJet>*>(nullptr));
   }
   // Static variable to force the class initialization
   static ::ROOT::TGenericClassInfo *_R__UNIQUE_DICT_(Init) = GenerateInitInstanceLocal(static_cast<const ROOT::VecOps::RVec<FatJet>*>(nullptr)); R__UseDummy(_R__UNIQUE_DICT_(Init));

   // Dictionary for non-ClassDef classes
   static TClass *ROOTcLcLVecOpscLcLRVeclEFatJetgR_Dictionary() {
      TClass* theClass =::ROOT::GenerateInitInstanceLocal(static_cast<const ROOT::VecOps::RVec<FatJet>*>(nullptr))->GetClass();
      ROOTcLcLVecOpscLcLRVeclEFatJetgR_TClassManip(theClass);
   return theClass;
   }

   static void ROOTcLcLVecOpscLcLRVeclEFatJetgR_TClassManip(TClass* ){
   }

} // end of namespace ROOT

namespace ROOT {
   // Wrappers around operator new
   static void *new_ROOTcLcLVecOpscLcLRVeclEFatJetgR(void *p) {
      return  p ? ::new(static_cast<::ROOT::Internal::TOperatorNewHelper*>(p)) ROOT::VecOps::RVec<FatJet> : new ROOT::VecOps::RVec<FatJet>;
   }
   static void *newArray_ROOTcLcLVecOpscLcLRVeclEFatJetgR(Long_t nElements, void *p) {
      return p ? ::new(static_cast<::ROOT::Internal::TOperatorNewHelper*>(p)) ROOT::VecOps::RVec<FatJet>[nElements] : new ROOT::VecOps::RVec<FatJet>[nElements];
   }
   // Wrapper around operator delete
   static void delete_ROOTcLcLVecOpscLcLRVeclEFatJetgR(void *p) {
      delete (static_cast<ROOT::VecOps::RVec<FatJet>*>(p));
   }
   static void deleteArray_ROOTcLcLVecOpscLcLRVeclEFatJetgR(void *p) {
      delete [] (static_cast<ROOT::VecOps::RVec<FatJet>*>(p));
   }
   static void destruct_ROOTcLcLVecOpscLcLRVeclEFatJetgR(void *p) {
      typedef ROOT::VecOps::RVec<FatJet> current_t;
      (static_cast<current_t*>(p))->~current_t();
   }
} // end of namespace ROOT for class ROOT::VecOps::RVec<FatJet>

namespace ROOT {
   static TClass *ROOTcLcLVecOpscLcLRVeclEElectrongR_Dictionary();
   static void ROOTcLcLVecOpscLcLRVeclEElectrongR_TClassManip(TClass*);
   static void *new_ROOTcLcLVecOpscLcLRVeclEElectrongR(void *p = nullptr);
   static void *newArray_ROOTcLcLVecOpscLcLRVeclEElectrongR(Long_t size, void *p);
   static void delete_ROOTcLcLVecOpscLcLRVeclEElectrongR(void *p);
   static void deleteArray_ROOTcLcLVecOpscLcLRVeclEElectrongR(void *p);
   static void destruct_ROOTcLcLVecOpscLcLRVeclEElectrongR(void *p);

   // Function generating the singleton type initializer
   static TGenericClassInfo *GenerateInitInstanceLocal(const ROOT::VecOps::RVec<Electron>*)
   {
      ROOT::VecOps::RVec<Electron> *ptr = nullptr;
      static ::TVirtualIsAProxy* isa_proxy = new ::TIsAProxy(typeid(ROOT::VecOps::RVec<Electron>));
      static ::ROOT::TGenericClassInfo 
         instance("ROOT::VecOps::RVec<Electron>", -2, "ROOT/RVec.hxx", 1529,
                  typeid(ROOT::VecOps::RVec<Electron>), ::ROOT::Internal::DefineBehavior(ptr, ptr),
                  &ROOTcLcLVecOpscLcLRVeclEElectrongR_Dictionary, isa_proxy, 0,
                  sizeof(ROOT::VecOps::RVec<Electron>) );
      instance.SetNew(&new_ROOTcLcLVecOpscLcLRVeclEElectrongR);
      instance.SetNewArray(&newArray_ROOTcLcLVecOpscLcLRVeclEElectrongR);
      instance.SetDelete(&delete_ROOTcLcLVecOpscLcLRVeclEElectrongR);
      instance.SetDeleteArray(&deleteArray_ROOTcLcLVecOpscLcLRVeclEElectrongR);
      instance.SetDestructor(&destruct_ROOTcLcLVecOpscLcLRVeclEElectrongR);
      instance.AdoptCollectionProxyInfo(TCollectionProxyInfo::Generate(TCollectionProxyInfo::Pushback< ROOT::VecOps::RVec<Electron> >()));
      return &instance;
   }
   TGenericClassInfo *GenerateInitInstance(const ROOT::VecOps::RVec<Electron>*)
   {
      return GenerateInitInstanceLocal(static_cast<ROOT::VecOps::RVec<Electron>*>(nullptr));
   }
   // Static variable to force the class initialization
   static ::ROOT::TGenericClassInfo *_R__UNIQUE_DICT_(Init) = GenerateInitInstanceLocal(static_cast<const ROOT::VecOps::RVec<Electron>*>(nullptr)); R__UseDummy(_R__UNIQUE_DICT_(Init));

   // Dictionary for non-ClassDef classes
   static TClass *ROOTcLcLVecOpscLcLRVeclEElectrongR_Dictionary() {
      TClass* theClass =::ROOT::GenerateInitInstanceLocal(static_cast<const ROOT::VecOps::RVec<Electron>*>(nullptr))->GetClass();
      ROOTcLcLVecOpscLcLRVeclEElectrongR_TClassManip(theClass);
   return theClass;
   }

   static void ROOTcLcLVecOpscLcLRVeclEElectrongR_TClassManip(TClass* ){
   }

} // end of namespace ROOT

namespace ROOT {
   // Wrappers around operator new
   static void *new_ROOTcLcLVecOpscLcLRVeclEElectrongR(void *p) {
      return  p ? ::new(static_cast<::ROOT::Internal::TOperatorNewHelper*>(p)) ROOT::VecOps::RVec<Electron> : new ROOT::VecOps::RVec<Electron>;
   }
   static void *newArray_ROOTcLcLVecOpscLcLRVeclEElectrongR(Long_t nElements, void *p) {
      return p ? ::new(static_cast<::ROOT::Internal::TOperatorNewHelper*>(p)) ROOT::VecOps::RVec<Electron>[nElements] : new ROOT::VecOps::RVec<Electron>[nElements];
   }
   // Wrapper around operator delete
   static void delete_ROOTcLcLVecOpscLcLRVeclEElectrongR(void *p) {
      delete (static_cast<ROOT::VecOps::RVec<Electron>*>(p));
   }
   static void deleteArray_ROOTcLcLVecOpscLcLRVeclEElectrongR(void *p) {
      delete [] (static_cast<ROOT::VecOps::RVec<Electron>*>(p));
   }
   static void destruct_ROOTcLcLVecOpscLcLRVeclEElectrongR(void *p) {
      typedef ROOT::VecOps::RVec<Electron> current_t;
      (static_cast<current_t*>(p))->~current_t();
   }
} // end of namespace ROOT for class ROOT::VecOps::RVec<Electron>

namespace {
  void TriggerDictionaryInitialization_libAnalyzers_Impl() {
    static const char* headers[] = {
"include/AnalyzerCore.h",
"include/DY.h",
"include/DiLepton.h",
"include/DiLeptonBase.h",
"include/FullHadronicTriggerTnP.h",
"include/GeneratorBase.h",
"include/GetEffLumi.h",
"include/LRSM_TBChannel.h",
"include/LRSM_TBChannel_notusingbjet.h",
"include/LRSM_TBChannel_wtagging.h",
"include/MeasTrigEff.h",
"include/MeasureJetTaggingEff.h",
"include/MuonTnPProducer.h",
"include/ParseEleIDVariables.h",
"include/ParseMuIDVariables.h",
"include/Reproduce20_002.h",
"include/SKNanoLoader.h",
"include/TTbar_test.h",
"include/TestGetLeptonType.h",
"include/ttbar_hadronic.h",
nullptr
    };
    static const char* includePaths[] = {
"/data6/Users/achihwan/miniforge3/envs/Nano/include",
"/home/achihwan/SKNanoAnalyzer/Analyzers",
"/home/achihwan/SKNanoAnalyzer/external/lhapdf/redhat/include",
"/data6/Users/achihwan/miniforge3/envs/Nano/lib/python3.12/site-packages/correctionlib/include",
"/home/achihwan/SKNanoAnalyzer/Analyzers/include",
"/home/achihwan/SKNanoAnalyzer/Analyzers/../AnalyzerTools/include",
"/home/achihwan/SKNanoAnalyzer/Analyzers/../DataFormats/include",
"/home/achihwan/SKNanoAnalyzer/Analyzers/../external/KinematicFitter/include",
"/data6/Users/achihwan/miniforge3/envs/Nano/include",
"/home/achihwan/SKNanoAnalyzer/external/lhapdf/redhat/include",
"/data6/Users/achihwan/miniforge3/envs/Nano/lib/python3.12/site-packages/correctionlib/include",
"/home/achihwan/SKNanoAnalyzer/Analyzers/include",
"/home/achihwan/SKNanoAnalyzer/Analyzers/../AnalyzerTools/include",
"/home/achihwan/SKNanoAnalyzer/Analyzers/../DataFormats/include",
"/home/achihwan/SKNanoAnalyzer/Analyzers/../external/KinematicFitter/include",
"/home/achihwan/SKNanoAnalyzer/AnalyzerTools/include",
"/home/achihwan/SKNanoAnalyzer/AnalyzerTools/../DataFormats/include",
"/home/achihwan/SKNanoAnalyzer/external/RoccoR",
"/home/achihwan/SKNanoAnalyzer/_deps/yaml-cpp-src/include",
"/data6/Users/achihwan/miniforge3/envs/Nano/include/onnxruntime/core/session",
"/home/achihwan/SKNanoAnalyzer/external/KinematicFitter/include",
"/home/achihwan/SKNanoAnalyzer/_deps/json-src/include/",
"/data6/Users/achihwan/miniforge3/envs/Nano/include/",
"/data6/Users/achihwan/SKNanoAnalyzer/Analyzers/",
nullptr
    };
    static const char* fwdDeclCode = R"DICTFWDDCLS(
#line 1 "libAnalyzers dictionary forward declarations' payload"
#pragma clang diagnostic ignored "-Wkeyword-compat"
#pragma clang diagnostic ignored "-Wignored-attributes"
#pragma clang diagnostic ignored "-Wreturn-type-c-linkage"
extern int __Cling_AutoLoading_Map;
class __attribute__((annotate("$clingAutoload$SKNanoLoader.h")))  __attribute__((annotate("$clingAutoload$include/AnalyzerCore.h")))  SKNanoLoader;
class __attribute__((annotate("$clingAutoload$include/AnalyzerCore.h")))  AnalyzerCore;
class __attribute__((annotate("$clingAutoload$include/DY.h")))  DY;
class __attribute__((annotate("$clingAutoload$DiLeptonBase.h")))  __attribute__((annotate("$clingAutoload$include/DiLepton.h")))  DiLeptonBase;
class __attribute__((annotate("$clingAutoload$include/FullHadronicTriggerTnP.h")))  FullHadronicTriggerTnP;
class __attribute__((annotate("$clingAutoload$include/GeneratorBase.h")))  GeneratorBase;
class __attribute__((annotate("$clingAutoload$include/GetEffLumi.h")))  GetEffLumi;
class __attribute__((annotate("$clingAutoload$include/LRSM_TBChannel.h")))  LRSM_TBChannel;
class __attribute__((annotate("$clingAutoload$include/LRSM_TBChannel_notusingbjet.h")))  LRSM_TBChannel_notusingbjet;
class __attribute__((annotate("$clingAutoload$include/LRSM_TBChannel_wtagging.h")))  LRSM_TBChannel_wtagging;
class __attribute__((annotate("$clingAutoload$include/MeasureJetTaggingEff.h")))  MeasureJetTaggingEff;
class __attribute__((annotate("$clingAutoload$include/ParseEleIDVariables.h")))  ParseEleIDVariables;
class __attribute__((annotate("$clingAutoload$include/ParseMuIDVariables.h")))  ParseMuIDVariables;
class __attribute__((annotate("$clingAutoload$include/Reproduce20_002.h")))  Reproduce20_002;
class __attribute__((annotate("$clingAutoload$include/TTbar_test.h")))  TTbar_test;
class __attribute__((annotate("$clingAutoload$include/TestGetLeptonType.h")))  TestGetLeptonType;
class __attribute__((annotate("$clingAutoload$include/ttbar_hadronic.h")))  ttbar_hadronic;
)DICTFWDDCLS";
    static const char* payloadCode = R"DICTPAYLOAD(
#line 1 "libAnalyzers dictionary payload"

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
#include "include/AnalyzerCore.h"
#include "include/DY.h"
#include "include/DiLepton.h"
#include "include/DiLeptonBase.h"
#include "include/FullHadronicTriggerTnP.h"
#include "include/GeneratorBase.h"
#include "include/GetEffLumi.h"
#include "include/LRSM_TBChannel.h"
#include "include/LRSM_TBChannel_notusingbjet.h"
#include "include/LRSM_TBChannel_wtagging.h"
#include "include/MeasTrigEff.h"
#include "include/MeasureJetTaggingEff.h"
#include "include/MuonTnPProducer.h"
#include "include/ParseEleIDVariables.h"
#include "include/ParseMuIDVariables.h"
#include "include/Reproduce20_002.h"
#include "include/SKNanoLoader.h"
#include "include/TTbar_test.h"
#include "include/TestGetLeptonType.h"
#include "include/ttbar_hadronic.h"

#undef  _BACKWARD_BACKWARD_WARNING_H
)DICTPAYLOAD";
    static const char* classesHeaders[] = {
"AnalyzerCore", payloadCode, "@",
"DY", payloadCode, "@",
"DiLeptonBase", payloadCode, "@",
"FullHadronicTriggerTnP", payloadCode, "@",
"GeneratorBase", payloadCode, "@",
"GetEffLumi", payloadCode, "@",
"LRSM_TBChannel", payloadCode, "@",
"LRSM_TBChannel_notusingbjet", payloadCode, "@",
"LRSM_TBChannel_wtagging", payloadCode, "@",
"MeasureJetTaggingEff", payloadCode, "@",
"ParseEleIDVariables", payloadCode, "@",
"ParseMuIDVariables", payloadCode, "@",
"Reproduce20_002", payloadCode, "@",
"SKNanoLoader", payloadCode, "@",
"TTbar_test", payloadCode, "@",
"TestGetLeptonType", payloadCode, "@",
"ttbar_hadronic", payloadCode, "@",
nullptr
};
    static bool isInitialized = false;
    if (!isInitialized) {
      TROOT::RegisterModule("libAnalyzers",
        headers, includePaths, payloadCode, fwdDeclCode,
        TriggerDictionaryInitialization_libAnalyzers_Impl, {}, classesHeaders, /*hasCxxModule*/false);
      isInitialized = true;
    }
  }
  static struct DictInit {
    DictInit() {
      TriggerDictionaryInitialization_libAnalyzers_Impl();
    }
  } __TheDictionaryInitializer;
}
void TriggerDictionaryInitialization_libAnalyzers() {
  TriggerDictionaryInitialization_libAnalyzers_Impl();
}

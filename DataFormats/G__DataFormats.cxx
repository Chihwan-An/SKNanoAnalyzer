// Do NOT change. Changes will be lost next time file is generated

#define R__DICTIONARY_FILENAME G__DataFormats
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
#include "include/Electron.h"
#include "include/Event.h"
#include "include/FatJet.h"
#include "include/Gen.h"
#include "include/GenDressedLepton.h"
#include "include/GenIsolatedPhoton.h"
#include "include/GenJet.h"
#include "include/GenVisTau.h"
#include "include/Jet.h"
#include "include/JetTaggingParameter.h"
#include "include/LHE.h"
#include "include/Lepton.h"
#include "include/Muon.h"
#include "include/Particle.h"
#include "include/Photon.h"
#include "include/Tau.h"
#include "include/TrigObj.h"

// Header files passed via #pragma extra_include

// The generated code does not explicitly qualify STL entities
namespace std {} using namespace std;

namespace ROOT {
   static void *new_Particle(void *p = nullptr);
   static void *newArray_Particle(Long_t size, void *p);
   static void delete_Particle(void *p);
   static void deleteArray_Particle(void *p);
   static void destruct_Particle(void *p);

   // Function generating the singleton type initializer
   static TGenericClassInfo *GenerateInitInstanceLocal(const ::Particle*)
   {
      ::Particle *ptr = nullptr;
      static ::TVirtualIsAProxy* isa_proxy = new ::TInstrumentedIsAProxy< ::Particle >(nullptr);
      static ::ROOT::TGenericClassInfo 
         instance("Particle", ::Particle::Class_Version(), "Particle.h", 16,
                  typeid(::Particle), ::ROOT::Internal::DefineBehavior(ptr, ptr),
                  &::Particle::Dictionary, isa_proxy, 4,
                  sizeof(::Particle) );
      instance.SetNew(&new_Particle);
      instance.SetNewArray(&newArray_Particle);
      instance.SetDelete(&delete_Particle);
      instance.SetDeleteArray(&deleteArray_Particle);
      instance.SetDestructor(&destruct_Particle);
      return &instance;
   }
   TGenericClassInfo *GenerateInitInstance(const ::Particle*)
   {
      return GenerateInitInstanceLocal(static_cast<::Particle*>(nullptr));
   }
   // Static variable to force the class initialization
   static ::ROOT::TGenericClassInfo *_R__UNIQUE_DICT_(Init) = GenerateInitInstanceLocal(static_cast<const ::Particle*>(nullptr)); R__UseDummy(_R__UNIQUE_DICT_(Init));
} // end of namespace ROOT

namespace ROOT {
   static void *new_Lepton(void *p = nullptr);
   static void *newArray_Lepton(Long_t size, void *p);
   static void delete_Lepton(void *p);
   static void deleteArray_Lepton(void *p);
   static void destruct_Lepton(void *p);

   // Function generating the singleton type initializer
   static TGenericClassInfo *GenerateInitInstanceLocal(const ::Lepton*)
   {
      ::Lepton *ptr = nullptr;
      static ::TVirtualIsAProxy* isa_proxy = new ::TInstrumentedIsAProxy< ::Lepton >(nullptr);
      static ::ROOT::TGenericClassInfo 
         instance("Lepton", ::Lepton::Class_Version(), "Lepton.h", 6,
                  typeid(::Lepton), ::ROOT::Internal::DefineBehavior(ptr, ptr),
                  &::Lepton::Dictionary, isa_proxy, 4,
                  sizeof(::Lepton) );
      instance.SetNew(&new_Lepton);
      instance.SetNewArray(&newArray_Lepton);
      instance.SetDelete(&delete_Lepton);
      instance.SetDeleteArray(&deleteArray_Lepton);
      instance.SetDestructor(&destruct_Lepton);
      return &instance;
   }
   TGenericClassInfo *GenerateInitInstance(const ::Lepton*)
   {
      return GenerateInitInstanceLocal(static_cast<::Lepton*>(nullptr));
   }
   // Static variable to force the class initialization
   static ::ROOT::TGenericClassInfo *_R__UNIQUE_DICT_(Init) = GenerateInitInstanceLocal(static_cast<const ::Lepton*>(nullptr)); R__UseDummy(_R__UNIQUE_DICT_(Init));
} // end of namespace ROOT

namespace ROOT {
   static void *new_Electron(void *p = nullptr);
   static void *newArray_Electron(Long_t size, void *p);
   static void delete_Electron(void *p);
   static void deleteArray_Electron(void *p);
   static void destruct_Electron(void *p);

   // Function generating the singleton type initializer
   static TGenericClassInfo *GenerateInitInstanceLocal(const ::Electron*)
   {
      ::Electron *ptr = nullptr;
      static ::TVirtualIsAProxy* isa_proxy = new ::TInstrumentedIsAProxy< ::Electron >(nullptr);
      static ::ROOT::TGenericClassInfo 
         instance("Electron", ::Electron::Class_Version(), "Electron.h", 14,
                  typeid(::Electron), ::ROOT::Internal::DefineBehavior(ptr, ptr),
                  &::Electron::Dictionary, isa_proxy, 4,
                  sizeof(::Electron) );
      instance.SetNew(&new_Electron);
      instance.SetNewArray(&newArray_Electron);
      instance.SetDelete(&delete_Electron);
      instance.SetDeleteArray(&deleteArray_Electron);
      instance.SetDestructor(&destruct_Electron);
      return &instance;
   }
   TGenericClassInfo *GenerateInitInstance(const ::Electron*)
   {
      return GenerateInitInstanceLocal(static_cast<::Electron*>(nullptr));
   }
   // Static variable to force the class initialization
   static ::ROOT::TGenericClassInfo *_R__UNIQUE_DICT_(Init) = GenerateInitInstanceLocal(static_cast<const ::Electron*>(nullptr)); R__UseDummy(_R__UNIQUE_DICT_(Init));
} // end of namespace ROOT

namespace ROOT {
   static void *new_Jet(void *p = nullptr);
   static void *newArray_Jet(Long_t size, void *p);
   static void delete_Jet(void *p);
   static void deleteArray_Jet(void *p);
   static void destruct_Jet(void *p);

   // Function generating the singleton type initializer
   static TGenericClassInfo *GenerateInitInstanceLocal(const ::Jet*)
   {
      ::Jet *ptr = nullptr;
      static ::TVirtualIsAProxy* isa_proxy = new ::TInstrumentedIsAProxy< ::Jet >(nullptr);
      static ::ROOT::TGenericClassInfo 
         instance("Jet", ::Jet::Class_Version(), "Jet.h", 7,
                  typeid(::Jet), ::ROOT::Internal::DefineBehavior(ptr, ptr),
                  &::Jet::Dictionary, isa_proxy, 4,
                  sizeof(::Jet) );
      instance.SetNew(&new_Jet);
      instance.SetNewArray(&newArray_Jet);
      instance.SetDelete(&delete_Jet);
      instance.SetDeleteArray(&deleteArray_Jet);
      instance.SetDestructor(&destruct_Jet);
      return &instance;
   }
   TGenericClassInfo *GenerateInitInstance(const ::Jet*)
   {
      return GenerateInitInstanceLocal(static_cast<::Jet*>(nullptr));
   }
   // Static variable to force the class initialization
   static ::ROOT::TGenericClassInfo *_R__UNIQUE_DICT_(Init) = GenerateInitInstanceLocal(static_cast<const ::Jet*>(nullptr)); R__UseDummy(_R__UNIQUE_DICT_(Init));
} // end of namespace ROOT

namespace ROOT {
   static void *new_Gen(void *p = nullptr);
   static void *newArray_Gen(Long_t size, void *p);
   static void delete_Gen(void *p);
   static void deleteArray_Gen(void *p);
   static void destruct_Gen(void *p);

   // Function generating the singleton type initializer
   static TGenericClassInfo *GenerateInitInstanceLocal(const ::Gen*)
   {
      ::Gen *ptr = nullptr;
      static ::TVirtualIsAProxy* isa_proxy = new ::TInstrumentedIsAProxy< ::Gen >(nullptr);
      static ::ROOT::TGenericClassInfo 
         instance("Gen", ::Gen::Class_Version(), "Gen.h", 6,
                  typeid(::Gen), ::ROOT::Internal::DefineBehavior(ptr, ptr),
                  &::Gen::Dictionary, isa_proxy, 4,
                  sizeof(::Gen) );
      instance.SetNew(&new_Gen);
      instance.SetNewArray(&newArray_Gen);
      instance.SetDelete(&delete_Gen);
      instance.SetDeleteArray(&deleteArray_Gen);
      instance.SetDestructor(&destruct_Gen);
      return &instance;
   }
   TGenericClassInfo *GenerateInitInstance(const ::Gen*)
   {
      return GenerateInitInstanceLocal(static_cast<::Gen*>(nullptr));
   }
   // Static variable to force the class initialization
   static ::ROOT::TGenericClassInfo *_R__UNIQUE_DICT_(Init) = GenerateInitInstanceLocal(static_cast<const ::Gen*>(nullptr)); R__UseDummy(_R__UNIQUE_DICT_(Init));
} // end of namespace ROOT

namespace ROOT {
   static void *new_Muon(void *p = nullptr);
   static void *newArray_Muon(Long_t size, void *p);
   static void delete_Muon(void *p);
   static void deleteArray_Muon(void *p);
   static void destruct_Muon(void *p);

   // Function generating the singleton type initializer
   static TGenericClassInfo *GenerateInitInstanceLocal(const ::Muon*)
   {
      ::Muon *ptr = nullptr;
      static ::TVirtualIsAProxy* isa_proxy = new ::TInstrumentedIsAProxy< ::Muon >(nullptr);
      static ::ROOT::TGenericClassInfo 
         instance("Muon", ::Muon::Class_Version(), "Muon.h", 15,
                  typeid(::Muon), ::ROOT::Internal::DefineBehavior(ptr, ptr),
                  &::Muon::Dictionary, isa_proxy, 4,
                  sizeof(::Muon) );
      instance.SetNew(&new_Muon);
      instance.SetNewArray(&newArray_Muon);
      instance.SetDelete(&delete_Muon);
      instance.SetDeleteArray(&deleteArray_Muon);
      instance.SetDestructor(&destruct_Muon);
      return &instance;
   }
   TGenericClassInfo *GenerateInitInstance(const ::Muon*)
   {
      return GenerateInitInstanceLocal(static_cast<::Muon*>(nullptr));
   }
   // Static variable to force the class initialization
   static ::ROOT::TGenericClassInfo *_R__UNIQUE_DICT_(Init) = GenerateInitInstanceLocal(static_cast<const ::Muon*>(nullptr)); R__UseDummy(_R__UNIQUE_DICT_(Init));
} // end of namespace ROOT

namespace ROOT {
   static void *new_Event(void *p = nullptr);
   static void *newArray_Event(Long_t size, void *p);
   static void delete_Event(void *p);
   static void deleteArray_Event(void *p);
   static void destruct_Event(void *p);

   // Function generating the singleton type initializer
   static TGenericClassInfo *GenerateInitInstanceLocal(const ::Event*)
   {
      ::Event *ptr = nullptr;
      static ::TVirtualIsAProxy* isa_proxy = new ::TInstrumentedIsAProxy< ::Event >(nullptr);
      static ::ROOT::TGenericClassInfo 
         instance("Event", ::Event::Class_Version(), "Event.h", 11,
                  typeid(::Event), ::ROOT::Internal::DefineBehavior(ptr, ptr),
                  &::Event::Dictionary, isa_proxy, 4,
                  sizeof(::Event) );
      instance.SetNew(&new_Event);
      instance.SetNewArray(&newArray_Event);
      instance.SetDelete(&delete_Event);
      instance.SetDeleteArray(&deleteArray_Event);
      instance.SetDestructor(&destruct_Event);
      return &instance;
   }
   TGenericClassInfo *GenerateInitInstance(const ::Event*)
   {
      return GenerateInitInstanceLocal(static_cast<::Event*>(nullptr));
   }
   // Static variable to force the class initialization
   static ::ROOT::TGenericClassInfo *_R__UNIQUE_DICT_(Init) = GenerateInitInstanceLocal(static_cast<const ::Event*>(nullptr)); R__UseDummy(_R__UNIQUE_DICT_(Init));
} // end of namespace ROOT

namespace ROOT {
   static void *new_FatJet(void *p = nullptr);
   static void *newArray_FatJet(Long_t size, void *p);
   static void delete_FatJet(void *p);
   static void deleteArray_FatJet(void *p);
   static void destruct_FatJet(void *p);

   // Function generating the singleton type initializer
   static TGenericClassInfo *GenerateInitInstanceLocal(const ::FatJet*)
   {
      ::FatJet *ptr = nullptr;
      static ::TVirtualIsAProxy* isa_proxy = new ::TInstrumentedIsAProxy< ::FatJet >(nullptr);
      static ::ROOT::TGenericClassInfo 
         instance("FatJet", ::FatJet::Class_Version(), "FatJet.h", 7,
                  typeid(::FatJet), ::ROOT::Internal::DefineBehavior(ptr, ptr),
                  &::FatJet::Dictionary, isa_proxy, 4,
                  sizeof(::FatJet) );
      instance.SetNew(&new_FatJet);
      instance.SetNewArray(&newArray_FatJet);
      instance.SetDelete(&delete_FatJet);
      instance.SetDeleteArray(&deleteArray_FatJet);
      instance.SetDestructor(&destruct_FatJet);
      return &instance;
   }
   TGenericClassInfo *GenerateInitInstance(const ::FatJet*)
   {
      return GenerateInitInstanceLocal(static_cast<::FatJet*>(nullptr));
   }
   // Static variable to force the class initialization
   static ::ROOT::TGenericClassInfo *_R__UNIQUE_DICT_(Init) = GenerateInitInstanceLocal(static_cast<const ::FatJet*>(nullptr)); R__UseDummy(_R__UNIQUE_DICT_(Init));
} // end of namespace ROOT

namespace ROOT {
   static void *new_GenDressedLepton(void *p = nullptr);
   static void *newArray_GenDressedLepton(Long_t size, void *p);
   static void delete_GenDressedLepton(void *p);
   static void deleteArray_GenDressedLepton(void *p);
   static void destruct_GenDressedLepton(void *p);

   // Function generating the singleton type initializer
   static TGenericClassInfo *GenerateInitInstanceLocal(const ::GenDressedLepton*)
   {
      ::GenDressedLepton *ptr = nullptr;
      static ::TVirtualIsAProxy* isa_proxy = new ::TInstrumentedIsAProxy< ::GenDressedLepton >(nullptr);
      static ::ROOT::TGenericClassInfo 
         instance("GenDressedLepton", ::GenDressedLepton::Class_Version(), "GenDressedLepton.h", 6,
                  typeid(::GenDressedLepton), ::ROOT::Internal::DefineBehavior(ptr, ptr),
                  &::GenDressedLepton::Dictionary, isa_proxy, 4,
                  sizeof(::GenDressedLepton) );
      instance.SetNew(&new_GenDressedLepton);
      instance.SetNewArray(&newArray_GenDressedLepton);
      instance.SetDelete(&delete_GenDressedLepton);
      instance.SetDeleteArray(&deleteArray_GenDressedLepton);
      instance.SetDestructor(&destruct_GenDressedLepton);
      return &instance;
   }
   TGenericClassInfo *GenerateInitInstance(const ::GenDressedLepton*)
   {
      return GenerateInitInstanceLocal(static_cast<::GenDressedLepton*>(nullptr));
   }
   // Static variable to force the class initialization
   static ::ROOT::TGenericClassInfo *_R__UNIQUE_DICT_(Init) = GenerateInitInstanceLocal(static_cast<const ::GenDressedLepton*>(nullptr)); R__UseDummy(_R__UNIQUE_DICT_(Init));
} // end of namespace ROOT

namespace ROOT {
   static void *new_GenIsolatedPhoton(void *p = nullptr);
   static void *newArray_GenIsolatedPhoton(Long_t size, void *p);
   static void delete_GenIsolatedPhoton(void *p);
   static void deleteArray_GenIsolatedPhoton(void *p);
   static void destruct_GenIsolatedPhoton(void *p);

   // Function generating the singleton type initializer
   static TGenericClassInfo *GenerateInitInstanceLocal(const ::GenIsolatedPhoton*)
   {
      ::GenIsolatedPhoton *ptr = nullptr;
      static ::TVirtualIsAProxy* isa_proxy = new ::TInstrumentedIsAProxy< ::GenIsolatedPhoton >(nullptr);
      static ::ROOT::TGenericClassInfo 
         instance("GenIsolatedPhoton", ::GenIsolatedPhoton::Class_Version(), "GenIsolatedPhoton.h", 6,
                  typeid(::GenIsolatedPhoton), ::ROOT::Internal::DefineBehavior(ptr, ptr),
                  &::GenIsolatedPhoton::Dictionary, isa_proxy, 4,
                  sizeof(::GenIsolatedPhoton) );
      instance.SetNew(&new_GenIsolatedPhoton);
      instance.SetNewArray(&newArray_GenIsolatedPhoton);
      instance.SetDelete(&delete_GenIsolatedPhoton);
      instance.SetDeleteArray(&deleteArray_GenIsolatedPhoton);
      instance.SetDestructor(&destruct_GenIsolatedPhoton);
      return &instance;
   }
   TGenericClassInfo *GenerateInitInstance(const ::GenIsolatedPhoton*)
   {
      return GenerateInitInstanceLocal(static_cast<::GenIsolatedPhoton*>(nullptr));
   }
   // Static variable to force the class initialization
   static ::ROOT::TGenericClassInfo *_R__UNIQUE_DICT_(Init) = GenerateInitInstanceLocal(static_cast<const ::GenIsolatedPhoton*>(nullptr)); R__UseDummy(_R__UNIQUE_DICT_(Init));
} // end of namespace ROOT

namespace ROOT {
   static void *new_GenJet(void *p = nullptr);
   static void *newArray_GenJet(Long_t size, void *p);
   static void delete_GenJet(void *p);
   static void deleteArray_GenJet(void *p);
   static void destruct_GenJet(void *p);

   // Function generating the singleton type initializer
   static TGenericClassInfo *GenerateInitInstanceLocal(const ::GenJet*)
   {
      ::GenJet *ptr = nullptr;
      static ::TVirtualIsAProxy* isa_proxy = new ::TInstrumentedIsAProxy< ::GenJet >(nullptr);
      static ::ROOT::TGenericClassInfo 
         instance("GenJet", ::GenJet::Class_Version(), "GenJet.h", 6,
                  typeid(::GenJet), ::ROOT::Internal::DefineBehavior(ptr, ptr),
                  &::GenJet::Dictionary, isa_proxy, 4,
                  sizeof(::GenJet) );
      instance.SetNew(&new_GenJet);
      instance.SetNewArray(&newArray_GenJet);
      instance.SetDelete(&delete_GenJet);
      instance.SetDeleteArray(&deleteArray_GenJet);
      instance.SetDestructor(&destruct_GenJet);
      return &instance;
   }
   TGenericClassInfo *GenerateInitInstance(const ::GenJet*)
   {
      return GenerateInitInstanceLocal(static_cast<::GenJet*>(nullptr));
   }
   // Static variable to force the class initialization
   static ::ROOT::TGenericClassInfo *_R__UNIQUE_DICT_(Init) = GenerateInitInstanceLocal(static_cast<const ::GenJet*>(nullptr)); R__UseDummy(_R__UNIQUE_DICT_(Init));
} // end of namespace ROOT

namespace ROOT {
   static void *new_GenVisTau(void *p = nullptr);
   static void *newArray_GenVisTau(Long_t size, void *p);
   static void delete_GenVisTau(void *p);
   static void deleteArray_GenVisTau(void *p);
   static void destruct_GenVisTau(void *p);

   // Function generating the singleton type initializer
   static TGenericClassInfo *GenerateInitInstanceLocal(const ::GenVisTau*)
   {
      ::GenVisTau *ptr = nullptr;
      static ::TVirtualIsAProxy* isa_proxy = new ::TInstrumentedIsAProxy< ::GenVisTau >(nullptr);
      static ::ROOT::TGenericClassInfo 
         instance("GenVisTau", ::GenVisTau::Class_Version(), "GenVisTau.h", 6,
                  typeid(::GenVisTau), ::ROOT::Internal::DefineBehavior(ptr, ptr),
                  &::GenVisTau::Dictionary, isa_proxy, 4,
                  sizeof(::GenVisTau) );
      instance.SetNew(&new_GenVisTau);
      instance.SetNewArray(&newArray_GenVisTau);
      instance.SetDelete(&delete_GenVisTau);
      instance.SetDeleteArray(&deleteArray_GenVisTau);
      instance.SetDestructor(&destruct_GenVisTau);
      return &instance;
   }
   TGenericClassInfo *GenerateInitInstance(const ::GenVisTau*)
   {
      return GenerateInitInstanceLocal(static_cast<::GenVisTau*>(nullptr));
   }
   // Static variable to force the class initialization
   static ::ROOT::TGenericClassInfo *_R__UNIQUE_DICT_(Init) = GenerateInitInstanceLocal(static_cast<const ::GenVisTau*>(nullptr)); R__UseDummy(_R__UNIQUE_DICT_(Init));
} // end of namespace ROOT

namespace ROOT {
   static void *new_LHE(void *p = nullptr);
   static void *newArray_LHE(Long_t size, void *p);
   static void delete_LHE(void *p);
   static void deleteArray_LHE(void *p);
   static void destruct_LHE(void *p);

   // Function generating the singleton type initializer
   static TGenericClassInfo *GenerateInitInstanceLocal(const ::LHE*)
   {
      ::LHE *ptr = nullptr;
      static ::TVirtualIsAProxy* isa_proxy = new ::TInstrumentedIsAProxy< ::LHE >(nullptr);
      static ::ROOT::TGenericClassInfo 
         instance("LHE", ::LHE::Class_Version(), "LHE.h", 6,
                  typeid(::LHE), ::ROOT::Internal::DefineBehavior(ptr, ptr),
                  &::LHE::Dictionary, isa_proxy, 4,
                  sizeof(::LHE) );
      instance.SetNew(&new_LHE);
      instance.SetNewArray(&newArray_LHE);
      instance.SetDelete(&delete_LHE);
      instance.SetDeleteArray(&deleteArray_LHE);
      instance.SetDestructor(&destruct_LHE);
      return &instance;
   }
   TGenericClassInfo *GenerateInitInstance(const ::LHE*)
   {
      return GenerateInitInstanceLocal(static_cast<::LHE*>(nullptr));
   }
   // Static variable to force the class initialization
   static ::ROOT::TGenericClassInfo *_R__UNIQUE_DICT_(Init) = GenerateInitInstanceLocal(static_cast<const ::LHE*>(nullptr)); R__UseDummy(_R__UNIQUE_DICT_(Init));
} // end of namespace ROOT

namespace ROOT {
   static void *new_Photon(void *p = nullptr);
   static void *newArray_Photon(Long_t size, void *p);
   static void delete_Photon(void *p);
   static void deleteArray_Photon(void *p);
   static void destruct_Photon(void *p);

   // Function generating the singleton type initializer
   static TGenericClassInfo *GenerateInitInstanceLocal(const ::Photon*)
   {
      ::Photon *ptr = nullptr;
      static ::TVirtualIsAProxy* isa_proxy = new ::TInstrumentedIsAProxy< ::Photon >(nullptr);
      static ::ROOT::TGenericClassInfo 
         instance("Photon", ::Photon::Class_Version(), "Photon.h", 6,
                  typeid(::Photon), ::ROOT::Internal::DefineBehavior(ptr, ptr),
                  &::Photon::Dictionary, isa_proxy, 4,
                  sizeof(::Photon) );
      instance.SetNew(&new_Photon);
      instance.SetNewArray(&newArray_Photon);
      instance.SetDelete(&delete_Photon);
      instance.SetDeleteArray(&deleteArray_Photon);
      instance.SetDestructor(&destruct_Photon);
      return &instance;
   }
   TGenericClassInfo *GenerateInitInstance(const ::Photon*)
   {
      return GenerateInitInstanceLocal(static_cast<::Photon*>(nullptr));
   }
   // Static variable to force the class initialization
   static ::ROOT::TGenericClassInfo *_R__UNIQUE_DICT_(Init) = GenerateInitInstanceLocal(static_cast<const ::Photon*>(nullptr)); R__UseDummy(_R__UNIQUE_DICT_(Init));
} // end of namespace ROOT

namespace ROOT {
   static void *new_Tau(void *p = nullptr);
   static void *newArray_Tau(Long_t size, void *p);
   static void delete_Tau(void *p);
   static void deleteArray_Tau(void *p);
   static void destruct_Tau(void *p);

   // Function generating the singleton type initializer
   static TGenericClassInfo *GenerateInitInstanceLocal(const ::Tau*)
   {
      ::Tau *ptr = nullptr;
      static ::TVirtualIsAProxy* isa_proxy = new ::TInstrumentedIsAProxy< ::Tau >(nullptr);
      static ::ROOT::TGenericClassInfo 
         instance("Tau", ::Tau::Class_Version(), "Tau.h", 7,
                  typeid(::Tau), ::ROOT::Internal::DefineBehavior(ptr, ptr),
                  &::Tau::Dictionary, isa_proxy, 4,
                  sizeof(::Tau) );
      instance.SetNew(&new_Tau);
      instance.SetNewArray(&newArray_Tau);
      instance.SetDelete(&delete_Tau);
      instance.SetDeleteArray(&deleteArray_Tau);
      instance.SetDestructor(&destruct_Tau);
      return &instance;
   }
   TGenericClassInfo *GenerateInitInstance(const ::Tau*)
   {
      return GenerateInitInstanceLocal(static_cast<::Tau*>(nullptr));
   }
   // Static variable to force the class initialization
   static ::ROOT::TGenericClassInfo *_R__UNIQUE_DICT_(Init) = GenerateInitInstanceLocal(static_cast<const ::Tau*>(nullptr)); R__UseDummy(_R__UNIQUE_DICT_(Init));
} // end of namespace ROOT

namespace ROOT {
   static void *new_TrigObj(void *p = nullptr);
   static void *newArray_TrigObj(Long_t size, void *p);
   static void delete_TrigObj(void *p);
   static void deleteArray_TrigObj(void *p);
   static void destruct_TrigObj(void *p);

   // Function generating the singleton type initializer
   static TGenericClassInfo *GenerateInitInstanceLocal(const ::TrigObj*)
   {
      ::TrigObj *ptr = nullptr;
      static ::TVirtualIsAProxy* isa_proxy = new ::TInstrumentedIsAProxy< ::TrigObj >(nullptr);
      static ::ROOT::TGenericClassInfo 
         instance("TrigObj", ::TrigObj::Class_Version(), "TrigObj.h", 6,
                  typeid(::TrigObj), ::ROOT::Internal::DefineBehavior(ptr, ptr),
                  &::TrigObj::Dictionary, isa_proxy, 4,
                  sizeof(::TrigObj) );
      instance.SetNew(&new_TrigObj);
      instance.SetNewArray(&newArray_TrigObj);
      instance.SetDelete(&delete_TrigObj);
      instance.SetDeleteArray(&deleteArray_TrigObj);
      instance.SetDestructor(&destruct_TrigObj);
      return &instance;
   }
   TGenericClassInfo *GenerateInitInstance(const ::TrigObj*)
   {
      return GenerateInitInstanceLocal(static_cast<::TrigObj*>(nullptr));
   }
   // Static variable to force the class initialization
   static ::ROOT::TGenericClassInfo *_R__UNIQUE_DICT_(Init) = GenerateInitInstanceLocal(static_cast<const ::TrigObj*>(nullptr)); R__UseDummy(_R__UNIQUE_DICT_(Init));
} // end of namespace ROOT

//______________________________________________________________________________
atomic_TClass_ptr Particle::fgIsA(nullptr);  // static to hold class pointer

//______________________________________________________________________________
const char *Particle::Class_Name()
{
   return "Particle";
}

//______________________________________________________________________________
const char *Particle::ImplFileName()
{
   return ::ROOT::GenerateInitInstanceLocal((const ::Particle*)nullptr)->GetImplFileName();
}

//______________________________________________________________________________
int Particle::ImplFileLine()
{
   return ::ROOT::GenerateInitInstanceLocal((const ::Particle*)nullptr)->GetImplFileLine();
}

//______________________________________________________________________________
TClass *Particle::Dictionary()
{
   fgIsA = ::ROOT::GenerateInitInstanceLocal((const ::Particle*)nullptr)->GetClass();
   return fgIsA;
}

//______________________________________________________________________________
TClass *Particle::Class()
{
   if (!fgIsA.load()) { R__LOCKGUARD(gInterpreterMutex); fgIsA = ::ROOT::GenerateInitInstanceLocal((const ::Particle*)nullptr)->GetClass(); }
   return fgIsA;
}

//______________________________________________________________________________
atomic_TClass_ptr Lepton::fgIsA(nullptr);  // static to hold class pointer

//______________________________________________________________________________
const char *Lepton::Class_Name()
{
   return "Lepton";
}

//______________________________________________________________________________
const char *Lepton::ImplFileName()
{
   return ::ROOT::GenerateInitInstanceLocal((const ::Lepton*)nullptr)->GetImplFileName();
}

//______________________________________________________________________________
int Lepton::ImplFileLine()
{
   return ::ROOT::GenerateInitInstanceLocal((const ::Lepton*)nullptr)->GetImplFileLine();
}

//______________________________________________________________________________
TClass *Lepton::Dictionary()
{
   fgIsA = ::ROOT::GenerateInitInstanceLocal((const ::Lepton*)nullptr)->GetClass();
   return fgIsA;
}

//______________________________________________________________________________
TClass *Lepton::Class()
{
   if (!fgIsA.load()) { R__LOCKGUARD(gInterpreterMutex); fgIsA = ::ROOT::GenerateInitInstanceLocal((const ::Lepton*)nullptr)->GetClass(); }
   return fgIsA;
}

//______________________________________________________________________________
atomic_TClass_ptr Electron::fgIsA(nullptr);  // static to hold class pointer

//______________________________________________________________________________
const char *Electron::Class_Name()
{
   return "Electron";
}

//______________________________________________________________________________
const char *Electron::ImplFileName()
{
   return ::ROOT::GenerateInitInstanceLocal((const ::Electron*)nullptr)->GetImplFileName();
}

//______________________________________________________________________________
int Electron::ImplFileLine()
{
   return ::ROOT::GenerateInitInstanceLocal((const ::Electron*)nullptr)->GetImplFileLine();
}

//______________________________________________________________________________
TClass *Electron::Dictionary()
{
   fgIsA = ::ROOT::GenerateInitInstanceLocal((const ::Electron*)nullptr)->GetClass();
   return fgIsA;
}

//______________________________________________________________________________
TClass *Electron::Class()
{
   if (!fgIsA.load()) { R__LOCKGUARD(gInterpreterMutex); fgIsA = ::ROOT::GenerateInitInstanceLocal((const ::Electron*)nullptr)->GetClass(); }
   return fgIsA;
}

//______________________________________________________________________________
atomic_TClass_ptr Jet::fgIsA(nullptr);  // static to hold class pointer

//______________________________________________________________________________
const char *Jet::Class_Name()
{
   return "Jet";
}

//______________________________________________________________________________
const char *Jet::ImplFileName()
{
   return ::ROOT::GenerateInitInstanceLocal((const ::Jet*)nullptr)->GetImplFileName();
}

//______________________________________________________________________________
int Jet::ImplFileLine()
{
   return ::ROOT::GenerateInitInstanceLocal((const ::Jet*)nullptr)->GetImplFileLine();
}

//______________________________________________________________________________
TClass *Jet::Dictionary()
{
   fgIsA = ::ROOT::GenerateInitInstanceLocal((const ::Jet*)nullptr)->GetClass();
   return fgIsA;
}

//______________________________________________________________________________
TClass *Jet::Class()
{
   if (!fgIsA.load()) { R__LOCKGUARD(gInterpreterMutex); fgIsA = ::ROOT::GenerateInitInstanceLocal((const ::Jet*)nullptr)->GetClass(); }
   return fgIsA;
}

//______________________________________________________________________________
atomic_TClass_ptr Gen::fgIsA(nullptr);  // static to hold class pointer

//______________________________________________________________________________
const char *Gen::Class_Name()
{
   return "Gen";
}

//______________________________________________________________________________
const char *Gen::ImplFileName()
{
   return ::ROOT::GenerateInitInstanceLocal((const ::Gen*)nullptr)->GetImplFileName();
}

//______________________________________________________________________________
int Gen::ImplFileLine()
{
   return ::ROOT::GenerateInitInstanceLocal((const ::Gen*)nullptr)->GetImplFileLine();
}

//______________________________________________________________________________
TClass *Gen::Dictionary()
{
   fgIsA = ::ROOT::GenerateInitInstanceLocal((const ::Gen*)nullptr)->GetClass();
   return fgIsA;
}

//______________________________________________________________________________
TClass *Gen::Class()
{
   if (!fgIsA.load()) { R__LOCKGUARD(gInterpreterMutex); fgIsA = ::ROOT::GenerateInitInstanceLocal((const ::Gen*)nullptr)->GetClass(); }
   return fgIsA;
}

//______________________________________________________________________________
atomic_TClass_ptr Muon::fgIsA(nullptr);  // static to hold class pointer

//______________________________________________________________________________
const char *Muon::Class_Name()
{
   return "Muon";
}

//______________________________________________________________________________
const char *Muon::ImplFileName()
{
   return ::ROOT::GenerateInitInstanceLocal((const ::Muon*)nullptr)->GetImplFileName();
}

//______________________________________________________________________________
int Muon::ImplFileLine()
{
   return ::ROOT::GenerateInitInstanceLocal((const ::Muon*)nullptr)->GetImplFileLine();
}

//______________________________________________________________________________
TClass *Muon::Dictionary()
{
   fgIsA = ::ROOT::GenerateInitInstanceLocal((const ::Muon*)nullptr)->GetClass();
   return fgIsA;
}

//______________________________________________________________________________
TClass *Muon::Class()
{
   if (!fgIsA.load()) { R__LOCKGUARD(gInterpreterMutex); fgIsA = ::ROOT::GenerateInitInstanceLocal((const ::Muon*)nullptr)->GetClass(); }
   return fgIsA;
}

//______________________________________________________________________________
atomic_TClass_ptr Event::fgIsA(nullptr);  // static to hold class pointer

//______________________________________________________________________________
const char *Event::Class_Name()
{
   return "Event";
}

//______________________________________________________________________________
const char *Event::ImplFileName()
{
   return ::ROOT::GenerateInitInstanceLocal((const ::Event*)nullptr)->GetImplFileName();
}

//______________________________________________________________________________
int Event::ImplFileLine()
{
   return ::ROOT::GenerateInitInstanceLocal((const ::Event*)nullptr)->GetImplFileLine();
}

//______________________________________________________________________________
TClass *Event::Dictionary()
{
   fgIsA = ::ROOT::GenerateInitInstanceLocal((const ::Event*)nullptr)->GetClass();
   return fgIsA;
}

//______________________________________________________________________________
TClass *Event::Class()
{
   if (!fgIsA.load()) { R__LOCKGUARD(gInterpreterMutex); fgIsA = ::ROOT::GenerateInitInstanceLocal((const ::Event*)nullptr)->GetClass(); }
   return fgIsA;
}

//______________________________________________________________________________
atomic_TClass_ptr FatJet::fgIsA(nullptr);  // static to hold class pointer

//______________________________________________________________________________
const char *FatJet::Class_Name()
{
   return "FatJet";
}

//______________________________________________________________________________
const char *FatJet::ImplFileName()
{
   return ::ROOT::GenerateInitInstanceLocal((const ::FatJet*)nullptr)->GetImplFileName();
}

//______________________________________________________________________________
int FatJet::ImplFileLine()
{
   return ::ROOT::GenerateInitInstanceLocal((const ::FatJet*)nullptr)->GetImplFileLine();
}

//______________________________________________________________________________
TClass *FatJet::Dictionary()
{
   fgIsA = ::ROOT::GenerateInitInstanceLocal((const ::FatJet*)nullptr)->GetClass();
   return fgIsA;
}

//______________________________________________________________________________
TClass *FatJet::Class()
{
   if (!fgIsA.load()) { R__LOCKGUARD(gInterpreterMutex); fgIsA = ::ROOT::GenerateInitInstanceLocal((const ::FatJet*)nullptr)->GetClass(); }
   return fgIsA;
}

//______________________________________________________________________________
atomic_TClass_ptr GenDressedLepton::fgIsA(nullptr);  // static to hold class pointer

//______________________________________________________________________________
const char *GenDressedLepton::Class_Name()
{
   return "GenDressedLepton";
}

//______________________________________________________________________________
const char *GenDressedLepton::ImplFileName()
{
   return ::ROOT::GenerateInitInstanceLocal((const ::GenDressedLepton*)nullptr)->GetImplFileName();
}

//______________________________________________________________________________
int GenDressedLepton::ImplFileLine()
{
   return ::ROOT::GenerateInitInstanceLocal((const ::GenDressedLepton*)nullptr)->GetImplFileLine();
}

//______________________________________________________________________________
TClass *GenDressedLepton::Dictionary()
{
   fgIsA = ::ROOT::GenerateInitInstanceLocal((const ::GenDressedLepton*)nullptr)->GetClass();
   return fgIsA;
}

//______________________________________________________________________________
TClass *GenDressedLepton::Class()
{
   if (!fgIsA.load()) { R__LOCKGUARD(gInterpreterMutex); fgIsA = ::ROOT::GenerateInitInstanceLocal((const ::GenDressedLepton*)nullptr)->GetClass(); }
   return fgIsA;
}

//______________________________________________________________________________
atomic_TClass_ptr GenIsolatedPhoton::fgIsA(nullptr);  // static to hold class pointer

//______________________________________________________________________________
const char *GenIsolatedPhoton::Class_Name()
{
   return "GenIsolatedPhoton";
}

//______________________________________________________________________________
const char *GenIsolatedPhoton::ImplFileName()
{
   return ::ROOT::GenerateInitInstanceLocal((const ::GenIsolatedPhoton*)nullptr)->GetImplFileName();
}

//______________________________________________________________________________
int GenIsolatedPhoton::ImplFileLine()
{
   return ::ROOT::GenerateInitInstanceLocal((const ::GenIsolatedPhoton*)nullptr)->GetImplFileLine();
}

//______________________________________________________________________________
TClass *GenIsolatedPhoton::Dictionary()
{
   fgIsA = ::ROOT::GenerateInitInstanceLocal((const ::GenIsolatedPhoton*)nullptr)->GetClass();
   return fgIsA;
}

//______________________________________________________________________________
TClass *GenIsolatedPhoton::Class()
{
   if (!fgIsA.load()) { R__LOCKGUARD(gInterpreterMutex); fgIsA = ::ROOT::GenerateInitInstanceLocal((const ::GenIsolatedPhoton*)nullptr)->GetClass(); }
   return fgIsA;
}

//______________________________________________________________________________
atomic_TClass_ptr GenJet::fgIsA(nullptr);  // static to hold class pointer

//______________________________________________________________________________
const char *GenJet::Class_Name()
{
   return "GenJet";
}

//______________________________________________________________________________
const char *GenJet::ImplFileName()
{
   return ::ROOT::GenerateInitInstanceLocal((const ::GenJet*)nullptr)->GetImplFileName();
}

//______________________________________________________________________________
int GenJet::ImplFileLine()
{
   return ::ROOT::GenerateInitInstanceLocal((const ::GenJet*)nullptr)->GetImplFileLine();
}

//______________________________________________________________________________
TClass *GenJet::Dictionary()
{
   fgIsA = ::ROOT::GenerateInitInstanceLocal((const ::GenJet*)nullptr)->GetClass();
   return fgIsA;
}

//______________________________________________________________________________
TClass *GenJet::Class()
{
   if (!fgIsA.load()) { R__LOCKGUARD(gInterpreterMutex); fgIsA = ::ROOT::GenerateInitInstanceLocal((const ::GenJet*)nullptr)->GetClass(); }
   return fgIsA;
}

//______________________________________________________________________________
atomic_TClass_ptr GenVisTau::fgIsA(nullptr);  // static to hold class pointer

//______________________________________________________________________________
const char *GenVisTau::Class_Name()
{
   return "GenVisTau";
}

//______________________________________________________________________________
const char *GenVisTau::ImplFileName()
{
   return ::ROOT::GenerateInitInstanceLocal((const ::GenVisTau*)nullptr)->GetImplFileName();
}

//______________________________________________________________________________
int GenVisTau::ImplFileLine()
{
   return ::ROOT::GenerateInitInstanceLocal((const ::GenVisTau*)nullptr)->GetImplFileLine();
}

//______________________________________________________________________________
TClass *GenVisTau::Dictionary()
{
   fgIsA = ::ROOT::GenerateInitInstanceLocal((const ::GenVisTau*)nullptr)->GetClass();
   return fgIsA;
}

//______________________________________________________________________________
TClass *GenVisTau::Class()
{
   if (!fgIsA.load()) { R__LOCKGUARD(gInterpreterMutex); fgIsA = ::ROOT::GenerateInitInstanceLocal((const ::GenVisTau*)nullptr)->GetClass(); }
   return fgIsA;
}

//______________________________________________________________________________
atomic_TClass_ptr LHE::fgIsA(nullptr);  // static to hold class pointer

//______________________________________________________________________________
const char *LHE::Class_Name()
{
   return "LHE";
}

//______________________________________________________________________________
const char *LHE::ImplFileName()
{
   return ::ROOT::GenerateInitInstanceLocal((const ::LHE*)nullptr)->GetImplFileName();
}

//______________________________________________________________________________
int LHE::ImplFileLine()
{
   return ::ROOT::GenerateInitInstanceLocal((const ::LHE*)nullptr)->GetImplFileLine();
}

//______________________________________________________________________________
TClass *LHE::Dictionary()
{
   fgIsA = ::ROOT::GenerateInitInstanceLocal((const ::LHE*)nullptr)->GetClass();
   return fgIsA;
}

//______________________________________________________________________________
TClass *LHE::Class()
{
   if (!fgIsA.load()) { R__LOCKGUARD(gInterpreterMutex); fgIsA = ::ROOT::GenerateInitInstanceLocal((const ::LHE*)nullptr)->GetClass(); }
   return fgIsA;
}

//______________________________________________________________________________
atomic_TClass_ptr Photon::fgIsA(nullptr);  // static to hold class pointer

//______________________________________________________________________________
const char *Photon::Class_Name()
{
   return "Photon";
}

//______________________________________________________________________________
const char *Photon::ImplFileName()
{
   return ::ROOT::GenerateInitInstanceLocal((const ::Photon*)nullptr)->GetImplFileName();
}

//______________________________________________________________________________
int Photon::ImplFileLine()
{
   return ::ROOT::GenerateInitInstanceLocal((const ::Photon*)nullptr)->GetImplFileLine();
}

//______________________________________________________________________________
TClass *Photon::Dictionary()
{
   fgIsA = ::ROOT::GenerateInitInstanceLocal((const ::Photon*)nullptr)->GetClass();
   return fgIsA;
}

//______________________________________________________________________________
TClass *Photon::Class()
{
   if (!fgIsA.load()) { R__LOCKGUARD(gInterpreterMutex); fgIsA = ::ROOT::GenerateInitInstanceLocal((const ::Photon*)nullptr)->GetClass(); }
   return fgIsA;
}

//______________________________________________________________________________
atomic_TClass_ptr Tau::fgIsA(nullptr);  // static to hold class pointer

//______________________________________________________________________________
const char *Tau::Class_Name()
{
   return "Tau";
}

//______________________________________________________________________________
const char *Tau::ImplFileName()
{
   return ::ROOT::GenerateInitInstanceLocal((const ::Tau*)nullptr)->GetImplFileName();
}

//______________________________________________________________________________
int Tau::ImplFileLine()
{
   return ::ROOT::GenerateInitInstanceLocal((const ::Tau*)nullptr)->GetImplFileLine();
}

//______________________________________________________________________________
TClass *Tau::Dictionary()
{
   fgIsA = ::ROOT::GenerateInitInstanceLocal((const ::Tau*)nullptr)->GetClass();
   return fgIsA;
}

//______________________________________________________________________________
TClass *Tau::Class()
{
   if (!fgIsA.load()) { R__LOCKGUARD(gInterpreterMutex); fgIsA = ::ROOT::GenerateInitInstanceLocal((const ::Tau*)nullptr)->GetClass(); }
   return fgIsA;
}

//______________________________________________________________________________
atomic_TClass_ptr TrigObj::fgIsA(nullptr);  // static to hold class pointer

//______________________________________________________________________________
const char *TrigObj::Class_Name()
{
   return "TrigObj";
}

//______________________________________________________________________________
const char *TrigObj::ImplFileName()
{
   return ::ROOT::GenerateInitInstanceLocal((const ::TrigObj*)nullptr)->GetImplFileName();
}

//______________________________________________________________________________
int TrigObj::ImplFileLine()
{
   return ::ROOT::GenerateInitInstanceLocal((const ::TrigObj*)nullptr)->GetImplFileLine();
}

//______________________________________________________________________________
TClass *TrigObj::Dictionary()
{
   fgIsA = ::ROOT::GenerateInitInstanceLocal((const ::TrigObj*)nullptr)->GetClass();
   return fgIsA;
}

//______________________________________________________________________________
TClass *TrigObj::Class()
{
   if (!fgIsA.load()) { R__LOCKGUARD(gInterpreterMutex); fgIsA = ::ROOT::GenerateInitInstanceLocal((const ::TrigObj*)nullptr)->GetClass(); }
   return fgIsA;
}

//______________________________________________________________________________
void Particle::Streamer(TBuffer &R__b)
{
   // Stream an object of class Particle.

   if (R__b.IsReading()) {
      R__b.ReadClassBuffer(Particle::Class(),this);
   } else {
      R__b.WriteClassBuffer(Particle::Class(),this);
   }
}

namespace ROOT {
   // Wrappers around operator new
   static void *new_Particle(void *p) {
      return  p ? new(p) ::Particle : new ::Particle;
   }
   static void *newArray_Particle(Long_t nElements, void *p) {
      return p ? new(p) ::Particle[nElements] : new ::Particle[nElements];
   }
   // Wrapper around operator delete
   static void delete_Particle(void *p) {
      delete (static_cast<::Particle*>(p));
   }
   static void deleteArray_Particle(void *p) {
      delete [] (static_cast<::Particle*>(p));
   }
   static void destruct_Particle(void *p) {
      typedef ::Particle current_t;
      (static_cast<current_t*>(p))->~current_t();
   }
} // end of namespace ROOT for class ::Particle

//______________________________________________________________________________
void Lepton::Streamer(TBuffer &R__b)
{
   // Stream an object of class Lepton.

   if (R__b.IsReading()) {
      R__b.ReadClassBuffer(Lepton::Class(),this);
   } else {
      R__b.WriteClassBuffer(Lepton::Class(),this);
   }
}

namespace ROOT {
   // Wrappers around operator new
   static void *new_Lepton(void *p) {
      return  p ? new(p) ::Lepton : new ::Lepton;
   }
   static void *newArray_Lepton(Long_t nElements, void *p) {
      return p ? new(p) ::Lepton[nElements] : new ::Lepton[nElements];
   }
   // Wrapper around operator delete
   static void delete_Lepton(void *p) {
      delete (static_cast<::Lepton*>(p));
   }
   static void deleteArray_Lepton(void *p) {
      delete [] (static_cast<::Lepton*>(p));
   }
   static void destruct_Lepton(void *p) {
      typedef ::Lepton current_t;
      (static_cast<current_t*>(p))->~current_t();
   }
} // end of namespace ROOT for class ::Lepton

//______________________________________________________________________________
void Electron::Streamer(TBuffer &R__b)
{
   // Stream an object of class Electron.

   if (R__b.IsReading()) {
      R__b.ReadClassBuffer(Electron::Class(),this);
   } else {
      R__b.WriteClassBuffer(Electron::Class(),this);
   }
}

namespace ROOT {
   // Wrappers around operator new
   static void *new_Electron(void *p) {
      return  p ? new(p) ::Electron : new ::Electron;
   }
   static void *newArray_Electron(Long_t nElements, void *p) {
      return p ? new(p) ::Electron[nElements] : new ::Electron[nElements];
   }
   // Wrapper around operator delete
   static void delete_Electron(void *p) {
      delete (static_cast<::Electron*>(p));
   }
   static void deleteArray_Electron(void *p) {
      delete [] (static_cast<::Electron*>(p));
   }
   static void destruct_Electron(void *p) {
      typedef ::Electron current_t;
      (static_cast<current_t*>(p))->~current_t();
   }
} // end of namespace ROOT for class ::Electron

//______________________________________________________________________________
void Jet::Streamer(TBuffer &R__b)
{
   // Stream an object of class Jet.

   if (R__b.IsReading()) {
      R__b.ReadClassBuffer(Jet::Class(),this);
   } else {
      R__b.WriteClassBuffer(Jet::Class(),this);
   }
}

namespace ROOT {
   // Wrappers around operator new
   static void *new_Jet(void *p) {
      return  p ? new(p) ::Jet : new ::Jet;
   }
   static void *newArray_Jet(Long_t nElements, void *p) {
      return p ? new(p) ::Jet[nElements] : new ::Jet[nElements];
   }
   // Wrapper around operator delete
   static void delete_Jet(void *p) {
      delete (static_cast<::Jet*>(p));
   }
   static void deleteArray_Jet(void *p) {
      delete [] (static_cast<::Jet*>(p));
   }
   static void destruct_Jet(void *p) {
      typedef ::Jet current_t;
      (static_cast<current_t*>(p))->~current_t();
   }
} // end of namespace ROOT for class ::Jet

//______________________________________________________________________________
void Gen::Streamer(TBuffer &R__b)
{
   // Stream an object of class Gen.

   if (R__b.IsReading()) {
      R__b.ReadClassBuffer(Gen::Class(),this);
   } else {
      R__b.WriteClassBuffer(Gen::Class(),this);
   }
}

namespace ROOT {
   // Wrappers around operator new
   static void *new_Gen(void *p) {
      return  p ? new(p) ::Gen : new ::Gen;
   }
   static void *newArray_Gen(Long_t nElements, void *p) {
      return p ? new(p) ::Gen[nElements] : new ::Gen[nElements];
   }
   // Wrapper around operator delete
   static void delete_Gen(void *p) {
      delete (static_cast<::Gen*>(p));
   }
   static void deleteArray_Gen(void *p) {
      delete [] (static_cast<::Gen*>(p));
   }
   static void destruct_Gen(void *p) {
      typedef ::Gen current_t;
      (static_cast<current_t*>(p))->~current_t();
   }
} // end of namespace ROOT for class ::Gen

//______________________________________________________________________________
void Muon::Streamer(TBuffer &R__b)
{
   // Stream an object of class Muon.

   if (R__b.IsReading()) {
      R__b.ReadClassBuffer(Muon::Class(),this);
   } else {
      R__b.WriteClassBuffer(Muon::Class(),this);
   }
}

namespace ROOT {
   // Wrappers around operator new
   static void *new_Muon(void *p) {
      return  p ? new(p) ::Muon : new ::Muon;
   }
   static void *newArray_Muon(Long_t nElements, void *p) {
      return p ? new(p) ::Muon[nElements] : new ::Muon[nElements];
   }
   // Wrapper around operator delete
   static void delete_Muon(void *p) {
      delete (static_cast<::Muon*>(p));
   }
   static void deleteArray_Muon(void *p) {
      delete [] (static_cast<::Muon*>(p));
   }
   static void destruct_Muon(void *p) {
      typedef ::Muon current_t;
      (static_cast<current_t*>(p))->~current_t();
   }
} // end of namespace ROOT for class ::Muon

//______________________________________________________________________________
void Event::Streamer(TBuffer &R__b)
{
   // Stream an object of class Event.

   if (R__b.IsReading()) {
      R__b.ReadClassBuffer(Event::Class(),this);
   } else {
      R__b.WriteClassBuffer(Event::Class(),this);
   }
}

namespace ROOT {
   // Wrappers around operator new
   static void *new_Event(void *p) {
      return  p ? new(p) ::Event : new ::Event;
   }
   static void *newArray_Event(Long_t nElements, void *p) {
      return p ? new(p) ::Event[nElements] : new ::Event[nElements];
   }
   // Wrapper around operator delete
   static void delete_Event(void *p) {
      delete (static_cast<::Event*>(p));
   }
   static void deleteArray_Event(void *p) {
      delete [] (static_cast<::Event*>(p));
   }
   static void destruct_Event(void *p) {
      typedef ::Event current_t;
      (static_cast<current_t*>(p))->~current_t();
   }
} // end of namespace ROOT for class ::Event

//______________________________________________________________________________
void FatJet::Streamer(TBuffer &R__b)
{
   // Stream an object of class FatJet.

   if (R__b.IsReading()) {
      R__b.ReadClassBuffer(FatJet::Class(),this);
   } else {
      R__b.WriteClassBuffer(FatJet::Class(),this);
   }
}

namespace ROOT {
   // Wrappers around operator new
   static void *new_FatJet(void *p) {
      return  p ? new(p) ::FatJet : new ::FatJet;
   }
   static void *newArray_FatJet(Long_t nElements, void *p) {
      return p ? new(p) ::FatJet[nElements] : new ::FatJet[nElements];
   }
   // Wrapper around operator delete
   static void delete_FatJet(void *p) {
      delete (static_cast<::FatJet*>(p));
   }
   static void deleteArray_FatJet(void *p) {
      delete [] (static_cast<::FatJet*>(p));
   }
   static void destruct_FatJet(void *p) {
      typedef ::FatJet current_t;
      (static_cast<current_t*>(p))->~current_t();
   }
} // end of namespace ROOT for class ::FatJet

//______________________________________________________________________________
void GenDressedLepton::Streamer(TBuffer &R__b)
{
   // Stream an object of class GenDressedLepton.

   if (R__b.IsReading()) {
      R__b.ReadClassBuffer(GenDressedLepton::Class(),this);
   } else {
      R__b.WriteClassBuffer(GenDressedLepton::Class(),this);
   }
}

namespace ROOT {
   // Wrappers around operator new
   static void *new_GenDressedLepton(void *p) {
      return  p ? new(p) ::GenDressedLepton : new ::GenDressedLepton;
   }
   static void *newArray_GenDressedLepton(Long_t nElements, void *p) {
      return p ? new(p) ::GenDressedLepton[nElements] : new ::GenDressedLepton[nElements];
   }
   // Wrapper around operator delete
   static void delete_GenDressedLepton(void *p) {
      delete (static_cast<::GenDressedLepton*>(p));
   }
   static void deleteArray_GenDressedLepton(void *p) {
      delete [] (static_cast<::GenDressedLepton*>(p));
   }
   static void destruct_GenDressedLepton(void *p) {
      typedef ::GenDressedLepton current_t;
      (static_cast<current_t*>(p))->~current_t();
   }
} // end of namespace ROOT for class ::GenDressedLepton

//______________________________________________________________________________
void GenIsolatedPhoton::Streamer(TBuffer &R__b)
{
   // Stream an object of class GenIsolatedPhoton.

   if (R__b.IsReading()) {
      R__b.ReadClassBuffer(GenIsolatedPhoton::Class(),this);
   } else {
      R__b.WriteClassBuffer(GenIsolatedPhoton::Class(),this);
   }
}

namespace ROOT {
   // Wrappers around operator new
   static void *new_GenIsolatedPhoton(void *p) {
      return  p ? new(p) ::GenIsolatedPhoton : new ::GenIsolatedPhoton;
   }
   static void *newArray_GenIsolatedPhoton(Long_t nElements, void *p) {
      return p ? new(p) ::GenIsolatedPhoton[nElements] : new ::GenIsolatedPhoton[nElements];
   }
   // Wrapper around operator delete
   static void delete_GenIsolatedPhoton(void *p) {
      delete (static_cast<::GenIsolatedPhoton*>(p));
   }
   static void deleteArray_GenIsolatedPhoton(void *p) {
      delete [] (static_cast<::GenIsolatedPhoton*>(p));
   }
   static void destruct_GenIsolatedPhoton(void *p) {
      typedef ::GenIsolatedPhoton current_t;
      (static_cast<current_t*>(p))->~current_t();
   }
} // end of namespace ROOT for class ::GenIsolatedPhoton

//______________________________________________________________________________
void GenJet::Streamer(TBuffer &R__b)
{
   // Stream an object of class GenJet.

   if (R__b.IsReading()) {
      R__b.ReadClassBuffer(GenJet::Class(),this);
   } else {
      R__b.WriteClassBuffer(GenJet::Class(),this);
   }
}

namespace ROOT {
   // Wrappers around operator new
   static void *new_GenJet(void *p) {
      return  p ? new(p) ::GenJet : new ::GenJet;
   }
   static void *newArray_GenJet(Long_t nElements, void *p) {
      return p ? new(p) ::GenJet[nElements] : new ::GenJet[nElements];
   }
   // Wrapper around operator delete
   static void delete_GenJet(void *p) {
      delete (static_cast<::GenJet*>(p));
   }
   static void deleteArray_GenJet(void *p) {
      delete [] (static_cast<::GenJet*>(p));
   }
   static void destruct_GenJet(void *p) {
      typedef ::GenJet current_t;
      (static_cast<current_t*>(p))->~current_t();
   }
} // end of namespace ROOT for class ::GenJet

//______________________________________________________________________________
void GenVisTau::Streamer(TBuffer &R__b)
{
   // Stream an object of class GenVisTau.

   if (R__b.IsReading()) {
      R__b.ReadClassBuffer(GenVisTau::Class(),this);
   } else {
      R__b.WriteClassBuffer(GenVisTau::Class(),this);
   }
}

namespace ROOT {
   // Wrappers around operator new
   static void *new_GenVisTau(void *p) {
      return  p ? new(p) ::GenVisTau : new ::GenVisTau;
   }
   static void *newArray_GenVisTau(Long_t nElements, void *p) {
      return p ? new(p) ::GenVisTau[nElements] : new ::GenVisTau[nElements];
   }
   // Wrapper around operator delete
   static void delete_GenVisTau(void *p) {
      delete (static_cast<::GenVisTau*>(p));
   }
   static void deleteArray_GenVisTau(void *p) {
      delete [] (static_cast<::GenVisTau*>(p));
   }
   static void destruct_GenVisTau(void *p) {
      typedef ::GenVisTau current_t;
      (static_cast<current_t*>(p))->~current_t();
   }
} // end of namespace ROOT for class ::GenVisTau

//______________________________________________________________________________
void LHE::Streamer(TBuffer &R__b)
{
   // Stream an object of class LHE.

   if (R__b.IsReading()) {
      R__b.ReadClassBuffer(LHE::Class(),this);
   } else {
      R__b.WriteClassBuffer(LHE::Class(),this);
   }
}

namespace ROOT {
   // Wrappers around operator new
   static void *new_LHE(void *p) {
      return  p ? new(p) ::LHE : new ::LHE;
   }
   static void *newArray_LHE(Long_t nElements, void *p) {
      return p ? new(p) ::LHE[nElements] : new ::LHE[nElements];
   }
   // Wrapper around operator delete
   static void delete_LHE(void *p) {
      delete (static_cast<::LHE*>(p));
   }
   static void deleteArray_LHE(void *p) {
      delete [] (static_cast<::LHE*>(p));
   }
   static void destruct_LHE(void *p) {
      typedef ::LHE current_t;
      (static_cast<current_t*>(p))->~current_t();
   }
} // end of namespace ROOT for class ::LHE

//______________________________________________________________________________
void Photon::Streamer(TBuffer &R__b)
{
   // Stream an object of class Photon.

   if (R__b.IsReading()) {
      R__b.ReadClassBuffer(Photon::Class(),this);
   } else {
      R__b.WriteClassBuffer(Photon::Class(),this);
   }
}

namespace ROOT {
   // Wrappers around operator new
   static void *new_Photon(void *p) {
      return  p ? new(p) ::Photon : new ::Photon;
   }
   static void *newArray_Photon(Long_t nElements, void *p) {
      return p ? new(p) ::Photon[nElements] : new ::Photon[nElements];
   }
   // Wrapper around operator delete
   static void delete_Photon(void *p) {
      delete (static_cast<::Photon*>(p));
   }
   static void deleteArray_Photon(void *p) {
      delete [] (static_cast<::Photon*>(p));
   }
   static void destruct_Photon(void *p) {
      typedef ::Photon current_t;
      (static_cast<current_t*>(p))->~current_t();
   }
} // end of namespace ROOT for class ::Photon

//______________________________________________________________________________
void Tau::Streamer(TBuffer &R__b)
{
   // Stream an object of class Tau.

   if (R__b.IsReading()) {
      R__b.ReadClassBuffer(Tau::Class(),this);
   } else {
      R__b.WriteClassBuffer(Tau::Class(),this);
   }
}

namespace ROOT {
   // Wrappers around operator new
   static void *new_Tau(void *p) {
      return  p ? new(p) ::Tau : new ::Tau;
   }
   static void *newArray_Tau(Long_t nElements, void *p) {
      return p ? new(p) ::Tau[nElements] : new ::Tau[nElements];
   }
   // Wrapper around operator delete
   static void delete_Tau(void *p) {
      delete (static_cast<::Tau*>(p));
   }
   static void deleteArray_Tau(void *p) {
      delete [] (static_cast<::Tau*>(p));
   }
   static void destruct_Tau(void *p) {
      typedef ::Tau current_t;
      (static_cast<current_t*>(p))->~current_t();
   }
} // end of namespace ROOT for class ::Tau

//______________________________________________________________________________
void TrigObj::Streamer(TBuffer &R__b)
{
   // Stream an object of class TrigObj.

   if (R__b.IsReading()) {
      R__b.ReadClassBuffer(TrigObj::Class(),this);
   } else {
      R__b.WriteClassBuffer(TrigObj::Class(),this);
   }
}

namespace ROOT {
   // Wrappers around operator new
   static void *new_TrigObj(void *p) {
      return  p ? new(p) ::TrigObj : new ::TrigObj;
   }
   static void *newArray_TrigObj(Long_t nElements, void *p) {
      return p ? new(p) ::TrigObj[nElements] : new ::TrigObj[nElements];
   }
   // Wrapper around operator delete
   static void delete_TrigObj(void *p) {
      delete (static_cast<::TrigObj*>(p));
   }
   static void deleteArray_TrigObj(void *p) {
      delete [] (static_cast<::TrigObj*>(p));
   }
   static void destruct_TrigObj(void *p) {
      typedef ::TrigObj current_t;
      (static_cast<current_t*>(p))->~current_t();
   }
} // end of namespace ROOT for class ::TrigObj

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

namespace {
  void TriggerDictionaryInitialization_libDataFormats_Impl() {
    static const char* headers[] = {
"include/Electron.h",
"include/Event.h",
"include/FatJet.h",
"include/Gen.h",
"include/GenDressedLepton.h",
"include/GenIsolatedPhoton.h",
"include/GenJet.h",
"include/GenVisTau.h",
"include/Jet.h",
"include/JetTaggingParameter.h",
"include/LHE.h",
"include/Lepton.h",
"include/Muon.h",
"include/Particle.h",
"include/Photon.h",
"include/Tau.h",
"include/TrigObj.h",
nullptr
    };
    static const char* includePaths[] = {
"/data6/Users/achihwan/miniforge3/envs/Nano/include",
"/home/achihwan/SKNanoAnalyzer/DataFormats",
"/home/achihwan/SKNanoAnalyzer/DataFormats/include",
"/home/achihwan/SKNanoAnalyzer/DataFormats/../AnalyzerTools/include",
"/data6/Users/achihwan/miniforge3/envs/Nano/lib/python3.12/site-packages/correctionlib/include",
"/home/achihwan/SKNanoAnalyzer/external/RoccoR",
"/data6/Users/achihwan/miniforge3/envs/Nano/include",
"/home/achihwan/SKNanoAnalyzer/DataFormats/include",
"/home/achihwan/SKNanoAnalyzer/DataFormats/../AnalyzerTools/include",
"/data6/Users/achihwan/miniforge3/envs/Nano/lib/python3.12/site-packages/correctionlib/include",
"/home/achihwan/SKNanoAnalyzer/external/RoccoR",
"/data6/Users/achihwan/miniforge3/envs/Nano/include/",
"/data6/Users/achihwan/SKNanoAnalyzer/DataFormats/",
nullptr
    };
    static const char* fwdDeclCode = R"DICTFWDDCLS(
#line 1 "libDataFormats dictionary forward declarations' payload"
#pragma clang diagnostic ignored "-Wkeyword-compat"
#pragma clang diagnostic ignored "-Wignored-attributes"
#pragma clang diagnostic ignored "-Wreturn-type-c-linkage"
extern int __Cling_AutoLoading_Map;
class __attribute__((annotate("$clingAutoload$Particle.h")))  __attribute__((annotate("$clingAutoload$include/Electron.h")))  Particle;
class __attribute__((annotate("$clingAutoload$Lepton.h")))  __attribute__((annotate("$clingAutoload$include/Electron.h")))  Lepton;
class __attribute__((annotate("$clingAutoload$include/Electron.h")))  Electron;
class __attribute__((annotate("$clingAutoload$Jet.h")))  __attribute__((annotate("$clingAutoload$include/Event.h")))  Jet;
class __attribute__((annotate("$clingAutoload$Gen.h")))  __attribute__((annotate("$clingAutoload$include/Event.h")))  Gen;
class __attribute__((annotate("$clingAutoload$Muon.h")))  __attribute__((annotate("$clingAutoload$include/Event.h")))  Muon;
class __attribute__((annotate("$clingAutoload$include/Event.h")))  Event;
class __attribute__((annotate("$clingAutoload$include/FatJet.h")))  FatJet;
class __attribute__((annotate("$clingAutoload$include/GenDressedLepton.h")))  GenDressedLepton;
class __attribute__((annotate("$clingAutoload$include/GenIsolatedPhoton.h")))  GenIsolatedPhoton;
class __attribute__((annotate("$clingAutoload$include/GenJet.h")))  GenJet;
class __attribute__((annotate("$clingAutoload$include/GenVisTau.h")))  GenVisTau;
class __attribute__((annotate("$clingAutoload$include/LHE.h")))  LHE;
class __attribute__((annotate("$clingAutoload$include/Photon.h")))  Photon;
class __attribute__((annotate("$clingAutoload$include/Tau.h")))  Tau;
class __attribute__((annotate("$clingAutoload$include/TrigObj.h")))  TrigObj;
)DICTFWDDCLS";
    static const char* payloadCode = R"DICTPAYLOAD(
#line 1 "libDataFormats dictionary payload"

#ifndef __ROOFIT_NOBANNER
  #define __ROOFIT_NOBANNER 1
#endif
#ifndef __ROOFIT_NOBANNER
  #define __ROOFIT_NOBANNER 1
#endif

#define _BACKWARD_BACKWARD_WARNING_H
// Inline headers
#include "include/Electron.h"
#include "include/Event.h"
#include "include/FatJet.h"
#include "include/Gen.h"
#include "include/GenDressedLepton.h"
#include "include/GenIsolatedPhoton.h"
#include "include/GenJet.h"
#include "include/GenVisTau.h"
#include "include/Jet.h"
#include "include/JetTaggingParameter.h"
#include "include/LHE.h"
#include "include/Lepton.h"
#include "include/Muon.h"
#include "include/Particle.h"
#include "include/Photon.h"
#include "include/Tau.h"
#include "include/TrigObj.h"

#undef  _BACKWARD_BACKWARD_WARNING_H
)DICTPAYLOAD";
    static const char* classesHeaders[] = {
"Electron", payloadCode, "@",
"Event", payloadCode, "@",
"FatJet", payloadCode, "@",
"Gen", payloadCode, "@",
"GenDressedLepton", payloadCode, "@",
"GenIsolatedPhoton", payloadCode, "@",
"GenJet", payloadCode, "@",
"GenVisTau", payloadCode, "@",
"Jet", payloadCode, "@",
"LHE", payloadCode, "@",
"Lepton", payloadCode, "@",
"Muon", payloadCode, "@",
"Particle", payloadCode, "@",
"Photon", payloadCode, "@",
"Tau", payloadCode, "@",
"TrigObj", payloadCode, "@",
nullptr
};
    static bool isInitialized = false;
    if (!isInitialized) {
      TROOT::RegisterModule("libDataFormats",
        headers, includePaths, payloadCode, fwdDeclCode,
        TriggerDictionaryInitialization_libDataFormats_Impl, {}, classesHeaders, /*hasCxxModule*/false);
      isInitialized = true;
    }
  }
  static struct DictInit {
    DictInit() {
      TriggerDictionaryInitialization_libDataFormats_Impl();
    }
  } __TheDictionaryInitializer;
}
void TriggerDictionaryInitialization_libDataFormats() {
  TriggerDictionaryInitialization_libDataFormats_Impl();
}

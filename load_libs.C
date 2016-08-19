#include "TString.h"
#include "TSystem.h"
#include "TInterpreter.h"
#include "Cintex/Cintex.h"

#include <iostream>

void load_libs()
{
#ifdef __CINT__
  cout << "Run load_libs.C+" << endl;
  exit(1);
#else
  gSystem->SetAclicMode(TSystem::kDebug);
  // gSystem->SetFPEMask(kDivByZero);
  // MnvH1D hides approximately everything, so just turn off the pages
  // of compiler warnings. It would have been easier to do this by
  // using SetFlagsDebug(), but those flags get put before the default
  // settings in the compile line, and so the default settings win
  TString makeSharedLib(gSystem->GetMakeSharedLib());
  makeSharedLib.ReplaceAll("-Woverloaded-virtual", "-Wno-overloaded-virtual");
  gSystem->SetMakeSharedLib(makeSharedLib);

  gInterpreter->ExecuteMacro("util/tree/loadTreeWrapper.C");

  const int kNIncDirs=2;
  TString incDirs[kNIncDirs] = {
    TString::Format("%s", gSystem->Getenv("PLOTUTILSROOT")),
    //DC:190816 TString::Format("%s", gSystem->Getenv("UNFOLDUTILSROOT")),
  };

  for(int i=0; i<kNIncDirs; ++i){
    //cout << "Adding " << incDirs[i] << endl;
    gInterpreter->AddIncludePath(incDirs[i]);
  }

  const int kNLibs=3;
  // TODO: Will probably need to add T2KReweight and the neut reweight library to this
  const char* libs[kNLibs] = {
    "libplotutils.so",
    "libCintex.so",
    "libUnfoldUtils.so"
  };

  for(int i=0; i<kNLibs; ++i){
    cout << "Loading " << libs[i] << endl;
    gSystem->Load(libs[i]);
  }
  ROOT::Cintex::Cintex::Enable();

//DC:190816  gSystem->CompileMacro("util/plot/GridCanvas.cxx", "k");
  // Long complicated reason to do this because of using TExec to set colour palettes
//DC:190816  gSystem->CompileMacro("util/plot/myPlotStyle.h", "k");

#endif
}



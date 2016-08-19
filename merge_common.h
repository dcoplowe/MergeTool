#ifndef MERGE_COMMON_H
#define MERGE_COMMON_H

#include "TString.h"
#include "TChain.h"
#include "TChainElement.h"
#include "TFile.h"
#include "TBranch.h"
#include "TLeaf.h"
#include "TStopwatch.h"
#include "TSystem.h"
#include "TList.h"

#include <iostream>
#include <string>
#include <cassert>
#include <stdlib.h>

using namespace std;

//======================================================================
bool isGoodFile(const char* filename)
{
  TFile f(filename);
  if(f.IsZombie()) return false;
  TTree* meta=(TTree*)f.Get("Meta");
  if(!meta) return false;
  if(!meta->GetBranch("POT_Total")) return false;
  if(!meta->GetBranch("POT_Used")) return false;
  if(!f.Get("Truth")) return false;
  return true;
}

//======================================================================
double getTChainPOT(TChain& ch, const char* branch="POT_Used")
{
  double sumPOTUsed=0;

  TObjArray *fileElements=ch.GetListOfFiles();
  TIter next(fileElements);
  TChainElement *chEl=0;
  while (( chEl=(TChainElement*)next() )) {
    TFile f(chEl->GetTitle());
    TTree* t=(TTree*)f.Get("Meta");
    if(!t){
      cout << "No Meta tree in file " << chEl->GetTitle() << endl;
      continue;
    }
    assert(t->GetEntries()==1);
    t->GetEntry(0);
    TLeaf* lUsed=t->GetLeaf(branch);
    if(lUsed)         sumPOTUsed+=lUsed->GetValue();
  }

  return sumPOTUsed;
}

//======================================================================

// Get the POT in a Resurrection MC file by parsing the filename,
// since resurrection MC files don't store their POT correctly
double getResurrectionMCFilePOT(const char* filename)
{
  // Parse the filename to work out how many subruns went into this
  // file, and multiply it by the known POT-per-file for MC to get the
  // total POT
  TString basename(gSystem->BaseName(filename));
  // Filenames look like:
  //
  // SIM_minerva_00013200_Subruns_0079-0080-0081-0082-0083_MECAnaTool_Ana_Tuple_v10r8p1.root
  //
  // so first split at underscores, then get the 5th entry and split it at dashes
  TObjArray* split1=basename.Tokenize("_");
  // The 4th item ought to be "Subruns", otherwise something has gone wrong
  TObjString* check=(TObjString*)split1->At(3);
  if(check->GetString()!=TString("Subruns")){
    cout << "Can't parse " << basename << " because item ought to be Subruns but is " << check << endl;
    exit(1);
  }
  TString subrunList=((TObjString*)split1->At(4))->GetString();
  TObjArray* split2=subrunList.Tokenize("-");
  const int nSubruns=split2->GetEntries();
  const double potPerSubrun=1e17;

  delete split1;
  delete split2;

  return nSubruns*potPerSubrun;
}

#endif

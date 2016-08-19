//******************************************************************
//*
//* mergeTrees.C
//* 
//* Laura Fields
//* 10 May 2012
//* 
//* Merges many analysis ntuples into one (or several files) with selected branches
//*
//******************************************************************

#ifndef __CINT__
#include "glob.h"
#endif

#include "merge_common.h"

#include <iostream>
#include <string>
#include <unistd.h>
//#include <cstdlib>

#include <iostream>
#include <iomanip>
#include <cstdlib>
#include <unistd.h>
#include <fstream>
#include <sstream>
#include <assert.h>

using namespace std;

//======================================================================
// Like mergeTrees, but keeps all the branches in CCQEAntiNuTool, and
// only merges one run.
void mergeMCRun2(const char* inDirBase, const char* outDir, int run, const char* tag="MECAnaTool", const char* treeName="CC1P1Pi") {
  //******************************************************************
  //* Set Location of output files
  //******************************************************************
  TString output=TString::Format("%s/merged_%s_run%08d.root", outDir, tag, run);

  //******************************************************************
  //* Load Input Ntuples
  //******************************************************************
  TChain inChain(treeName);
  TChain inChainTruth("Truth");

  string runStr(TString::Format("%08d", run));
  string runStrParts[4];
  for(int i=0; i<4; ++i) runStrParts[i]=runStr.substr(i*2, 2);
  TString inGlob(TString::Format("%s/%s/%s/%s/%s/SIM_*%s_*_%s*.root",
                                 inDirBase,
                                 runStrParts[0].c_str(),
                                 runStrParts[1].c_str(),
                                 runStrParts[2].c_str(),
                                 runStrParts[3].c_str(),
                                 runStr.c_str(),
                                 tag));
  
  cout << "Filename glob is " << inGlob << endl;
  cout << "Output filename is " << output << endl;

  glob_t g;
  glob(inGlob.Data(), 0, 0, &g);

  cout << "Total files " << g.gl_pathc << ". Adding good files" << endl;

  int nFiles=0;
  for(int i=0; i<(int)g.gl_pathc; ++i){
    if(i%100==0) cout << i << " " << flush;
    const char* filename=g.gl_pathv[i];
    if(isGoodFile(filename)){
      inChain.Add(filename);
      inChainTruth.Add(filename);
      ++nFiles;
    }
    else{
      cout << "Skipping " << filename << endl;
    }
  }
  cout << endl;

  // For summing up the POT totals from the Meta tree
  double sumPOTUsed=getTChainPOT(inChainTruth, "POT_Used");
  double sumPOTTotal=getTChainPOT(inChainTruth, "POT_Total");
  int nFilesTotal=g.gl_pathc;
  cout << "Added " << nFiles << " files out of " << nFilesTotal << " in run " << run << endl;
  cout << "POT totals: Total=" << sumPOTTotal << " Used=" << sumPOTUsed << endl;
  globfree(&g);

  if(nFiles==0){
    cout << "No files added, nothing to do..." << endl;
    return;
  }
  
  TStopwatch ts;
  TFile* fout=new TFile(output, "RECREATE");
  cout << "Merging ana tree" << endl;
  //setBranchStatuses(inChain);
  fout->cd(); // Just in case the surrounding lines get separated
  inChain.Merge(fout, 32000, "keep SortBasketsByBranch");

  cout << "Merging truth tree" << endl;
  //setBranchStatuses(inChainTruth);
  fout->cd();
  TTree* outTreeTruth=inChainTruth.CopyTree("mc_vtx[2]>5891 && mc_vtx[2]<8439");
  outTreeTruth->Write();
  
  // inChainTruth.Merge(fout, 32000, "keep SortBasketsByBranch");

  fout->cd();
  TTree* newMetaTree=new TTree("Meta", "Titles are stupid");
  newMetaTree->Branch("POT_Used", &sumPOTUsed);
  newMetaTree->Branch("POT_Total", &sumPOTTotal);
  //if(!mc) newMetaTree->Branch("POT_Unanalyzable", &sumPOTUnanalyzable);
  newMetaTree->Fill();
  newMetaTree->Write();
  ts.Stop();
  cout << "Merging time:" << endl;
  ts.Print();
}

int main(int argc, char *argv[])
{
    char const * user_name = getenv("$USER");
    
    cout << user_name << endl;
    
    string per_dir = "/pnfs/minerva/persistent/users/";// + &user_name;
    string infile = per_dir + "CC1P1Pi_PL13C_180816/grid/central_value/minerva/ana/v10r8p9";
    string outfile = per_dir;
    string treename = "CC1P1Pi";
    bool nominal = true;
    
    char cc;
    while((cc = getopt(argc, argv, "i:o:f:t:h:")) != -1){
        switch (cc){
                case 'i': infile = optarg; break;
                case 'o': outfile += optarg; break;
                case 'f': nominal = false; break;
                case 't': treename = optarg; break;
                case 'h':
                    std::cout << argv[0] << std::endl
                    << "*********************** Run Options ***********************" << std::endl
                    << " Default is to get and save files to the persistent drive  " << std::endl
                    << " however other locations can be defined using the -f option" << std::endl
                    << " In this case the full dir. location must be defined for   " << std::endl
                    << " input and output files.                                   " << std::endl
                    << " -i : \tset Set input file dir in persistent (or full dir. " << std::endl
                    << "      \tset when -f is called.                             " << std::endl
                    << " -o : \tset Set output file directory (or full dir. when -f" << std::endl
                    << "      \tset is called.                                     " << std::endl
                    << " -f : \tset Use full paths for input and output files      " << std::endl
                    << " -t : \tset Set name of analysis tree. Default is CC1P1Pi  " << std::endl
                    << "***********************************************************" << std::endl;
                    return 1; break;
                default: std::cout << "Running with default options" << std::endl;
        }
    }
    
    std::cout << "   Input Name: " << infile << std::endl;
    std::cout << "  Output Name:" << outfile << std::endl;
    std::cout << "Analysis Tree: " << treename << std::endl;
    
    mergeMCRun2("/pnfs/minerva/persistent/users/dcoplowe/CC1P1Pi_PL13C_180816/grid/central_value/minerva/ana/v10r8p9", "/pnfs/minerva/persistent/users/dcoplowe/", 13200, "CC1P1Pi","CC1P1Pi");
    return 0;
}


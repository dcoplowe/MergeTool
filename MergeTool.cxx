#ifndef _MERGETOOL_
#define _MERGETOOL_

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

#ifndef __CINT__
#include "glob.h"
#endif

using namespace std;

class MergeTool {
public:
    MergeTool(){};
    ~MergeTool(){};
    
    void EachRun(const char* inDirBase, const char* outDir, int run, const char* tag="CC1P1Pi", const char* treeName="CC1P1Pi", const char* save_name = "");
    void AllRuns(const char* outDir, const char* tag="CC1P1Pi", const char* treeName="CC1P1Pi", const char* save_name = "");
    
private:
    void Merge(TChain &inChain, TChain &inChainTruth, TString inGlob, TString output, int run = 0);
    double getTChainPOT(TChain& ch, const char* branch = "POT_Used");
    bool isGoodFile(const char* filename);
};

#endif

void MergeTool::Merge(TChain &inChain, TChain &inChainTruth, TString inGlob, TString output, int run){
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
    TTree* outTreeTruth=inChainTruth.CopyTree("");
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


void MergeTool::EachRun(const char* inDirBase, const char* outDir, int run, const char* tag, const char* treeName, const char* save_name){
    
    TString output=TString::Format("%s/merged_%s_%s_run%08d.root", outDir, tag, save_name, run);
    TChain inChain(treeName);
    TChain inChainTruth("Truth");
    
    string runStr(TString::Format("%08d", run));
    string runStrParts[4];
    for(int i=0; i<4; ++i) runStrParts[i]=runStr.substr(i*2, 2);
    TString inGlob(TString::Format("%s/%s/%s/%s/%s/SIM_*%s_*_%s*.root", inDirBase, runStrParts[0].c_str(), runStrParts[1].c_str(), runStrParts[2].c_str(), runStrParts[3].c_str(), runStr.c_str(), tag));

    Merge(inChain, inChainTruth, inGlob, output, run);
}

void MergeTool::AllRuns(const char* outDir, const char* tag, const char* treeName, const char* save_name){
    
    TString output=TString::Format("%s/%s_Full.root", outDir, save_name);
    
    //******************************************************************
    //* Load Input Ntuples
    //******************************************************************
    TChain inChain(treeName);
    TChain inChainTruth("Truth");
    
    TString inGlob(TString::Format("%s/merged_%s_%s_run*.root",outDir, tag, save_name));
    
    Merge(inChain, inChainTruth, inGlob, output);
}

double MergeTool::getTChainPOT(TChain& ch, const char* branch)
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

bool MergeTool::isGoodFile(const char* filename)
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


int main(int argc, char *argv[])
{
    char const * user_name = getenv("USER");
    if(!user_name){
        std::cout << "[WARNING]: Environment variable \"USER\" not found." << std::endl;
        
    }
    
    char const * anal_name = getenv("ANATREE");
    if(!anal_name){
        std::cout << "[WARNING]: Environment variable \"ANATREE\" not set. To set see requirements file." << std::endl;
    }
    
    string username(user_name);
    string anatree(anal_name);// t -- set tree
    
    string ana_save_name = "SMILE";// s -- savename
    
    string infile;
    bool re_opt_i = false;
    
    string run_s;
    bool re_opt_n = false;
    
    string outfile;
    bool re_opt_o = false;
    
    bool is_per_dir = false;
    
    char cc;
    while ((cc = getopt(argc, argv, "i:o:t:s:n:h::p::")) != -1) {
        switch (cc){
            case 'i': infile = optarg;  re_opt_i = true; break;
            case 'o': outfile = optarg; re_opt_o = true; break;
            case 't': anatree = optarg; break;
            case 'n': run_s = optarg; re_opt_n = true; break;
            case 's': ana_save_name = optarg; break;
            case 'm': merge = true; break;
            case 'p': is_per_dir = true; break;
            case 'h':
            //cout << argv[0] << endl
                cout << "|*********************** Run Options ****************************|" << endl
                << "| Default is to merge files in the root directory of your analy- |" << endl
                << "| sis. The number of runs also needs to be defined and can be    |" << endl
                << "| either a single run or run range (see below).                  |" << endl
                << "|                                                                |" << endl
                << "| Options:                                                       |" << endl
                << "|   -i   :  Set input file dir if \"-per\" is defined only the   |" << endl
                << "|        :  root directory of analysis in your persistent direc- |" << endl
                << "|        :  tory is required.                                    |" << endl
                << "|        :                                                       |" << endl
                << "|   -n   :  Run or run range: start-end e.g 13200-13250          |" << endl
                << "|        :  will run over 50 runs from 13200 to 13250.           |" << endl
                << "|        :                                                       |" << endl
                << "|   -o   :  Set output file directory.                           |" << endl
                << "|        :                                                       |" << endl
                << "|  -per  :  Assume in/out files are in the users persistent      |" << endl
                << "|        :  directory. In such cases \"-i\" becomes the root     |" << endl
                << "|        :  directory name. (e.g. -per -i CC1P1Pi_PL13C_290916   |" << endl
                << "|        :  to merge files in /pnfs/minerva/persistent/users/    |" << endl
                << "|        :  dcoplowe/CC1P1Pi_PL13C_290916/                       |" << endl
                << "|        :                                                       |" << endl
                << "|   -t   :  Set name of analysis tree.                           |"
                << "         :  Default is currently " << analname << endl
                << "|        :  If not set, this can be set in the PlotUtils requir- |" << endl
                << "|        :  ements files.                                        |" << endl
                << "|        :                                                       |" << endl
                << "|   -s   :  Set save name.                                       |" << endl
                << "|        :                                                       |" << endl
                << "| -merge :  Combine runs into a single root file.                |" << endl
                << "|        :                                                       |" << endl
                << "| -help  :  Print this.                                          |" << endl
                << "|        :                                                       |" << endl
                << "|****************************************************************|" << endl;
                return 1; break;
            default: return 1;
        }
    }
    
    if(!(re_opt_i || re_opt_n)){
        cout << "|============ Minimum Requirements to run ============|" << endl;
        cout << "|                                                     |" << endl;
        cout << "|    -i     Set input file dir name in persistent     |" << endl;
        cout << "|    -n     Number of runs to merge                   |" << endl;
        cout << "|    -help  For more options.                         |" << endl;
        cout << "|_____________________________________________________|" << endl;
        return 0;
    }
    
    int first_run = -999;
    int last_run =  -999;
    TString run_ts = run_s;
    if(run_ts.Contains("-",TString::kExact)){
        TString tmp_first( run_ts(0,run_ts.First("-")) );
        first_run = tmp_first.Atoi();
        
        TString tmp_last( run_ts(run_ts.First("-") + 1, run_ts.Length()) );
        last_run = tmp_last.Atoi();
    }
    else{
        first_run = run_ts.Atoi();
        last_run = first_run;
    }

    if(is_per_dir){
        string per_dir = "/pnfs/minerva/persistent/users/" + username + "/";
        
        string tmp_in(infile);
        infile = per_dir + tmp_in;
        
        string tmp_out(outfile);
        outfile = per_dir + tmp_out;
    }
    
    if(!re_opt_o) outfile = infile;
    
    MergeTool * merger = new MergeTool();
    
    if(merge){
            cout<< "Only merging merged runs" << endl;
            merger->AllRuns(infile.c_str(), analname.c_str(), anatree.c_str(), ana_save_name.c_str());
    }
    else{
        cout << "Merging sub-runs for each run and then merging runs" << endl;
        for(int i=first_run; i < last_run + 1; i++){
            cout << "Merging Run " << i << endl;
            merger->EachRun(infile.c_str(), outfile.c_str(), i, analname.c_str(), anatree.c_str(), ana_save_name.c_str());
        }
    }
    
    delete merger;
    cout << "Finished merging files." << endl;
    return 0;
}
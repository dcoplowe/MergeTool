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
#include <stdlib.h>// or cstdlib is c++

#ifndef __CINT__
#include "glob.h"
#endif

using namespace std;

class MergeTool {
public:
    MergeTool() : m_realdata(false), m_fullpath(false), m_checkmeta(true) {};
    ~MergeTool(){};
    
    void EachRun(const char* inDirBase, const char* outDir, int run, const char* tag="CC1P1Pi", const char* treeName="sel", const char* save_name = "");
    void SingleMerge(const char* inDirBase, const char* outDir, int first_run, int last_run, const char* tag="CC1P1Pi", const char* treeName="sel", const char* save_name = "");
    void AllRuns(const char* outDir, const char* tag="CC1P1Pi", const char* treeName="sel", const char* save_name = "");

    void SetMinervaRelease(char const * var){ minerva_release = string(var); }
    void CheckMetaData(bool var){ m_checkmeta = var; }
    void IsRealData(bool var){ m_realdata = var; }
    void FullPath(bool var){ m_fullpath = var; }
    
private:
    void Merge(TChain &inChain, TChain &inChainTruth, TString inGlob, TString output);
    void Merge(const char* treeName, const char* truthname, TString inGlob, TString output);
    
    double getTChainPOT(TChain * ch, const char* branch = "POT_Used");
    double getTChainPOT(TChain& ch, const char* branch = "POT_Used");
    bool isGoodFile(const char* filename);
    bool GoodMetaData(const char* filename);
    string minerva_release;
    bool m_realdata;
    bool m_fullpath;
    bool m_checkmeta;
};

#endif

void MergeTool::Merge(TChain &inChain, TChain &inChainTruth, TString inGlob, TString output){
    cout << "Filename glob is " << inGlob << endl;
    cout << "Output filename is " << output << endl;
    
    //Set boolians in initialisation:
    
    glob_t g;
    glob(inGlob.Data(), 0, 0, &g);
    
    cout << "Total files " << g.gl_pathc << ". Adding good files" << endl;
    
    //Mode this up from below:
    TStopwatch ts;
    TFile* fout=new TFile(output, "RECREATE");
    cout << "Merging ana tree" << endl;
    //setBranchStatuses(inChain);
    fout->cd(); // Just in case the surrounding lines get separated
    //End
    
    int nFiles=0;
    for(int i=0; i<(int)g.gl_pathc; ++i){
        if(i%100==0) cout << i << " " << flush;
        const char* filename=g.gl_pathv[i];
        if(isGoodFile(filename) && GoodMetaData(filename)){
            inChain.Add(filename);
            if(!m_realdata) inChainTruth.Add(filename);
            //else inChain
            ++nFiles;
        }
        else{
            cout << "Skipping " << filename << endl;
        }
    }
    cout << endl;
    
    // For summing up the POT totals from the Meta tree
    
//    double sumPOTUsed = 0.;=getTChainPOT(inChainTruth, "POT_Used");
//    double sumPOTTotal = 0.;
    
//    if(!m_realdata){
    double sumPOTUsed  = getTChainPOT(inChain, "POT_Used");
    double sumPOTTotal = getTChainPOT(inChain, "POT_Total");
//    }
    //else{
    //    sumPOTUsed  = getTChainPOT(inChainTruth, "POT_Used");
    //    sumPOTTotal = getTChainPOT(inChainTruth, "POT_Total");
    //}
    
    int nFilesTotal=g.gl_pathc;
    cout << "Added " << nFiles << " files out of " << nFilesTotal << endl;
    cout << "POT totals: Total = " << sumPOTTotal << " Used = " << sumPOTUsed << endl;
    globfree(&g);
    
    if(nFiles==0){
        cout << "No files added, nothing to do..." << endl;
        return;
    }
    
//    TStopwatch ts;
//    TFile* fout=new TFile(output, "RECREATE");
//    cout << "Merging ana tree" << endl;
//    //setBranchStatuses(inChain);
//    fout->cd(); // Just in case the surrounding lines get separated
    inChain.Merge(fout, 32000, "keep SortBasketsByBranch");
    
    if(!m_realdata){
    cout << "Merging truth tree" << endl;
    //setBranchStatuses(inChainTruth);
    fout->cd();
    TTree* outTreeTruth=inChainTruth.CopyTree("");
    outTreeTruth->Write();
    }
    // inChainTruth.Merge(fout, 32000, "keep SortBasketsByBranch");
    
    fout->cd();
    TTree* newMetaTree=new TTree("Meta", "");
    newMetaTree->Branch("POT_Used", &sumPOTUsed);
    newMetaTree->Branch("POT_Total", &sumPOTTotal);
    //if(!mc) newMetaTree->Branch("POT_Unanalyzable", &sumPOTUnanalyzable);
    newMetaTree->Fill();
    newMetaTree->Write();
    ts.Stop();
    cout << "Merging time:" << endl;
    ts.Print();
    
    fout->Close();
}

void MergeTool::Merge(const char* treeName, const char* truthname, TString inGlob, TString output){
    
    cout << "Filename glob is " << inGlob << endl;
    cout << "Output filename is " << output << endl;
    
    //Set boolians in initialisation:
    
    glob_t g;
    glob(inGlob.Data(), 0, 0, &g);
    
    cout << "Total files " << g.gl_pathc << ". Adding good files" << endl;
    
    //Mode this up from below:
    TStopwatch ts;
    TFile* fout=new TFile(output, "RECREATE");
    cout << "Merging ana tree" << endl;
    //setBranchStatuses(inChain);
    fout->cd(); // Just in case the surrounding lines get separated
    //End
    
    TChain inChain(treeName);
    TChain inChainTruth(truthname);
    
    int nFiles=0;
    for(int i=0; i<(int)g.gl_pathc; ++i){
        if(i%100==0) cout << i << " " << flush;
        const char* filename=g.gl_pathv[i];
        if(isGoodFile(filename) && GoodMetaData(filename)){
            inChain.Add(filename);
            if(!m_realdata) inChainTruth.Add(filename);
            //else inChain
            ++nFiles;
        }
        else{
            cout << "Skipping " << filename << endl;
        }
    }
    cout << endl;
    
    // For summing up the POT totals from the Meta tree
    
    //    double sumPOTUsed = 0.;=getTChainPOT(inChainTruth, "POT_Used");
    //    double sumPOTTotal = 0.;
    
    //    if(!m_realdata){
    double sumPOTUsed  = getTChainPOT(inChain, "POT_Used");
    double sumPOTTotal = getTChainPOT(inChain, "POT_Total");
    //    }
    //else{
    //    sumPOTUsed  = getTChainPOT(inChainTruth, "POT_Used");
    //    sumPOTTotal = getTChainPOT(inChainTruth, "POT_Total");
    //}
    
    int nFilesTotal=g.gl_pathc;
    cout << "Added " << nFiles << " files out of " << nFilesTotal << endl;
    cout << "POT totals: Total = " << sumPOTTotal << " Used = " << sumPOTUsed << endl;
    globfree(&g);
    
    if(nFiles==0){
        cout << "No files added, nothing to do..." << endl;
        return;
    }
    
    //    TStopwatch ts;
    //    TFile* fout=new TFile(output, "RECREATE");
    //    cout << "Merging ana tree" << endl;
    //    //setBranchStatuses(inChain);
    //    fout->cd(); // Just in case the surrounding lines get separated
    inChain.Merge(fout, 32000, "keep SortBasketsByBranch");
    
    if(!m_realdata){
        cout << "Merging truth tree" << endl;
        //setBranchStatuses(inChainTruth);
        fout->cd();
        TTree* outTreeTruth=inChainTruth.CopyTree("");
        outTreeTruth->Write();
    }
    // inChainTruth.Merge(fout, 32000, "keep SortBasketsByBranch");
    
    fout->cd();
    TTree* newMetaTree=new TTree("Meta", "");
    newMetaTree->Branch("POT_Used", &sumPOTUsed);
    newMetaTree->Branch("POT_Total", &sumPOTTotal);
    //if(!mc) newMetaTree->Branch("POT_Unanalyzable", &sumPOTUnanalyzable);
    newMetaTree->Fill();
    newMetaTree->Write();
    ts.Stop();
    cout << "Merging time:" << endl;
    ts.Print();
    
    fout->Close();
    
    
}


void MergeTool::EachRun(const char* inDirBase, const char* outDir, int run, const char* tag, const char* treeName, const char* save_name){
    
    TString output=TString::Format("%s/merged_%s_%s_run%08d.root", outDir, tag, save_name, run);
//    TChain inChain(treeName);
//    TChain inChainTruth("Truth");
    
    string runStr(TString::Format("%08d", run));
    string runStrParts[4];
    for(int i=0; i<4; ++i) runStrParts[i]=runStr.substr(i*2, 2);
    
    string subpath;
    if(!m_fullpath) subpath = "grid/central_value/minerva/ana/";
    else subpath = "";
    
    TString inGlob(TString::Format("%s/%s%s/%s/%s/%s/%s/%s_*%s_*_%s*.root",
                                   inDirBase,
                                   subpath.c_str(),
                                   minerva_release.c_str(),
                                   runStrParts[0].c_str(),
                                   runStrParts[1].c_str(),
                                   runStrParts[2].c_str(),
                                   runStrParts[3].c_str(),
                                   m_realdata ? "MV" : "SIM",
                                   runStr.c_str(),
                                   tag));

    Merge(treeName, "Truth", inGlob, output);

//    Merge(inChain, inChainTruth, inGlob, output);

}

void MergeTool::AllRuns(const char* outDir, const char* tag, const char* treeName, const char* save_name){
    
    TString output=TString::Format("%s/%s_Full.root", outDir, save_name);
    
    //******************************************************************
    //* Load Input Ntuples
    //******************************************************************
//    TChain inChain(treeName);
//    TChain inChainTruth("Truth");
    
    TString inGlob(TString::Format("%s/merged_%s_%s_run*.root",outDir, tag, save_name));
    
    Merge(treeName, "Truth", inGlob, output);
    
//    Merge(inChain, inChainTruth, inGlob, output);
//    Merge(inChain, inChainTruth, inGlob, output);


}

void MergeTool::SingleMerge(const char* inDirBase, const char* outDir, int first_run, int last_run, const char* tag, const char* treeName, const char* save_name){
    
    TString output=TString::Format("%s/merged_%s_%s_run%08d-%08d.root", outDir, tag, save_name, first_run, last_run);
    
    int nFiles=0;
    int nFilesTotal= 0;

    //Moved this up from below:
    TStopwatch ts;
    TFile * fout = new TFile(output, "RECREATE");
    cout << "Merging ana tree" << endl;
    //setBranchStatuses(inChain);
    fout->cd(); // Just in case the surrounding lines get separated
    //END
    
    TChain * inChain = new TChain(treeName);
    TChain * inChainTruth = new TChain("Truth");
    
    int range = last_run - first_run;
    for(int run = first_run; run < first_run + range; run++){
        string runStr(TString::Format("%08d", run));
        string runStrParts[4];
        for(int i=0; i<4; ++i) runStrParts[i]=runStr.substr(i*2, 2);
        
        string subpath;
        if(!m_fullpath) subpath = "grid/central_value/minerva/ana/";
        else subpath = "";
        
        TString inGlob(TString::Format("%s/%s%s/%s/%s/%s/%s/%s_*%s_*_%s*.root",
                                       inDirBase,
                                       subpath.c_str(),
                                       minerva_release.c_str(),
                                       runStrParts[0].c_str(),
                                       runStrParts[1].c_str(),
                                       runStrParts[2].c_str(),
                                       runStrParts[3].c_str(),
                                       m_realdata ? "MV" : "SIM",
                                       runStr.c_str(),
                                       tag));
        
        glob_t g;
        glob(inGlob.Data(), 0, 0, &g);
        
        nFilesTotal += g.gl_pathc;
        cout << "Total files " << g.gl_pathc << ". Adding good files" << endl;

        for(int i=0; i<(int)g.gl_pathc; ++i){
            if(i%100==0) cout << i << " " << flush;
            const char* filename=g.gl_pathv[i];
            if(isGoodFile(filename) && GoodMetaData(filename)){
                inChain->Add(filename);
                if(!m_realdata) inChainTruth->Add(filename);
                //else inChain
                ++nFiles;
            }
            else{
                cout << "Skipping " << filename << endl;
            }
        }
        globfree(&g);

        cout << endl;
    }
    
    // For summing up the POT totals from the Meta tree
    
    //    double sumPOTUsed = 0.;=getTChainPOT(inChainTruth, "POT_Used");
    //    double sumPOTTotal = 0.;
    
    //    if(!m_realdata){
    cout << "Getting POT and summing." << endl;
    double sumPOTUsed  = getTChainPOT(inChain, "POT_Used");
    double sumPOTTotal = getTChainPOT(inChain, "POT_Total");
    cout << "Finished counting POT." << endl;
    //    }
    //else{
    //    sumPOTUsed  = getTChainPOT(inChainTruth, "POT_Used");
    //    sumPOTTotal = getTChainPOT(inChainTruth, "POT_Total");
    //}
    
    cout << "Added " << nFiles << " files out of " << nFilesTotal << endl;
    cout << "POT totals: Total = " << sumPOTTotal << " Used = " << sumPOTUsed << endl;
    
    if(nFiles==0){
        cout << "No files added, nothing to do..." << endl;
        return;
    }
    
//    TStopwatch ts;
//    TFile* fout=new TFile(output, "RECREATE");
//    cout << "Merging ana tree" << endl;
//    //setBranchStatuses(inChain);
//    fout->cd(); // Just in case the surrounding lines get separated
    inChain->Merge(fout, 32000, "keep SortBasketsByBranch");
    
    if(!m_realdata){
        cout << "Merging truth tree" << endl;
        //setBranchStatuses(inChainTruth);
        fout->cd();
        TTree* outTreeTruth=inChainTruth->CopyTree("");
        outTreeTruth->Write();
    }
    // inChainTruth.Merge(fout, 32000, "keep SortBasketsByBranch");
    
    fout->cd();
    TTree* newMetaTree=new TTree("Meta", "");
    newMetaTree->Branch("POT_Used", &sumPOTUsed);
    newMetaTree->Branch("POT_Total", &sumPOTTotal);
    //if(!mc) newMetaTree->Branch("POT_Unanalyzable", &sumPOTUnanalyzable);
    newMetaTree->Fill();
    newMetaTree->Write();
    ts.Stop();
    cout << "Merging time:" << endl;
    ts.Print();
    
    fout->Close();
    
    delete fout;//Added 210117
    
}

double MergeTool::getTChainPOT(TChain& ch, const char* branch)
{
    double sumPOTUsed=0;
    
    TObjArray * fileElements = ch.GetListOfFiles();
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
        
        if(lUsed) delete lUsed;//Added 210117
        delete t;//Added 210117
    }
    
    delete fileElements;//Added 210117
    
    return sumPOTUsed;
}

double MergeTool::getTChainPOT(TChain * ch, const char* branch)
{
    double sumPOTUsed=0;
    
    TObjArray * fileElements = ch->GetListOfFiles();
    TIter next(fileElements);
    TChainElement *chEl=0;
    while (( chEl=(TChainElement*)next() )) {
        TFile f(chEl->GetTitle());
        TTree * t = (TTree*)f.Get("Meta");
        if(!t){
            cout << "No Meta tree in file " << chEl->GetTitle() << endl;
            continue;
        }
        assert(t->GetEntries()==1);
        t->GetEntry(0);
        TLeaf* lUsed=t->GetLeaf(branch);
        if(lUsed)         sumPOTUsed+=lUsed->GetValue();
        cout << "MergeTool::getTChainPOT(TChain * ch, const char* branch) :: Looping. " << chEl->GetTitle() << endl;

        if(lUsed) delete lUsed;//Added 210117
        f.Close();
//        delete t;//Added 210117
    }
    
//    delete fileElements;//Added 210117
    
    cout << "MergeTool::getTChainPOT(TChain * ch, const char* branch) :: delete fileElements. " << chEl->GetTitle() << endl;

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
    if(!m_realdata && !f.Get("Truth")) return false;//Check this.
    delete meta;//Added 210117
    return true;
}

bool MergeTool::GoodMetaData(const char* filename){
    
    if(!m_checkmeta) return true;
    
    TFile f(filename);
    if(f.IsZombie()) return false;
    TTree* meta=(TTree*)f.Get("Meta");
    if(!meta) return false;
    if(meta->GetEntriesFast() == 0) return false;
    if(!meta->GetBranch("POT_Used")) return false;
    delete meta;//Added 210117
    return true;
}

int main(int argc, char *argv[])
{
    char const * user_name = getenv("USER");
    if(!user_name){
        std::cout << "[WARNING]: Environment variable \"USER\" not found." << std::endl;
        
    }
    
    char const * anal_tree = getenv("ANATREENAME");
    if(!anal_tree){
        std::cout << "[WARNING]: Environment variable \"ANATREE\" not set. To set see requirements file." << std::endl;
    }
    
    char const * anal_tool = getenv("ANATOOLNAME");
    if(!anal_tool){
        std::cout << "[WARNING]: Environment variable \"ANATOOLNAME\" not set. To set see requirements file." << std::endl;
    }
    
    char const * minerva_release = getenv("MINERVA_RELEASE");
    if(!minerva_release){
        std::cerr << "[ERROR]: environment variable \"MINERVA_RELEASE\" not set. "
        "Cannot determine source tree location." << std::endl;
        return 1;
    }
    
    string username(user_name);
    string anatree(anal_tree);// t -- set tree
    string analtool(anal_tool);
    
    string ana_save_name = "minerva";// s -- savename
    
    string infile;
    bool re_opt_i = false;
    
    string run_s;
    bool re_opt_n = false;
    
    string outfile;
    bool re_opt_o = false;
    
    bool is_per_dir = false;
    
    int merge = -1;
    
    bool meta_data_check = true;
    
    bool real_data = false;
    
    bool full_path = false;
    
    char cc;
    while ((cc = getopt(argc, argv, "i:o:t:s:n:h:p:a:m:c:d:f:v:")) != -1) {
        switch (cc){
            case 'v': minerva_release = optarg; break;
            case 'i': infile = optarg;  re_opt_i = true; break;
            case 'o': outfile = optarg; re_opt_o = true; break;
            case 't': anatree = optarg; break;
            case 'n': run_s = optarg; re_opt_n = true; break;
            case 's': ana_save_name = optarg; break;
            case 'm': merge = atoi(optarg); break;
            case 'p': is_per_dir = true; break;
            case 'a': analtool = optarg; break;
            case 'c': meta_data_check = false; break;
            case 'd': real_data = true; break;
            case 'f': full_path = true; break;
            case 'h':
            //cout << argv[0] << endl
                cout << "|************************************************** Example *******************************************************|" << endl
                << "| To merge from runs 13200 to 13260 of an analysis output into a single root file do:                              |" << endl
                << "|                                                                                                                  |" << endl
                << "| MergeTool.exe -i /pnfs/minerva/persistant/persistent/users/dcoplowe/CC1P1Pi_PL13C_111216 -n 13200-13260          |" << endl
                << "|                                                                                                                  |" << endl
                << "| This will save the output into the -i directory                                                                  |" << endl
                << "|                                                                                                                  |" << endl
                << "| To merge individual runs do:                                                                                     |" << endl
                << "|                                                                                                                  |" << endl
                << "| MergeTool.exe -i /pnfs/minerva/persistant/persistent/users/dcoplowe/CC1P1Pi_PL13C_111216 -n 13200-13260 -m=1     |" << endl
                << "|                                                                                                                  |" << endl
                << "| To combine the output of -m=1 do:                                                                                |" << endl
                << "|                                                                                                                  |" << endl
                << "| MergeTool.exe -i /pnfs/minerva/persistant/persistent/users/dcoplowe/CC1P1Pi_PL13C_111216 -n 13200-13260 -m=2     |" << endl
                << "|                                                                                                                  |" << endl
                << "|******************************************************************************************************************|" << endl
                << " " << endl
                << " " << endl
                << "|*********************** Run Options ****************************|" << endl
                << "| Default is to merge files in the root directory of your analy- |" << endl
                << "| sis. The number of runs also needs to be defined and can be    |" << endl
                << "| either a single run or run range (see below).                  |" << endl
                << "|                                                                |" << endl
                << "| Options:                                                       |" << endl
                << "|   -i   :  Set input file dir if \"-per\" is defined only the   |" << endl
                << "|        :  root directory of your analysis in your persistent   |" << endl
                << "|        :  directory is required.                               |" << endl
                << "|        :                                                       |" << endl
                << "|   -n   :  Run or run range: start-end e.g 13200-13250          |" << endl
                << "|        :  will run over 50 runs from 13200 to 13250.           |" << endl
                << "|        :                                                       |" << endl
                << "|   -o   :  Set output file directory.                           |" << endl
                << "|        :                                                       |" << endl
                << "|  -per  :  Assume in/out files are in the users persistent      |" << endl
                << "|        :  directory. In such cases \"-i\" becomes the root       |" << endl
                << "|        :  directory name. (e.g. -per -i CC1P1Pi_PL13C_290916   |" << endl
                << "|        :  to merge files in /pnfs/minerva/persistent/users/    |" << endl
                << "|        :  dcoplowe/CC1P1Pi_PL13C_290916/                       |" << endl
                << "|        :                                                       |" << endl
                << "|   -a   :  Analysis tool name (Your analysis tool used to make  |" << endl
                << "|        :  the root files you want to merge)                    |" << endl
                << "         :  Default is currently " << analtool << endl
                << "|        :                                                       |" << endl
                << "|   -t   :  Set name of analysis tree.                           |" << endl
                << "         :  Default is currently " << anatree << endl
                << "|        :  If not set, this can be set in the PlotUtils requir- |" << endl
                << "|        :  ements files.                                        |" << endl
                << "|        :                                                       |" << endl
                << "|   -s   :  Set save name.                                       |" << endl
                << "|        :                                                       |" << endl
                << "|   -v   :  Set version of input anatuple files (vXrYpZ).        |" << endl
                << "|        :  If not specified, use current release.               |" << endl
                << "|        :                                                       |" << endl
                << "| -m     :  Option 1 (-m=1): Combine each run into single root   |" << endl
                << "|        :  file.                                                |" << endl
                << "|        :  Option 2 (-m=2): Combine output of (-m-1) into a     |" << endl
                << "|        :  single root file.                                    |" << endl
                << "|        :                                                       |" << endl
                << "| -check :  Merge without checking POT in Meta tree is good      |" << endl
                << "|        :  (exists).                                            |" << endl
                << "|        :                                                       |" << endl
                << "| -data  :  Run on a data file (merge without Truth tree). Def-  |" << endl
                << "|        :  ault is to assume you are running on MC.             |" << endl
                << "|        :                                                       |" << endl
                << "| -full  :  Add full directory path. This enables you to merge   |" << endl
                << "|        :  special runs where the sub directories are not       |" << endl
                << "|        :  grid/central_value/minerva/ana. When \"-full\" is      |" << endl
                << "|        :  defined simply set the full path up to and including |" << endl
                << "|        :  ana using \"-i\".                                      |" << endl
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
        cout << "|    -i     Set input file dir name                   |" << endl;
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
    
    cout << "|---------------------------------- Inputs ----------------------------------" << endl;
    if(meta_data_check) cout << "|                   Checking POT info is good. (Switch off using -check)" << endl;
    if(is_per_dir || full_path || real_data) cout << "| Option(s) called: " << endl;
    if(is_per_dir) cout << "|                   (-per)   In/out files are in persistent." << endl;
//    if(merge) cout << "|                   (-merge) Merge the merged run files." << endl;
    if(real_data) cout << "|                   (-real) Merging real data files." << endl;
    if(full_path) cout << "|                   (-full) User defining full path." << endl;
    cout << "| Input  (-i): " << infile << endl;
    cout << "| N Runs (-n): " << run_s << endl;
    cout << "| Output (-o): " << outfile << endl;
    cout << "| Analysis Tree Name (-t): " << anatree << endl;
    cout << "| Analysis Tool Name (-a): " << analtool << endl;
    cout << "| Optional Save Name (-s): " << ana_save_name << endl;
    cout << "|--------------------------------- Running ----------------------------------" << endl;
    
    MergeTool * merger = new MergeTool();
    merger->CheckMetaData(meta_data_check);
    merger->SetMinervaRelease(minerva_release);
    merger->IsRealData(real_data);
    merger->FullPath(full_path);
    
    if(merge == 2){
            cout<< "Only merging merged runs" << endl;
            merger->AllRuns(infile.c_str(), analtool.c_str(), anatree.c_str(), ana_save_name.c_str());
    }
    else if(merge == 1){
        cout << "Merging sub-runs for each run" << endl;
        for(int i=first_run; i < last_run + 1; i++){
            cout << "Merging Run " << i << endl;
            merger->EachRun(infile.c_str(), outfile.c_str(), i, analtool.c_str(), anatree.c_str(), ana_save_name.c_str());
        }
    }
    else{
        cout << "Merging sub-runs for each run into a single root file" << endl;
        merger->SingleMerge(infile.c_str(), outfile.c_str(), first_run, last_run, analtool.c_str(), anatree.c_str(), ana_save_name.c_str());
    }
    
    delete merger;
    cout << "|-------------------------- Finished merging files --------------------------" << endl;
    return 0;
}

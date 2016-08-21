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


/*void Merger(TChain inChain, TChain inChainTruth, TString inGlob, TString output, int run = 0){
    
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
}*/

//======================================================================
// Like mergeTrees, but keeps all the branches in CCQEAntiNuTool, and
// only merges one run.
void mergeMCRun2(const char* inDirBase, const char* outDir, int run, const char* tag="CC1P1Pi", const char* treeName="CC1P1Pi", const char* save_name = "") {
    //******************************************************************
    //* Set Location of output files
    //******************************************************************
    TString output=TString::Format("%s/merged_%s_%s_run%08d.root", outDir, tag, save_name, run);
    
    //******************************************************************
    //* Load Input Ntuples
    //******************************************************************
    TChain inChain(treeName);
    TChain inChainTruth("Truth");
    
    string runStr(TString::Format("%08d", run));
    string runStrParts[4];
    for(int i=0; i<4; ++i) runStrParts[i]=runStr.substr(i*2, 2);
    TString inGlob(TString::Format("%s/%s/%s/%s/%s/SIM_*%s_*_%s*.root", inDirBase, runStrParts[0].c_str(), runStrParts[1].c_str(), runStrParts[2].c_str(), runStrParts[3].c_str(), runStr.c_str(), tag));
    
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
    //Merger(inChain, inChainTruth, inGlob, output, run);
}

void mergeAllRuns(const char* outDir, const char* tag="CC1P1Pi", const char* treeName="CC1P1Pi", const char* save_name = "") {
    //******************************************************************
    //* Set Location of output files
    //******************************************************************
    TString output=TString::Format("%s/%s_Full.root", outDir, save_name);
    
    //******************************************************************
    //* Load Input Ntuples
    //******************************************************************
    TChain inChain(treeName);
    TChain inChainTruth("Truth");
    
    TString inGlob(TString::Format("%s/merged_%s_%s_run*.root",outDir, tag, save_name));
    
    //From here is generic to both:
    
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
    cout << "Added " << nFiles << " files out of " << nFilesTotal << endl;
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
    //Merger(inChain, inChainTruth, inGlob, output);
}

int main(int argc, char *argv[])
{
    char const * user_name = getenv("MT_USER");
    if(!user_name){
        std::cerr << "[ERROR]: environment variable \"MT_USER\" not set. "
        "Cannot determine source tree location." << std::endl;
        return 1;
    }
    
    char const * anal_name = getenv("MT_ANAL");
    if(!anal_name){
        std::cerr << "[ERROR]: environment variable \"MT_ANAL\" not set. "
        "Cannot determine source tree location." << std::endl;
        return 1;
    }
    
    char const * minerva_release = getenv("MINERVA_RELEASE");
    if(!minerva_release){
        std::cerr << "[ERROR]: environment variable \"MINERVA_RELEASE\" not set. "
        "Cannot determine source tree location." << std::endl;
        return 1;
    }
    
    string username(user_name);
    string analname(anal_name);
    string minervarelease(minerva_release);
    
    string per_dir = "/pnfs/minerva/persistent/users/" + username + "/";
    string infile = per_dir;
    string outfile = per_dir;
    string treename = analname;
    string ana_save_name = analname;
    bool nominal = true;
    bool full_merge = false;
    string merge_opt;
    
    string run_s;
    int per_len = (int)strlen(per_dir.c_str());
    
    bool re_opt_i = false;
    bool re_opt_n = false;
    
    char cc;
    while((cc = getopt(argc, argv, "i:o:f:t:h::n:a:s:m::")) != -1){
        switch (cc){
                case 'i': infile += optarg; re_opt_i = true; break;
                case 'o': outfile += optarg; break;
                case 'f': nominal = false; break;
                case 't': treename = optarg; break;
                case 'n': run_s = optarg; re_opt_n = true; break;
                case 'a': analname = optarg; break;
                case 's': ana_save_name = optarg; break;
                case 'm': full_merge = true; if(optarg) merge_opt = optarg; break;
                case 'h':
                cout << argv[0] << endl
                << "*********************** Run Options ***********************" << endl
                << " Default is to get and save files to the persistent drive  " << endl
                << " however other locations can be defined using the -f option" << endl
                << " In this case the full dir. location must be defined for   " << endl
                << " input and output files.                                   " << endl
                << " -i : \t Set input file dir in persistent (or full dir. " << endl
                << "      \t excluding grid/central/... when -f is called). " << endl
                << " -o : \t Set output file directory (or full dir. when   " << endl
                << "      \t -f is called).                                 " << endl
                << " -f : \t Use full paths for input and output files      " << endl
                << " -t : \t Set name of analysis tree. Default is " << analname << endl
                << " -n : \t Run or run range: start-end e.g 13200-13250    " << endl
                << "      \t will run over 50 runs from 13200 to 13250.     " << endl
                << " -a : \t Define the analysis name, defualt is " << analname <<"." << endl
                << " -s : \t Set save name.                                 " << endl
                << " -m : \t Merge the runs. If \"merge\" is given as an arg" << endl
                << "      \t then only the merging of runs will be done.    " << endl
                << "***********************************************************" << endl;
                return 1; break;
            default: return 1;
        }
    }
    
    cout << "Still works" << endl;
    
    if(!(re_opt_i || re_opt_n)){
        cout << "|============ Minimum Requirements to run ============|" << endl;
        cout << "|                                                     |" << endl;
        cout << "|    -i Set input file dir name in persistent         |" << endl;
        cout << "|    -n Number of runs to merge                       |" << endl;
        cout << "|_____________________________________________________|" << endl;
        return 0;
    }
    
    
    if(!nominal){
        TString tmp_infile = infile;
        TString noper_infile( tmp_infile(per_len, (int)tmp_infile.Length()) );
        infile = noper_infile.Data();
        
        TString tmp_outfile = outfile;
        TString noper_outfile(tmp_outfile(per_len, (int)tmp_outfile.Length()) );
        outfile = noper_outfile.Data();
    }
    
    infile += "/grid/central_value/minerva/ana/" + minervarelease + "/";
    
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
    
    cout << "    Input Name: " << infile << endl;
    cout << "   Output Name: " << outfile << endl;
    cout << " Analysis Tree: " << treename << endl;
    cout << " Analysis Name: " << ana_save_name << endl;
    cout << "Merging run(s): " << run_s << endl;
    
    if(full_merge){
        cout << "Full Merge Called" << endl;
    }
    
    if(full_merge){
        if(merge_opt == "merge"){
            cout<< "Only merging merged runs" << endl;
            mergeAllRuns(outfile.c_str(), analname.c_str(), treename.c_str(), ana_save_name.c_str());
        }
        else{
            cout << "Merging sub-runs for each run and then merging runs" << endl;
            for(int i=first_run; i < last_run + 1; i++){
                cout << "Merging Run " << i << endl;
                mergeMCRun2(infile.c_str(), outfile.c_str(), i, analname.c_str(), treename.c_str(), ana_save_name.c_str());
            }
            mergeAllRuns(outfile.c_str(), analname.c_str(), treename.c_str(), ana_save_name.c_str());
        }
        
    }
    else{
        for(int i=first_run; i < last_run + 1; i++){
            cout << "Merging Run " << i << endl;
            mergeMCRun2(infile.c_str(), outfile.c_str(), i, analname.c_str(), treename.c_str(), ana_save_name.c_str());
        }
        
    }

    cout << "File Merge Finished" << endl;
    return 0;
}

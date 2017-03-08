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
#include "TTree.h"

#ifndef __CINT__
#include "glob.h"
#endif

using namespace std;

class MergeTool {
public:
    MergeTool(std::string root_indir, std::string minerva_release, std::string analysis_name, std::string analysis_tree, bool is_mc, bool check_meta_data);
    ~MergeTool(){;}
    
    void SetRange(int start = -999, int finish = -999){ m_start = start; m_finish = finish; }
    void SetOutFileName(std::string outfilename){ m_outfilename = outfilename; }
    void Run();
    
private:
    
    std::string m_root_indir;
    std::string m_minerva_release;
    std::string m_analysis_name;
    std::string m_analysis_tree;

    bool m_is_mc;
    bool m_check_meta_data;
    
    int m_00;
    int m_start;
    int m_finish;
    
    std::string m_outfilename;
    
    std::string m_subpath;
    std::string m_basedir;
    
    void InspectDir();
    bool GoodFile(const char* filename);
    bool GoodMeta(const char* filename);
    double getTChainPOT(TChain * ch, const char* branch);
    
};

#endif

MergeTool::MergeTool(std::string root_indir, std::string minerva_release, std::string analysis_name, std::string analysis_tree, bool is_mc, bool check_meta_data) : m_root_indir(root_indir), m_minerva_release(minerva_release), m_analysis_name(analysis_name), m_analysis_tree(analysis_tree), m_is_mc(is_mc), m_check_meta_data(check_meta_data), m_00(0), m_start(-999), m_finish(-999), m_outfilename("") {

    cout << "m_root_indir      = " << m_root_indir << endl;
    cout << "m_minerva_release = " << m_minerva_release << endl;
    cout << "m_analysis_name   = " << m_analysis_name << endl;
    cout << "m_analysis_tree   = " << m_analysis_tree << endl;
    cout << "m_is_mc           = " << (m_is_mc ? "Yes" : "No") << endl;
    cout << "m_check_meta_data = " << (m_check_meta_data ? "Yes" : "No") << endl;
    cout << "m_start           = " << m_start << endl;
    cout << "m_finish          = " << m_finish << endl;
    cout << "m_outfilename     = " << m_outfilename << endl;
    
    m_subpath = "grid/central_value/minerva/ana";
    m_basedir = m_root_indir + "/" + m_subpath + "/" + m_minerva_release + "/";

    cout << "m_basedir         = " << m_basedir << endl;

}

void MergeTool::Run(){
    
    InspectDir();

    if(m_outfilename.empty()){
        m_outfilename = Form("merged_runs%08d-%08d.root", m_start, m_finish);
    }

//    TFile * outfile = new TFile( (m_root_indir + m_outfilename).c_str(), "RECREATE");
//    outfile->cd();
    
    TChain * recon = new TChain(m_analysis_tree.c_str());
    TChain * truth = new TChain("Truth");
    
    int n_files = 0;
    int n_mergedfiles = 0;
    
//    TStopwatch ts;
    
    for(int run = m_start; run < m_finish + 1; run++){
        
        string run_s = Form("%.8d", run);//This was .8d but we have already found this bit.
        string run_spars[3];
        for (int i = 0; i < 3; i++) run_spars[i] = run_s.substr( 2*(i + 1), 2);
        
        string flist = Form("%s%s/%s/%s/%s_*%s_*_%s*.root", m_basedir.c_str(),
                            run_spars[0].c_str(), run_spars[1].c_str(), run_spars[2].c_str(),
                            m_is_mc ? "SIM" : "MV", run_s.c_str(), m_analysis_name.c_str());
        
//        cout << "flist = " << flist << endl;
        
        glob_t g;
        glob(flist.c_str(), 0, 0, &g);
        
        n_files += g.gl_pathc;
        cout << " Run " << run << " : Adding " << g.gl_pathc << " files." << endl;
        
        for (int i = 0; i < (int)g.gl_pathc; i++){
            const char* filename=g.gl_pathv[i];
    
            if(GoodFile(filename) && GoodMeta(filename)){
//                outfile->cd();
                recon->Add(filename);
                if(m_is_mc) truth->Add(filename);
                n_mergedfiles++;
            }
            else cout << "Skipping bad file: " << filename << endl;
        }
        globfree(&g);
    }
    
    TFile * outfile = new TFile( (m_root_indir + m_outfilename).c_str(), "RECREATE");
    outfile->cd();
    
    cout << "Merging " << n_mergedfiles << "/" << n_files << " (" << (double)(100*n_mergedfiles/n_files) << "%) files." << endl;
    cout << "Producing recon tree: " << m_analysis_tree << "." << endl;
//    outfile->cd(); // Just in case the surrounding lines get separated
//    recon->Merge(outfile, 32000, "keep SortBasketsByBranch");

    TTree * recon_clone = (TTree*)recon->CloneTree(0);
    Int_t recon_entries = recon->GetEntries();
    
    for(Int_t evt = 0; evt < recon_entries; evt++){
        recon->GetEntry(evt);
        recon_clone->Fill();
    }

    recon_clone->Write();
    
//    
//    
//    
//    if(m_is_mc){
//        cout << "Producing truth tree: Truth." << endl;
//        outfile->cd();
////        truth->Merge(outfile, 32000, "keep SortBasketsByBranch");
//        TTree * truth_copy = truth->CopyTree("");
//        truth_copy->Write();
//    }
    
    cout << "Producing Meta tree." << endl;
    double sumPOTUsed  = getTChainPOT(recon, "POT_Used");
    double sumPOTTotal = getTChainPOT(recon, "POT_Total");
    cout << "POT Breakdown: Total = " << sumPOTTotal << " Used = " << sumPOTUsed << endl;
    outfile->cd();
    TTree * meta = new TTree("Meta", "");
    meta->Branch("POT_Used", &sumPOTUsed);
    meta->Branch("POT_Total", &sumPOTTotal);
    meta->Fill();
    meta->Write();
    
    outfile->Close();
    delete outfile;

    
    
}

bool MergeTool::GoodFile(const char* filename){
    TFile f(filename);
    if(f.IsZombie()) return false;
    TTree* meta=(TTree*)f.Get("Meta");
    if(!meta) return false;
    if(!meta->GetBranch("POT_Total")) return false;
    if(!meta->GetBranch("POT_Used")) return false;
    if(m_is_mc && !f.Get("Truth")) return false;//Check this.
    delete meta;//Added 210117
    return true;
}

bool MergeTool::GoodMeta(const char* filename){
    if(!m_check_meta_data) return true;
    TFile f(filename);
    if(f.IsZombie()) return false;
    TTree* meta=(TTree*)f.Get("Meta");
    if(!meta) return false;
    if(meta->GetEntriesFast() == 0) return false;
    if(!meta->GetBranch("POT_Used")) return false;
    delete meta;//Added 210117
    return true;
}

double MergeTool::getTChainPOT(TChain * ch, const char* branch){
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
    }
    return sumPOTUsed;
}

void MergeTool::InspectDir(){
    
//    std::string subpath = "grid/central_value/minerva/ana";
//    std::string basedir = m_root_indir + "/" + subpath + "/" + m_minerva_release + "/";
//    
//    cout << "basedir = " << basedir << endl;
    
//    cout << "m_start == " << m_start << "  ||  m_finish == " << m_finish << endl;
    
    //    if(m_start == -999 || m_finish == -999){
    
    //find start directory:
    
    cout << "Inspecting direcrtory structure..." << endl;
    
    for(int a = 0; a < 100; a++){
        
        std::string sub1 = Form("%.2d",a);
        
        std::string tmp = m_basedir + sub1;
        
        //            cout << "Checking if " << tmp << " exists" << endl;
        
        glob_t g;
        glob(tmp.c_str(), 0, 0, &g);
        
        int npaths = g.gl_pathc;
        
        //            cout << "g.gl_pathc = " << g.gl_pathc << endl;
        
        if(npaths == 1){
            m_basedir += sub1;
            m_basedir += "/";
//            m_00 = a;
            //                cout << "Found path: " << basedir << endl;
            break;
        }
    }
    
    int start_base = -999;
    std::string start_base_s;
    
    for(int a = 0; a < 100; a++){
        
        std::string sub1 = Form("%.2d",a);
        
        std::string tmp = m_basedir + sub1;
        
        //            cout << "Checking if " << tmp << " exists" << endl;
        
        glob_t g;
        glob(tmp.c_str(), 0, 0, &g);
        
        int npaths = g.gl_pathc;
        
        //            cout << "g.gl_pathc = " << g.gl_pathc << endl;
        
        if(npaths == 1){
            start_base = a;
            start_base_s = sub1;
            //                cout << "Found path: " << tmp << endl;
            break;
        }
    }
    
    start_base *= 1e4;
    start_base_s += "/";
    
    for(int a = 0; a < 100; a++){
        
        std::string sub1 = Form("%.2d",a);
        
        std::string tmp = m_basedir + start_base_s + sub1;
        
        //            cout << "Checking if " << tmp << " exists" << endl;
        
        glob_t g;
        glob(tmp.c_str(), 0, 0, &g);
        
        int npaths = g.gl_pathc;
        
        //            cout << "g.gl_pathc = " << g.gl_pathc << endl;
        
        if(npaths == 1){
            a *= 1e2;
            start_base += a;
            start_base_s += sub1;
            start_base_s += "/";
            //                cout << "Found path: " << tmp << endl;
            break;
        }
    }
    
    //        cout << "start_base   = " << start_base << endl;
    //        cout << "start_base_s = " << start_base_s << endl;
    
    for(int a = 0; a < 100; a++){
        
        std::string sub1 = Form("%.2d",a);
        
        std::string tmp = m_basedir + start_base_s + sub1;
        
        //            cout << "Checking if " << tmp << " exists" << endl;
        
        glob_t g;
        glob(tmp.c_str(), 0, 0, &g);
        
        int npaths = g.gl_pathc;
        
        //            cout << "g.gl_pathc = " << g.gl_pathc << endl;
        
        if(npaths == 1){
            a *= 1e2;
            start_base += a;
            start_base_s += sub1;
            start_base_s += "/";
            //                cout << "Found path: " << tmp << endl;
            break;
        }
    }
    
    //        cout << "start_base   = " << start_base << endl;
    //        cout << "start_base_s = " << start_base_s << endl;
    
    int end_base = -999;
    std::string end_base_s;
    
    for(int a = 99; a > -1; a--){
        
        std::string sub1 = Form("%.2d",a);
        
        std::string tmp = m_basedir + sub1;
        
        //            cout << "Checking if " << tmp << " exists" << endl;
        
        glob_t g;
        glob(tmp.c_str(), 0, 0, &g);
        
        int npaths = g.gl_pathc;
        
        //            cout << "g.gl_pathc = " << g.gl_pathc << endl;
        
        if(npaths == 1){
            end_base = a;
            end_base_s = sub1;
            //                cout << "Found path: " << tmp << endl;
            break;
        }
    }
    
    end_base *= 1e4;
    end_base_s += "/";
    
    for(int a = 99; a > -1; a--){
        
        std::string sub1 = Form("%.2d",a);
        
        std::string tmp = m_basedir + end_base_s + sub1;
        
        //            cout << "Checking if " << tmp << " exists" << endl;
        
        glob_t g;
        glob(tmp.c_str(), 0, 0, &g);
        
        int npaths = g.gl_pathc;
        
        //            cout << "g.gl_pathc = " << g.gl_pathc << endl;
        
        if(npaths == 1){
            a *= 1e2;
            end_base += a;
            end_base_s += sub1;
            end_base_s += "/";
            //                cout << "Found path: " << tmp << endl;
            break;
        }
    }
    
    //        cout << "end_base   = " << end_base << endl;
    //        cout << "end_base_s = " << end_base_s << endl;
    
    for(int a = 99; a > -1; a--){
        
        std::string sub1 = Form("%.2d",a);
        
        std::string tmp = m_basedir + end_base_s + sub1;
        
        //            cout << "Checking if " << tmp << " exists" << endl;
        
        glob_t g;
        glob(tmp.c_str(), 0, 0, &g);
        
        int npaths = g.gl_pathc;
        
        //            cout << "g.gl_pathc = " << g.gl_pathc << endl;
        
        if(npaths == 1){
            end_base += a;
            end_base_s += sub1;
            end_base_s += "/";
            //                cout << "Found path: " << tmp << endl;
            break;
        }
    }
    //
//    cout << "start_base   = " << start_base << endl;
//    cout << "start_base_s = " << start_base_s << endl;
//    cout << "end_base   = " << end_base << endl;
//    cout << "end_base_s = " << end_base_s << endl;
    if(m_start == -999)  m_start = start_base;
    if(m_finish == -999) m_finish = end_base;
//    cout << "m_start   = " << m_start << endl;
//    cout << "m_finish = " << m_finish << endl;
    
    if(m_start == -999 || m_finish == -999){
        cout << "Error could not determine range... Input manually (-n Start-Finish)" << endl;
        exit(0);
    }
}

int main(int argc, char *argv[])
{
//    string username( getenv("USER") );
//    if(username.empty()){
//        std::cout << "[WARNING]: Environment variable \"USER\" not found." << std::endl;
//    }
    
    string analysis_tree( getenv("ANATREENAME") );
    if(analysis_tree.empty()){
        std::cout << "[WARNING]: Environment variable \"ANATREE\" not set. To set see requirements file." << std::endl;
    }
    
    string analysis_name( getenv("ANATOOLNAME") );
    if(analysis_name.empty()){
        std::cout << "[WARNING]: Environment variable \"ANATOOLNAME\" not set. To set see requirements file." << std::endl;
    }
    
    string minerva_release( getenv("MINERVA_RELEASE") );
    if(minerva_release.empty()){
        std::cerr << "[WARNING]: environment variable \"MINERVA_RELEASE\" not set. "
        "Cannot determine source tree location." << std::endl;
        return 1;
    }
    
    string root_indir;
    string run_s;
    string outfilename;
    bool is_mc = true;
    bool check_meta_data = true;
    
    char cc;
    while ((cc = getopt(argc, argv, "v:i:o:t:a:d::p::n:h")) != -1) {
        switch (cc){
            case 'v':   minerva_release = string(optarg);   break;
            case 'i':   root_indir      = string(optarg);   break;
            case 'o':   outfilename     = string(optarg);   break;
            case 't':   analysis_tree   = string(optarg);   break;
            case 'a':   analysis_name   = string(optarg);   break;
            case 'd':   is_mc           = false;            break;
            case 'p':   check_meta_data = false;            break;
            case 'n':   run_s           = string(optarg);   break;
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
                << "|******************************************************************************************************************|" << endl;
//                << " " << endl
//                << " " << endl
//                << "|*********************** Run Options ****************************|" << endl
//                << "| Default is to merge files in the root directory of your analy- |" << endl
//                << "| sis. The number of runs also needs to be defined and can be    |" << endl
//                << "| either a single run or run range (see below).                  |" << endl
//                << "|                                                                |" << endl
//                << "| Options:                                                       |" << endl
//                << "|   -i   :  Set input file dir if \"-per\" is defined only the   |" << endl
//                << "|        :  root directory of your analysis in your persistent   |" << endl
//                << "|        :  directory is required.                               |" << endl
//                << "|        :                                                       |" << endl
//                << "|   -n   :  Run or run range: start-end e.g 13200-13250          |" << endl
//                << "|        :  will run over 50 runs from 13200 to 13250.           |" << endl
//                << "|        :                                                       |" << endl
//                << "|   -o   :  Set output file directory.                           |" << endl
//                << "|        :                                                       |" << endl
//                << "|  -per  :  Assume in/out files are in the users persistent      |" << endl
//                << "|        :  directory. In such cases \"-i\" becomes the root       |" << endl
//                << "|        :  directory name. (e.g. -per -i CC1P1Pi_PL13C_290916   |" << endl
//                << "|        :  to merge files in /pnfs/minerva/persistent/users/    |" << endl
//                << "|        :  dcoplowe/CC1P1Pi_PL13C_290916/                       |" << endl
//                << "|        :                                                       |" << endl
//                << "|   -a   :  Analysis tool name (Your analysis tool used to make  |" << endl
//                << "|        :  the root files you want to merge)                    |" << endl
//                << "         :  Default is currently " << analtool << endl
//                << "|        :                                                       |" << endl
//                << "|   -t   :  Set name of analysis tree.                           |" << endl
//                << "         :  Default is currently " << anatree << endl
//                << "|        :  If not set, this can be set in the PlotUtils requir- |" << endl
//                << "|        :  ements files.                                        |" << endl
//                << "|        :                                                       |" << endl
//                << "|   -s   :  Set save name.                                       |" << endl
//                << "|        :                                                       |" << endl
//                << "|   -v   :  Set version of input anatuple files (vXrYpZ).        |" << endl
//                << "|        :  If not specified, use current release.               |" << endl
//                << "|        :                                                       |" << endl
//                << "| -m     :  Option 1 (-m 1): Combine each run into single root   |" << endl
//                << "|        :  file.                                                |" << endl
//                << "|        :  Option 2 (-m 2): Combine output of (-m 1) into a     |" << endl
//                << "|        :  single root file.                                    |" << endl
//                << "|        :                                                       |" << endl
//                << "| -check :  Merge without checking POT in Meta tree is good      |" << endl
//                << "|        :  (exists).                                            |" << endl
//                << "|        :                                                       |" << endl
//                << "| -data  :  Run on a data file (merge without Truth tree). Def-  |" << endl
//                << "|        :  ault is to assume you are running on MC.             |" << endl
//                << "|        :                                                       |" << endl
//                << "| -full  :  Add full directory path. This enables you to merge   |" << endl
//                << "|        :  special runs where the sub directories are not       |" << endl
//                << "|        :  grid/central_value/minerva/ana. When \"-full\" is      |" << endl
//                << "|        :  defined simply set the full path up to and including |" << endl
//                << "|        :  ana using \"-i\".                                      |" << endl
//                << "|        :                                                       |" << endl
//                << "| -help  :  Print this.                                          |" << endl
//                << "|        :                                                       |" << endl
//                << "|****************************************************************|" << endl;
                return 1; break;
            default: return 1;
        }
    }
    
    if(root_indir.empty()){
        cout << "|============ Minimum Requirements to run ============|" << endl;
        cout << "|                                                     |" << endl;
        cout << "|    -i     Set input file dir name                   |" << endl;
        cout << "|           This will merge all analysis root files   |" << endl;
        cout << "|           in root directory defined by -i.          |" << endl;
        cout << "|                                                     |" << endl;
//        cout << "|    -n     Number of runs to merge                   |" << endl;
        cout << "|    -help  For more options.                         |" << endl;
        cout << "|_____________________________________________________|" << endl;
        return 0;
    }
    
    int first_run = -999;
    int last_run =  -999;
    
    if(!run_s.empty()){
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
    }
    
    cout << "|---------------------------------- Inputs ----------------------------------" << endl;
    cout << "|               Input  (-i): " << root_indir << endl;
    if(!outfilename.empty()) cout << "|               Output (-o): " << outfilename << endl;
    cout << "|   Analysis Tree Name (-t): " << analysis_tree << endl;
    cout << "|   Analysis Tool Name (-a): " << analysis_name<< endl;
    cout << "|      MINERvA Release (-t): " << minerva_release << endl;
    if(!run_s.empty()) cout << "|             N Run(s) (-n): " << run_s << endl;
    cout << "|  Merge Data/MC files (-d): " << (is_mc ? "MC" : "DATA") << endl;
    cout << "| Check if POT is good (-p): " << (check_meta_data ? "Yes" : "No") << endl;
    cout << "|--------------------------------- Running ----------------------------------" << endl;
    
    MergeTool * merge = new MergeTool(root_indir, minerva_release, analysis_name, analysis_tree, is_mc, check_meta_data);
    merge->SetRange(first_run, last_run);
    if(!outfilename.empty()) merge->SetOutFileName(outfilename);
    merge->Run();
    delete merge;
    cout << "|-------------------------- Finished merging files --------------------------" << endl;
    return 0;
}


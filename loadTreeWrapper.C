void add_inc_path(TString path)
{
  TString oldpath=gSystem->GetIncludePath();
  oldpath+=" -I";
  oldpath+=path;
  gSystem->SetIncludePath(oldpath);
}

void loadTreeWrapper()
{
  if(TString(gSystem->HostName()).Contains("rochester")){
    add_inc_path("/home/prodrigues/minerva//mec/ana");
    add_inc_path("/home/prodrigues/minerva//mec/ana/util/tree");
  }
  else{
    add_inc_path("/minerva/app/users/rodriges/cmtuser/mec/ana");
    add_inc_path("/minerva/app/users/rodriges/cmtuser/mec/ana/util/tree");
  }
  gSystem->CompileMacro("TreeWrapper.cxx", "k");
  gSystem->CompileMacro("ChainWrapper.cxx", "k");
}

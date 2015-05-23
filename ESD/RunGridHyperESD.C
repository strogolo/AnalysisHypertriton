class AliAnalysisGrid;
class AliAnalysisAlien;

fstream lFiles;
TString startitme, optStr;


void GetRunSample(TString who = "all", Int_t &nskip, Int_t &nrun){
  if(who.Contains("stefano",TString::kIgnoreCase)){
    nskip = 0;
    nrun = 2;
  }
  else if(who.Contains("stefania",TString::kIgnoreCase)){
    nskip = 20;
    nrun = 20;
  }
  else if(who.Contains("elena",TString::kIgnoreCase)){
    nskip = 40;
    nrun = 20;
  }
  else{
    nskip = 0;
    nrun = 1;
  }
  return;
}

void RunGridHyperESD(TString runmode="test", Bool_t isMC=kFALSE, TString fname="test"){

  TString who = runmode;
  if (who.Contains("stefano",TString::kIgnoreCase) || who.Contains("stefania",TString::kIgnoreCase) || who.Contains("elena",TString::kIgnoreCase) || who.Contains("all",TString::kIgnoreCase)){
    runmode = "full";
  }
  
  ///////////////////////////////////////////////////////Log
  TString logStr= Form("***Runmode(%s) MC(%i) AnalName(%s)***",runmode.Data(),isMC,fname.Data());
  optStr=Form("OPT: R_%s M_%i",runmode.Data(),isMC);
  gSystem->Exec("date >> logRunAnalysis.txt");
  startitme=gSystem->GetFromPipe("date");
  lFiles.open("logRunAnalysis.txt",ios::in | ios::out | ios::ate);
  if (lFiles.is_open()){
    lFiles << logStr<<endl;
    lFiles<<"DATE: "<<startitme<<endl;
    lFiles<<optStr<<endl;    
  }
  else cout << "Unable to open file";
  //////////////////////////////////////////////////////
  
  //Include path
  //  gSystem->SetIncludePath("-I$ROOTSYS/include -I${ALICE_ROOT}/include -I${ALICE_ROOT}/ANALYSIS/macros/ -I./");
  //cout<<"Include path: "<<gSystem->GetIncludePath()<<endl;
  gROOT->ProcessLine(".include $ALICE_ROOT/include");

  gSystem->AddIncludePath("-I$ALICE_ROOT/include");
  gSystem->AddIncludePath("-I$ALICE_PHYSICS/include");

  
  // Load common libraries
  gSystem->Load("libCore.so");
  gSystem->Load("libTree.so");
  gSystem->Load("libGeom.so");
  gSystem->Load("libPhysics.so");
  gSystem->Load("libVMC.so");
  gSystem->Load("libMinuit.so");
  gSystem->Load("libSTEERBase.so");
  gSystem->Load("libESD.so");
  gSystem->Load("libAOD.so");
  gSystem->Load("libANALYSIS.so");
  gSystem->Load("libOADB.so");
  gSystem->Load("libANALYSISalice.so");   
  gSystem->Load("libCORRFW.so");
  gSystem->Load("libPWGHFbase.so");
  gSystem->Load("libPWGHFvertexingHF.so");
  gSystem->Load("libPWGLFSTRANGENESS.so");

  //----Create the analysis manager----
  
  AliAnalysisManager *mgr = new AliAnalysisManager("testAnalysis");
  
  //Define the use of V0 in the string
  //if(useV0) fname += "_V0Mult";
  //else fname += "_RefMult";
  
  // Create and configure the alien handler plugin
  AliAnalysisGrid *alienHandler = CreateAlienHandler(runmode,isMC,fname,who);  
  if (!alienHandler) return;	
  mgr->SetGridHandler(alienHandler);
  lFiles.close();

  AliESDInputHandler* esdH = new AliESDInputHandler();
  mgr->SetInputEventHandler(esdH);


  // Apply the event selection
  gROOT->LoadMacro("$ALICE_PHYSICS/OADB/macros/AddTaskPhysicsSelection.C");
  AliPhysicsSelectionTask *physSelTask = AddTaskPhysicsSelection(isMC,kTRUE);
  
  if(isMC){
    AliMCEventHandler *mch = new AliMCEventHandler();
    mgr->SetMCtruthEventHandler(mch);
    physSelTask->GetPhysicsSelection()->SetAnalyzeMC(); 
  }

  //-----------------------------------------------------------------------
  // Other tasks
  // Centrality selection
  gROOT->LoadMacro("$ALICE_PHYSICS/OADB/macros/AddTaskCentrality.C");
  AliCentralitySelectionTask *taskCentr = AddTaskCentrality();
  gROOT->LoadMacro("$ALICE_ROOT/ANALYSIS/macros/AddTaskPIDResponse.C");
   AliAnalysisTaskSE *setupTask = AddTaskPIDResponse(isMC,kTRUE);
  
   //gROOT->LoadMacro("$ALICE_ROOT/ANALYSIS/macros/AddTaskPIDqa.C");
   //AliAnalysisTaskPIDqa *pidQA = AddTaskPIDqa();

   //gROOT->LoadMacro("./AliAnalysisTaskHypertriton3.cxx++g");
   //gROOT->LoadMacro("./AddTaskHypertriton3.C");
   gROOT->LoadMacro("$ALICE_PHYSICS/PWGLF/STRANGENESS/Hypernuclei/Hyp3Body/AddTaskHypertriton3.C");

   AliAnalysisTaskHypertriton3 *taskHyper = AddTaskHypertriton3(kFALSE,kFALSE);//isMC= choose if MC or Data; FillTree= choose if filling the TTree or not
   //gROOT->LoadMacro("$ALICE_PHYSICS/PWGLF/STRANGENESS/Hypernuclei/AddTask_Helium3Pi.C");
   //AliAnalysisTaskHelium3Pi *tskHypertr = AddTask_Helium3Pi("hypertriton_Ramona");

   //gROOT->LoadMacro("$ALICE_PHYSICS/PWGLF/STRANGENESS/Hypernuclei/AddTask_doenigus_HdibaryonLPpi.C");
   //AliAnalysisTaskHdibaryonLPpi *tskHyper = AddTask_doenigus_HdibaryonLPpi();
   
  mgr->InitAnalysis();
  mgr->PrintStatus();
  // Start analysis in grid.
  mgr->StartAnalysis("grid");
};


AliAnalysisGrid* CreateAlienHandler(TString runmode, Bool_t isMC, TString fname, TString &who) {
  // Check if user has a valid token, otherwise make one. This has limitations
  // One can always follow the standard procedure of calling alien-token-init then
  //   source /tmp/gclient_env_$UID in the current shell.
  //if (!AliAnalysisGrid::CreateToken()) return NULL;
  
  AliAnalysisAlien *plugin = new AliAnalysisAlien();
  // Set the run mode (can be "full", "test", "offline", "submit" or "terminate")
  plugin->SetRunMode(runmode.Data());
  plugin->SetNtestFiles(2);

  // Set versions of used packages

  plugin->SetAPIVersion("V1.1x");
  plugin->SetROOTVersion("v5-34-08-7");
  plugin->SetAliROOTVersion("v5-06-02");
  plugin->SetAliPhysicsVersion("vAN-20150131");

  if(isMC){
    plugin->SetGridDataDir("/alice/sim/2014/LHC14a6/");
    plugin->SetDataPattern("*/*AliESDs.root");
    plugin->SetRunPrefix("");
    
    plugin->SetGridWorkingDir(Form("%s_MC",fname.Data()));
    plugin->SetExecutable(Form("%s_MC.sh",fname.Data()));
    plugin->SetJDLName("taskHypertriton_LHC12d3_MC.jdl");
    plugin->SetAnalysisMacro("taskHypertriton_LHC12d3_MC.C");
    plugin->SetSplitMaxInputFileNumber(40);
  }else{
    plugin->SetGridDataDir("/alice/data/2011/LHC11h_2/");
    plugin->SetDataPattern("*ESDs/pass2/*AliESDs.root");
    plugin->SetRunPrefix("000"); 
    plugin->SetGridWorkingDir(Form("%s_DATA",fname.Data()));
//     plugin->SetGridWorkingDir(Form("Spectra/DATA/%s_DATA",fname.Data()));
    plugin->SetExecutable(Form("%s_DATA.sh",fname.Data()));
    plugin->SetJDLName("taskHypertriton_LHC11h2_DATA.jdl");
    plugin->SetAnalysisMacro("taskHypertriton_LHC11h2_DATA.C");
    plugin->SetSplitMaxInputFileNumber(100);     
  }
  
  //Int_t* runs=0x0;
  Int_t nruns = 108;
  
  Int_t ntorun = 0;
  Int_t ntoskip = 0;
  Int_t nrunssplit = 1;
  
  GetRunSample(who,ntoskip,ntorun);
  
  if(isMC){
    nrunssplit=10;
    //ntorun=1;
    //ntoskip=0;
  }
  else{
    nrunssplit=1;
    //ntorun=22;
    //ntoskip=0;
  }
    
  //runs=new Int_t[nruns];
  // MC - LHC12d3 runlist - 108 entries
  /* Int_t runs[108]={168107, 170387, 167915, 167920, 167985, 167987, 167988, 168069, 168076, 168105, 
		   168108, 168115, 168310, 168311, 168322, 168325, 168341, 168342, 168361, 168362,
		   168458, 168460, 168464, 168467, 168511, 168512, 168514, 168777, 168826, 168988,
		   168992, 169035, 169040, 169044, 169045, 169091, 169094, 169099, 169138, 169144,
		   169145, 169148, 169156, 169160, 169167, 169238, 169411, 169415, 169417, 169418,
		   169419, 169420, 169475, 169498, 169504, 169506, 169512, 169515, 169550, 169553,
		   169554, 169555, 169557, 169586, 169587, 169588, 169590, 169591, 169835, 169837,
		   169838, 169846, 169855, 169858, 169859, 169923, 169965, 170027, 170040, 170081,
		   170083, 170084, 170085, 170088, 170089, 170091, 170155, 170159, 170163, 170193,
		   170203, 170204, 170207, 170228, 170230, 170268, 170269, 170270, 170306, 170308,
		   170309, 170311, 170312, 170313, 170315, 170388, 170572, 170593};  //108
  */

  // Data - LHC11h runlist - 172 entries
  Int_t runs[172]={167693, 167706, 167711, 167713, 167806, 167807, 167808, 167813, 167814, 167818, //10
		   167902, 167903, 167915, 167920, 167921, 167985, 167986, 167987, 167988, 168066, //20
		   168068, 168069, 168076, 168103, 168104, 168105, 168107, 168108, 168115, 168171, //30
		   168172, 168173, 168175, 168181, 168203, 168204, 168205, 168206, 168207, 168208, //40
		   168212, 168213, 168310, 168311, 168318, 168322, 168325, 168341, 168342, 168356, //50
		   168361, 168362, 168458, 168460, 168461, 168464, 168467, 168511, 168512, 168514, //60
		   168777, 168826, 168984, 168988, 168992, 169035, 169040, 169044, 169045, 169091, //70
		   169094, 169099, 169138, 169143, 169144, 169145, 169148, 169156, 169160, 169167, //80
		   169236, 169238, 169411, 169415, 169417, 169418, 169419, 169420, 169475, 169498, //90
		   169504, 169506, 169512, 169515, 169550, 169553, 169554, 169555, 169557, 169584, //100
		   169586, 169587, 169588, 169590, 169591, 169628, 169683, 169835, 169837, 169838, //110
		   169846, 169855, 169858, 169859, 169918, 169919, 169920, 169922, 169923, 169924, //120
		   169956, 169961, 169965, 169969, 169975, 169981, 170027, 170036, 170038, 170040, //130
		   170081, 170083, 170084, 170085, 170088, 170089, 170091, 170152, 170155, 170159, //140
		   170162, 170163, 170193, 170195, 170203, 170204, 170205, 170207, 170208, 170228, //150
		   170230, 170264, 170267, 170268, 170269, 170270, 170306, 170308, 170309, 170311, //160
		   170312, 170313, 170315, 170387, 170388, 170389, 170390, 170546, 170552, 170556, //170
		   170572, 170593}; //172
  
  if(1){//Intended for testing
    if(isMC) runs[0]=170387; // 169586MC - Less populated Run for grid test
    else runs[0]=170387;//DATA
    ntorun=1;
    ntoskip=0;
  }
  
  for(int i=ntoskip; i<ntorun+ntoskip && i<nruns; i++){
    if(i>=nruns){
      ::Fatal(" runs selection : ","Wrong definition of Start-Stop");
      break;
    }    
    cout<<"Adding run number "<<runs[i]<<" element "<<i+1<<" of "<<nruns<<endl;
    plugin->AddRunNumber(runs[i]);
    /////////////////////////////////////Log
    lFiles<<Form("**(%i/%i) Run #%i#",i+1,nruns,runs[i])<<endl;
    ////////////////////////////////////////
  }
  
  plugin->SetOutputToRunNo();
  plugin->SetNrunsPerMaster(nrunssplit);
  
  plugin->SetGridOutputDir("output"); // In this case will be $HOME/work/output

  cout << "-->>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> 1" << endl;

  plugin->SetAnalysisSource("AliAnalysisTaskHypertriton3.cxx");

  cout << "-->>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> 2" << endl;

  //plugin->SetAdditionalLibs("libSTEERBase.so libESD.so AliAnalysisTaskHypertriton3.h AliAnalysisTaskHypertriton3.cxx libGui.so libProof.so libMinuit.so libRAWDatabase.so libRAWDatarec.so libCDB.so libSTEER.so libITSbase.so libITSrec.so libANALYSIS.so libANALYSISalice.so libPWGHFbase.so libPWGHFvertexingHF.so libOADB.so");
  plugin->SetAdditionalLibs("AliAnalysisTaskHypertriton3.h AliAnalysisTaskHypertriton3.cxx libPWGflowBase.so libPWGflowTasks.so libPWGLFSTRANGENESS.so"); // 
  plugin->AddIncludePath("-I. -I$ROOTSYS/include -I$ALICE_ROOT -I$ALICE_PHYSICS/include -g");

  cout << "-->>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> 3" << endl;

  
  // plugin->AddIncludePath("-I. -I$ROOTSYS/include -I$ALICE_ROOT -I$ALICE_ROOT/include -I$ALICE_ROOT/ITS -I$ALICE_ROOT/TPC -I$ALICE_ROOT/CONTAINERS  -I$ALICE_ROOT/STEER/STEER -I$ALICE_ROOT/STEER/STEERBase -I$ALICE_ROOT/STEER/ESD -I$ALICE_ROOT/STEER/CDB -I$ALICE_ROOT/TRD -I$ALICE_ROOT/macros -I$ALICE_ROOT/ANALYSIS -g");
  //plugin->SetAdditionalLibs("AliAnalysisTaskHypertriton3.h AliAnalysisTaskHypertriton3.cxx libGui.so libProof.so libMinuit.so libRAWDatabase.so libRAWDatarec.so libCDB.so libSTEER.so libITSbase.so libITSrec.so libANALYSIS.so libANALYSISalice.so");
  
  // Declare the output file names separated by blancs.
  // (can be like: file.root or file.root@ALICE::Niham::File)
  plugin->SetDefaultOutputs();
  //  plugin->SetOutputFiles("AnalysisResults.root");
  // Optionally define the files to be archived.
  //   plugin->SetOutputArchive("log_archive.zip:stdout,stderr@ALICE::NIHAM::File root_archive.zip:*.root@ALICE::NIHAM::File");
  //plugin->SetOutputArchive("log_archive.zip:stdout,stderr");
  // Optionally set number of failed jobs that will trigger killing waiting sub-jobs.
  plugin->SetMergeViaJDL(kFALSE);
  //  plugin->SetMaxInitFailed(5);
  // Optionally resubmit threshold.
  plugin->SetMasterResubmitThreshold(90);
  // Optionally set time to live (default 30000 sec)
  plugin->SetTTL(20000);
  // Optionally set input format (default xml-single)
  plugin->SetInputFormat("xml-single");
  // Optionally modify the name of the generated JDL (default analysis.jdl)
  // Optionally modify job price (default 1)
  plugin->SetPrice(1);      
  // Optionally modify split mode (default 'se')    
  plugin->SetSplitMode("se");
  cout << "-->>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> 4" << endl;
  
  //delete runs;
  return plugin;

  
}

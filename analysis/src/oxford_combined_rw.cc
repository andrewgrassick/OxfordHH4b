// oxford_res_fr.cc

#include "YODA/Histo1D.h"
#include "YODA/Histo2D.h"

#include "oxford_combined_rw.h"
#include "utils.h"
#include "settings.h"

#include "fastjet/Selector.hh"
#include "fastjet/ClusterSequence.hh"
#include "fastjet/tools/MassDropTagger.hh"
#include "fastjet/contrib/VariableRPlugin.hh"

#include <algorithm>

using namespace fastjet::contrib;

// pT cut for constituent b-quarks (GeV)
const double pt_btagging = 15;

// Boosted jet radius
const double BoostJetR=1.0;
const double jetR = 0.3; // Boosted subjet radius for ghost-association

// Resolved jet radius
const double ResJetR=0.4;

// beta-exponent for LST energy correlations
const double LST_beta = 2;

// Switch to run with Variable-R jets
const bool doVR = true;
 
// Set VR parameters
static double const jet_Rmax    =1.0;
static double const jet_Rmin    =0.2;
static double const jet_Rho     =500.;

//Instantiate VR plugin
static const VariableRPlugin lvjet_pluginAKT(jet_Rho, jet_Rmin, jet_Rmax, VariableRPlugin::AKTLIKE);
static const fastjet::JetDefinition VR_AKT(&lvjet_pluginAKT);


OxfordCombinedRWAnalysis::OxfordCombinedRWAnalysis(std::string const& sampleName):
Analysis("oxford_combined_rw", sampleName)
{
  // ********************* Histogram settings******************

  const double DeltaRmin = 0;
  const double DeltaRmax = 5;

  const double DeltaPhimin = -3.2;
  const double DeltaPhimax = 3.2;

  const double DeltaEtamin = -2.5;
  const double DeltaEtamax = 2.5;
  
  const double m_min = 0.;
  const double m_max = 180.; 
 
  const double pt_min = 0.;
  const double pt_max = 900.;
  
  const double m_HH_min = 0.;
  const double m_HH_max = 600.; 
  
  const double pt_HH_min = 0.;
  const double pt_HH_max = 300.;
  
  const int nbins = 30;
  
  // ********************* Histogram definitions ******************

  const int nAnalysis = 3;  const int nCuts = 3;
  const std::string aString[nAnalysis] = {"_res", "_inter", "_boost"};
  const std::string cString[nCuts] = {"_C0", "_C1", "_C2"};

  for (int i=0; i< nAnalysis; i++)
  {
    BookHistogram(new YODA::Histo1D( nCuts+1, 0, nCuts+1 ), "CF" + aString[i]);
    BookHistogram(new YODA::Histo1D( nCuts+1, 0, nCuts+1 ), "CFN" + aString[i]);

    for (int j=0; j< nCuts; j++)
    {
      const std::string suffix = aString[i] + cString[j];

      BookHistogram(new YODA::Histo1D(nbins, pt_min, pt_max), "pt_H0" + suffix);
      BookHistogram(new YODA::Histo1D(nbins, pt_min, pt_max), "pt_H1" + suffix);

      BookHistogram(new YODA::Histo1D(nbins, m_min, m_max), "m_H0" + suffix);
      BookHistogram(new YODA::Histo1D(nbins, m_min, m_max), "m_H1" + suffix);

      BookHistogram(new YODA::Histo1D(nbins, pt_HH_min, pt_HH_max), "pt_HH" + suffix);
      BookHistogram(new YODA::Histo1D(nbins, m_HH_min, m_HH_max), "m_HH" + suffix);
      BookHistogram(new YODA::Histo1D(nbins, DeltaRmin, DeltaRmax), "dR_HH" + suffix);

      BookHistogram(new YODA::Histo1D(nbins, DeltaPhimin, DeltaPhimax), "dPhi_HH" + suffix);
      BookHistogram(new YODA::Histo1D(nbins, DeltaEtamin, DeltaEtamax), "dEta_HH" + suffix);

      BookHistogram(new YODA::Histo2D(nbins, pt_min, pt_max, nbins, pt_min, pt_max), "ptHptH" + suffix);
      BookHistogram(new YODA::Histo2D(nbins, m_min, m_max, nbins, m_min, m_max), "mHmH" + suffix);

      BookHistogram(new YODA::Histo1D(nbins, 0, 200), "split12_fj1" + suffix);
      BookHistogram(new YODA::Histo1D(nbins, 0, 200), "split12_fj2" + suffix);
      BookHistogram(new YODA::Histo1D(nbins, 0, 200), "split12_fj" + suffix);  

      BookHistogram(new YODA::Histo1D(nbins, 0, 1), "tau21_fj1" + suffix); 
      BookHistogram(new YODA::Histo1D(nbins, 0, 1), "tau21_fj2" + suffix);
      BookHistogram(new YODA::Histo1D(nbins, 0, 1), "tau21_fj" + suffix);      

      BookHistogram(new YODA::Histo1D(nbins*2, 0, 1), "C2_fj1" + suffix);  
      BookHistogram(new YODA::Histo1D(nbins*2, 0, 1), "C2_fj2" + suffix);
      BookHistogram(new YODA::Histo1D(nbins*2, 0, 1), "C2_fj" + suffix);   

      BookHistogram(new YODA::Histo1D(nbins*2, 0, 1), "D2_fj1" + suffix);  
      BookHistogram(new YODA::Histo1D(nbins*2, 0, 1), "D2_fj2" + suffix);
      BookHistogram(new YODA::Histo1D(nbins*2, 0, 1), "D2_fj" + suffix);       
      
      // Additional histograms from unreweighted analysis
      BookHistogram(new YODA::Histo1D(10, 0, 10), "N_SmallRJets" + suffix);
      BookHistogram(new YODA::Histo1D(10, 0, 10), "N_SmallRJets_BJets" + suffix);
      BookHistogram(new YODA::Histo1D(10, 0, 10), "N_SmallRJets_BTagged" + suffix);
    }
  }
  
  BookHistogram(new YODA::Histo1D(10, 0, 10), "N_SmallRTrackJets_res_C0");
  BookHistogram(new YODA::Histo1D(10, 0, 10), "N_SmallRSelTrackJets_res_C0");
  
  BookHistogram(new YODA::Histo1D(10, 0, 10), "N_SmallRTrackJets_inter_C0");
  BookHistogram(new YODA::Histo1D(10, 0, 10), "N_SmallRSelTrackJets_inter_C0");
  
  BookHistogram(new YODA::Histo1D(10, 0, 10), "N_SmallRTrackJets_boost_C0");
  BookHistogram(new YODA::Histo1D(10, 0, 10), "N_SmallRSelTrackJets_boost_C0");

  
  // ********************* Ntuple definition **********************
  const std::string tupleSpec = "# signal source weight pt_H0 pt_H1 pt_HH m_H0 m_H1 m_HH dR_HH dPhi_HH dEta_HH";
  outputNTuple<<tupleSpec<<std::endl;

  const std::string root = "." + GetRoot() + GetSample() + "/";

  const std::string resDir = root+"resNTuple.dat";
  const std::string intDir = root+"intNTuple.dat";
  const std::string bstDir = root+"bstNTuple.dat";

  resNTuple.open(resDir.c_str());
  intNTuple.open(intDir.c_str());
  bstNTuple.open(bstDir.c_str());

  resNTuple << tupleSpec <<" pt_H0_sub0 pt_H0_sub1 pt_H1_sub0 pt_H1_sub1"<<std::endl;
  intNTuple << tupleSpec <<" split12_fj tau21_fj C2_fj D2_fj"<<std::endl;
  bstNTuple << tupleSpec <<" split12_fj1 split12_fj2 tau21_fj1 tau21_fj2 C2_fj1 C2_fj2 D2_fj1 D2_fj2"<<std::endl;


  // ********************* Cutflow histograms  **********************

  // Category overlap
  BookHistogram(new YODA::Histo1D( 7, 0, 7 ), "Categories_C1");
//   BookHistogram(new YODA::Histo1D( 7, 0, 7 ), "Categories_C2");

}

void OxfordCombinedRWAnalysis::Analyse(bool const& signal, double const& weightnorm, finalState const& fs)
{
  Analysis::Analyse(signal, weightnorm, fs);

  // Set initial weight
  const double event_weight = weightnorm;

    // ********************************* Simple event categorisation  ***************************************

  fastjet::JetDefinition akt_res(fastjet::antikt_algorithm, ResJetR);
  fastjet::ClusterSequence cs_akt_res(fs, akt_res);
  std::vector<fastjet::PseudoJet> smallRJets = sorted_by_pt( cs_akt_res.inclusive_jets()  ); // Get all the jets (no pt cut here)
  
  // Basic kinematic cuts
  std::vector<fastjet::PseudoJet> smallRJetsSel;
  for( size_t i = 0; i < smallRJets.size(); i++){
    
    if( smallRJets.at(i).pt() < 40. ) continue;
    if( fabs( smallRJets.at(i).eta() ) > 2.5 ) continue;
    
    smallRJetsSel.push_back( smallRJets.at(i) );
  }
  
  smallRJetsSel = sorted_by_pt( smallRJetsSel  ); // Resort
  std::vector<bool> isFakeSR_vec;         // Vector specifying if each jet is fake or not
  BTagging( smallRJetsSel, isFakeSR_vec );

  if (smallRJetsSel.size() != isFakeSR_vec.size())
    std::cerr << "Error: vector sizes do not match! "<<std::endl;

  // Form small-R b-jets vector
  std::vector<fastjet::PseudoJet> bJets;
  const int nBTaggedJets = std::min(smallRJetsSel.size(), (size_t)4);   // Both for now
  const int nJets = std::min(smallRJetsSel.size(), (size_t)4);          // Both for now
  for( int i = 0; i < nBTaggedJets; i++)
    bJets.push_back(smallRJetsSel[i]);
  
  int nBJets = bJets.size();
  
  //=======================================================
  // Small-R track jets from charged final state particles
  //=======================================================
//  finalState fsc;
//  //Select only charged fs particles
//  for(int i=0; i<(int)fs.size(); i++){
//      int userid = fs.at(i).user_index();
//      if(abs(userid)<6) fsc.push_back(fs.at(i));
//  }
  
  fastjet::JetDefinition jd_subjets(fastjet::antikt_algorithm, jetR);
  fastjet::ClusterSequence cs_subjets(fs, jd_subjets);
  std::vector<fastjet::PseudoJet> trackjets = sorted_by_pt( cs_subjets.inclusive_jets()  );
  
  // Basic kinematic cuts
  std::vector<fastjet::PseudoJet> trackjetsSel;
  for( size_t i = 0; i < trackjets.size(); i++){
    
    if( trackjets.at(i).pt() < 50 ) continue;
    if( fabs( trackjets.at(i).eta() ) > 2.5 ) continue;
    
    trackjetsSel.push_back( trackjets.at(i) );
  }
      
  //===============
  // Large-R jets
  //===============
  std::vector<fastjet::PseudoJet> largeRJets;

  fastjet::JetDefinition akt_boost(fastjet::antikt_algorithm, BoostJetR);
  fastjet::ClusterSequence cs_akt_fr(fs, akt_boost);
  fastjet::ClusterSequence cs_akt_vr(fs, VR_AKT);

  if( !doVR ){
        largeRJets = sorted_by_pt( cs_akt_fr.inclusive_jets()  ); // Get all the jets (no pt cut here)
  }
  else{
        largeRJets = sorted_by_pt( cs_akt_vr.inclusive_jets()  ); // Get all the jets (no pt cut here)
        //std::cout << "Running Variable-R jets " << std::endl;
  }

  std::vector<fastjet::PseudoJet> bbFatJets;
  
  // Basic kinematic cuts
  std::vector<fastjet::PseudoJet> largeRJetsSel;
  for( size_t i = 0; i < largeRJets.size(); i++){
 
    //-----------------------------------------------------
    // pt acceptance cut
    if( largeRJets.at(i).pt() < 200. ) continue;
    //-----------------------------------------------------
    // Pseudorapidity acceptance cut
    if( fabs( largeRJets.at(i).eta() ) > 2.0 ) continue;

    //-----------------------------------------------------
    // Check if jet is mass-drop tagged
    fastjet::JetDefinition CA10(fastjet::cambridge_algorithm, 1.0);
    fastjet::ClusterSequence cs_sub( largeRJets.at(i).constituents(), CA10);
    fastjet::PseudoJet ca_jet = sorted_by_pt(cs_sub.inclusive_jets())[0];

    // parameters are specified in settings.h
    fastjet::MassDropTagger md_tagger(mu, ycut);
    fastjet::PseudoJet tagged_jet = md_tagger(ca_jet);
    
    // Skip jet if it fails mass-drop tagging
    if (tagged_jet == 0 ) continue;

    //-----------------------------------------------------
 
    largeRJetsSel.push_back( largeRJets.at(i) );
  }

  largeRJetsSel = sorted_by_pt(largeRJetsSel);  // Resort
  std::vector<int> nBSubjetsLR_vec; // Vector specifying how many real b subjets there are
  BTagging( largeRJetsSel, trackjetsSel, nBSubjetsLR_vec);

  if( largeRJetsSel.size() != nBSubjetsLR_vec.size() )
    std::cout << "ERROR: b-tagging vector sizes don't match number of fat jets" << std::endl;
  
  const int nFatJets = (int)largeRJetsSel.size();
  const int nBBTaggedFatJets = std::min(largeRJetsSel.size(), (size_t)2); ;

  for( int i = 0; i < nBBTaggedFatJets; i++)
    bbFatJets.push_back( largeRJetsSel.at(i) );
  
  
  //=============================
  // C0: Cutflow before cuts
  // ============================
  FillHistogram("CF_res", event_weight, 0.1);
  FillHistogram("CF_inter", event_weight, 0.1);
  FillHistogram("CF_boost", event_weight, 0.1);

  FillHistogram("CFN_res", 1., 0.1);
  FillHistogram("CFN_inter", 1., 0.1);
  FillHistogram("CFN_boost", 1., 0.1);
  
  // Jet multiplicity histograms before cuts
  FillHistogram("N_SmallRJets_res_C0", event_weight, nJets);
  FillHistogram("N_SmallRJets_BJets_res_C0", event_weight, nBJets);
  FillHistogram("N_SmallRJets_BTagged_res_C0", event_weight, nBTaggedJets);
  
  FillHistogram("N_SmallRJets_inter_C0", event_weight, nJets);
  FillHistogram("N_SmallRJets_BJets_inter_C0", event_weight, nBJets);
  FillHistogram("N_SmallRJets_BTagged_inter_C0", event_weight, nBTaggedJets);

  FillHistogram("N_SmallRJets_boost_C0", event_weight, nJets);
  FillHistogram("N_SmallRJets_BJets_boost_C0", event_weight, nBJets);
  FillHistogram("N_SmallRJets_BTagged_boost_C0", event_weight, nBTaggedJets);  
  
  // Track jet multiplicity histograms before cuts
  FillHistogram("N_SmallRTrackJets_res_C0", event_weight, trackjets.size());
  FillHistogram("N_SmallRSelTrackJets_res_C0", event_weight, trackjetsSel.size());
  
  FillHistogram("N_SmallRTrackJets_inter_C0", event_weight, trackjets.size());
  FillHistogram("N_SmallRSelTrackJets_inter_C0", event_weight, trackjetsSel.size());
  
  FillHistogram("N_SmallRTrackJets_boost_C0", event_weight, trackjets.size());
  FillHistogram("N_SmallRSelTrackJets_boost_C0", event_weight, trackjetsSel.size());
  
  //===================================================
  // C1: Basic kinematic cuts + Higgs mass window cut
  //===================================================

  // Event categorised
  bool selected = false;
  
  bool isRes_C1 = false;
  bool isInter_C1 = false; 
  bool isBoost_C1 = false;

  // Boosted
  if( nFatJets >= 2 )    
      if( fabs(largeRJetsSel[0].m() - 125.) < 40.0 && fabs(largeRJetsSel[1].m() - 125.) < 40.0 )
      {
        HiggsFill(largeRJetsSel[0], largeRJetsSel[1], "boost", 1, event_weight);
        BoostFill(largeRJetsSel[0], largeRJetsSel[1], "boost", 1, event_weight);
        selected = true;
	isBoost_C1 = true;
      }
  
  // Resolved
  if( nJets >= 4 && selected == false)
  {
      // Reconstruct Higgs candidates from small-R jets
      std::vector<fastjet::PseudoJet> higgs_res;
      std::vector<fastjet::PseudoJet> higgs0_res;
      std::vector<fastjet::PseudoJet> higgs1_res;

      Reco_Resolved( smallRJetsSel, higgs_res, higgs0_res, higgs1_res );
    
      // Higgs mass window cut
      if( fabs(higgs_res[0].m() - 125.) < 40.0 && fabs(higgs_res[1].m() - 125.) < 40.0 )
      {
        HiggsFill(higgs_res[0], higgs_res[1], "res", 1, event_weight);
        selected = true;
	isRes_C1 = true;
      }
  }

  // Intermediate
  if( nJets >= 2 &&  nFatJets == 1 && selected == false)
  {
    // Reconstruct Higgs candidates from large-R and small-R jets
    std::vector<fastjet::PseudoJet> higgs_inter;
    int nBJets_SR = 0;
    const bool isRecoInter = Reco_Intermediate( smallRJetsSel, isFakeSR_vec, largeRJetsSel[0], nBJets_SR, higgs_inter );

    // Check if reconstruction was successful
    if( isRecoInter )
      if( fabs(higgs_inter[0].m() - 125.) < 40.0 && fabs(higgs_inter[1].m() - 125.) < 40.0 )
      {
        HiggsFill(higgs_inter[0], higgs_inter[1], "inter", 1, event_weight);
        BoostFill(largeRJetsSel[0], "inter", 1, event_weight);
	isInter_C1 = true;
      }
  }
  
  if( isRes_C1 && !isInter_C1 && !isBoost_C1 )		FillHistogram("Categories_C1", 1.0, 0.1);
  else if( !isRes_C1 && isInter_C1 && !isBoost_C1 )	FillHistogram("Categories_C1", 1.0, 1.1);
  else if( !isRes_C1 && !isInter_C1 && isBoost_C1 )	FillHistogram("Categories_C1", 1.0, 2.1);
  else if( isRes_C1 && isInter_C1 && !isBoost_C1 )	FillHistogram("Categories_C1", 1.0, 3.1);
  else if( isRes_C1 && !isInter_C1 && isBoost_C1 )	FillHistogram("Categories_C1", 1.0, 4.1);
  else if( !isRes_C1 && isInter_C1 && isBoost_C1 )	FillHistogram("Categories_C1", 1.0, 5.1);
  else if( isRes_C1 && isInter_C1 && isBoost_C1 )	FillHistogram("Categories_C1", 1.0, 6.1);

  //=============================
  // C2: b-tagging
  //=============================
  
  if( nBBTaggedFatJets >= 2 )
    if( fabs(bbFatJets[0].m() - 125.) < 40.0 && fabs(bbFatJets[1].m() - 125.) < 40.0 )
    {
      // b-tagging weights
      const double nB = nBSubjetsLR_vec[0] + nBSubjetsLR_vec[1];      // Number of true b-subjets
      const double nF = 4 - nB;  // Number of fake b-subjets

      // Reweighted event weight
      const double boost_weight = pow(btag_prob,nB)*pow(btag_mistag,nF)*event_weight;
      const fastjet::PseudoJet dihiggs_boost = bbFatJets[0] + bbFatJets[1];

      HiggsFill(bbFatJets[0], bbFatJets[1], "boost", 2, boost_weight);
      BoostFill(bbFatJets[0], bbFatJets[1], "boost", 2, boost_weight);

      // Calculate some substructure variables
      std::vector<double> split12_vec;
      split12_vec = SplittingScales( bbFatJets );

      std::vector<double> tau21_vec;
      tau21_vec = NSubjettiness( bbFatJets, BoostJetR );

      // C2 energy correlation double-ratio
      const double C2_fj1 = LST_C2(LST_beta, bbFatJets[0]);
      const double C2_fj2 = LST_C2(LST_beta, bbFatJets[1]);

      // D2 energy correlation double-ratio
      const double D2_fj1 = LMN_D2(LST_beta, bbFatJets[0]);
      const double D2_fj2 = LMN_D2(LST_beta, bbFatJets[1]);

      // Fill tuple
      bstNTuple << signal <<"\t"<<GetSample()<<"\t"<<boost_weight << "\t"
                << bbFatJets[0].pt() << "\t"
                << bbFatJets[1].pt() << "\t"
                << dihiggs_boost.pt() << "\t"
                << bbFatJets[0].m() << "\t"
                << bbFatJets[1].m() << "\t"
                << dihiggs_boost.m() << "\t"
                << bbFatJets[0].delta_R(bbFatJets[1])  << "\t"
                << getDPhi(bbFatJets[0].phi(), bbFatJets[1].phi())  << "\t"
                << fabs( bbFatJets[0].eta() - bbFatJets[1].eta())  << "\t"
                << split12_vec[0] << "\t"
                << split12_vec[1] << "\t"
                << tau21_vec[0] << "\t"
                << tau21_vec[1] << "\t"
                << C2_fj1 << "\t"
                << C2_fj2 << "\t"
                << D2_fj1 << "\t"
                << D2_fj2 << "\t"
                <<std::endl;

      Pass(boost_weight);
      Cut("BoostedCut", event_weight - boost_weight );
      return;
    }

  // Intermediate
  if( nBTaggedJets >= 2 &&  nBBTaggedFatJets == 1 )
  {
    // Reconstruct Higgs candidates from bb-tagged large-R and b-tagged small-R jets
    std::vector<fastjet::PseudoJet> higgs_inter;
    int nBJets_SR;
    const bool isRecoInter = Reco_Intermediate( smallRJetsSel, isFakeSR_vec, largeRJetsSel[0], nBJets_SR, higgs_inter );
    
    // Check if reconstruction was successful
    if( isRecoInter )
      if( fabs(higgs_inter[0].m() - 125.) < 40.0 && fabs(higgs_inter[1].m() - 125.) < 40.0 )
      {
        // Determine number of fake bJets
        const int nB = nBJets_SR + nBSubjetsLR_vec[0];
        const int nF = 4 - nB;

        // Error checking
        if (nB > 4)
        {
          std::cerr << "ERROR: number of reconstructed b quarks > 4!"<<std::endl;
          exit(-1);
        }

        // Reweighted event weight
        const double inter_weight = pow(btag_prob,nB)*pow(btag_mistag,nF)*event_weight;
        const fastjet::PseudoJet dihiggs_inter = higgs_inter[0] + higgs_inter[1];

        HiggsFill(higgs_inter[0], higgs_inter[1], "inter", 2, inter_weight);
        BoostFill(largeRJetsSel[0], "inter", 2, inter_weight);

        // Calculate some substructure variables
        const double split12 = SplittingScales( largeRJetsSel[0] );
        const double tau21 = NSubjettiness( largeRJetsSel[0], BoostJetR );
        const double C2 = LST_C2(LST_beta, largeRJetsSel[0]);
        const double D2 = LMN_D2(LST_beta, largeRJetsSel[0]);

        // Fill tuple
        intNTuple << signal <<"\t"<<GetSample()<<"\t"<<inter_weight << "\t"
                  << higgs_inter[0].pt() << "\t"
                  << higgs_inter[1].pt() << "\t"
                  << dihiggs_inter.pt() << "\t"
                  << higgs_inter[0].m() << "\t"
                  << higgs_inter[1].m() << "\t"
                  << dihiggs_inter.m() << "\t"
                  << higgs_inter[0].delta_R(higgs_inter[1]) << "\t"
                  << getDPhi(higgs_inter[0].phi(), higgs_inter[1].phi()) << "\t"
                  << fabs( higgs_inter[0].eta() - higgs_inter[1].eta())  << "\t"
                  << split12 << "\t"
                  << tau21 << "\t"
                  << C2 << "\t"
                  << D2 << "\t"
                  <<std::endl;


        // Final
        Pass(inter_weight);
        Cut("IntermediateCut", event_weight - inter_weight );
        return;
     }
  }

  // Resolved
  if( nBTaggedJets >= 4 )
  {
    // Reconstruct Higgs candidates from b-tagged small-R jets
    std::vector<fastjet::PseudoJet> higgs_res;
    std::vector<fastjet::PseudoJet> higgs0_res;
    std::vector<fastjet::PseudoJet> higgs1_res;

    Reco_Resolved( smallRJetsSel, higgs_res, higgs0_res, higgs1_res );
    
    // Higgs mass window cut
    if( fabs(higgs_res[0].m() - 125.) < 40.0 && fabs(higgs_res[1].m() - 125.) < 40.0 )
    {
      // Determine number of real and fake b-jets
      int nB = 0;
      for (int i=0; i<nBTaggedJets; i++)
        if (!isFakeSR_vec[i])
          nB++;

      const int nF = 4 - nB;

      // Reweighted event weight
      const double res_weight = pow(btag_prob,nB)*pow(btag_mistag,nF)*event_weight;
      const fastjet::PseudoJet dihiggs_res = higgs_res[0] + higgs_res[1];

      HiggsFill(higgs_res[0], higgs_res[1], "res", 2, res_weight);

      // Fill tuple
      resNTuple << signal <<"\t"<<GetSample()<<"\t"<<res_weight << "\t"
                << higgs_res[0].pt() << "\t"
                << higgs_res[1].pt() << "\t"
                << dihiggs_res.pt() << "\t"
                << higgs_res[0].m() << "\t"
                << higgs_res[1].m() << "\t"
                << dihiggs_res.m() << "\t"
                << higgs_res[0].delta_R(higgs_res[1]) << "\t"
                << getDPhi(higgs_res[0].phi(), higgs_res[1].phi()) << "\t"
                << fabs( higgs_res[0].eta() - higgs_res[1].eta())  << "\t"
                << higgs0_res[0].pt() << "\t"
                << higgs0_res[1].pt() << "\t"
                << higgs1_res[0].pt() << "\t"
                << higgs1_res[1].pt() << "\t"
                <<std::endl;

      Pass(res_weight);
      Cut("ResolvedCut", event_weight - res_weight);
      return;
    }
  }

  return Cut ("Uncategorised", event_weight);
}


void OxfordCombinedRWAnalysis::BTagging( std::vector<fastjet::PseudoJet> const& jets_vec, std::vector<bool>& isFake_vec  )
{
  
  // Loop over all jets
  for( size_t i=0; i<jets_vec.size(); i++){

    // Get the jet constituents
    const std::vector<fastjet::PseudoJet>& jet_constituents = jets_vec[i].constituents();
    bool isbJet = false;

    // Loop over constituents and look for b quarks
    // b quarks must be above some minimum pt
    for(size_t j=0; j<jet_constituents.size(); j++)
    {
      // Flavour of jet constituent
      const int userid= jet_constituents.at(j).user_index();
      const double pt_bcandidate = jet_constituents.at(j).pt();

      if(abs(userid) == 5 && pt_bcandidate > pt_btagging)
        isbJet = true;    
    }

    // Push opposite of isBJet!
    isFake_vec.push_back(!isbJet);
  }
         
}

void OxfordCombinedRWAnalysis::BTagging( std::vector<fastjet::PseudoJet> const& largeRJets, std::vector<fastjet::PseudoJet> const& trackjets, std::vector<int>& nBSubJets_vec )
{
    // Loop over all fat jets
    for( size_t i=0; i<largeRJets.size(); i++)
    {
      // Get ghost associated track jets
      std::vector<fastjet::PseudoJet> subjets;
      get_assoc_trkjets( largeRJets.at(i), trackjets, subjets, false);

      int nBSubJets = 0;

      // Loop over subjets
      const int nSubjets = std::min((int)subjets.size(), 2); // Restrict to leading 2 subjets
      for( int j=0; j < nSubjets; j++)
      {
        bool isBJet = false;
        // Loop over constituents and look for b quarks
        // b quarks must be above some minimum pt 
        const std::vector<fastjet::PseudoJet>& subjet_constituents = sorted_by_pt( subjets[j].constituents() );
        for(size_t k=0; k < subjet_constituents.size(); k++)
        {
          // Flavour of jet constituent
          const int userid= subjet_constituents.at(k).user_index();
          const double pt_bcandidate = subjet_constituents.at(k).pt();

          if( abs(userid) == 5 && pt_bcandidate > pt_btagging )
          {
            isBJet = true;
            break;
          }
        }

        if (isBJet)
          nBSubJets++;
           
      }//end of loop over subjets
  	   
      nBSubJets_vec.push_back(nBSubJets);

    }//end of loop over jets
}


void OxfordCombinedRWAnalysis::Reco_Resolved( std::vector<fastjet::PseudoJet> const& bjets, // Input b-jets
                                              std::vector<fastjet::PseudoJet>& higgs_vec,   // Returned Higgs candidates
                                              std::vector<fastjet::PseudoJet>& higgs0_vec,  // Leading higgs subjets
                                              std::vector<fastjet::PseudoJet>& higgs1_vec ) // Subleading higgs subjets
{

    // Get the pairing that minimizes |m_dj1 - m_dj2|
    double dijet_mass[4][4];
    for(int ijet=0;ijet<4;ijet++)
      for(int jjet=0;jjet<4;jjet++)
      {
        // Compute jet masses
        const fastjet::PseudoJet sum = bjets[ijet] + bjets[jjet];
        dijet_mass[ijet][jjet] = sum.m();
      }

    double mdj_diff_min = 1e20; // Some large number to begin
    int jet1_id1=10,jet1_id2=10,jet2_id1=10,jet2_id2=10;

    for(int ijet=0;ijet<4;ijet++)
      for(int jjet=ijet+1;jjet<4;jjet++)
        for(int ijet2=0;ijet2<4;ijet2++)
          for(int jjet2=ijet2+1;jjet2<4;jjet2++)
          {
            const double mdj1 = dijet_mass[ijet][jjet];
            const double mdj2 = dijet_mass[ijet2][jjet2];
            const double min_dj = fabs(mdj1 - mdj2);

            if(min_dj <  mdj_diff_min && ijet != ijet2  && jjet != jjet2 && jjet !=ijet2 && ijet != jjet2 )
            {
              mdj_diff_min = min_dj;
              jet1_id1 = ijet;
              jet1_id2 = jjet;
              jet2_id1 = ijet2;
              jet2_id2 = jjet2;
            }
          }

    // Construct the Higgs candidates
    fastjet::PseudoJet higgs1 = bjets.at( jet1_id1) + bjets.at( jet1_id2); 
    fastjet::PseudoJet higgs2 = bjets.at( jet2_id1) + bjets.at( jet2_id2);
    
    if( higgs1.pt() > higgs2.pt()){
      higgs_vec.push_back(higgs1);
      higgs_vec.push_back(higgs2);

      higgs0_vec.push_back(bjets.at( jet1_id1));
      higgs0_vec.push_back(bjets.at( jet1_id2));

      higgs1_vec.push_back(bjets.at( jet2_id1));
      higgs1_vec.push_back(bjets.at( jet2_id2));

    }
    else{
      higgs_vec.push_back(higgs2);
      higgs_vec.push_back(higgs1);  

      higgs1_vec.push_back(bjets.at( jet1_id1));
      higgs1_vec.push_back(bjets.at( jet1_id2));

      higgs0_vec.push_back(bjets.at( jet2_id1));
      higgs0_vec.push_back(bjets.at( jet2_id2));  
    }

    // Sort vectors
    higgs0_vec = sorted_by_pt(higgs0_vec);
    higgs1_vec = sorted_by_pt(higgs1_vec);

}


bool OxfordCombinedRWAnalysis::Reco_Intermediate( std::vector<fastjet::PseudoJet> const& bjets, 
                                                  std::vector<bool> const& isFakeSR_vec,
                                                  fastjet::PseudoJet const& fatjet, 
                                                  int& nBjets,
                                                  std::vector<fastjet::PseudoJet>& higgs_vec )
{
  
    // Identify small-R jets separated from merged Higgs
    std::vector<fastjet::PseudoJet> bjets_separated;
    std::vector<bool> bjets_separated_isFake;
    
    for( size_t i = 0; i < bjets.size(); i++ )
    {  
      double dR = bjets.at(i).delta_R(fatjet);
      if( dR < 1.0 ) continue;
      
      bjets_separated.push_back( bjets.at(i) );
      bjets_separated_isFake.push_back( isFakeSR_vec[i] );
    }
    
    if( bjets_separated.size() < 2 ) return false;
    
    double mdj_diff_min = 1e20; // Some large number to begin
    int bjet_id1(100), bjet_id2(100);
    
    // Get the pairing that minimizes |m_dj1 - m_dj2|
    for(int ijet=0; ijet < (int)bjets_separated.size(); ijet++)
      for(int jjet=0; jjet < (int)bjets_separated.size(); jjet++)
      {
        // Compute jet masses
        const fastjet::PseudoJet sum = bjets_separated[ijet] + bjets_separated[jjet];
	
	double diff = fabs( sum.m() - fatjet.m() );
	if( diff < mdj_diff_min ){
	  
	  bjet_id1 = ijet;
	  bjet_id2 = jjet;
	  mdj_diff_min = diff;
	}
      }
    
    // Construct the Higgs candidates
    fastjet::PseudoJet higgs1 = fatjet; 
    fastjet::PseudoJet higgs2 = bjets_separated.at( bjet_id1) + bjets_separated.at( bjet_id2);
    nBjets = (bjets_separated_isFake[bjet_id1] ? 0:1) + (bjets_separated_isFake[bjet_id2] ? 0:1);
    
    if( higgs1.pt() > higgs2.pt()){
      higgs_vec.push_back(higgs1);
      higgs_vec.push_back(higgs2);
    }
    else{
      higgs_vec.push_back(higgs2);
      higgs_vec.push_back(higgs1);    
    }
    
    return true;
}

// General fill for reconstructed higgs quantities
void OxfordCombinedRWAnalysis::HiggsFill(fastjet::PseudoJet const& H0,
                                         fastjet::PseudoJet const& H1,
                                         std::string const& analysis, 
                                         size_t const& cut, 
                                         double const& weight)
{
  if (H0.pt() < H1.pt())
    std::cerr << "HiggsFill WARNING: pT ordering incorrect! "<<analysis<<"  "<<cut<<"  "<<H0.pt() << "  "<<H1.pt()<<std::endl;

  const std::string cutStr = "_C"+std::to_string(static_cast<long long int>(cut));
  const std::string suffix = "_" + analysis + cutStr;

  // Record cutflow
  FillHistogram("CF_" +analysis, weight, cut + 0.1);
  FillHistogram("CFN_"+analysis, 1., cut + 0.1);

  // Histograms for reconstructed Higgs candidates
  FillHistogram("pt_H0" + suffix, weight, H0.pt());
  FillHistogram("pt_H1" + suffix, weight, H1.pt());

  FillHistogram("m_H0" + suffix, weight, H0.m());
  FillHistogram("m_H1" + suffix, weight, H1.m());

  FillHistogram("ptHptH" + suffix, weight, H0.pt(), H1.pt());
  FillHistogram("mHmH" + suffix, weight, H0.m(), H1.m());

  FillHistogram("dR_HH" + suffix, weight, H0.delta_R(H1) );
  FillHistogram("dPhi_HH" + suffix, weight, getDPhi(H0.phi(), H1.phi()) );
  FillHistogram("dEta_HH" + suffix, weight, fabs( H0.eta() - H1.eta()) );

  // Reconstruct di-Higgs system
  const fastjet::PseudoJet dihiggs = H0 + H1;

  // Histograms for reconstructed di-Higgs system
  FillHistogram("m_HH" + suffix, weight, dihiggs.m());
  FillHistogram("pt_HH" + suffix, weight, dihiggs.pt());

}


void OxfordCombinedRWAnalysis::BoostFill( fastjet::PseudoJet const& H,
                                          std::string const& analysis, 
                                          size_t const& cut, 
                                          double const& weight )
{
  const std::string cutStr = "_C"+std::to_string(static_cast<long long int>(cut));
  const std::string suffix = "_" + analysis + cutStr;

  // Splitting scales
  const double split12 = SplittingScales( H );

  // 2-subjettiness / 1-subjettiness
  const double tau21 = NSubjettiness( H, BoostJetR );

  // C2/D2 energy correlation double-ratio
  const double C2 = LST_C2(LST_beta, H);
  const double D2 = LMN_D2(LST_beta, H);

  FillHistogram("split12_fj" + suffix, weight, split12);
  FillHistogram("tau21_fj" + suffix, weight, tau21);
  FillHistogram("C2_fj" + suffix, weight, C2);
  FillHistogram("D2_fj" + suffix, weight, D2);
}

void OxfordCombinedRWAnalysis::BoostFill( fastjet::PseudoJet const& H0,
                                          fastjet::PseudoJet const& H1,
                                          std::string const& analysis, 
                                          size_t const& cut, 
                                          double const& weight )
{
  if (H0.pt() < H1.pt())
    std::cerr << "BoostFill WARNING: pT ordering incorrect! "<<analysis<<"  "<<cut<<"  "<<H0.pt() << "  "<<H1.pt()<<std::endl;

  const std::string cutStr = "_C"+std::to_string(static_cast<long long int>(cut));
  const std::string suffix = "_" + analysis + cutStr;

  // Splitting scales
  const double split12_fj1 = SplittingScales( H0 );
  const double split12_fj2 = SplittingScales( H1 );

  // 2-subjettiness / 1-subjettiness
  const double tau21_fj1 = NSubjettiness( H0, BoostJetR );
  const double tau21_fj2 = NSubjettiness( H1, BoostJetR );

  // C2 energy correlation double-ratio
  const double C2_fj1 = LST_C2(LST_beta, H0);
  const double C2_fj2 = LST_C2(LST_beta, H1);

  // D2 energy correlation double-ratio
  const double D2_fj1 = LMN_D2(LST_beta, H0);
  const double D2_fj2 = LMN_D2(LST_beta, H1);

  FillHistogram("split12_fj1" + suffix, weight, split12_fj1);
  FillHistogram("split12_fj1" + suffix, weight, split12_fj2);

  FillHistogram("tau21_fj1" + suffix, weight, tau21_fj1);
  FillHistogram("tau21_fj2" + suffix, weight, tau21_fj2);

  FillHistogram("C2_fj1" + suffix, weight, C2_fj1);
  FillHistogram("C2_fj2" + suffix, weight, C2_fj2);

  FillHistogram("D2_fj1" + suffix, weight, D2_fj1);
  FillHistogram("D2_fj2" + suffix, weight, D2_fj2);
}

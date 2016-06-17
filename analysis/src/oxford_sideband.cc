// oxford_res_fr.cc

#include "YODA/Histo1D.h"
#include "YODA/Histo2D.h"

#include "oxford_sideband.h"
#include "utils.h"
#include "run.h"

#include "fastjet/Selector.hh"
#include "fastjet/ClusterSequence.hh"
#include "fastjet/tools/MassDropTagger.hh"
#include "fastjet/tools/Filter.hh"

#include "fastjet/contrib/SoftKiller.hh"
#include "fastjet/contrib/EnergyCorrelator.hh"
#include "fastjet/contrib/Nsubjettiness.hh"

#include <algorithm>

using std::vector;
using namespace fastjet::contrib;
using namespace fastjet;

// LST energy correlations
const EnergyCorrelatorC2 C2(2, EnergyCorrelator::pt_R);
const EnergyCorrelatorD2 D2(2, EnergyCorrelator::pt_R);

// tau2/tau1 NSubjettiness
const NsubjettinessRatio tau21(2,1, KT_Axes(), UnnormalizedMeasure(1));

// Debugging
const bool debug = false;

// Analysis settings
const int nAnalysis = 3;  const int nCuts = 4;
const std::string aString[nAnalysis] = {"_res", "_inter", "_boost"};
const std::string cString[nCuts] = {"_GEN", "_RCO", "_SIG", "_SDB"};


// **************************** Reconstruction helper functions ****************************

static vector<PseudoJet> trimJets( vector<PseudoJet> const& injets)
{
  const double Rfilt = 0.2; const double pt_fraction_min = 0.05;
  const fastjet::Filter trimmer(Rfilt, fastjet::SelectorPtFractionMin(pt_fraction_min));
  vector<PseudoJet> trimmedJets;
  for (size_t i=0; i<injets.size(); i++)
      trimmedJets.push_back(trimmer(injets[i]));
  return trimmedJets; 
}

// Check if jets are mass-drop tagged
static vector<PseudoJet> MDtagJets( vector<PseudoJet> const& injets)
{
  // Mass-drop tagger
  double const mu = 0.67;
  double const ycut = 0.09;

  const fastjet::JetDefinition CA10(fastjet::cambridge_algorithm, 1.0);
  const fastjet::MassDropTagger md_tagger(mu, ycut);
  vector<PseudoJet> MDTJets;
  for (size_t i=0; i<injets.size(); i++)
  {
    const fastjet::ClusterSequence cs_sub( injets[i].constituents(), CA10);
    vector<PseudoJet> ca_jets = sorted_by_pt( cs_sub.inclusive_jets() );
    const PseudoJet ca_jet = ca_jets[0];
    const PseudoJet tagged_jet = md_tagger(ca_jet);
    if ( tagged_jet != 0 )
      MDTJets.push_back(injets[i]);
  }
  return MDTJets;
}

// Returns the two hardest GA subjets for each input largeR jet
static vector< vector<PseudoJet> > getSubJets( vector<PseudoJet> const& largeRJets, vector<PseudoJet> const& trackJets )
{
  vector< vector<PseudoJet> > largeRsubJets;
  for ( PseudoJet jet : largeRJets )
  {
    vector<PseudoJet> subJets;
    get_assoc_trkjets( jet, trackJets, subJets, false);
    largeRsubJets.push_back( SelectorNHardest(2)(sorted_by_pt(subJets)) );
  }
  return largeRsubJets;
}

// Small-R B-tagging
static vector<btagType> BTagging( vector<PseudoJet> const& jets_vec )
{
  vector<btagType> btag_vec;
  for( auto jet : jets_vec )
  {
    btagType type = NTAG;
    const vector<PseudoJet>& jet_constituents = SelectorPtMin(15)(jet.constituents());
    for( auto constituent : jet_constituents )
    {
      const int userid= constituent.user_index();
      if(abs(userid) == 5) type = BTAG;
      if(abs(userid) == 4 && type != BTAG ) type = CTAG;
      if( type == NTAG ) type = LTAG;
    }
    btag_vec.push_back(type);
  }
  return btag_vec;
}

// nTag: How many b-tags are required
// nB: How many true b-jets are present
// nC: How many true c-jets are present
// nL: How many light jets are present
static double btagProb( int const& nTag, int const& nB, int const& nC, int const& nL)
{
  // Choose working point with high purity
  const double btag_prob = 0.80; // Probability of correct b tagging
  const double btag_mistag = 0.01; // Mistag probability  
  const double ctag_prob = 0.1;//0.17; // c-mistag rate ~1/6.

  // Probability of all permutations
  double totalProb=0;  

  // Loop over all possible classification permutations
  for (int iB=0; iB<=std::min(nB, nTag); iB++)           // iB b-jets tagged as b
    for (int iC=0; iC<=std::min(nC, nTag-iB); iC++)      // iC c-jets tagged as b
      for (int iL=0; iL<=std::min(nL, nTag-iB-iC); iL++) // iL l-jets tagged as b
      {
        const double bProb = pow(btag_prob, iB)*pow(1.0-btag_prob,nB-iB);
        const double cProb = pow(ctag_prob, iC)*pow(1.0-ctag_prob,nC-iC);
        const double lProb = pow(btag_mistag, iL)*pow(1.0-btag_mistag, nL-iL);

        // Does the current permutation have the correct number of b-Tags?
        const int permutationTags = iB+iC+iL;   //Number of b-Tags in current permutation
        if (permutationTags == nTag) 
          totalProb += bProb*cProb*lProb;
      }

  return totalProb;
}


OxfordSidebandAnalysis::OxfordSidebandAnalysis(runCard const& run, sampleCard const& sample, int const& subsample):
Analysis("sideband", run, sample, subsample),
subtractPU(run.npileup > 0)
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
  
  const double eta_min = -6.;
  const double eta_max = +6.;
  
  const double phi_min = -3.15;
  const double phi_max = +3.15;
  
  const double m_HH_min = 0.;
  const double m_HH_max = 600.; 
  
  const double pt_HH_min = 0.;
  const double pt_HH_max = 300.;
  
  const double chi_HH_min = 0.;
  const double chi_HH_max = 30.;
  
  const int nbins = 30;
  
  // ********************* Histogram definitions ******************

  for (int i=0; i< nAnalysis; i++)
  {
    BookHistogram(new YODA::Histo1D( nCuts, 0, nCuts ), "CF" + aString[i]);
    BookHistogram(new YODA::Histo1D( nCuts, 0, nCuts ), "CFN" + aString[i]);

    for (int j=0; j< nCuts; j++)
    {
      const std::string suffix = aString[i] + cString[j];
      
      BookHistogram(new YODA::Histo1D(nbins, pt_min, pt_max), "pt_smallR" + suffix);
      BookHistogram(new YODA::Histo1D(nbins, eta_min, eta_max), "eta_smallR" + suffix);
      BookHistogram(new YODA::Histo1D(nbins, m_min, m_max), "m_smallR" + suffix);
      
      BookHistogram(new YODA::Histo1D(nbins, pt_min, pt_max), "pt_largeR" + suffix);
      BookHistogram(new YODA::Histo1D(nbins, eta_min, eta_max), "eta_largeR" + suffix);
      BookHistogram(new YODA::Histo1D(nbins, m_min, m_max), "m_largeR" + suffix);
      
      BookHistogram(new YODA::Histo1D(nbins, pt_min, pt_max), "pt_H0" + suffix);
      BookHistogram(new YODA::Histo1D(nbins, pt_min, pt_max), "pt_H1" + suffix);

      BookHistogram(new YODA::Histo1D(nbins, m_min, m_max), "m_H0" + suffix);
      BookHistogram(new YODA::Histo1D(nbins, m_min, m_max), "m_H1" + suffix);
      
      BookHistogram(new YODA::Histo1D(nbins, eta_min, eta_max), "eta_H0" + suffix);
      BookHistogram(new YODA::Histo1D(nbins, eta_min, eta_max), "eta_H1" + suffix);
      
      BookHistogram(new YODA::Histo1D(nbins, phi_min, phi_max), "phi_H0" + suffix);
      BookHistogram(new YODA::Histo1D(nbins, phi_min, phi_max), "phi_H1" + suffix);

      BookHistogram(new YODA::Histo1D(nbins, pt_HH_min, pt_HH_max), "pt_HH" + suffix);
      BookHistogram(new YODA::Histo1D(nbins, m_HH_min, m_HH_max), "m_HH" + suffix);
      BookHistogram(new YODA::Histo1D(nbins, DeltaRmin, DeltaRmax), "dR_HH" + suffix);

      BookHistogram(new YODA::Histo1D(nbins, DeltaPhimin, DeltaPhimax), "dPhi_HH" + suffix);
      BookHistogram(new YODA::Histo1D(nbins, DeltaEtamin, DeltaEtamax), "dEta_HH" + suffix);

      BookHistogram(new YODA::Histo1D(nbins, chi_HH_min, chi_HH_max), "chi_HH" + suffix);
      
      BookHistogram(new YODA::Histo2D(nbins, pt_min, pt_max, nbins, pt_min, pt_max), "ptHptH" + suffix);
      BookHistogram(new YODA::Histo2D(nbins, m_min, m_max, nbins, m_min, m_max), "mHmH" + suffix);
      
      BookHistogram(new YODA::Histo1D(nbins, pt_min, pt_max), "pt_leadSJ_fj1" + suffix);
      BookHistogram(new YODA::Histo1D(nbins, pt_min, pt_max), "pt_subleadSJ_fj1" + suffix);

      BookHistogram(new YODA::Histo1D(nbins, pt_min, pt_max), "pt_leadSJ_fj2" + suffix);
      BookHistogram(new YODA::Histo1D(nbins, pt_min, pt_max), "pt_subleadSJ_fj2" + suffix);
      
      BookHistogram(new YODA::Histo1D(nbins, pt_min, pt_max), "pt_leadSJ_fj" + suffix);
      BookHistogram(new YODA::Histo1D(nbins, pt_min, pt_max), "pt_subleadSJ_fj" + suffix);

      BookHistogram(new YODA::Histo1D(nbins, eta_min, eta_max), "eta_leadSJ_fj1" + suffix);
      BookHistogram(new YODA::Histo1D(nbins, eta_min, eta_max), "eta_subleadSJ_fj1" + suffix);

      BookHistogram(new YODA::Histo1D(nbins, eta_min, eta_max), "eta_leadSJ_fj2" + suffix);
      BookHistogram(new YODA::Histo1D(nbins, eta_min, eta_max), "eta_subleadSJ_fj2" + suffix);
      
      BookHistogram(new YODA::Histo1D(nbins, eta_min, eta_max), "eta_leadSJ_fj" + suffix);
      BookHistogram(new YODA::Histo1D(nbins, eta_min, eta_max), "eta_subleadSJ_fj" + suffix);
      
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
    
  // ********************* Ntuple definition **********************

  const std::string tupleSpec = "# signal source weight pt_H0 pt_H1 pt_HH m_H0 m_H1 m_HH dR_HH dPhi_HH dEta_HH chi_HH pt_H0_sub0 pt_H0_sub1 pt_H1_sub0 pt_H1_sub1";

  const std::string root = "." + GetRoot() + GetSample() + "/";
  std::stringstream suffix; suffix << "." <<GetSubSample() <<".dat";

  const std::string resDir = root+"resNTuple"+suffix.str();
  const std::string intDir = root+"intNTuple"+suffix.str();
  const std::string bstDir = root+"bstNTuple"+suffix.str();

  resNTuple.open(resDir.c_str());
  intNTuple.open(intDir.c_str());
  bstNTuple.open(bstDir.c_str());

  resNTuple << tupleSpec <<std::endl;
  intNTuple << tupleSpec <<" split12_fj tau21_fj C2_fj D2_fj"<<std::endl;
  bstNTuple << tupleSpec <<" split12_fj1 split12_fj2 tau21_fj1 tau21_fj2 C2_fj1 C2_fj2 D2_fj1 D2_fj2"<<std::endl;

  std::cout << "Oxford PU subtraction: " << subtractPU << std::endl;
}

void OxfordSidebandAnalysis::Analyse(bool const& signal, double const& weightnorm, finalState const& ifs)
{
  Analysis::Analyse(signal, weightnorm, ifs);

  // Perform softKiller subtraction
  const fastjet::contrib::SoftKiller soft_killer(2.5, 0.4);
  const finalState fs = subtractPU ? soft_killer(ifs):ifs;

   // Set initial weight
  const double event_weight = weightnorm;

  // *************************************** General cut selectors *************************************

  const Selector LR_kinematics = SelectorNHardest(2) * ( SelectorAbsRapMax(2.0) && SelectorPtMin(200.0) );
  const Selector SR_kinematics = SelectorNHardest(4) * ( SelectorAbsRapMax(2.5) && SelectorPtMin(40.0) );
  const Selector TR_kinematics = SelectorAbsRapMax(2.5) && SelectorPtMin(50.0);

  // ********************************* Jet clustering  ***************************************
  
  // Cluster small-R jets
  const double ResJetR=0.4;
  const fastjet::JetDefinition akt_res(fastjet::antikt_algorithm, ResJetR);
  const fastjet::ClusterSequence cs_akt_res(fs, akt_res);
  const vector<PseudoJet> smallRJets = sorted_by_pt( SR_kinematics(cs_akt_res.inclusive_jets())  ); 
  const vector<btagType> tagType_SR = BTagging( smallRJets );

  // Cluster small-R track jets 
  const double GAjetR = 0.3; // Boosted subjet radius for ghost-association
  const fastjet::JetDefinition jd_subjets(fastjet::antikt_algorithm, GAjetR);
  const fastjet::ClusterSequence cs_subjets(fs, jd_subjets);
  const vector<PseudoJet> trackJets = sorted_by_pt( TR_kinematics(cs_subjets.inclusive_jets() ) );

  // Cluster large-R jets
  const double BoostJetR=1.0;
  const fastjet::JetDefinition akt_boost(fastjet::antikt_algorithm, BoostJetR);
  const fastjet::ClusterSequence cs_akt_bst(fs, akt_boost);
  const vector<PseudoJet> largeRJets_noTrim = LR_kinematics(cs_akt_bst.inclusive_jets()); 
  const vector<PseudoJet> largeRJets_Trim = subtractPU ? trimJets(largeRJets_noTrim): largeRJets_noTrim;
  const vector<PseudoJet> largeRJets = sorted_by_pt( MDtagJets(largeRJets_Trim) );
  const vector< vector<PseudoJet> > largeRsubJets = getSubJets(largeRJets, trackJets);
  vector< const vector<btagType> > tagType_LR;
  for ( auto subjets : largeRsubJets )
    tagType_LR.push_back( BTagging(subjets) );

  // ***************************************** Initial histograms **********************************************

  FillHistogram("CF_res", event_weight, 0.1);
  FillHistogram("CF_inter", event_weight, 0.1);
  FillHistogram("CF_boost", event_weight, 0.1);

  FillHistogram("CFN_res", 1., 0.1);
  FillHistogram("CFN_inter", 1., 0.1);
  FillHistogram("CFN_boost", 1., 0.1);

  // **************************************** Boosted analysis *********************************************
  
  ResolvedAnalysis( smallRJets, tagType_SR, signal, event_weight );

  return;
}


void OxfordSidebandAnalysis::ResolvedAnalysis( vector<PseudoJet> const& srj,  vector<btagType> const& btags, bool const& signal, double const& event_weight )
{
  if( srj.size() >= 4 )
  {
    // Reconstruct Higgs candidates from small-R jets
    typedef std::pair<PseudoJet,PseudoJet> dijet;
    typedef std::pair<dijet,dijet> hpair;

    vector<hpair> hcand = { hpair(dijet(srj[0],srj[1]), dijet(srj[2],srj[3])),
                            hpair(dijet(srj[0],srj[2]), dijet(srj[1],srj[3])),
                            hpair(dijet(srj[0],srj[3]), dijet(srj[1],srj[2]))};
    auto bc = std::min_element(hcand.begin(), hcand.end(), [](hpair& p1, hpair& p2){
          return( fabs( (p1.first.first+p1.first.second).m() - (p1.second.first+p1.second.second).m() ) < 
                  fabs( (p2.first.first+p2.first.second).m() - (p2.second.first+p2.second.second).m() ));
      } );

    // Construct pseudojets
    dijet& hp1 = (*bc).first; dijet& hp2 = (*bc).second; 
    PseudoJet higgs1 = hp1.first + hp1.second;
    PseudoJet higgs2 = hp2.first + hp2.second;
    const fastjet::PseudoJet dihiggs = higgs1 + higgs2;

    // Sort by pT
    if (higgs1.pt() < higgs2.pt()) std::swap(higgs1, higgs2);
    if (hp1.first.pt() < hp1.second.pt()) std::swap(hp1.first, hp1.second);
    if (hp2.first.pt() < hp2.second.pt()) std::swap(hp2.first, hp2.second);

    // b-Tagging rate
    const int nB = std::count(btags.begin(), btags.begin() + 4, BTAG);  // Number of b's
    const int nC = std::count(btags.begin(), btags.begin() + 4, CTAG);  // Number of c's
    const int nL = std::count(btags.begin(), btags.begin() + 4, LTAG);  // Number of u,d,s,g
    const double selEff = btagProb( 4, nB, nC, nL);
    const double selWgt = selEff*event_weight;
    // Reco fill
    HiggsFill( higgs1, higgs2, "res", 1, selWgt );

    const double diffHiggs_0 = fabs(higgs1.m() - 125.);
    const double diffHiggs_1 = fabs(higgs2.m() - 125.);
    const double massWindow = 40.0;

    if( ( diffHiggs_0 < massWindow ) && ( diffHiggs_1 < massWindow ) )
    {
      HiggsFill( higgs1, higgs2, "res", 2, selWgt );
      resNTuple << signal <<"\t"<<GetSample()<<"\t"<<selWgt << "\t"
          << higgs1.pt() << "\t"
          << higgs2.pt() << "\t"
          << dihiggs.pt() << "\t"
          << higgs1.m() << "\t"
          << higgs2.m() << "\t"
          << dihiggs.m() << "\t"
          << higgs1.delta_R(higgs2) << "\t"
          << getDPhi(higgs1.phi(), higgs2.phi()) << "\t"
          << fabs( higgs1.eta() - higgs2.eta())  << "\t"
          << Chi( higgs1, higgs2)  << "\t"
          << hp1.first.pt() << "\t"
          << hp1.second.pt() << "\t"
          << hp2.first.pt() << "\t"
          << hp2.second.pt() << "\t"
          <<std::endl;
    }
    else
      HiggsFill( higgs1, higgs2, "res", 3, selWgt );

  }



  //       Pass(res_weight); 
  //       passed_weight += res_weight;

}
 
// General fill for reconstructed higgs quantities
void OxfordSidebandAnalysis::HiggsFill(fastjet::PseudoJet const& H0,
                                         fastjet::PseudoJet const& H1,
                                         std::string const& analysis, 
                                         size_t const& cut, 
                                         double const& weight)
{
  if (H0.pt() < H1.pt())
    std::cerr << "HiggsFill WARNING: pT ordering incorrect! "<<analysis<<"  "<<cut<<"  "<<H0.pt() << "  "<<H1.pt()<<std::endl;

  if( debug ) std::cout << "HiggsFill INFO: cut = " << cut << std::endl;
  if( debug ) std::cout << "HiggsFill INFO: analysis = " << analysis << std::endl;

  // Histo fill suffix
  const std::string suffix = "_" + analysis + cString[cut];
    
  // Record cutflow
  FillHistogram("CF_" +analysis, weight, cut + 0.1);
  FillHistogram("CFN_"+analysis, 1., cut + 0.1);

  // Histograms for reconstructed Higgs candidates
  FillHistogram("pt_H0" + suffix, weight, H0.pt());
  FillHistogram("pt_H1" + suffix, weight, H1.pt());

  FillHistogram("m_H0" + suffix, weight, H0.m());
  FillHistogram("m_H1" + suffix, weight, H1.m());
  
  FillHistogram("eta_H0" + suffix, weight, H0.eta());
  FillHistogram("eta_H1" + suffix, weight, H1.eta());
  
  FillHistogram("phi_H0" + suffix, weight, H0.phi());
  FillHistogram("phi_H1" + suffix, weight, H1.phi());

  FillHistogram("ptHptH" + suffix, weight, H0.pt(), H1.pt());
  FillHistogram("mHmH" + suffix, weight, H0.m(), H1.m());

  FillHistogram("dR_HH" + suffix, weight, H0.delta_R(H1) );
  FillHistogram("dPhi_HH" + suffix, weight, getDPhi(H0.phi(), H1.phi()) );
  FillHistogram("dEta_HH" + suffix, weight, fabs( H0.eta() - H1.eta()) );
  FillHistogram("chi_HH" + suffix, weight, Chi(H0,H1) );

  // Reconstruct di-Higgs system
  const fastjet::PseudoJet dihiggs = H0 + H1;

  // Histograms for reconstructed di-Higgs system
  FillHistogram("m_HH" + suffix, weight, dihiggs.m());
  FillHistogram("pt_HH" + suffix, weight, dihiggs.pt());

}

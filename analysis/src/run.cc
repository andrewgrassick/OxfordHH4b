#include "run.h"

#include "settings.h"
#include "utils.h"
#include "analysis.h"

#include "ucl.h"
#include "ucl_vr.h"
#include "durham.h"
#include "oxford_res_vr.h"
#include "oxford_res_fr.h"
#include "oxford_boost_vr.h"
#include "oxford_boost_fr.h"
#include "variableR.h"
#include "amcatnlo.h"
#include "btag_test.h"

#include <vector>
#include <cmath>

// List of samples
static std::vector<eventSample> samples;

	// **************** PLEASE MODIFY  ****************

// Global run parameters
const int nSamples = 3;
const int max_evt = 1E7;
// const int max_evt = 1000;

	// **************** DO NOT MODIFY  ****************

int GetNSamples() {return nSamples;};

eventSample GetSample( int const& isample )
{
	// Populate typical info
	string eventfile;
	string samplename;
	double xsec_norm = 1E3; // pb -> fb
	bool signal;
	bool hepmc;
	int nevt_sample;

	switch (isample)
	{

	// **************** PLEASE MODIFY  ****************
	  case 0: 
	  eventfile="HH_sm_eft_100K.lhe";
	  samplename="diHiggs";
	  signal = true;
	  hepmc = false;
	  nevt_sample = 1E5;
	  break;

	  case 1: 
	  eventfile="SHERPA_QCD_2b2j.hepmc";
	  samplename="SHERPA_QCD2b2j";
	  signal = false;
	  hepmc = true;
	  nevt_sample = 3E6;
	  break;

	  case 2: 
	  eventfile="SHERPA_QCD_4b.hepmc";
	  samplename="SHERPA_QCD4b";
	  signal = false;
	  hepmc = true;
	  nevt_sample = 3E6;
	  break;
	// **************** DO NOT MODIFY  ****************

	  default:
	  std::cout<<"Invalid Monte Carlo sample, exit"<<std::endl;
	  exit(-10);
	  break;
	}

  eventSample newsamp = 
  {
  	eventfile,
  	samplename,
  	xsec_norm,
  	signal,
  	hepmc,
  	fmin(nevt_sample, max_evt)
  };

  return newsamp;
}

void InitAnalyses(	std::vector<Analysis*>& HH4bAnalyses,
					std::vector<Analysis*>& signalAnalyses,
					std::vector<Analysis*>& backgroundAnalyses)
{
	// **************** PLEASE MODIFY  ****************
  HH4bAnalyses.push_back(new AMCAnalysis("total"));


  HH4bAnalyses.push_back(new UCLAnalysis("total")); 
  HH4bAnalyses.push_back(new DurhamAnalysis("total"));
  HH4bAnalyses.push_back(new UCLVRAnalysis("total"));
  HH4bAnalyses.push_back(new OxfordResVRAnalysis("total"));
  HH4bAnalyses.push_back(new OxfordResFRAnalysis("total"));
//   HH4bAnalyses.push_back(new OxfordBoostVRAnalysis("total"));
  HH4bAnalyses.push_back(new OxfordBoostFRAnalysis("total"));

  signalAnalyses.push_back(new UCLAnalysis("signal"));
  signalAnalyses.push_back(new DurhamAnalysis("signal"));
  signalAnalyses.push_back(new UCLVRAnalysis("signal"));
  signalAnalyses.push_back(new OxfordResVRAnalysis("signal"));
  signalAnalyses.push_back(new OxfordResFRAnalysis("signal"));
//   signalAnalyses.push_back(new OxfordBoostVRAnalysis("signal"));
  signalAnalyses.push_back(new OxfordBoostFRAnalysis("signal"));

  backgroundAnalyses.push_back(new UCLAnalysis("background"));
  backgroundAnalyses.push_back(new DurhamAnalysis("background"));
  backgroundAnalyses.push_back(new UCLVRAnalysis("background"));
  backgroundAnalyses.push_back(new OxfordResVRAnalysis("background"));
  backgroundAnalyses.push_back(new OxfordResFRAnalysis("background"));
//   backgroundAnalyses.push_back(new OxfordBoostVRAnalysis("background"));
  backgroundAnalyses.push_back(new OxfordBoostFRAnalysis("background"));

	// **************** DO NOT MODIFY  ****************

}

void InitSampleAnalyses( std::vector<Analysis*>& sampleAnalyses, std::string const& samplename )
{
	// **************** PLEASE MODIFY  ****************
	sampleAnalyses.push_back(new bTagTestAnalysis(samplename));

	sampleAnalyses.push_back(new AMCAnalysis(samplename));
	sampleAnalyses.push_back(new UCLAnalysis(samplename));
	sampleAnalyses.push_back(new DurhamAnalysis(samplename));
	sampleAnalyses.push_back(new UCLVRAnalysis(samplename));
// 	sampleAnalyses.push_back(new OxfordResVRAnalysis(samplename));
	sampleAnalyses.push_back(new OxfordResFRAnalysis(samplename));
	sampleAnalyses.push_back(new OxfordBoostVRAnalysis(samplename));
	sampleAnalyses.push_back(new OxfordBoostFRAnalysis(samplename));
	// **************** DO NOT MODIFY  ****************

}
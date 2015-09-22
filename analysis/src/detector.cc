#include "detector.h"
#include "run.h"
#include "utils.h"
#include "hepmc.h"

#include <math.h> 

#include "fastjet/PseudoJet.hh"
#include "fastjet/contrib/SoftKiller.hh"

#include "Pythia8/Pythia.h"
#include "pythia.h"

using namespace std;

// SHERPA MinBias
static std::ifstream *pileupStream;
static int pileupCount = 0;

// Pythia MinBias
Pythia8::Pythia pythiaPileup;
bool pythiaInit = false;
void initPythiaPileup()
{
  pythiaPileup.readString("Next:numberShowInfo = 0");
  pythiaPileup.readString("Next:numberShowProcess = 0");
  pythiaPileup.readString("Next:numberShowEvent = 0");

  pythiaPileup.readString("Random:setSeed = on");
  pythiaPileup.readString("Random:seed = 10000002");
  pythiaPileup.readString("SoftQCD:all = on");
  pythiaPileup.settings.parm("Beams:eCM", 14000);
  pythiaPileup.init();

  pythiaInit = true;
}

// Detector resolution
static const double phiRes=0.1;
static const double etaRes=0.1;

// SoftKiller
static const fastjet::contrib::SoftKiller soft_killer(2.5, 0.4);

void AddPileup( int const& nPileup, finalState& particles )
{
	if (!pythiaInit) initPythiaPileup();
    
    // Generate a number of pileup events. Add them to sumEvent.
	Pythia8::Event sumEvent;
    for (int iPileup = 0; iPileup < nPileup; ++iPileup) {
      pythiaPileup.next();
      sumEvent += pythiaPileup.event;
    }

    double dummy;
	get_final_state_particles(pythiaPileup, particles, dummy);


	/* SHERPA MinBias
	for ( int iEvent = 0; iEvent < nPileup; iEvent++ )
	{
		// Have to delete stream rather than reset it, HepMC adds some user information
		// to the stream which must be cleared.
		 if (pileupCount >= npileupTotal())
		 {
			delete pileupStream;
			pileupStream = 0;
		 }

		 if (!pileupStream)
		 	pileupStream = new std::ifstream( std::string(SAMPLEDIR) + minBiasFile() );

		 double dummy;
		 get_final_state_particles(*pileupStream, particles, dummy);
		 pileupCount++;
	}
	*/
}

void DetectorSim(finalState input, finalState& output)
{
	if (pileupSimulated())
	{
		AddPileup(npileupEvents(), input);
		input = soft_killer(input);
		//cout << input.size()<<"  "<<sk.size()<<endl;
	}

	for(size_t i=0; i<input.size(); i++)
	{
		// Detector granularity
		const double newEta = floor(input[i].eta()/etaRes)*etaRes + etaRes/2.0;
		const double newPhi = floor(input[i].phi()/phiRes)*phiRes + phiRes/2.0;

		// Lengthwise gaussian smear
		const double sm = box_muller(1.0,0.01*GetESmear());

		// Reconstruct smeared jet
		const double pT = sm*input[i].pt();
		const double px = pT*cos(newPhi);
		const double py = pT*sin(newPhi);
		const double pz = pT*sinh(newEta);
		const double E = sqrt(input[i].m2()+px*px + py*py + pz*pz);

 		// Form PseudoJet
    	fastjet::PseudoJet jet(px, py, pz, E);
    	jet.set_user_index(input[i].user_index());
    	if (input[i].has_user_info())
    		jet.set_user_info(new JetInfo(input[i].user_info<JetInfo>()));

    	output.push_back(jet);
	}
}
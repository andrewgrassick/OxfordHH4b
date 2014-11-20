// ucl.h
#pragma once

#include "analysis.h"

#include <iostream>
#include <vector>


/*
This is a basic xsec analysis according to the aMC@NLO paper cuts
 */
class AMCAnalysis : public Analysis
{
	public:
		AMCAnalysis(std::string const& sampleName);

		void Analyse(bool const& signal, double const& weight_norm, finalState const&);

	private:
};
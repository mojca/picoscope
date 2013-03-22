#include <cmath>
#include <vector>

using namespace std;

// possible values for n-gamma discrimination with dt=200ps:
//   length: 650, dt1: 175, length: 250
// we assume that the signals are negative
template<typename T> void calculate_and_write_integrals_i(const std::vector<T>inData, unsigned int integral2_dt1, unsigned int integral2_length, FILE *f)
{
	unsigned int i_peak = 0;
	unsigned int i_trigger_half_peak;
	unsigned int i_start_integrate2;
	bool problem = false;

	// TODO: it could happen that inData was double, so integrals would have to be double as well
	// and it would be nice to have a function that would work with both,
	// but as long as inData can be 8bit characters, we cannot afford to use "T" type for the integral
	// NB: we cannot be sure that int is ok for this!!!
	int integral1 = 0, integral2 = 0;

	// calculate the minimum (negative maximum first)
	for(unsigned int i=0; i<static_cast<unsigned int>(inData.size()); i++) {
		integral1 -= inData[i];
		if(inData[i] < inData[i_peak]) {
			i_peak = i;
		}
		// overflow
		// TODO: fix this
		if(inData[i] == -127)
			problem = true;
	}
	if(i_peak == 0)
		problem = true;
	// find the last point before the peak that exceeds half the size of the peak
	// i_trigger_half_peak=i_peak-1;
	// while(inData[i_trigger_half_peak]*2 >= inData[i_peak] && i_trigger_half_peak > 0) i_trigger_half_peak--;
	// i_trigger_half_peak++;
	for(i_trigger_half_peak=0; inData[i_trigger_half_peak+1]*2.0 >= inData[i_peak] && i_trigger_half_peak < i_peak; i_trigger_half_peak++);

	// if i_trigger_half_peak==0, something must be wrong!!!
	if(i_trigger_half_peak < 4 || i_trigger_half_peak+1 >= i_peak)
		problem = true;

	// where to start integrating for the second integral
	i_start_integrate2 = i_trigger_half_peak + integral2_dt1;
	if(i_start_integrate2 + integral2_length > static_cast<unsigned int>(inData.size())) {
		problem = true;
	} else {
		for(unsigned int i=0; i<integral2_length; i++) {
			integral2 -= inData[i_start_integrate2+i];
		}
	}
	// * first integral - sum of all points
	// * second integral - discriminating between gammas and neutrons
	// * maximum - should correlate with the first integral
	// * i_start_integrate2 - should not be an outlier
	// * "-1" if there was a problem
	fprintf(f, "%d\t%d\t%d\t%d\t%d\t%d\n", integral1, integral2, -inData[i_peak], i_peak, i_start_integrate2, problem ? 1 : 0);
}

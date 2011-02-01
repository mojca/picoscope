
Measurement::Measurement(Picoscope pico) {
	
}

// TODO: improve this; check for number of channels
void Measurement::EnableChannels(bool a, bool b, bool c, bool d) {
	/*if(getSeries() == PICO_4000) {
		if(a) channels[PS4000_CHANNEL_A].Enable(); else channels[PS4000_CHANNEL_A].Disable();
		if(b) channels[PS4000_CHANNEL_B].Enable(); else channels[PS4000_CHANNEL_B].Disable();
		if(c) channels[PS4000_CHANNEL_C].Enable(); else channels[PS4000_CHANNEL_C].Disable();
		if(d) channels[PS4000_CHANNEL_D].Enable(); else channels[PS4000_CHANNEL_D].Disable();
	}*/
	if(a) channels[0].Enable(); else channels[0].Disable();
	if(b) channels[1].Enable(); else channels[1].Disable();
	if(c) channels[2].Enable(); else channels[2].Disable();
	if(d) channels[3].Enable(); else channels[3].Disable();
}

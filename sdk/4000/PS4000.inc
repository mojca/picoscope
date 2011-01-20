type 
		TRIGGER_CONDITIONS Record
			channelA : Integer;
			channelB : Integer;
			channelC : Integer;
			channelD : Integer;
			external : Integer;
			aux : Integer;
			pulseWidthQualifier : Integer;
end;

type
		PWQ_CONDITIONS Record
			channelA : Integer;
			channelB : Integer;
			channelC : Integer;
			channelD : Integer;
			external : Integer;
			aux : Integer;
end;

type
		TRIGGER_CHANNEL_PROPERTIES Record
		  thresholdUpper : smallint;
			thresholdUpperHysteresis : Word;
		  thresholdLower : smallint;
	    thresholdLowerHysteresis : Word;
	    channel : Integer;
      thresholdMode : Integer;
end;

ps4000StreamingReady = procedure
	(
		handle : smallint;
		noOfSamples : Integer;
		startIndex : LongWord;
		overflow : smallint;
		triggerAt : LongWord;
		triggered : smallint;
		autoStop : smallint;
		pParameter : Pointer
	); 
	{$IFDEF WIN32} stdcall; {$ENDIF}

ps4000DataReady = procedure
	(
		handle : smallint;
		noOfSamples: Integer;
		overflow : smallint;
		triggerAt: LongWord;
		triggered : smallint;
		pParameter : Pointer
	); 
	{$IFDEF WIN32} stdcall; {$ENDIF}

function ps4000OpenUnit
  (
	  handle : Pointer
	) : Integer;
{$IFDEF WIN32} stdcall; external 'ps4000.dll'{$ENDIF}


function ps4000OpenUnitAsync
  (
	  status : Pointer
	) : Integer;
{$IFDEF WIN32} stdcall; external 'ps4000.dll'{$ENDIF}

function ps4000OpenUnitEx 
  (
	  handle : Pointer;
		serial : Pointer
	) : Integer;
{$IFDEF WIN32} stdcall; external 'ps4000.dll'{$ENDIF}

function ps4000OpenUnitAsyncEx
  (
	  status : Pointer;
		serial : Pointer
	) : Integer;
{$IFDEF WIN32} stdcall; external 'ps4000.dll'{$ENDIF}


function ps4000OpenUnitProgress
	(
	  handle : Pointer;
	  progressPercent : Pointer;
	  complete : Pointer
	) : Integer; 
{$IFDEF WIN32} stdcall; external 'ps4000.dll'{$ENDIF}

function ps4000GetUnitInfo
 	(
	  handle : smallint;
	  string : Pointer;
	  stringLength : smallint;
	  requiredSize : Pointer;
	  info : Integer
	) : Integer;
{$IFDEF WIN32} stdcall; external 'ps4000.dll'{$ENDIF}

function ps4000FlashLed
	(
	  handle : smallint;
		start : smallint
	) : Integer;
{$IFDEF WIN32} stdcall; external 'ps4000.dll'{$ENDIF}

function ps4000IsLedFlashing
	(
	  handle : smallint;
		status : Pointer
	) : Integer;
{$IFDEF WIN32} stdcall; external 'ps4000.dll'{$ENDIF}

function ps4000CloseUnit
	(
	  handle : smallint
	) : Integer;
{$IFDEF WIN32} stdcall; external 'ps4000.dll'{$ENDIF}

function ps4000MemorySegments
	(
	  handle : smallint;
		nSegments : Word;
		nMaxSamples : Integer
	) : Integer;
{$IFDEF WIN32} stdcall; external 'ps4000.dll'{$ENDIF}

function ps4000SetChannel
 	(
	  handle : smallint;
		channel : Integer;
	  enabled : smallint;
	  dc : smallint;
		range : Integer
	) : Integer;
{$IFDEF WIN32} stdcall; external 'ps4000.dll'{$ENDIF}


function ps4000SetNoOfCaptures 
	(
	handle  : smallint;
	nCaptures : Word
	) : Integer;
{$IFDEF WIN32} stdcall; external 'ps4000.dll'{$ENDIF}

function ps4000GetTimebase
	(
	   handle : smallint;
	   timebase : LongWord;
	   noSamples : Integer;
	   timeIntervalNanoseconds : Pointer;
	   oversample : smallint;
		 maxSamples : Pointer;
		 segmentIndex : Word
	) : Integer;
{$IFDEF WIN32} stdcall; external 'ps4000.dll'{$ENDIF}

function ps4000GetTimebase2
	(
	   handle : smallint;
	   timebase : LongWord;
	   noSamples : Integer;
	   timeIntervalNanoseconds : Pointer;
	   oversample : smallint;
		 maxSamples : Pointer;
		 segmentIndex : Word
	) : Integer;
{$IFDEF WIN32} stdcall; external 'ps4000.dll'{$ENDIF}

function ps4000SetSigGenArbitrary
	(
	 	handle : smallint;
	 	offsetVoltage : Integer;
	 	pkToPk : LongWord;
	 	startDeltaPhase : LongWord;
	 	stopDeltaPhase : LongWord;
	 	deltaPhaseIncrement : LongWord;
	 	dwellCount : LongWord;
	 	arbitraryWaveform : Pointer;
	 	arbitraryWaveformSize : Integer;
		sweepType : Integer;
		whiteNoise : smallint;
		indexMode : Integer;
		shots : LongWord;
		sweeps : LongWord;
		triggerType : Integer;
		triggerSource : Integer;
		extInThreshold : smallint
	) : Integer;
{$IFDEF WIN32} stdcall; external 'ps4000.dll'{$ENDIF}

function ps4000SetSigGenBuiltIn
	(
		handle : smallint;
		offsetVoltage : Integer;
		pkToPk : LongWord;
		waveType : smallint;
		startFrequency : Single;
		stopFrequency : Single;
		increment : Single;
		dwellTime : Single;
		sweepType : Integer;
		whiteNoise : smallint;
		shots : LongWord;
		sweeps : LongWord;
		triggerType : Integer;
		triggerSource : Integer;
		extInThreshold : smallint
		) : Integer;
{$IFDEF WIN32} stdcall; external 'ps4000.dll'{$ENDIF}

function ps4000SigGenSoftwareControl
	(
		handle : smallint;
		state : smallint
	) : Integer;
{$IFDEF WIN32} stdcall; external 'ps4000.dll'{$ENDIF}

function ps4000SetEts
  (
		handle : smallint;
		mode : Integer;
		etsCycles : smallint;
		etsInterleave : smallint;
		sampleTimePicoseconds : Pointer
	) : Integer;
{$IFDEF WIN32} stdcall; external 'ps4000.dll'{$ENDIF}


function ps4000SetTriggerChannelProperties
	(
		handle : smallint;
		channelProperties : TRIGGER_CHANNEL_PROPERTIES;
		nChannelProperties : smallint;
		auxOutputEnable : smallint;
		autoTriggerMilliseconds : Integer
	) : Integer;
{$IFDEF WIN32} stdcall; external 'ps4000.dll'{$ENDIF}


function ps4000SetTriggerChannelConditions
	(
		handle : smallint;
		conditions : Pointer;
		nConditions : smallint
	) : Integer;
{$IFDEF WIN32} stdcall; external 'ps4000.dll'{$ENDIF}

function ps4000SetTriggerChannelDirections
	(
		handle : smallint;
		channelA : Integer;
		channelB : Integer;
		channelC : Integer;
		channelD : Integer;
		ext : Integer;
		aux : Integer
	) : Integer;
{$IFDEF WIN32} stdcall; external 'ps4000.dll'{$ENDIF}

function ps4000SetTriggerDelay
	(
		handle : smallint;
		delay : LongWord
	) : Integer;
{$IFDEF WIN32} stdcall; external 'ps4000.dll'{$ENDIF}

function ps4000SetPulseWidthQualifier
	(
		handle : smallint;
		conditions : PWQ_CONDITIONS;
		nConditions : smallint;
		direction : Integer;
		lower : LongWord;
		upper : LongWord;
		type : Integer
	) : Integer;
{$IFDEF WIN32} stdcall; external 'ps4000.dll'{$ENDIF}

function ps4000IsTriggerOrPulseWidthQualifierEnabled
	(
		handle : smallint;
		* triggerEnabled  : Pointer;
		* pulseWidthQualifierEnabled : Pointer
	) : Integer;
{$IFDEF WIN32} stdcall; external 'ps4000.dll'{$ENDIF}

function ps4000GetTriggerTimeOffset
	(
		handle : smallint;
		timeUpper : Pointer;
		timeLower : Pointer;
		timeUnits : Pointer;
		segmentIndex : Word
	) : Integer;
{$IFDEF WIN32} stdcall; external 'ps4000.dll'{$ENDIF}


function ps4000GetTriggerTimeOffset64
	(
		handle : smallint;
		time : Pointer;
		timeUnits : Pointer;
		segmentIndex : Word
	) : Integer;
{$IFDEF WIN32} stdcall; external 'ps4000.dll'{$ENDIF}

function ps4000GetValuesTriggerTimeOffsetBulk
	(
	  handle : smallint;
		timesUpper : Pointer;
		timesLower : Pointer;
		timeUnits : Pointer;
		fromSegmentIndex : Word;
		toSegmentIndex : Word
	) : Integer;
{$IFDEF WIN32} stdcall; external 'ps4000.dll'{$ENDIF}

function ps4000GetValuesTriggerTimeOffsetBulk64
	(
	  handle : smallint;
		times : Pointer;
		timeUnits : Pointer;
		fromSegmentIndex : Word;
		toSegmentIndex : Word
	) : Integer;
{$IFDEF WIN32} stdcall; external 'ps4000.dll'{$ENDIF}

function ps4000SetDataBufferBulk
(
   handle : smallint;
	 channel : Integer;
	 buffer : Pointer;
   bufferLth : Integer;
	 waveform : Word
) : Integer;
{$IFDEF WIN32} stdcall; external 'ps4000.dll'{$ENDIF}


function ps4000SetDataBuffers
(
   handle : smallint;
	 channel : Integer;
	 bufferMax : Pointer;
	 bufferMin : Pointer;
   bufferLth : Integer
) : Integer;
{$IFDEF WIN32} stdcall; external 'ps4000.dll'{$ENDIF}

function ps4000SetDataBufferWithMode
(
   handle : smallint;
	 channel : Integer;
	 buffer : Pointer;
   bufferLth : Integer;
	 mode : Integer
) : Integer;
{$IFDEF WIN32} stdcall; external 'ps4000.dll'{$ENDIF}

function ps4000SetDataBuffersWithMode
(
   handle : smallint;
	 channel : Integer;
	 bufferMax : Pointer;
	 bufferMin : Pointer;
   bufferLth : Integer;
	 mode : Integer
) : Integer;
{$IFDEF WIN32} stdcall; external 'ps4000.dll'{$ENDIF}

function ps4000SetDataBuffer
(
   handle : smallint;
	 channel : Integer;
	 buffer : Pointer;
   bufferLth : Integer
) : Integer;
{$IFDEF WIN32} stdcall; external 'ps4000.dll'{$ENDIF}

function ps4000SetEtsTimeBuffer
(
   handle : smallint;
	 buffer : Pointer;
	 bufferLth : Integer
) : Integer;
{$IFDEF WIN32} stdcall; external 'ps4000.dll'{$ENDIF}

function ps4000SetEtsTimeBuffers
(
   handle : smallint;
	 timeUpper : Pointer;
	 timeLower : Pointer;
	 bufferLth : Integer
) : Integer;
{$IFDEF WIN32} stdcall; external 'ps4000.dll'{$ENDIF}

function ps4000RunBlock
	(
		handle : smallint;
		noOfPreTriggerSamples : Integer;
		noOfPostTriggerSamples : Integer;
		timebase : LongWord;
		oversample : smallint;
		timeIndisposedMs : Pointer;
		segmentIndex : Word;
		lpReady : ps4000BlockReady;
		pParameter : Pointer;
	) : Integer;
{$IFDEF WIN32} stdcall; external 'ps4000.dll'{$ENDIF}

function ps4000RunStreaming
  (
	  handle : smallint;
		sampleInterval : Pointer;
		sampleIntervalTimeUnits : Integer;
	  maxPreTriggerSamples : LongWord;
	  maxPostPreTriggerSamples : LongWord;
		autoStop : smallint;
		downSampleRatio : LongWord;
    overviewBufferSize : LongWord
	) : Integer;
{$IFDEF WIN32} stdcall; external 'ps4000.dll'{$ENDIF}

function ps4000RunStreamingEx
  (
	  handle : smallint;
		sampleInterval : Pointer;
		sampleIntervalTimeUnits : Integer;
	  maxPreTriggerSamples : LongWord;
	  maxPostPreTriggerSamples : LongWord;
		autoStop : smallint;
		downSampleRatio : LongWord;
		downSampleRatioMode : smallint;
    overviewBufferSize : LongWord
	) : Integer;
{$IFDEF WIN32} stdcall; external 'ps4000.dll'{$ENDIF}

function ps4000GetStreamingLatestValues
  (
    handle : smallint; 
    lpPs4000Ready : ps4000StreamingReady;
		pParameter : Pointer
  ) : Integer;  
{$IFDEF WIN32} stdcall; external 'ps4000.dll'{$ENDIF}

function ps4000NoOfStreamingValues
	(
	  handle : smallint;
		noOfValues : Pointer
	) : Integer;
{$IFDEF WIN32} stdcall; external 'ps4000.dll'{$ENDIF}

function ps4000GetMaxDownSampleRatio
	(
	  handle : smallint;
		noOfUnaggreatedSamples : LongWord;
		maxDownSampleRatio : Pointer;
		downSampleRatioMode : smallint;
		segmentIndex : Word
	) : Integer;
{$IFDEF WIN32} stdcall; external 'ps4000.dll'{$ENDIF}

function ps4000GetValues
	(
	  handle : smallint;
		startIndex : LongWord;
	  noOfSamples : Pointer;
	  downSampleRatio : LongWord;
		downSampleRatioMode : smallint;
		segmentIndex : Word;
		overflow : Pointer
	) : Integer;
{$IFDEF WIN32} stdcall; external 'ps4000.dll'{$ENDIF}

function ps4000GetValuesBulk
	(
	  handle : smallint;
		noOfSamples : Pointer;
		fromSegmentIndex : Word;
		toSegmentIndex : Word;
		overflow : Pointer
	) : Integer;
{$IFDEF WIN32} stdcall; external 'ps4000.dll'{$ENDIF}

function ps4000GetValuesAsync
	(
	  handle : smallint;
		startIndex : LongWord;
	  noOfSamples : LongWord;
	  downSampleRatio : LongWord;
		downSampleRatioMode : smallint;
		segmentIndex : Word;
	  lpDataReady : Pointer;
		pParameter : Pointer
	) : Integer;
{$IFDEF WIN32} stdcall; external 'ps4000.dll'{$ENDIF}

function ps4000Stop
	(
	  handle : smallint
	) : Integer;
{$IFDEF WIN32} stdcall; external 'ps4000.dll'{$ENDIF}

function ps4000SetProbe
	(
	  handle : smallint;
		probe : Integer;
		range : Integer
	) : Integer;
{$IFDEF WIN32} stdcall; external 'ps4000.dll'{$ENDIF}

function ps4000HoldOff
	(
	  handle : smallint;	
		holdoff : Int64;
		type : Integer
	) : Integer;
{$IFDEF WIN32} stdcall; external 'ps4000.dll'{$ENDIF}

function ps4000GetProbe
	(
	  handle : smallint;
		probe : Pointer
	) : Integer;
{$IFDEF WIN32} stdcall; external 'ps4000.dll'{$ENDIF}

function ps4000GetChannelInformation 
	(
		handle : smallint; 
		info : Integer;
		probe : Integer; 
		ranges : Pointer;
		length : Pointer;
		channels : Integer
  ) : Integer;
{$IFDEF WIN32} stdcall; external 'ps4000.dll'{$ENDIF}

function ps4000SetFrequencyCounter
  (
    handle : smallint; 
    channel : Integer; 
    enabled : smallint;
    range : Integer; 
    thresholdMajor : smallint; 
    thresholdMinor : smallint
  ) : Integer;
{$IFDEF WIN32} stdcall; external 'ps4000.dll'{$ENDIF}

function ps4000EnumerateUnits
	(
		count : Pointer;
		serials : Pointer;
		serialLth : Pointer
	) : Integer;
{$IFDEF WIN32} stdcall; external 'ps4000.dll'{$ENDIF}
/**************************************************************************
*
* Filename:    PS6000ConsoleExample.cs
*
* Copyright:   Pico Technology Limited 2010
*
* Author:      MJL
*
* Description:
*   This is a console-mode program that demonstrates how to use the
*   PS6000 driver using .NET
*
* Examples:
*    Collect a block of samples immediately
*    Collect a block of samples when a trigger event occurs
*    Collect a stream of data immediately
*    Collect a stream of data when a trigger event occurs
*
* History:
*     14Dec06	MJL	Created for PS5000
*	  04Jan10	RPM Modified for PS6000
*	  07Mar12   CPY Added Siggen example Enabled A / B variants. Reformatted outputs for Dlock & Streaming data handlers
*
* Revision Info: "file %n date %f revision %v"
*						""
*
***************************************************************************/

using System;
using System.IO;
using System.Threading;

namespace ps6000example
{
  struct ChannelSettings
  {
    public Imports.PS6000Coupling DCcoupled;
    public Imports.Range range;
    public bool enabled;
  }

  class Pwq
  {
    public Imports.PwqConditions[] conditions;
    public short nConditions;
    public Imports.ThresholdDirection direction;
    public uint lower;
    public uint upper;
    public Imports.PulseWidthType type;

    public Pwq(Imports.PwqConditions[] conditions,
        short nConditions,
        Imports.ThresholdDirection direction,
        uint lower, uint upper,
        Imports.PulseWidthType type)
    {
      this.conditions = conditions;
      this.nConditions = nConditions;
      this.direction = direction;
      this.lower = lower;
      this.upper = upper;
      this.type = type;
    }
  }

  class ConsoleExample
  {
    private readonly short _handle;
    public const int BUFFER_SIZE = 1024;
    public const int MAX_CHANNELS = 4;
    public const int QUAD_SCOPE = 4;
    public const int DUAL_SCOPE = 2;

    uint _timebase = 8;
    short _oversample = 1;
    bool _scaleVoltages = true;

    ushort[] inputRanges = { 10, 20, 50, 100, 200, 500, 1000, 2000, 5000, 10000, 20000, 50000 };
    bool _ready = false;
    short _trig = 0;
    uint _trigAt = 0;
    int _sampleCount;
    uint _startIndex;
    bool _autoStop;

    private ChannelSettings [] _channelSettings;
    private int _channelCount;
    private Imports.Range _firstRange;
    private Imports.Range _lastRange;
    private Imports.ps6000BlockReady _callbackDelegate;
    private bool _AWG;

    /****************************************************************************
     * Callback
     * used by PS6000 data streaimng collection calls, on receipt of data.
     * used to set global flags etc checked by user routines
     ****************************************************************************/
    void StreamingCallback(short handle,
                            int noOfSamples,
                            uint startIndex,
                            short ov,
                            uint triggerAt,
                            short triggered,
                            short autoStop,
                            IntPtr pVoid)
    {
      // used for streaming
      _sampleCount = noOfSamples;
      _startIndex = startIndex;
      _autoStop = autoStop != 0;

      // flag to say done reading data
      _ready = true;

      // flags to show if & where a trigger has occurred
      _trig = triggered;
      _trigAt = triggerAt;
    }

    /****************************************************************************
     * Callback
     * used by PS6000 data block collection calls, on receipt of data.
     * used to set global flags etc checked by user routines
     ****************************************************************************/
    void BlockCallback(short handle, short status, IntPtr pVoid)
    {
      // flag to say done reading data
      _ready = true;
    }

    /****************************************************************************
     * SetDefaults - restore default settings
     ****************************************************************************/
    void SetDefaults()
    {
       short status;

      for (int i = 0; i < _channelCount; i++) // reset channels to most recent settings
      {
        status = Imports.SetChannel(_handle, Imports.Channel.ChannelA + i,
                           (short)(_channelSettings[(int)(Imports.Channel.ChannelA + i)].enabled ? 1 : 0),
                            _channelSettings[i].DCcoupled,
                           _channelSettings[(int)(Imports.Channel.ChannelA + i)].range, 0, Imports.PS6000BandwidthLimiter.PS6000_BW_FULL);
      }
    }

    /****************************************************************************
     * adc_to_mv
     *
     * Convert an 16-bit ADC count into millivolts
     ****************************************************************************/
    int adc_to_mv(int raw, int ch)
    {
      return (raw * inputRanges[ch]) / Imports.MaxValue;
    }

    /****************************************************************************
     * mv_to_adc
     *
     * Convert a millivolt value into a 16-bit ADC count
     *
     *  (useful for setting trigger thresholds)
     ****************************************************************************/
    short mv_to_adc(short mv, short ch)
    {
      return (short)((mv * Imports.MaxValue) / inputRanges[ch]);
    }

    /****************************************************************************
     * BlockDataHandler
     * - Used by all block data routines
     * - acquires data (user sets trigger mode before calling), displays 10 items
     *   and saves all to data.txt
     * Input :
     * - unit : the unit to use.
     * - text : the text to display before the display of data slice
     * - offset : the offset into the data buffer to start the display's slice.
     ****************************************************************************/
    void BlockDataHandler(string text, int offset)
    {
      short status;
      uint sampleCount = BUFFER_SIZE;      
      PinnedArray<short>[] minPinned = new PinnedArray<short>[_channelCount];
      PinnedArray<short>[] maxPinned = new PinnedArray<short>[_channelCount];

      int timeIndisposed;

      for (int i = 0; i < _channelCount; i++)
      {
        short [] minBuffers = new short[sampleCount];
        short [] maxBuffers = new short[sampleCount];
        minPinned[i] = new PinnedArray<short>(minBuffers);      
        maxPinned[i] = new PinnedArray<short>(maxBuffers);
        status = Imports.SetDataBuffers(_handle, (Imports.Channel)i, maxBuffers, minBuffers, (int)sampleCount, Imports.PS6000DownSampleRatioMode.PS6000_RATIO_MODE_NONE);
      }

      /*  find the maximum number of samples, the time interval (in timeUnits),
       *		 the most suitable time units, and the maximum _oversample at the current _timebase*/
      int timeInterval;
      int maxSamples;
      while (Imports.GetTimebase(_handle, _timebase, (int)sampleCount, out timeInterval, _oversample, out maxSamples, 0) != 0)
      {
        _timebase++;
      }
      Console.WriteLine("Timebase: {0}\toversample:{1}", _timebase, _oversample);

      /* Start it collecting, then wait for completion*/
      _ready = false;
      _callbackDelegate = BlockCallback;
      status = Imports.RunBlock(_handle, 0, (int)sampleCount, _timebase, _oversample, out timeIndisposed, 0, _callbackDelegate,
                                     IntPtr.Zero);

     
      Console.WriteLine("Waiting for data...Press a key to abort");

      while (!_ready && !Console.KeyAvailable)
      {
        Thread.Sleep(100);
      }
      if(Console.KeyAvailable) Console.ReadKey(true); // clear the key

      Imports.Stop(_handle);

      if (_ready)
      {
        short overflow;
        status = Imports.GetValues(_handle, 0, ref sampleCount, 1, Imports.PS6000DownSampleRatioMode.PS6000_RATIO_MODE_NONE, 0, out overflow);

        /* Print out the first 10 readings, converting the readings to mV if required */
        Console.WriteLine(text);
        Console.WriteLine("Value {0}", (_scaleVoltages) ? ("mV") : ("ADC Counts"));

        for (int ch = 0; ch < _channelCount; ch++)
        {
            if (_channelSettings[ch].enabled)
                Console.Write("   Ch{0}    ", (char)('A' + ch));
        }
        Console.WriteLine();

        for (int i = offset; i < offset + 10; i++)
        {
          for (int ch = 0; ch < _channelCount; ch++)
          {
            if (_channelSettings[ch].enabled)
            {
                Console.Write("{0,6}    ", _scaleVoltages ?
                                  adc_to_mv(maxPinned[ch].Target[i], (int)_channelSettings[(int)(Imports.Channel.ChannelA + ch)].range)  // If _scaleVoltages, show mV values
                                  : maxPinned[ch].Target[i]);                                                                           // else show ADC counts
            }
          }
          Console.WriteLine();
        }

        sampleCount = Math.Min(sampleCount, BUFFER_SIZE);
        TextWriter writer = new StreamWriter("block.txt", false);

        writer.Write("For each of the enabled Channels, results shown are....");
        writer.WriteLine();
        writer.WriteLine("Time interval Maximum Aggregated value ADC Count & mV, Minimum Aggregated value ADC Count & mV");
        writer.WriteLine();

        writer.Write("Time  ");
        for (int ch = 0; ch < _channelCount; ch++)
            if (_channelSettings[ch].enabled)
                writer.Write("Ch  Max ADC    Max mV   Min ADC    Min mV   ");
        writer.WriteLine();

        for (int i = 0; i < sampleCount; i++)
        {
          writer.Write("{0,4}  ", (i * timeInterval));
          for (int ch = 0; ch < _channelCount; ch++)
          {
              if (_channelSettings[ch].enabled)
              {
                  writer.Write("Ch{0} {1,7}   {2,7}   {3,7}   {4,7}   ",
                                 (char)('A' + ch),
                                 maxPinned[ch].Target[i],
                                 adc_to_mv(maxPinned[ch].Target[i],
                                           (int)_channelSettings[(int)(Imports.Channel.ChannelA + ch)].range),
                                 minPinned[ch].Target[i],
                                 adc_to_mv(minPinned[ch].Target[i],
                                           (int)_channelSettings[(int)(Imports.Channel.ChannelA + ch)].range));
              }
          }
          writer.WriteLine();
        }
        writer.Close();
      }
      else
      {
        Console.WriteLine("data collection aborted");
        WaitForKey();
      }
      foreach (PinnedArray<short> p in minPinned)
      {
          if (p != null)
              p.Dispose();
      }
      foreach (PinnedArray<short> p in maxPinned)
      {
          if (p != null)
              p.Dispose();
      }
    }



    /****************************************************************************
     * RapidBlockDataHandler
     * - Used by all the CollectBlockRapid routine
     * - acquires data (user sets trigger mode before calling), displays 10 items
     * Input :
     * - nRapidCaptures : the user specified number of blocks to capture
     ****************************************************************************/
    private void RapidBlockDataHandler(uint nRapidCaptures)
    {
        short status;
        int numChannels = _channelCount;
        uint numSamples = BUFFER_SIZE;

        // Run the rapid block capture
        int timeIndisposed;
        _ready = false;


        //  find the maximum number of samples, the time interval (in timeUnits),
      	//	 the most suitable time units, and the maximum _oversample at the current _timebase
        int timeInterval;
        int maxSamples;
        while (Imports.GetTimebase(_handle, _timebase, (int)numSamples, out timeInterval, _oversample, out maxSamples, 0) != 0)
        {
            _timebase++;
        }
        Console.WriteLine("Timebase: {0}\toversample:{1}", _timebase, _oversample);


        _callbackDelegate = BlockCallback;
        Imports.RunBlock(_handle,
                    0,
                    (int)numSamples,
                    _timebase,
                    _oversample,
                    out timeIndisposed,
                    0,
                    _callbackDelegate,
                    IntPtr.Zero);


        Console.WriteLine("Waiting for data...Press a key to abort");

        while (!_ready && !Console.KeyAvailable)
        {
            Thread.Sleep(100);
        }
        if (Console.KeyAvailable) Console.ReadKey(true); // clear the key

        Imports.Stop(_handle);


        // Set up the data arrays and pin them
        short[][][] values = new short[nRapidCaptures][][];
        PinnedArray<short>[,] pinned = new PinnedArray<short>[nRapidCaptures, numChannels];

        for (ushort segment = 0; segment < nRapidCaptures; segment++)
        {
            values[segment] = new short[numChannels][];
            for (short channel = 0; channel < numChannels; channel++)
            {
                if (_channelSettings[channel].enabled)
                {
                    values[segment][channel] = new short[numSamples];
                    pinned[segment, channel] = new PinnedArray<short>(values[segment][channel]);

                    status = Imports.SetDataBuffersRapid(_handle,
                                           (Imports.Channel)channel,
                                           values[segment][channel],
                                           (int)numSamples,
                                           segment,
                                           Imports.PS6000DownSampleRatioMode.PS6000_RATIO_MODE_NONE);
                }
                else
                {
                    status = Imports.SetDataBuffersRapid(_handle,
                               (Imports.Channel)channel,
                                null,
                                0,
                                segment,
                                Imports.PS6000DownSampleRatioMode.PS6000_RATIO_MODE_NONE);

                }
            }
        }

        // Read the data
        short[] overflows = new short[nRapidCaptures];

        status = Imports.GetValuesRapid(_handle, ref numSamples, 0, nRapidCaptures - 1, 1, Imports.PS6000DownSampleRatioMode.PS6000_RATIO_MODE_NONE, overflows);

        /* Print out the first 10 readings, converting the readings to mV if required */
        Console.WriteLine("\nValues in {0}", (_scaleVoltages) ? ("mV") : ("ADC Counts"));

        for (int seg = 0; seg < nRapidCaptures; seg++)
        {
            Console.WriteLine("Capture {0}", seg);

            for (int ch = 0; ch < _channelCount; ch++)
            {
                if (_channelSettings[ch].enabled)
                    Console.Write("  Ch{0}   ", (char)('A' + ch));
            }
            Console.WriteLine();

            for (int i = 0; i < 10; i++)
            {
                for (int ch = 0; ch < _channelCount; ch++)
                {
                    if (_channelSettings[ch].enabled)
                    {
                        Console.Write("{0,6}\t", _scaleVoltages ?
                                            adc_to_mv(pinned[seg, ch].Target[i], (int)_channelSettings[(int)(Imports.Channel.ChannelA + ch)].range) // If _scaleVoltages, show mV values
                                            : pinned[seg, ch].Target[i]);                                                                             // else show ADC counts
                    }
                }
                Console.WriteLine();
            }
            Console.WriteLine();
        }

        // Un-pin the arrays
        foreach (PinnedArray<short> p in pinned)
        {
            if (p != null)
                p.Dispose();
        }
    }



    /****************************************************************************
     *  SetTrigger
     *  this function sets all the required trigger parameters, and calls the 
     *  triggering functions
     ****************************************************************************/
    short SetTrigger(Imports.TriggerChannelProperties[] channelProperties, short nChannelProperties, Imports.TriggerConditions[] triggerConditions, short nTriggerConditions, Imports.ThresholdDirection[] directions, Pwq pwq, uint delay, short auxOutputEnabled, int autoTriggerMs)
    {
      short status;

      if (
        (status =
         Imports.SetTriggerChannelProperties(_handle, channelProperties, nChannelProperties, auxOutputEnabled,
                                             autoTriggerMs)) != 0)
      {
        return status;
      }

      if ((status = Imports.SetTriggerChannelConditions(_handle, triggerConditions, nTriggerConditions)) != 0)
      {
        return status;
      }

      if (directions == null) directions = new Imports.ThresholdDirection[] { Imports.ThresholdDirection.None, 
        Imports.ThresholdDirection.None, Imports.ThresholdDirection.None, Imports.ThresholdDirection.None, 
        Imports.ThresholdDirection.None, Imports.ThresholdDirection.None};

      if ((status = Imports.SetTriggerChannelDirections(_handle,
                                                        directions[(int)Imports.Channel.ChannelA],
                                                        directions[(int)Imports.Channel.ChannelB],
                                                        directions[(int)Imports.Channel.ChannelC],
                                                        directions[(int)Imports.Channel.ChannelD],
                                                        directions[(int)Imports.Channel.External],
                                                        directions[(int)Imports.Channel.Aux])) != 0)
      {
        return status;
      }

      if ((status = Imports.SetTriggerDelay(_handle, delay)) != 0)
      {
        return status;
      }

      if (pwq == null) pwq = new Pwq(null, 0, Imports.ThresholdDirection.None, 0, 0, Imports.PulseWidthType.None);

      status = Imports.SetPulseWidthQualifier(_handle, pwq.conditions,
                                              pwq.nConditions, pwq.direction,
                                              pwq.lower, pwq.upper, pwq.type);

      return status;
    }


    /****************************************************************************
     * CollectBlockImmediate
     *  this function demonstrates how to collect a single block of data
     *  from the unit (start collecting immediately)
     ****************************************************************************/
    void CollectBlockImmediate()
    {
      Console.WriteLine("Collect block immediate...");
      Console.WriteLine("Press a key to start");
      WaitForKey();

      SetDefaults();

      /* Trigger disabled	*/
      SetTrigger(null, 0, null, 0, null, null, 0, 0, 0);

      BlockDataHandler("First 10 readings", 0);
    }


    /****************************************************************************
    *  CollectBlockRapid
    *  this function demonstrates how to collect blocks of data
    * using the RapidCapture function
    ****************************************************************************/
    void CollectBlockRapid()
    {

      ushort numRapidCaptures;
      short status;

      Console.WriteLine("Collect rapid block...");
      Console.WriteLine("Specify number of captures:");
      do
      {
        numRapidCaptures = ushort.Parse(Console.ReadLine());
      } while (Imports.SetNoOfRapidCaptures(_handle, numRapidCaptures) > 0);

      int maxSamples;
      status = Imports.MemorySegments(_handle, numRapidCaptures, out maxSamples);
      Console.WriteLine(status!=0?"Error:" + status: "");

      Console.WriteLine("Collecting {0} rapid blocks. Press a key to start", numRapidCaptures);

      WaitForKey();

      SetDefaults();

      /* Trigger is optional, disable it for now	*/
      SetTrigger(null, 0, null, 0, null, null, 0, 0, 0);

      RapidBlockDataHandler(numRapidCaptures);
    }


    /****************************************************************************
    * WaitForKey
    *  Waits for the user to press a key
    *  
    ****************************************************************************/
    private static void WaitForKey()
    {
      while(!Console.KeyAvailable) Thread.Sleep(100);
      if (Console.KeyAvailable) Console.ReadKey(true); // clear the key
    }

    /****************************************************************************
     * CollectBlockTriggered
     *  this function demonstrates how to collect a single block of data from the
     *  unit, when a trigger event occurs.
     ****************************************************************************/
    void CollectBlockTriggered()
    {
      short triggerVoltage = mv_to_adc(1000, (short)_channelSettings[(int)Imports.Channel.ChannelA].range); // ChannelInfo stores ADC counts
      
       Imports.TriggerChannelProperties[] sourceDetails = new Imports.TriggerChannelProperties[] {
        new Imports.TriggerChannelProperties(triggerVoltage,
                                             256*10,
                                             triggerVoltage,
                                             256*10,
                                             Imports.Channel.ChannelA,
                                             Imports.ThresholdMode.Level)};

      Imports.TriggerConditions[] conditions = new Imports.TriggerConditions[] {
              new Imports.TriggerConditions(Imports.TriggerState.True,
                                            Imports.TriggerState.DontCare,
                                            Imports.TriggerState.DontCare,
                                            Imports.TriggerState.DontCare,
                                            Imports.TriggerState.DontCare,
                                            Imports.TriggerState.DontCare,
                                            Imports.TriggerState.DontCare)};

      Imports.ThresholdDirection[] directions = new Imports.ThresholdDirection[]
	                                        { Imports.ThresholdDirection.Rising,
                                            Imports.ThresholdDirection.None, 
                                            Imports.ThresholdDirection.None, 
                                            Imports.ThresholdDirection.None, 
                                            Imports.ThresholdDirection.None,
                                            Imports.ThresholdDirection.None };

      Console.WriteLine("Collect block triggered...");
      Console.WriteLine("Collects when value rises past {0}mV",
                        adc_to_mv(sourceDetails[0].ThresholdMajor,
                                  (int)_channelSettings[(int)Imports.Channel.ChannelA].range));
      Console.WriteLine("Press a key to start...");
      WaitForKey();

      SetDefaults();

      /* Trigger enabled
       * Rising edge
       * Threshold = 100mV */
      SetTrigger(sourceDetails, 1, conditions, 1, directions, null, 0, 0, 0);

      BlockDataHandler("Ten readings after trigger", 0);
    }

    /****************************************************************************
     * Initialise unit' structure with Variant specific defaults
     ****************************************************************************/
    void GetDeviceInfo()
    {
        string[] description = {
                           "Driver Version",
                           "USB Version",
                           "Hardware Version",
                           "Variant Info",
                           "Serial",
                           "Error Code"
                         };
      int variant=0;

      System.Text.StringBuilder line = new System.Text.StringBuilder(80);

      if (_handle >= 0)
      {
          for (int i = 0; i < 5; i++)
          {
            short requiredSize;
            Imports.GetUnitInfo(_handle, line, 80, out requiredSize, i);
            Console.WriteLine("{0}: {1}", description[i], line);

           
            if (i == 3)
            {
                if (line.Length == 4) // Standard model, not A or B variant, convert model number into Hex i.e. 6402 -> 0x6402
                {
                    variant = int.Parse(line.ToString());
                    variant += 0x4B00;
                }
                else
                if (line.Length == 5)                           // A or B variant
                {
                    if (line[4] == 'A' || line[4] == 'a')       // i.e. 6402A -> 0xA402
                    {
                        line = line.Remove(4, 1);
                        variant = int.Parse(line.ToString());
                        variant += 0x8B00;
                    }
                    else
                    if (line[4] == 'B' || line[4] == 'b')       // i.e. 6402B -> 0xB402
                    {
                        line = line.Remove(4, 1);
                        variant = int.Parse(line.ToString());
                        variant += 0x9B00;
                    }
                }
            }
          }
         
         switch ((Imports.MODEL_TYPE)variant)
          {
              case Imports.MODEL_TYPE.MODEL_PS6402:
              case Imports.MODEL_TYPE.MODEL_PS6402B:

                  _firstRange = Imports.Range.Range_50MV;
                  _lastRange = Imports.Range.Range_20V;
                  _channelCount = QUAD_SCOPE;
                  _AWG = true;

                  _channelSettings = new ChannelSettings[MAX_CHANNELS];
                  for (int i = 0; i < MAX_CHANNELS; i++)
                  {
                      _channelSettings[i].enabled = true;
                      _channelSettings[i].DCcoupled = Imports.PS6000Coupling.PS6000_DC_1M;
                      _channelSettings[i].range = Imports.Range.Range_5V;
                  }

                  break;

              case Imports.MODEL_TYPE.MODEL_PS6402A:

                  _firstRange = Imports.Range.Range_50MV;
                  _lastRange = Imports.Range.Range_20V;
                  _channelCount = QUAD_SCOPE;
                  _AWG = false;

                  _channelSettings = new ChannelSettings[MAX_CHANNELS];
                  for (int i = 0; i < MAX_CHANNELS; i++)
                  {
                      _channelSettings[i].enabled = true;
                      _channelSettings[i].DCcoupled = Imports.PS6000Coupling.PS6000_DC_1M;
                      _channelSettings[i].range = Imports.Range.Range_5V;
                  }

                  break;

              case Imports.MODEL_TYPE.MODEL_PS6403:
              case Imports.MODEL_TYPE.MODEL_PS6403B:
                  _firstRange = Imports.Range.Range_50MV;
                  _lastRange = Imports.Range.Range_20V;
                  _channelCount = QUAD_SCOPE;
                  _AWG = true;

                  _channelSettings = new ChannelSettings[MAX_CHANNELS];
                  for (int i = 0; i < MAX_CHANNELS; i++)
                  {
                      _channelSettings[i].enabled = true;
                      _channelSettings[i].DCcoupled = Imports.PS6000Coupling.PS6000_DC_1M;
                      _channelSettings[i].range = Imports.Range.Range_5V;
                  }
                  break;

            
              case Imports.MODEL_TYPE.MODEL_PS6403A:
                  _firstRange = Imports.Range.Range_50MV;
                  _lastRange = Imports.Range.Range_20V;
                  _channelCount = QUAD_SCOPE;
                  _AWG = false;

                  _channelSettings = new ChannelSettings[MAX_CHANNELS];
                  for (int i = 0; i < MAX_CHANNELS; i++)
                  {
                      _channelSettings[i].enabled = true;
                      _channelSettings[i].DCcoupled = Imports.PS6000Coupling.PS6000_DC_1M;
                      _channelSettings[i].range = Imports.Range.Range_5V;
                  }
                  break;


              case Imports.MODEL_TYPE.MODEL_PS6404:
              case Imports.MODEL_TYPE.MODEL_PS6404B:
                 _firstRange = Imports.Range.Range_50MV;
                  _lastRange = Imports.Range.Range_20V;
                  _channelCount = QUAD_SCOPE;
                  _AWG = true;

                  _channelSettings = new ChannelSettings[MAX_CHANNELS];
                  for (int i = 0; i < MAX_CHANNELS; i++)
                  {
                      _channelSettings[i].enabled = true;
                      _channelSettings[i].DCcoupled = Imports.PS6000Coupling.PS6000_DC_1M;
                      _channelSettings[i].range = Imports.Range.Range_5V;
                  }
                  break;

              case Imports.MODEL_TYPE.MODEL_PS6404A:
                  _firstRange = Imports.Range.Range_50MV;
                  _lastRange = Imports.Range.Range_20V;
                  _channelCount = QUAD_SCOPE;
                  _AWG = false;

                  _channelSettings = new ChannelSettings[MAX_CHANNELS];
                  for (int i = 0; i < MAX_CHANNELS; i++)
                  {
                      _channelSettings[i].enabled = true;
                      _channelSettings[i].DCcoupled = Imports.PS6000Coupling.PS6000_DC_1M;
                      _channelSettings[i].range = Imports.Range.Range_5V;
                  }
                  break;

              case Imports.MODEL_TYPE.MODEL_PS6407:
                  _firstRange = Imports.Range.Range_100MV;          // fixed 100mV input on 6407
                  _lastRange = Imports.Range.Range_100MV;
                  _channelCount = QUAD_SCOPE;

                  _channelSettings = new ChannelSettings[MAX_CHANNELS];
                  for (int i = 0; i < MAX_CHANNELS; i++)
                  {
                      _channelSettings[i].enabled = true;
                      _channelSettings[i].DCcoupled = Imports.PS6000Coupling.PS6000_DC_50R;   // fixed 50ohm imput impedance on 6407
                      _channelSettings[i].range = Imports.Range.Range_100MV;
                  }
                 break;
          }
      }
    }

   
    /****************************************************************************
    * Select input voltage ranges for each channel
    ****************************************************************************/
    void SetVoltages()
    {
        bool valid = false;
        bool allChannelsOff = true;

        Console.WriteLine("Available voltage ranges are....\n");
        /* See what ranges are available... */
        for (int i = (int)_firstRange; i <= (int)_lastRange; i++)
        {
            Console.WriteLine("{0} . {1} mV", i, inputRanges[i]);
        }

        /* Ask the user to select a range */
        Console.WriteLine("Specify voltage range ({0}..{1})", (int)_firstRange, (int)_lastRange);
        Console.WriteLine("99 - switches channel off");
        do
        {
            for (int ch = 0; ch < _channelCount; ch++)
            {
                Console.WriteLine("");
                uint range = 8;

                do
                {
                    try
                    {
                        Console.WriteLine("Channel: {0}", (char)('A' + ch));
                        range = uint.Parse(Console.ReadLine());
                        valid = true;
                    }
                    catch (FormatException e)
                    {
                        valid = false;
                        Console.WriteLine("Error: " + e.Message);
                    }

                } while ((range != 99 && (range < (uint)_firstRange || range > (uint)_lastRange) || !valid));


                if (range != 99)
                {
                    _channelSettings[ch].range = (Imports.Range)range;
                    Console.WriteLine(" = {0} mV", inputRanges[range]);
                    _channelSettings[ch].enabled = true;
                    allChannelsOff = false;
                }
                else
                {
                    Console.WriteLine("Channel Switched off");
                    _channelSettings[ch].enabled = false;
                }
            }
            Console.Write(allChannelsOff?"At least one channels must be enabled\n":"");
        } while (allChannelsOff);

        SetDefaults();  // Set defaults now, so that if all but 1 channels get switched off, timebase updates to timebase 0 will work
    }


    /****************************************************************************
     *
     * Select _timebase, set _oversample to on and time units as nano seconds
     *
     ****************************************************************************/
    void SetTimebase()
    {
        int timeInterval;
        int maxSamples;
        bool valid = false;

        do
        {
            Console.WriteLine("Specify timebase");
            try
            {
                _timebase = uint.Parse(Console.ReadLine());
                valid = true;
            }
            catch(FormatException e)
            {
                valid = false;
                Console.WriteLine("Error: " + e.Message);
            }

        } while (!valid);

        while (Imports.GetTimebase(_handle, _timebase, BUFFER_SIZE, out timeInterval, 1, out maxSamples, 0) != 0)
        {
            Console.WriteLine("Selected timebase {0} could not be used", _timebase);
            _timebase++;
        }

        Console.WriteLine("Using Timebase {0} - {1} ns sampleinterval", _timebase, timeInterval);
        _oversample = 1;
    }

    /****************************************************************************
     * Stream Data Handler
     * - Used by the two stream data examples - untriggered and triggered
     * Inputs:
     * - unit - the unit to sample on
     * - preTrigger - the number of samples in the pre-trigger phase 
     *					(0 if no trigger has been set)
     ***************************************************************************/
    void StreamDataHandler(uint preTrigger)
    {      
      uint sampleCount = BUFFER_SIZE*100; /*  *100 is to make sure buffer large enough */
      short[][] minBuffers = new short[_channelCount][];
      short[][] maxBuffers = new short[_channelCount][];
      PinnedArray<short>[] minPinned = new PinnedArray<short>[_channelCount];
      PinnedArray<short>[] maxPinned = new PinnedArray<short>[_channelCount];
      int totalSamples = 0;
      uint triggeredAt = 0;
      uint sampleInterval = 1;
      short status;

      for (int i = 0; i < _channelCount; i++) // create data buffers
      {
          minBuffers[i] = new short[sampleCount ];
          maxBuffers[i] = new short[sampleCount ];
          minPinned[i] = new PinnedArray<short>(minBuffers[i]);
          maxPinned[i] = new PinnedArray<short>(maxBuffers[i]);
		  Imports.SetDataBuffers(_handle, (Imports.Channel)i, minBuffers[i], maxBuffers[i], (int)sampleCount, Imports.PS6000DownSampleRatioMode.PS6000_RATIO_MODE_NONE);
      }

      Console.WriteLine("Waiting for trigger...Press a key to abort");
      _autoStop = false;
      status = Imports.RunStreaming(_handle, ref sampleInterval,  Imports.ReportedTimeUnits.MicroSeconds,
																	preTrigger, 1000000 - preTrigger, 1, 1000, Imports.PS6000DownSampleRatioMode.PS6000_RATIO_MODE_AGGREGATE, sampleCount);
      Console.WriteLine("Run Streaming : {0} ", status);

      Console.WriteLine("Streaming data...Press a key to abort");

      TextWriter writer = new StreamWriter("stream.txt", false);


      writer.Write("For each of the enabled Channels, results shown are....");
      writer.WriteLine();
      writer.WriteLine("Maximum Aggregated value ADC Count & mV, Minimum Aggregated value ADC Count & mV");
      writer.WriteLine();

      for (int ch = 0; ch < _channelCount; ch++)
        if (_channelSettings[ch].enabled)
          writer.Write("Ch  Max ADC    Max mV   Min ADC    Min mV   ");
      writer.WriteLine();

      while (!_autoStop && !Console.KeyAvailable)
      {
        /* Poll until data is received. Until then, GetStreamingLatestValues wont call the callback */
        Thread.Sleep(100);
        _ready = false;
        status = Imports.GetStreamingLatestValues(_handle, StreamingCallback, IntPtr.Zero);

        Console.Write(status > 0?"Status =  {0}\n": "",status );

        if (_ready && _sampleCount > 0) /* can be ready and have no data, if autoStop has fired */
        {
            if (_trig > 0)
                triggeredAt = (uint)totalSamples + _trigAt;

            totalSamples += _sampleCount;
            Console.Write("\nCollected {0,4} samples, index = {1,5}, Total = {2,5}", _sampleCount, _startIndex, totalSamples );

            if (_trig > 0)
                Console.Write("\tTrig at Index {0}", triggeredAt);

            for (uint i = _startIndex; i < (_startIndex + _sampleCount); i++)
            {
                for (int ch = 0; ch < _channelCount; ch++)
                {
                    if (_channelSettings[ch].enabled)
                    {
                        writer.Write("Ch{0} {1,7}   {2,7}   {3,7}   {4,7}   ",
                                                    (char)('A' + ch),
                                                    minPinned[ch].Target[i],
                                                    adc_to_mv(minPinned[ch].Target[i], (int)_channelSettings[(int)(Imports.Channel.ChannelA + ch)].range),
                                                    maxPinned[ch].Target[i],
                                                    adc_to_mv(maxPinned[ch].Target[i], (int)_channelSettings[(int)(Imports.Channel.ChannelA + ch)].range));
                    }
                }
                writer.WriteLine();
            }
        }
      }
      if (Console.KeyAvailable) Console.ReadKey(true); // clear the key

      Imports.Stop(_handle);
      writer.Close();

      if (!_autoStop)
      {
        Console.WriteLine("data collection aborted");
        WaitForKey();
      }

      foreach (PinnedArray<short> p in minPinned)
      {
          if (p != null)
              p.Dispose();
      }
      foreach (PinnedArray<short> p in maxPinned)
      {
          if (p != null)
              p.Dispose();
      }
      
    }
    

    /****************************************************************************
     * CollectStreamingImmediate
     *  this function demonstrates how to collect a stream of data
     *  from the unit (start collecting immediately)
     ***************************************************************************/
    void CollectStreamingImmediate()
    {
      SetDefaults();

      Console.WriteLine("Collect streaming...");
      Console.WriteLine("Data is written to disk file (stream.txt)");
      Console.WriteLine("Press a key to start");
      WaitForKey();

      /* Trigger disabled	*/
      SetTrigger(null, 0, null, 0, null, null, 0, 0, 0);

      StreamDataHandler(0);
    }

    /****************************************************************************
     * CollectStreamingTriggered
     *  this function demonstrates how to collect a stream of data
     *  from the unit (start collecting on trigger)
     ***************************************************************************/
    void CollectStreamingTriggered()
    {
      short triggerVoltage = mv_to_adc( 1000, (short) _channelSettings[(int) Imports.Channel.ChannelA].range); // ChannelInfo stores ADC counts
      
      Imports.TriggerChannelProperties[] sourceDetails = new Imports.TriggerChannelProperties[] {
        new Imports.TriggerChannelProperties( triggerVoltage, 256 * 10, triggerVoltage, 256 * 10, Imports.Channel.ChannelA, Imports.ThresholdMode.Level )};
      
      Imports.TriggerConditions[] conditions = new Imports.TriggerConditions[] {
              new Imports.TriggerConditions(Imports.TriggerState.True,
                                            Imports.TriggerState.DontCare,
                                            Imports.TriggerState.DontCare,
                                            Imports.TriggerState.DontCare,
                                            Imports.TriggerState.DontCare,
                                            Imports.TriggerState.DontCare,
                                            Imports.TriggerState.DontCare)};

      Imports.ThresholdDirection[] directions = new Imports.ThresholdDirection[]
	                                        { Imports.ThresholdDirection.Rising,
                                            Imports.ThresholdDirection.None, 
                                            Imports.ThresholdDirection.None, 
                                            Imports.ThresholdDirection.None, 
                                            Imports.ThresholdDirection.None,
                                            Imports.ThresholdDirection.None };

      Console.WriteLine("Collect streaming triggered...");
      Console.WriteLine("Data is written to disk file (stream.txt)");
      Console.WriteLine("Press a key to start");
      WaitForKey();
      SetDefaults();

      /* Trigger enabled
       * Rising edge
       * Threshold = 100mV */

      SetTrigger(sourceDetails, 1, conditions, 1, directions, null, 0, 0, 0);

      StreamDataHandler(100000);
    }


    /*************************************************************************************
   * SetSignalGenerator
   *  this function demonstrates how to use the Signal Generator & 
   *  (where supported) AWG files (Values 0 .. 4192, up to 8192 lines)
   *  
   **************************************************************************************/
    void SetSignalGenerator()
    {
        short waveform = 0;
        char ch;
        uint pkpk = 1000000;
        int waveformSize = 0;
        Imports.PS6000ExtraOperations operation = Imports.PS6000ExtraOperations.PS6000_ES_OFF;
        string fileName;
        int offset = 0;
        uint frequency = 1000;
        short [] arbitraryWaveform = new short [8192];
        short status;
        string lines = string.Empty;
        int i = 0;


        do
        {
            Console.WriteLine("");
            Console.WriteLine("Signal Generator\n================\n");
            Console.WriteLine("0:\tSINE      \t6:\tGAUSSIAN");
            Console.WriteLine("1:\tSQUARE    \t7:\tHALF SINE");
            Console.WriteLine("2:\tTRIANGLE  \t8:\tDC VOLTAGE");
            Console.WriteLine("3:\tRAMP UP   \t9:\tWHITE NOISE");
            Console.WriteLine("4:\tRAMP DOWN");
            Console.WriteLine("5:\tSINC");
            Console.Write(_AWG ? "A:\tAWG WAVEFORM\t" : "");
            Console.WriteLine("X:\tSigGen Off");
            Console.WriteLine("");

            ch = Console.ReadKey(true).KeyChar;

            if (ch >= '0' && ch <= '9')
                waveform = (short)(ch - '0');
            else
                ch = char.ToUpper(ch);

            if (ch == 'A' && _AWG == false)         // Treat option 'A' as an invalid input if device doesn't support AWG
                ch = 'Z';
        }
        while (ch != 'A' && ch != 'X' && (ch < '0' || ch > '9'));


	    if(ch == 'X')				// If we're going to turn off siggen
	    {
		    Console.WriteLine("Signal generator Off");
		    waveform = 8;		// DC Voltage
		    pkpk = 0;				// 0V
		    waveformSize = 0;
		    operation = Imports.PS6000ExtraOperations.PS6000_ES_OFF;
	    }
	    else
	    if (ch == 'A')		// Set the AWG
	    {
		    waveformSize = 0;

		    Console.WriteLine("Select a waveform file to load: ");
            fileName = Console.ReadLine();

            // Open file & read in data - one number per line (at most 8192 lines), with values in (0..4095)

            StreamReader sr;
            try
            {
                sr = new StreamReader(fileName);
            }
            catch (FileNotFoundException)
            {
                Console.WriteLine("Cannot open file.");
                return;
            }

           
            while (((lines = sr.ReadLine()) != null) && i < 8192)
            {
                try
                {
                    arbitraryWaveform[i++] = short.Parse(lines);
                }
                catch (Exception e)
                {
                    Console.WriteLine("Error: " + e.Message);
                    sr.Close();
                    return;
                }
            }
            sr.Close();
           
       
            waveformSize =  (arbitraryWaveform.Length);
	    }
	    else			// Set one of the built in waveforms
	    {
		    switch (waveform)
		    {
			    case 8:
			    do 
			    {
				    Console.WriteLine("Enter offset in uV: (0 to 2500000)"); // Ask user to enter DC offset level;
				  
                    try
                    {
                        offset = Int32.Parse(Console.ReadLine());
                    }
                    catch (Exception e)
                    {
                        Console.WriteLine("Error: " + e.Message);
                    }
			    } while (offset < 0 || offset > 10000000);
			    operation = Imports.PS6000ExtraOperations.PS6000_ES_OFF;
			    break;

			    case 9:
				    operation = Imports.PS6000ExtraOperations.PS6000_WHITENOISE;
				    break;

			    default:
				    operation = Imports.PS6000ExtraOperations.PS6000_ES_OFF;
				    offset = 0;
                    break;
		    }
	    }

	    if(waveform < 8 || (ch == 'A' && _AWG))				// Find out frequency if required
	    {
		    do 
		    {
			    Console.WriteLine("Enter frequency in Hz: (1 to 10000000)"); // Ask user to enter signal frequency;
                try
                {
                    frequency = UInt32.Parse(Console.ReadLine());
                }
                catch (Exception e)
                {
                    Console.WriteLine("Error: " + e.Message);
                }
		    } while (frequency <= 0 || frequency > 10000000);
	    }

	    if (waveformSize > 0)		
	    {
		    double delta = ((frequency * waveformSize) / 8192.0) * 4294967296.0 * 8e-9; // delta >= 10
	
            status = Imports.SetSigGenArbitrary( _handle,
                                                0, 
							                    1000000, 
							                    (uint) delta, 
							                    (uint) delta, 
							                    0, 
							                    0, 
							                    arbitraryWaveform, 
							                    waveformSize, 
							                    0,
							                    0, 
							                    0, 
							                    0, 
							                    0, 
							                    0,
							                    0,
							                    0);

		    Console.WriteLine(status!=0?"SetSigGenArbitrary: Status Error 0x%x ":"", status);		// If status != 0, show the error
	    } 
	    else 
	    {
		    status = Imports.SetSigGenBuiltIn(_handle, offset, pkpk, waveform, (float)frequency, (float)frequency, 0, 0, 0, operation, 0, 0, 0, 0, 0);
		    Console.WriteLine(status!=0?"SetSigGenBuiltIn: Status Error 0x%x ":"", status);		// If status != 0, show the error
	    }
    }



    /*************************************************************************************
    * Run
    *  main menu
    *  
    **************************************************************************************/
    public void Run()
    {
      // setup devices
      GetDeviceInfo();
      _timebase = 1;

     

      // main loop - read key and call routine
      char ch = ' ';
      while (ch != 'X')
      {
        Console.WriteLine("\n");
        Console.WriteLine("B - immediate block             V - Set voltages");
        Console.WriteLine("T - triggered block             I - Set timebase");
		Console.WriteLine("R - rapid block                 A - ADC counts/mV");
        Console.WriteLine("S - immediate streaming");
		Console.WriteLine("W - triggered streaming");
        Console.WriteLine("G - Signal generator");
        Console.WriteLine("X - exit");
        Console.WriteLine("Operation:");

        ch = char.ToUpper(Console.ReadKey(true).KeyChar);

        Console.WriteLine("\n");
        switch (ch)
        {
          case 'B':
            CollectBlockImmediate();
            break;

          case 'T':
            CollectBlockTriggered();
            break;

          case 'R':
            CollectBlockRapid();
            break;

          case 'S':
            CollectStreamingImmediate();
            break;

          case 'W':
            CollectStreamingTriggered();
            break;

            case 'G':
            SetSignalGenerator();
            break;


          case 'V':
            SetVoltages();
            break;

          case 'I':
            SetTimebase();
            break;

          case 'A':
            _scaleVoltages = !_scaleVoltages;
            if (_scaleVoltages)
            {
              Console.WriteLine("Readings will be scaled in mV");
            }
            else
            {
              Console.WriteLine("Readings will be scaled in ADC counts");
            }
            break;

          case 'X':
            /* Handled by outer loop */
            break;

          default:
            Console.WriteLine("Invalid operation");
            break;
        }
      }      
    }

    private ConsoleExample(short handle)
    {
      _handle = handle;
    }

    static void Main()
    {
      Console.WriteLine("PS6000 driver example program");

      //open unit and show splash screen
      Console.WriteLine("\n\nOpening the device...");
      short handle;
			short status = Imports.OpenUnit(out handle, null);
      Console.WriteLine("Handle: {0}", handle);
      if (status != 0)
      {
        Console.WriteLine("Unable to open device");
        Console.WriteLine("Error code : {0}", status);
        WaitForKey();
      }
      else
      {
        Console.WriteLine("Device opened successfully\n");

        ConsoleExample consoleExample = new ConsoleExample(handle);
        consoleExample.Run();

        Imports.CloseUnit(handle);
      }
    }
  }
}  

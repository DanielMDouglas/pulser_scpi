#include <iostream>
#include <string>
#include <cstring>
#include <sstream>
#include <lxi.h>

// default pulse parameters
// changed by handleOpts
double A = 0;
double B = 0;
double dt = 0;

// pulse shape parameters
// should be constant across a given scan
// but you'll want to play with these
// to find the right settings

// number of samples in the waveform
const int NSamples = 1000;
// slope of the ramp
const double slope = 1;
// width of the ramp
const double pulse_width = 3;
// sample rate (1/s)
const std::string sampleRate = "FUNC:ARB:SRATE 10E3";

void sayUsage()
{
  // basically a help string
  std::cout << "Usage: set_pulser" 
  	    << " -A <height of pulse A> -B <height of pulse B> "
  	    << " -dt <interval between start of pulse A and start of pulse B> "
  	    << std::endl;
}

void handleOpts(int argc, char const * argv[])
{
  // collect the arguments,
  // save to variables where needed
  int opt = 0;
  while ( opt < argc ) {
    std::stringstream optValue;
    std::stringstream argValue;
    optValue << argv[++opt];
    
    if ( optValue.str() == "-A" ) {
      argValue << argv[++opt];
      argValue >> A;
    }
    if ( optValue.str() == "-B" ) {
      argValue << argv[++opt];
      argValue >> B;
    }
    if ( optValue.str() == "-dt" ) {
      argValue << argv[++opt];
      argValue >> dt;
    }
    if ( optValue.str() == "-h" ) {
      sayUsage();
    }    
  }
}

std::string make_waveform()
{
  // waveform should be a positive ramp of some length
  
  std::stringstream command;

  // define the data object "pulses"
  command << "DATA:ARB pulses";
  
  // positive pulses
  for ( int t = 0; t < NSamples/2; t++ ) {
    double V = 0;
    if ( t < pulse_width ) {
      // within the first pulse (starts at t = 0)
      V += t*A/pulse_width;
    }
    else {
      // after the first pulse
      V += A;
      if ( t > dt ) {
    	// after the start of the second pulse
    	if ( t < pulse_width + dt ) {
    	  // within the second pulse
    	  V += (t - dt)*B/pulse_width;
    	}
    	else {
    	  // after the second pulse
    	  V += B;
    	}
      }
    }

    command << ", ";
    command << V;
  }
  
  // negative pulses
  for ( int t = NSamples/2; t < NSamples; t++ ) {
    double V = A + B;
    if ( t < NSamples/2 + pulse_width ) {
      // within the first pulse (starts at t = 0)
      V -= (t - NSamples/2)*A/pulse_width;
    }
    else {
      // after the first pulse
      V -= A;
      if ( t > dt + NSamples/2 ) {
    	// after the start of the second pulse
    	if ( t < pulse_width + dt + NSamples/2 ) {
    	  // within the second pulse
    	  V -= (t - dt - NSamples/2)*B/pulse_width;
    	}
    	else {
    	  // after the second pulse
    	  V -= B;
    	}
      }
    }

    command << ", ";
    command << V;
  }

  return command.str();
}

int main(int argc, char const * argv[])
{
  handleOpts(argc, argv);
  
  int device, timeout = 1000;
  // device address depends on your setup
  // probably just use the one found by
  // the built-in DHCP
  std::string deviceAddr = "192.168.2.5";

  // Initialize LXI library
  std::cout << "Initializing LXI..." << std::endl;
  lxi_init();
  
  // Connect to LXI device
  std::cout << "Connecting to device at " << deviceAddr << " ..." << std::endl;
  device = lxi_connect(deviceAddr.c_str(), 0, "inst0", timeout, VXI11);

  // Build waveform
  std::string waveformCmd = make_waveform();

  std::string commands [] = {sampleRate, // sample rate
			     "FUNC:ARB:FILTER OFF", // ?
			     "FUNC:ARB:PTPEAK 10",  // ?
			     waveformCmd,           // actual waveform series
			     "FUNC:ARB pulses",     // use above data as a function
			     "OUTPUT ON",           // enable output
  };
  
  // Send SCPI command
  std::cout << "Sending command to device..." << std::endl;
  int maxStringSize = 50;
  for ( std::string command : commands ) {
    if ( command.size() < maxStringSize ) {
      std::cout << '\t' << command << std::endl;
    }
    else {
      std::cout << '\t' << command.substr(0, maxStringSize) << " ... " << std::endl;
    }
    lxi_send(device, command.c_str(), sizeof(command), timeout);
  }
  
  // Disconnect
  std::cout << "Disconnecting from device..." << std::endl;
  lxi_disconnect(device);
}

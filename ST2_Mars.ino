// Sources for Mars Time algorithm:
//
// NASA GISS Mars24 Sunclock - http://www.giss.nasa.gov/tools/mars24/
// Mars Clock by James Tauber - http://jtauber.github.com/mars-clock/
// Wikipedia - http://en.wikipedia.org/wiki/Timekeeping_on_Mars
//
// The actual conversion to Mars time is done by using Julian dates with a Jan 1, 2000 epoch. 32 bit float
// precision is not acceptable for this computation; 64 bit float works, but the Arduino environment does not 
// support it. However, Arduino does support uint64_t. So all Julian dates are scaled by 10^6, which provides
// adequate precision. 10^5 was used originally as a scalar, but accumulated rounding errors could leave results
// that were off by several seconds.
//
// The function ComputeMarsTime(uint64_t Epoch) looks at the UTC time and date, which should already have been
// updated by the RTC, and sets variables for Mars Sol, hours, and minutes. If Epoch is 0, the function returns MTC time
// (a.k.a. AMT or Airy-0 Mean Time, as the Airy-0 crater is accepted as the Mars prime meridian.) Sol is the Mars Sol Date,
// which is the number of Sols since December 29, 1873, the birth date of astronomer Carl Otto Lampland. This function also
// computes mission times if the Epoch parameter is specified, where Epoch is the offset between the MSD/MTC and the mission 
// clock * 10^6.

// Limitations:
//
// * The RTC clock is limited to the current century.
// * Sol display is limited to 5 digits, but that should last over 281 terrestrial years
// * While Unix times are used, we're using an unsigned 32 bit integer, so the classic "Death of Unix" in 2038
//   is not relevant; this should be good until approximately 2106.
// * The code most at risk for overflow is this:
//      uint64_t MSD = J2000 - 4500000LL;
//      MSD *= 1000000000LL;
//   Note that MSD is already multiplied by 10^6. This will cause an overflow in approximately 2050. However,
//   assuming the hardware even still works then, a number of software workarounds are possible, such as reducing
//   the accuracy by changing the multiplier to 10^5, or using a different epoch than J2000.
// * This code currently hard codes the difference between UTC and TAI, e.g. the number of leap seconds, to
//   35.

uint32_t MarsSol;
uint8_t MarsHourTens;
uint8_t MarsHourOnes;
uint8_t MarsMinTens;
uint8_t MarsMinOnes;

#define MTC_EPOCH  0LL
// The NASA GISS page confirms a mission clock for Curiosity based on a 137.42 E landing site.
#define MSL_EPOCH  49268618277LL
// Wikipedia states that that the Opportunity uses a mission time of AMT-01:01:06
#define MERB_EPOCH 46235042431LL
uint64_t CurrentEpoch;

void ComputeMarsTime(uint64_t Epoch)
{
  uint32_t UnixTime = 946684800L; // Jan 1, 2000 epoch
  uint8_t temp;
  temp = UTCYearsTens * 10 + UTCYearsOnes;
  UnixTime += (uint64_t) temp * 31536000L;
  // Now correct for leap years. Note that 2000 is NOT a leap year. We are also subtracting 1 because the current
  // year is dealt with later.
  if ( temp )
    temp--;
  temp -= temp % 4;
  temp /= 4;
  UnixTime += (uint64_t) temp * 86400L;
  if ( UTCMonthCode >= 2 )
    UnixTime += 2678400L;
  if ( UTCMonthCode >= 3 ) {
    UnixTime += 2419200L;
    // Correct for current year leap year. Note that 2000 is NOT a leap year.
    if ( UTCYearsTens != 0 || UTCYearsOnes != 0 ) {
      if ( ( UTCYearsTens * 10 + UTCYearsOnes ) % 4 == 0 )
        UnixTime += 86400L;
    }
  }
  if ( UTCMonthCode >= 4 )
    UnixTime += 2678400L;
  if ( UTCMonthCode >= 5 )
    UnixTime += 2592000L;  
  if ( UTCMonthCode >= 6 )
    UnixTime += 2678400L;
  if ( UTCMonthCode >= 7 )
    UnixTime += 2592000L;   
  if ( UTCMonthCode >= 8 )
    UnixTime += 2678400L;    
  if ( UTCMonthCode >= 9 )
    UnixTime += 2678400L;
  if ( UTCMonthCode >= 10 )
    UnixTime += 2592000L;  
  if ( UTCMonthCode >= 11 )
    UnixTime += 2678400L;
  if ( UTCMonthCode >= 12 )
    UnixTime += 2592000L;
  temp = UTCDateTens * 10 + UTCDateOnes;    
  UnixTime += (uint32_t) temp * 86400L;
  temp = UTCHourTens * 10 + UTCHourOnes;
  UnixTime += (uint32_t) temp * 3600L;
  temp = MinTens * 10 + MinOnes;
  UnixTime += (uint32_t) temp * 60L;
  temp = SecTens * 10 + SecOnes;
  UnixTime += (uint32_t) temp;

  uint64_t JD_UTC = (UnixTime * 1000000LL) / 86400LL + 2440587500000LL;
  uint64_t JD_TT = JD_UTC;
  JD_TT += (35LL*1000000LL + 32184000LL) / 86400LL; // Note: TODO MAKE LEAP SECOND CONFIGURABLE
  uint64_t J2000 = JD_TT - 2451545000000LL;
  uint64_t MSD = J2000 - 4500000LL;
  MSD *= 1000000000LL;
  MSD /= 1027491252LL;
  MSD += 44796000000LL - 960LL;
  MSD -= Epoch; // Epoch of mission time. Set to 0 for MTC.
  MarsSol = MSD / 1000000LL;
  uint32_t MTC = (MSD % 1000000L) * 24L;  
  byte MTCHour = MTC / 1000000L;
  MarsHourOnes = MTCHour % 10;
  MarsHourTens = ( MTCHour - MarsHourOnes ) / 10;
  MTC = (MTC % 1000000L)*60L;
  byte MTCMin = (MTC - MTC % 1000000L)/1000000L;
  MarsMinOnes = MTCMin % 10;
  MarsMinTens = ( MTCMin - MarsMinOnes ) / 10;
}

void ComputeMTCOld()
{
  uint64_t UnixTime = 946684800LL; // Jan 1, 2000 epoch
  uint8_t temp;
  temp = UTCYearsTens * 10 + UTCYearsOnes;
  UnixTime += (uint64_t) temp * 31536000LL;
  // Now correct for leap years. Note that 2000 is NOT a leap year. We are also subtracting 1 because the current
  // year is dealt with later.
  if ( temp )
    temp--;
  temp -= temp % 4;
  temp /= 4;
  UnixTime += (uint64_t) temp * 86400LL;
  if ( UTCMonthCode >= 2 )
    UnixTime += 2678400LL;
  if ( UTCMonthCode >= 3 ) {
    UnixTime += 2419200LL;
    if ( ( UTCYearsTens * 10 + UTCYearsOnes ) % 4 == 0 )
      UnixTime += 86400LL;
  }
  if ( UTCMonthCode >= 4 )
    UnixTime += 2678400LL;
  if ( UTCMonthCode >= 5 )
    UnixTime += 2592000LL;  
  if ( UTCMonthCode >= 6 )
    UnixTime += 2678400LL;
  if ( UTCMonthCode >= 7 )
    UnixTime += 2592000LL;   
  if ( UTCMonthCode >= 8 )
    UnixTime += 2678400LL;    
  if ( UTCMonthCode >= 9 )
    UnixTime += 2678400LL;
  if ( UTCMonthCode >= 10 )
    UnixTime += 2592000LL;  
  if ( UTCMonthCode >= 11 )
    UnixTime += 2678400LL;
  if ( UTCMonthCode >= 12 )
    UnixTime += 2592000LL;
  temp = UTCDateTens * 10 + UTCDateOnes;    
  UnixTime += (uint64_t) temp * 86400LL;
  temp = UTCHourTens * 10 + UTCHourOnes;
  UnixTime += (uint64_t) temp * 3600LL;
  temp = MinTens * 10 + MinOnes;
  UnixTime += (uint64_t) temp * 60LL;
  temp = SecTens * 10 + SecOnes;
  UnixTime += (uint64_t) temp;

  uint64_t JD_UTC = (UnixTime * 1000000LL) / 86400LL + 2440587500000LL;
  uint64_t JD_TT = JD_UTC;
  JD_TT += (35LL*1000000LL + 32184000LL) / 86400LL; // Note: TODO MAKE LEAP SECOND CONFIGURABLE
  uint64_t J2000 = JD_TT - 2451545000000LL;
  uint64_t MSD = J2000 - 4500000LL;
  MSD *= 1000000000LL;
  MSD /= 1027491252LL;
  MSD += 44796000000LL - 960LL;
  MarsSol = (MSD - MSD % 1000000LL)/1000000LL;
  uint32_t MTC = (MSD % 1000000L) * 24L;  
  byte MTCHour = (MTC - MTC % 1000000L)/1000000L;
  MarsHourOnes = MTCHour % 10;
  MarsHourTens = ( MTCHour - MarsHourOnes ) / 10;
  MTC = (MTC % 1000000L)*60L;
  byte MTCMin = (MTC - MTC % 1000000L)/1000000L;
  MarsMinOnes = MTCMin % 10;
  MarsMinTens = ( MTCMin - MarsMinOnes ) / 10;
}

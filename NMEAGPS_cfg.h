#ifndef NMEAGPS_CFG_H
#define NMEAGPS_CFG_H

// Enable the parsing of the following NMEA sentences
#define NMEAGPS_PARSE_GGA
#define NMEAGPS_PARSE_GLL
#define NMEAGPS_PARSE_GSA
#define NMEAGPS_PARSE_GSV
#define NMEAGPS_PARSE_RMC
#define NMEAGPS_PARSE_VTG
#define NMEAGPS_PARSE_ZDA

// Enable SATELLITE data
#define NMEAGPS_PARSE_SATELLITES
#define NMEAGPS_PARSE_SATELLITE_INFO

// Enable extended options
#define NMEAGPS_DERIVED_TYPES

// Parse and save the fix accuracy
#define NMEAGPS_PARSE_HDOP

#endif
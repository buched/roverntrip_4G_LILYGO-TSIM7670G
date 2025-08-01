/**
 * @file       TinyGsmClientSIM7672.h
 * @author     Volodymyr Shymanskyy
 * @license    LGPL-3.0
 * @copyright  Copyright (c) 2016 Volodymyr Shymanskyy
 * @date       Nov 2016
 */

#ifndef SRC_TINYGSMCLIENTSIM7672_H_
#define SRC_TINYGSMCLIENTSIM7672_H_

// #define TINY_GSM_DEBUG Serial
// #define TINY_GSM_USE_HEX

#define TINY_GSM_MUX_COUNT 4
#define TINY_GSM_BUFFER_READ_AND_CHECK_SIZE

#include "TinyGsmBattery.tpp"
#include "TinyGsmCalling.tpp"
#include "TinyGsmGPRS.tpp"
#include "TinyGsmGPS.tpp"
#include "TinyGsmGSMLocation.tpp"
#include "TinyGsmModem.tpp"
#include "TinyGsmSMS.tpp"
#include "TinyGsmTCP.tpp"
#include "TinyGsmTemperature.tpp"
#include "TinyGsmTime.tpp"
#include "TinyGsmNTP.tpp"
#include "TinyGsmMqttA76xx.h"
#include "TinyGsmHttpsComm.h"
#include "TinyGsmGPS_EX.tpp"
#include "TinyGsmFSComm.tpp"

#define GSM_NL "\r\n"
static const char GSM_OK[] TINY_GSM_PROGMEM    = "OK" GSM_NL;
static const char GSM_ERROR[] TINY_GSM_PROGMEM = "ERROR" GSM_NL;
#if defined       TINY_GSM_DEBUG
static const char GSM_CME_ERROR[] TINY_GSM_PROGMEM = GSM_NL "+CME ERROR:";
static const char GSM_CMS_ERROR[] TINY_GSM_PROGMEM = GSM_NL "+CMS ERROR:";
#endif

enum RegStatus {
  REG_NO_RESULT    = -1,
  REG_UNREGISTERED = 0,
  REG_SEARCHING    = 2,
  REG_DENIED       = 3,
  REG_OK_HOME      = 1,
  REG_OK_ROAMING   = 5,
  REG_UNKNOWN      = 4,
  REG_SMS_ONLY     = 6,
};

class TinyGsmSim7672 : public TinyGsmModem<TinyGsmSim7672>,
                       public TinyGsmGPRS<TinyGsmSim7672>,
                       public TinyGsmTCP<TinyGsmSim7672, TINY_GSM_MUX_COUNT>,
                       public TinyGsmSMS<TinyGsmSim7672>,
                       public TinyGsmGSMLocation<TinyGsmSim7672>,
                       public TinyGsmGPS<TinyGsmSim7672>,
                       public TinyGsmTime<TinyGsmSim7672>,
                       public TinyGsmNTP<TinyGsmSim7672>,
                       public TinyGsmBattery<TinyGsmSim7672>,
                       public TinyGsmTemperature<TinyGsmSim7672>,
                       public TinyGsmCalling<TinyGsmSim7672>,
                       public TinyGsmMqttA76xx<TinyGsmSim7672, TINY_GSM_MQTT_CLI_COUNT>,
                       public TinyGsmGPSEx<TinyGsmSim7672>,
                       public TinyGsmHttpsComm<TinyGsmSim7672,QUALCOMM_SIM7670G>,
                       public TinyGsmFSComm<TinyGsmSim7672,QUALCOMM_SIM7670G> 
{
  friend class TinyGsmModem<TinyGsmSim7672>;
  friend class TinyGsmGPRS<TinyGsmSim7672>;
  friend class TinyGsmTCP<TinyGsmSim7672, TINY_GSM_MUX_COUNT>;
  friend class TinyGsmSMS<TinyGsmSim7672>;
  friend class TinyGsmGPS<TinyGsmSim7672>;
  friend class TinyGsmGSMLocation<TinyGsmSim7672>;
  friend class TinyGsmTime<TinyGsmSim7672>;
  friend class TinyGsmNTP<TinyGsmSim7672>;
  friend class TinyGsmBattery<TinyGsmSim7672>;
  friend class TinyGsmTemperature<TinyGsmSim7672>;
  friend class TinyGsmCalling<TinyGsmSim7672>;
  friend class TinyGsmMqttA76xx<TinyGsmSim7672, TINY_GSM_MQTT_CLI_COUNT>;
  friend class TinyGsmHttpsComm<TinyGsmSim7672,QUALCOMM_SIM7670G>;
  friend class TinyGsmGPSEx<TinyGsmSim7672>;
  friend class TinyGsmFSComm<TinyGsmSim7672,QUALCOMM_SIM7670G>;


  /*
   * Inner Client
   */
 public:
  class GsmClientSim7672 : public GsmClient {
    friend class TinyGsmSim7672;

   public:
    GsmClientSim7672() {}

    explicit GsmClientSim7672(TinyGsmSim7672& modem, uint8_t mux = 0) {
      init(&modem, mux);
    }

    bool init(TinyGsmSim7672* modem, uint8_t mux = 0) {
      this->at       = modem;
      sock_available = 0;
      prev_check     = 0;
      sock_connected = false;
      got_data       = false;

      if (mux < TINY_GSM_MUX_COUNT) {
        this->mux = mux;
      } else {
        this->mux = (mux % TINY_GSM_MUX_COUNT);
      }
      at->sockets[this->mux] = this;

      return true;
    }

   public:
    virtual int connect(const char* host, uint16_t port, int timeout_s) {
      stop();
      TINY_GSM_YIELD();
      rx.clear();
      sock_connected = at->modemConnect(host, port, mux, false, timeout_s);
      return sock_connected;
    }
    TINY_GSM_CLIENT_CONNECT_OVERRIDES

    void stop(uint32_t maxWaitMs) {
      dumpModemBuffer(maxWaitMs);
      at->sendAT(GF("+CIPCLOSE="), mux);
      sock_connected = false;
      at->waitResponse();
    }
    void stop() override {
      stop(15000L);
    }

    /*
     * Extended API
     */

    String remoteIP() TINY_GSM_ATTR_NOT_IMPLEMENTED;
  };

  /*
   * Inner Secure Client
   */

  /*TODO(?))
  class GsmClientSecureSIM7672 : public GsmClientSim7672
  {
  public:
    GsmClientSecure() {}

    GsmClientSecure(TinyGsmSim7672& modem, uint8_t mux = 0)
     : public GsmClient(modem, mux)
    {}

  public:
    int connect(const char* host, uint16_t port, int timeout_s) override {
      stop();
      TINY_GSM_YIELD();
      rx.clear();
      sock_connected = at->modemConnect(host, port, mux, true, timeout_s);
      return sock_connected;
    }
    TINY_GSM_CLIENT_CONNECT_OVERRIDES
  };
  */

  /*
   * Constructor
   */
 public:
  explicit TinyGsmSim7672(Stream& stream) : stream(stream) {
    memset(sockets, 0, sizeof(sockets));
  }

  /*
   * Basic functions
   */
 protected:
  bool initImpl(const char* pin = NULL) {
    DBG(GF("### TinyGSM Version:"), TINYGSM_VERSION);
    DBG(GF("### TinyGSM Compiled Module:  TinyGsmClientSIM7672"));

    if (!testAT()) { return false; }

    sendAT(GF("E0"));  // Echo Off
    if (waitResponse() != 1) { return false; }

#ifdef TINY_GSM_DEBUG
    sendAT(GF("+CMEE=2"));  // turn on verbose error codes
#else
    sendAT(GF("+CMEE=0"));  // turn off error codes
#endif
    waitResponse();

    DBG(GF("### Modem:"), getModemName());

    // Disable time and time zone URC's
    sendAT(GF("+CTZR=0"));
    if (waitResponse(10000L) != 1) { return false; }

    // Enable automatic time zome update
    sendAT(GF("+CTZU=1"));
    if (waitResponse(10000L) != 1) { return false; }

    SimStatus ret = getSimStatus();
    // if the sim isn't ready and a pin has been provided, try to unlock the sim
    if (ret != SIM_READY && pin != NULL && strlen(pin) > 0) {
      simUnlock(pin);
      return (getSimStatus() == SIM_READY);
    } else {
      // if the sim is ready, or it's locked but no pin has been provided,
      // return true
      return (ret == SIM_READY || ret == SIM_LOCKED);
    }
  }

  String getModemNameImpl() {
    String name = "SIMCom SIM7672";

    sendAT(GF("+CGMM"));
    String res2;
    if (waitResponse(1000L, res2) != 1) { return name; }
    res2.replace(GSM_NL "OK" GSM_NL, "");
    res2.replace("_", " ");
    res2.trim();

    name = res2;
    DBG("### Modem:", name);
    return name;
  }

  bool factoryDefaultImpl() {  // these commands aren't supported
    return false;
  }

  // AT+GSN command
  String getIMEIImpl() {
    sendAT(GF("+CGSN"));
    streamSkipUntil('\n');  // skip first newline
    String res = stream.readStringUntil('\n');
    waitResponse();
    res.trim();
    return res;
  }
  /*
   * Power functions
   */
 protected:
  bool restartImpl(const char* pin = NULL) {
    if (!testAT()) { return false; }
    sendAT(GF("+CRESET"));
    if (waitResponse(10000L) != 1) { return false; }
    delay(5000L);  // TODO(?):  Test this delay!
    return init(pin);
  }

  bool powerOffImpl() {
    sendAT(GF("+CPOF"));
    return waitResponse() == 1;
  }

  bool radioOffImpl() {
    if (!setPhoneFunctionality(4)) { return false; }
    delay(3000);
    return true;
  }

  bool sleepEnableImpl(bool enable = true) {
    sendAT(GF("+CSCLK="), enable);
    return waitResponse() == 1;
  }

  bool setPhoneFunctionalityImpl(uint8_t fun, bool reset = false) {
    sendAT(GF("+CFUN="), fun, reset ? ",1" : "");
    return waitResponse(10000L) == 1;
  }

  /*
   * Generic network functions
   */
 public:
  RegStatus getRegistrationStatus() {
    return (RegStatus)getRegistrationStatusXREG("CREG");
  }

 protected:
  bool isNetworkConnectedImpl() {
    RegStatus s = getRegistrationStatus();
    return (s == REG_OK_HOME || s == REG_OK_ROAMING);
  }

 public:

  String getLocalIPImpl() {
    sendAT(GF("+IPADDR"));  // Inquire Socket PDP address
    // sendAT(GF("+CGPADDR=1"));  // Show PDP address
    String res;
    if (waitResponse(10000L, res) != 1) { return ""; }
    res.replace(GSM_NL "OK" GSM_NL, "");
    res.replace(GSM_NL, "");
    res.trim();
    return res;
  }

  bool setNetworkActive(){
    sendAT(GF("+NETOPEN"));  
    int res = waitResponse(GF("+NETOPEN: 0"),GF("+IP ERROR: Network is already opened")); 
    if (res != 1 && res != 2){
      return false;
    }
    return true;
  }

  bool setNetworkDeactivate(){
    sendAT(GF("+NETCLOSE"));  
    if (waitResponse() != 1){
      return false;
    }
    int res = waitResponse(GF("+NETCLOSE: 0"),GF("+NETCLOSE: 2")); 
    if (res != 1 && res != 2){
      return false;
    }
    return true;
  }

  bool getNetworkActive() {
    sendAT(GF("+NETOPEN?"));
    int res = waitResponse(GF("+NETOPEN: 1"));
    if (res == 1) { return true; }
    return false;
  }

  String getNetworkAPN() {
    sendAT("+CGDCONT?");
    if (waitResponse(GF(GSM_NL "+CGDCONT: ")) != 1) { return "ERROR"; }
    streamSkipUntil(',');
    streamSkipUntil(',');
    streamSkipUntil('\"');
    String res = stream.readStringUntil('\"');
    waitResponse();
    if (res == "") { res = "APN IS NOT SET"; }
    return res;
  }

  bool setNetworkAPN(String apn) {
    sendAT(GF("+CGDCONT=1,\"IP\",\""), apn, "\"");
    return waitResponse() == 1;
  }
  
  /*
  * Return code:
  *     -1 ping failed
  *     1 Ping success
  *     2 Ping time out
  *     3 Ping result
  * * */
  int ping(const char *url, String &resolved_ip_addr,
         uint32_t &rep_data_packet_size,
         uint32_t &tripTime,
         uint8_t &TTL)
  {
      uint8_t dest_addr_type = 1;
      uint8_t num_pings = 1;
      uint8_t data_packet_size = 64;
      uint32_t interval_time = 1000;
      uint32_t wait_time = 10000;
      uint8_t ttl = 0xFF;
      int result_type = -1;

      sendAT("+CPING=\"", url, "\"", ",", dest_addr_type, ",",
            num_pings, ",", data_packet_size, ",",
            interval_time, ",", wait_time, ",", ttl);

      if (waitResponse() != 1) {
          return -1;
      }
      if (waitResponse(10000UL, "+CPING: ") == 1) {
          result_type = streamGetIntBefore(',');
          switch (result_type) {
          case 1:
              resolved_ip_addr = stream.readStringUntil(',');
              rep_data_packet_size = streamGetIntBefore(',');
              tripTime = streamGetIntBefore(',');
              TTL = streamGetIntBefore('\n');
              break;
          case 2:
              break;
          case 3:
              break;
          default:
              break;
          }
      } 
      return result_type;
  }
  
  /*
   * GPRS functions
   */
 protected:
  bool gprsConnectImpl(const char* apn, const char* user = NULL,
                       const char* pwd = NULL) {
    gprsDisconnect();  // Make sure we're not connected first

    // Define the PDP context

    // The CGDCONT commands set up the "external" PDP context

    // Set the external authentication
    if (user && strlen(user) > 0) {
      sendAT(GF("+CGAUTH=1,0,\""), user, GF("\",\""), pwd, '"');
      waitResponse();
    }

    // Define external PDP context 1
    sendAT(GF("+CGDCONT=1,\"IP\",\""), apn, '"', ",\"0.0.0.0\",0,0");
    waitResponse();

    // Configure TCP parameters

    // Select TCP/IP application mode (command mode)
    sendAT(GF("+CIPMODE=0"));
    waitResponse();

    // Set Sending Mode - send without waiting for peer TCP ACK
    sendAT(GF("+CIPSENDMODE=0"));
    waitResponse();

    // Configure socket parameters
    // AT+CIPCCFG= <NmRetry>, <DelayTm>, <Ack>, <errMode>, <HeaderType>,
    //            <AsyncMode>, <TimeoutVal>
    // NmRetry = number of retransmission to be made for an IP packet
    //         = 10 (default)
    // DelayTm = number of milliseconds to delay before outputting received data
    //          = 0 (default)
    // Ack = sets whether reporting a string "Send ok" = 0 (don't report)
    // errMode = mode of reporting error result code = 0 (numberic values)
    // HeaderType = which data header of receiving data in multi-client mode
    //            = 1 (+RECEIVE,<link num>,<data length>)
    // AsyncMode = sets mode of executing commands
    //           = 0 (synchronous command executing)
    // TimeoutVal = minimum retransmission timeout in milliseconds = 75000
    sendAT(GF("+CIPCCFG=10,0,0,0,1,0,75000"));
    if (waitResponse() != 1) { return false; }

    // Configure timeouts for opening and closing sockets
    // AT+CIPTIMEOUT=<netopen_timeout> <cipopen_timeout>, <cipsend_timeout>
    sendAT(GF("+CIPTIMEOUT="), 75000, ',', 15000, ',', 15000);
    waitResponse();

    // Start the socket service

    // This activates and attaches to the external PDP context that is tied
    // to the embedded context for TCP/IP (ie AT+CGACT=1,1 and AT+CGATT=1)
    // Response may be an immediate "OK" followed later by "+NETOPEN: 0".
    // We to ignore any immediate response and wait for the
    // URC to show it's really connected.
    sendAT(GF("+NETOPEN"));
    if (waitResponse(75000L, GF(GSM_NL "+NETOPEN: 0")) != 1) { return false; }

    return true;
  }

  bool gprsDisconnectImpl() {
    // Close all sockets and stop the socket service
    // Note: On the LTE models, this single command closes all sockets and the
    // service
    sendAT(GF("+NETCLOSE"));
    if (waitResponse(60000L, GF(GSM_NL "+NETCLOSE: 0")) != 1) { return false; }

    return true;
  }

  bool isGprsConnectedImpl() {
    sendAT(GF("+NETOPEN?"));
    // May return +NETOPEN: 1, 0.  We just confirm that the first number is 1
    if (waitResponse(GF(GSM_NL "+NETOPEN: 1")) != 1) { return false; }
    waitResponse();

    sendAT(GF("+IPADDR"));  // Inquire Socket PDP address
    // sendAT(GF("+CGPADDR=1")); // Show PDP address
    if (waitResponse() != 1) { return false; }

    return true;
  }

  /*
   * SIM card functions
   */
 protected:
  // Gets the CCID of a sim card via AT+CCID
  String getSimCCIDImpl() {
    sendAT(GF("+CICCID"));
    if (waitResponse(GF(GSM_NL "+ICCID:")) != 1) { return ""; }
    String res = stream.readStringUntil('\n');
    waitResponse();
    res.trim();
    return res;
  }

  /*
   * Phone Call functions
   */
 protected:
  bool callHangupImpl() {
    sendAT(GF("+CHUP"));
    return waitResponse() == 1;
  }

  /*
   * Messaging functions
   */
 protected:
  // Follows all messaging functions per template

  /*
   * GSM Location functions
   */
 protected:
  // Can return a GSM-based location from CLBS as per the template

  /*
   * GPS/GNSS/GLONASS location functions
   */
 protected:

   bool gpsColdStartImpl() {
    sendAT(GF("+CGPSCOLD"));
    if (waitResponse(10000L) != 1) { return false; }
    return true;
  }

  bool gpsWarmStartImpl() {
    sendAT(GF("+CGPSWARM"));
    if (waitResponse(10000L) != 1) { return false; }
    return true;
  }

  bool gpsHotStartImpl() {
    sendAT(GF("+CGPSHOT"));
    if (waitResponse(10000L) != 1) { return false; }
    return true;
  }
  
  // enable GPS
  bool enableGPSImpl(int8_t power_en_pin ,uint8_t enable_level) {
    if(power_en_pin!= -1){
      sendAT("+CGDRT=",power_en_pin,",1");
      waitResponse();
      sendAT("+CGSETV=",power_en_pin,",",enable_level);
      waitResponse();
    }
    if(isEnableGPSImpl()){
      return true;
    }
    sendAT(GF("+CGNSSPWR=1"));
    if (waitResponse() != 1) { return false; }
    return true;
  }

  bool disableGPSImpl(int8_t power_en_pin ,uint8_t disable_level) {
    if(power_en_pin!= -1){
      sendAT("+CGSETV=",power_en_pin,",",disable_level);
      waitResponse();
      sendAT("+CGDRT=",power_en_pin,",0");
      waitResponse();
    }
    if(!isEnableGPSImpl()){
      return true;
    }
    sendAT(GF("+CGNSSPWR=0"));
    if (waitResponse() != 1) { return false; }
    return true;
  }

  bool isEnableGPSImpl(){
    sendAT(GF("+CGNSSPWR?"));
    if (waitResponse("+CGNSSPWR: 1") != 1) { return false; }
    waitResponse();
    return true;
  }

  bool enableAGPSImpl() {
    sendAT(GF("+CGNSSPWR?"));
    if (waitResponse("+CGNSSPWR: 1") != 1) { return false; }
    // +CGNSSPWR:<GNSS_Power_status>
    sendAT("+CAGPS");
    if (waitResponse(30000UL, "+AGPS: success") != 1) { return false; }
    return true;
  }
  
  // get the RAW GPS output
  String getGPSrawImpl() {
    sendAT(GF("+CGNSSINFO"));
    if (waitResponse(GF(GSM_NL "+CGNSSINFO:")) != 1) { return ""; }
    String res = stream.readStringUntil('\n');
    waitResponse();
    res.trim();
    return res;
  }

  bool getGPS_ExImpl(GPSInfo& info) {
    float lat = 0;
    float lon = 0;
    // +CGNSSINFO:[<mode>],
    // [<GPS-SVs>],[BEIDOU-SVs],[<GLONASS-SVs>],[<GALILEO-SVs>],
    // [<lat>],[<N/S>],[<log>],[<E/W>],[<date>],[<UTC-time>],[<alt>],[<speed>],[<course>],[<PDOP>],[HDOP],[VDOP]
    sendAT(GF("+CGNSSINFO"));
    if (waitResponse(GF(GSM_NL "+CGNSSINFO: ")) != 1) { return false; }

    info.isFix = streamGetIntBefore(',');  // mode 2=2D Fix or 3=3DFix
    if (info.isFix == 2 || info.isFix == 3) {
      int16_t ret = -9999;
      // GPS-SVs      satellite valid numbers
      ret                    = streamGetIntBefore(',');
      info.gps_satellite_num = ret != -9999 ? ret : 0;
      // BEIDOU-SVs   satellite valid numbers
      ret                       = streamGetIntBefore(',');
      info.beidou_satellite_num = ret != -9999 ? ret : 0;
      // GLONASS-SVs  satellite valid numbers
      ret                        = streamGetIntBefore(',');
      info.glonass_satellite_num = ret != -9999 ? ret : 0;
      // GALILEO-SVs  satellite valid numbers
      ret                        = streamGetIntBefore(',');
      info.galileo_satellite_num = ret != -9999 ? ret : 0;
      // Latitude in ddmm.mmmmmm
      lat = streamGetFloatBefore(',');
      // N/S Indicator, N=north or S=south
      info.NS_indicator = stream.read();
      streamSkipUntil(',');
      // Longitude in ddmm.mmmmmm
      lon = streamGetFloatBefore(',');
      // E/W Indicator, E=east or W=west
      info.EW_indicator = stream.read();
      streamSkipUntil(',');
      // Date. Output format is ddmmyy
      // Two digit day
      info.day = streamGetIntLength(2);
      // Two digit month
      info.month = streamGetIntLength(2);
      // Two digit year
      info.year = streamGetIntBefore(',');
      // UTC Time. Output format is hhmmss.s
      // Two digit hour
      info.hour = streamGetIntLength(2);
      // Two digit minute
      info.minute = streamGetIntLength(2);
      // 4 digit second with subseconds
      float secondWithSS = streamGetFloatBefore(',');
      info.second        = static_cast<int>(secondWithSS);
      // MSL Altitude. Unit is meters
      info.altitude = streamGetFloatBefore(',');
      // Speed Over Ground. Unit is knots.
      info.speed = streamGetFloatBefore(',');
      // Course Over Ground. Degrees.
      info.course = streamSkipUntil(',');
      // After set, will report GPS every x seconds
      streamSkipUntil(',');
      // Position Dilution Of Precision
      float pdop = streamGetFloatBefore(',');
      info.PDOP  = pdop != -9999.0F ? pdop : 0;
      // Horizontal Dilution Of Precision
      float hdop = streamGetFloatBefore(',');
      info.HDOP  = hdop != -9999.0F ? hdop : 0;
      // Vertical Dilution Of Precision
      float vdop = streamGetFloatBefore(',');
      info.VDOP  = vdop != -9999.0F ? vdop : 0;
      streamSkipUntil('\n');
      waitResponse();
      info.latitude  = (lat) * (info.NS_indicator == 'N' ? 1 : -1);
      info.longitude = (lon) * (info.EW_indicator == 'E' ? 1 : -1);
      if (info.year < 2000) { info.year += 2000; }
      return true;
    }

    waitResponse();
    return false;
  }
 

  // get GPS informations
  bool getGPSImpl(uint8_t *status,float* lat, float* lon, float* speed = 0, float* alt = 0,
                  int* vsat = 0, int* usat = 0, float* accuracy = 0,
                  int* year = 0, int* month = 0, int* day = 0, int* hour = 0,
                  int* minute = 0, int* second = 0) {
    sendAT(GF("+CGNSSINFO"));
    if (waitResponse(GF(GSM_NL "+CGNSSINFO:")) != 1) { return false; }

    uint8_t fixMode = streamGetIntBefore(',');  // mode 2=2D Fix or 3=3DFix
                                                // TODO(?) Can 1 be returned
    if (fixMode == 1 || fixMode == 2 || fixMode == 3) {
      // init variables
      float ilat = 0;
      char  north;
      float ilon = 0;
      char  east;
      float ispeed       = 0;
      float ialt         = 0;
      int   ivsat        = 0;
      int   iusat        = 0;
      float iaccuracy    = 0;
      int   iyear        = 0;
      int   imonth       = 0;
      int   iday         = 0;
      int   ihour        = 0;
      int   imin         = 0;
      float secondWithSS = 0;

      ivsat = streamGetIntBefore(',');    // GPS satellite valid numbers
      streamSkipUntil(',');               // GLONASS satellite valid numbers
      streamSkipUntil(',');               // BEIDOU satellite valid numbers
      streamSkipUntil(',');
      ilat  = streamGetFloatBefore(',');  // Latitude in ddmm.mmmmmm
      north =  stream.read();              // N/S Indicator, N=north or S=south
      streamSkipUntil(',');
      ilon = streamGetFloatBefore(',');  // Longitude in ddmm.mmmmmm
      east =  stream.read();              // E/W Indicator, E=east or W=west
      streamSkipUntil(',');

      // Date. Output format is ddmmyy
      iday   = streamGetIntLength(2);    // Two digit day
      imonth = streamGetIntLength(2);    // Two digit month
      iyear  = streamGetIntBefore(',');  // Two digit year

      // UTC Time. Output format is hhmmss.s
      ihour = streamGetIntLength(2);  // Two digit hour
      imin  = streamGetIntLength(2);  // Two digit minute
      secondWithSS =
          streamGetFloatBefore(',');  // 4 digit second with subseconds

      ialt   = streamGetFloatBefore(',');  // MSL Altitude. Unit is meters
      ispeed = streamGetFloatBefore(',');  // Speed Over Ground. Unit is knots.
      streamSkipUntil(',');                // Course Over Ground. Degrees.
      streamSkipUntil(',');  // After set, will report GPS every x seconds
      iaccuracy = streamGetFloatBefore(',');  // Position Dilution Of Precision
      streamSkipUntil(',');   // Horizontal Dilution Of Precision
      streamSkipUntil(',');   // Vertical Dilution Of Precision
      streamSkipUntil('\n');  // TODO(?) is one more field reported??

      if (status){
          *status = fixMode;
      }
      // Set pointers
      if (lat != NULL){
          *lat = (ilat) * (north == 'N' ? 1 : -1);
      }
      if (lon != NULL){
          *lon = (ilon) * (east == 'E' ? 1 : -1);
      }
      if (speed != NULL) *speed = ispeed;
      if (alt != NULL) *alt = ialt;
      if (vsat != NULL) *vsat = ivsat;
      if (usat != NULL) *usat = iusat;
      if (accuracy != NULL) *accuracy = iaccuracy;
      if (iyear < 2000) iyear += 2000;
      if (year != NULL) *year = iyear;
      if (month != NULL) *month = imonth;
      if (day != NULL) *day = iday;
      if (hour != NULL) *hour = ihour;
      if (minute != NULL) *minute = imin;
      if (second != NULL) *second = static_cast<int>(secondWithSS);

      waitResponse();
      return true;
    }
    waitResponse();
    return false;
  }

 bool setGPSBaudImpl(uint32_t baud){
    sendAT("+CGNSSIPR=",baud);
    return waitResponse(1000L) == 1;
  }

  /*
  * Model: SIM7670G
  * 1  -  GPS
  * 3  -  GPS + GLONASS
  * 5  -  GPS + GALILEO
  * 9  -  GPS + BDS
  * 13 -  GPS + GALILEO + BDS
  * 15 -  GPS + GLONASS + GALILEO + BDS
  * */
  bool setGPSModeImpl(uint8_t mode){
      sendAT("+CGNSSMODE=",mode);
      return waitResponse(1000L) == 1;
  }

  bool setGPSOutputRateImpl(uint8_t rate_hz){
      sendAT("+CGPSNMEARATE=",rate_hz);
      return waitResponse(1000L) == 1;
  }

  bool enableNMEAImpl(bool outputAtPort){
      if(outputAtPort){
        sendAT("+CGNSSTST=1");
      }else{
        sendAT("+CGNSSTST=0");
      }
      waitResponse(1000L);
    // Select the output port for NMEA sentence
      sendAT("+CGNSSPORTSWITCH=0,1");
      return waitResponse(1000L) == 1;
  }

  bool disableNMEAImpl(){
      sendAT("+CGNSSTST=0");
      waitResponse(1000L);
    // Select the output port for NMEA sentence
      sendAT("+CGNSSPORTSWITCH=1,0");
      return waitResponse(1000L) == 1;
  }

  bool configNMEASentenceImpl(bool CGA,bool GLL,bool GSA,bool GSV,bool RMC,bool VTG,bool ZDA,bool ANT){
      char buffer[32];
      snprintf(buffer,32,"%u,%u,%u,%u,%u,%u,%u,0", CGA, GLL, GSA, GSV, RMC, VTG, ZDA);
      sendAT("+CGNSSNMEA=",buffer);
      return waitResponse(1000L) == 1;
  }
  /*
   * Time functions
   */
 protected:
  // Can follow the standard CCLK function in the template

  /*
   * NTP server functions
   */
  // Can sync with server using CNTP as per template

  /*
   * Battery functions
   */
 protected:
  // returns volts, multiply by 1000 to get mV
  uint16_t getBattVoltageImpl() {
    sendAT(GF("+CBC"));
    if (waitResponse(GF(GSM_NL "+CBC:")) != 1) { return 0; }

    // get voltage in VOLTS
    float voltage = streamGetFloatBefore('\n');
    // Wait for final OK
    waitResponse();
    // Return millivolts
    uint16_t res = voltage * 1000;
    return res;
  }

  int8_t getBattPercentImpl() TINY_GSM_ATTR_NOT_AVAILABLE;

  uint8_t getBattChargeStateImpl() TINY_GSM_ATTR_NOT_AVAILABLE;

  bool getBattStatsImpl(uint8_t& chargeState, int8_t& percent,
                        uint16_t& milliVolts) {
    chargeState = 0;
    percent     = 0;
    milliVolts  = getBattVoltage();
    return true;
  }

  /*
   * Temperature functions
   */
 protected:
  // get temperature in degree celsius
  uint16_t getTemperatureImpl() {
    sendAT(GF("+CPMUTEMP"));
    if (waitResponse(GF(GSM_NL "+CPMUTEMP:")) != 1) { return 0; }
    // return temperature in C
    uint16_t res = streamGetIntBefore('\n');
    // Wait for final OK
    waitResponse();
    return res;
  }

  /*
   * Client related functions
   */
 protected:
  bool modemConnect(const char* host, uint16_t port, uint8_t mux,
                    bool ssl = false, int timeout_s = 15) {
    if (ssl) { DBG("SSL not yet supported on this module!"); }
    // Make sure we'll be getting data manually on this connection
    sendAT(GF("+CIPRXGET=1"));
    if (waitResponse() != 1) { return false; }

    // Establish a connection in multi-socket mode
    uint32_t timeout_ms = ((uint32_t)timeout_s) * 1000;
    sendAT(GF("+CIPOPEN="), mux, ',', GF("\"TCP"), GF("\",\""), host, GF("\","),
           port);
    // The reply is OK followed by +CIPOPEN: <link_num>,<err> where <link_num>
    // is the mux number and <err> should be 0 if there's no error
    if (waitResponse(timeout_ms, GF(GSM_NL "+CIPOPEN:")) != 1) { return false; }
    uint8_t opened_mux    = streamGetIntBefore(',');
    uint8_t opened_result = streamGetIntBefore('\n');
    if (opened_mux != mux || opened_result != 0) return false;
    return true;
  }

  int16_t modemSend(const void* buff, size_t len, uint8_t mux) {
    sendAT(GF("+CIPSEND="), mux, ',', (uint16_t)len);
    if (waitResponse(GF(">")) != 1) { return 0; }
    stream.write(reinterpret_cast<const uint8_t*>(buff), len);
    stream.flush();
    if (waitResponse(GF(GSM_NL "+CIPSEND:")) != 1) { return 0; }
    streamSkipUntil(',');  // Skip mux
    streamSkipUntil(',');  // Skip requested bytes to send
    // TODO(?):  make sure requested and confirmed bytes match
    return streamGetIntBefore('\n');
  }

  size_t modemRead(size_t size, uint8_t mux) {
    if (!sockets[mux]) return 0;
#ifdef TINY_GSM_USE_HEX
    sendAT(GF("+CIPRXGET=3,"), mux, ',', (uint16_t)size);
    if (waitResponse(GF("+CIPRXGET:")) != 1) { return 0; }
#else
    sendAT(GF("+CIPRXGET=2,"), mux, ',', (uint16_t)size);
    if (waitResponse(GF("+CIPRXGET:")) != 1) { return 0; }
#endif
    streamSkipUntil(',');  // Skip Rx mode 2/normal or 3/HEX
    streamSkipUntil(',');  // Skip mux/cid (connecion id)
    int16_t len_requested = streamGetIntBefore(',');
    //  ^^ Requested number of data bytes (1-1460 bytes)to be read
    int16_t len_confirmed = streamGetIntBefore('\n');
    // ^^ The data length which not read in the buffer
    for (int i = 0; i < len_requested; i++) {
      uint32_t startMillis = millis();
#ifdef TINY_GSM_USE_HEX
      while (stream.available() < 2 &&
             (millis() - startMillis < sockets[mux]->_timeout)) {
        TINY_GSM_YIELD();
      }
      char buf[4] = {
          0,
      };
      buf[0] = stream.read();
      buf[1] = stream.read();
      char c = strtol(buf, NULL, 16);
#else
      while (!stream.available() &&
             (millis() - startMillis < sockets[mux]->_timeout)) {
        TINY_GSM_YIELD();
      }
      char c = stream.read();
#endif
      sockets[mux]->rx.put(c);
    }
    // DBG("### READ:", len_requested, "from", mux);
    // sockets[mux]->sock_available = modemGetAvailable(mux);
    sockets[mux]->sock_available = len_confirmed;
    waitResponse();
    return len_requested;
  }

  size_t modemGetAvailable(uint8_t mux) {
    if (!sockets[mux]) return 0;
    sendAT(GF("+CIPRXGET=4,"), mux);
    size_t result = 0;
    if (waitResponse(GF("+CIPRXGET:")) == 1) {
      streamSkipUntil(',');  // Skip mode 4
      streamSkipUntil(',');  // Skip mux
      result = streamGetIntBefore('\n');
      waitResponse();
    }
    // DBG("### Available:", result, "on", mux);
    if (!result) { sockets[mux]->sock_connected = modemGetConnected(mux); }
    return result;
  }

  bool modemGetConnected(uint8_t mux) {
    // Read the status of all sockets at once
    sendAT(GF("+CIPCLOSE?"));
    if (waitResponse(GF("+CIPCLOSE:")) != 1) {
      // return false;  // TODO:  Why does this not read correctly?
    }
    for (int muxNo = 0; muxNo < TINY_GSM_MUX_COUNT; muxNo++) {
      // +CIPCLOSE:<link0_state>,<link1_state>,...,<link9_state>
      bool muxState = stream.parseInt();
      if (sockets[muxNo]) { sockets[muxNo]->sock_connected = muxState; }
    }
    waitResponse();  // Should be an OK at the end
    if (!sockets[mux]) return false;
    return sockets[mux]->sock_connected;
  }

  /*
   * Utilities
   */
 public:
  // TODO(vshymanskyy): Optimize this!
  int8_t waitResponse(uint32_t timeout_ms, String& data,
                      GsmConstStr r1 = GFP(GSM_OK),
                      GsmConstStr r2 = GFP(GSM_ERROR),
#if defined TINY_GSM_DEBUG
                      GsmConstStr r3 = GFP(GSM_CME_ERROR),
                      GsmConstStr r4 = GFP(GSM_CMS_ERROR),
#else
                      GsmConstStr r3 = NULL, GsmConstStr r4 = NULL,
#endif
                      GsmConstStr r5 = NULL) {
    /*String r1s(r1); r1s.trim();
    String r2s(r2); r2s.trim();
    String r3s(r3); r3s.trim();
    String r4s(r4); r4s.trim();
    String r5s(r5); r5s.trim();
    DBG("### ..:", r1s, ",", r2s, ",", r3s, ",", r4s, ",", r5s);*/
    data.reserve(64);
    uint8_t  index       = 0;
    uint32_t startMillis = millis();
    do {
      TINY_GSM_YIELD();
      while (stream.available() > 0) {
        TINY_GSM_YIELD();
        int8_t a = stream.read();
        // putchar(a);
        if (a <= 0) continue;  // Skip 0x00 bytes, just in case
        data += static_cast<char>(a);
        if (r1 && data.endsWith(r1)) {
          index = 1;
          goto finish;
        } else if (r2 && data.endsWith(r2)) {
          index = 2;
          goto finish;
        } else if (r3 && data.endsWith(r3)) {
#if defined TINY_GSM_DEBUG
          if (r3 == GFP(GSM_CME_ERROR)) {
            streamSkipUntil('\n');  // Read out the error
          }
#endif
          index = 3;
          goto finish;
        } else if (r4 && data.endsWith(r4)) {
          index = 4;
          goto finish;
        } else if (r5 && data.endsWith(r5)) {
          index = 5;
          goto finish;
        } else if (data.endsWith(GF(GSM_NL "+CIPRXGET:"))) {
          int8_t mode = streamGetIntBefore(',');
          if (mode == 1) {
            int8_t mux = streamGetIntBefore('\n');
            if (mux >= 0 && mux < TINY_GSM_MUX_COUNT && sockets[mux]) {
              sockets[mux]->got_data = true;
            }
            data = "";
            // DBG("### Got Data:", mux);
          } else {
            data += mode;
          }
        } else if (data.endsWith(GF(GSM_NL "+RECEIVE:"))) {
          int8_t  mux = streamGetIntBefore(',');
          int16_t len = streamGetIntBefore('\n');
          if (mux >= 0 && mux < TINY_GSM_MUX_COUNT && sockets[mux]) {
            sockets[mux]->got_data = true;
            if (len >= 0 && len <= 1024) { sockets[mux]->sock_available = len; }
          }
          data = "";
          // DBG("### Got Data:", len, "on", mux);
        } else if (data.endsWith(GF("+IPCLOSE:"))) {
          int8_t mux = streamGetIntBefore(',');
          streamSkipUntil('\n');  // Skip the reason code
          if (mux >= 0 && mux < TINY_GSM_MUX_COUNT && sockets[mux]) {
            sockets[mux]->sock_connected = false;
          }
          data = "";
          DBG("### Closed: ", mux);
        } else if (data.endsWith(GF("+CIPEVENT:"))) {
          // Need to close all open sockets and release the network library.
          // User will then need to reconnect.
          DBG("### Network error!");
          if (!isGprsConnected()) { gprsDisconnect(); }
          data = "";
        }
      }
    } while (millis() - startMillis < timeout_ms);
  finish:
    if (!index) {
      data.trim();
      if (data.length()) { DBG("### Unhandled:", data); }
      data = "";
    }
    // data.replace(GSM_NL, "/");
    // DBG('<', index, '>', data);
    return index;
  }

  int8_t waitResponse(uint32_t timeout_ms, GsmConstStr r1 = GFP(GSM_OK),
                      GsmConstStr r2 = GFP(GSM_ERROR),
#if defined TINY_GSM_DEBUG
                      GsmConstStr r3 = GFP(GSM_CME_ERROR),
                      GsmConstStr r4 = GFP(GSM_CMS_ERROR),
#else
                      GsmConstStr r3 = NULL, GsmConstStr r4 = NULL,
#endif
                      GsmConstStr r5 = NULL) {
    String data;
    return waitResponse(timeout_ms, data, r1, r2, r3, r4, r5);
  }

  int8_t waitResponse(GsmConstStr r1 = GFP(GSM_OK),
                      GsmConstStr r2 = GFP(GSM_ERROR),
#if defined TINY_GSM_DEBUG
                      GsmConstStr r3 = GFP(GSM_CME_ERROR),
                      GsmConstStr r4 = GFP(GSM_CMS_ERROR),
#else
                      GsmConstStr r3 = NULL, GsmConstStr r4 = NULL,
#endif
                      GsmConstStr r5 = NULL) {
    return waitResponse(1000, r1, r2, r3, r4, r5);
  }

 public:
  Stream& stream;

 protected:
  GsmClientSim7672* sockets[TINY_GSM_MUX_COUNT];
  const char*       gsmNL = GSM_NL;
};

#endif  // SRC_TINYGSMCLIENTSIM7672_H_

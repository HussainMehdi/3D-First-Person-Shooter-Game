#include <zoidcom.h>
//
// the server class
//

class Server : public ZCom_Control
{
protected:
  // number of users currently connected
  int      m_conncount;
public:
  // constructor - gets called when the server is created with new Server(...)
  Server( int _internalport, int _udpport )
  {
    m_conncount = 0;

    // this will allocate the sockets and create local bindings
    if ( !ZCom_initSockets( true, _udpport, _internalport, 0 ) )
    {
      cout<<"Failed to initialize sockets!\n";
      // exit program immediately
      exit(255);
    }

    // string shown in log output
    ZCom_setDebugName("ZCOM_SRV");

    // enable broadcast listening on _udpport
    this->ZCom_setDiscoverListener(eZCom_DiscoverEnable, _udpport);

    cout<<"Server running and listening on udp port: "<< _udpport;    
  }

protected:

  // this is called when a broadcast is received
  bool ZCom_cbDiscoverRequest(const ZCom_Address &_addr, ZCom_BitStream &_request, ZCom_BitStream &_reply) 
  {
    // print info to console
    cout<<"Discover request from" << _addr.toString() <<" , "<<   _request.getStringStatic();
    // send a nice string back to broadcaster
    _reply.addString("Zoidcom server here!");
    // tell zoidcom to reply to broadcast by returning true
    return true;
  };

  // unused callbacks are empty
  bool ZCom_cbConnectionRequest( ZCom_ConnID _id, ZCom_BitStream &_request, ZCom_BitStream &_reply ) { return false; }
  void ZCom_cbConnectionSpawned( ZCom_ConnID _id ) {};
  void ZCom_cbConnectionClosed( ZCom_ConnID _id, eZCom_CloseReason _reason, ZCom_BitStream &_reasondata ) {}
  void ZCom_cbDataReceived( ZCom_ConnID _id, ZCom_BitStream &_data ) {}
  bool ZCom_cbZoidRequest( ZCom_ConnID _id, zU8 _requested_level, ZCom_BitStream &_reason) {return false;}
  void ZCom_cbZoidResult(ZCom_ConnID _id, eZCom_ZoidResult _result, zU8 _new_level, ZCom_BitStream &_reason) {}
  void ZCom_cbConnectResult( ZCom_ConnID _id, eZCom_ConnectResult _result, ZCom_BitStream &_reply ) {}
  void ZCom_cbNodeRequest_Dynamic( ZCom_ConnID _id, ZCom_ClassID _requested_class, ZCom_BitStream *_announcedata, eZCom_NodeRole _role, ZCom_NodeID _net_id ) {}
  void ZCom_cbNodeRequest_Tag( ZCom_ConnID _id, ZCom_ClassID _requested_class, ZCom_BitStream *_announcedata, eZCom_NodeRole _role, zU32 _tag ) {}
  void ZCom_cbDiscovered( const ZCom_Address & _addr, ZCom_BitStream &_reply )  {}
};

//
// log output function - writes log from zoidcom to console
//

void logfunc(const char *_log)
{
  // comment out this line if you don't want to see zoidcom's internal logging
  cout<<_log;
}


//
// main function - program starts here
//

int start_server()
{
  // initialize zoidcom with console logging function
  ZoidCom *zcom = new ZoidCom(logfunc);
  if (!zcom)
    return -1;

  if (!zcom->Init())
  {
    cout<<"Problem initializing Zoidcom.\n";
    return -1;
  }

  // server operates on internal port 1 and UDP port 8899
  Server *srv = new Server( 1, 8899 );

  cout<<"Press CTRL+C to abort.\n";

  // zoidcom needs to get called regularly to get anything done so we enter the mainloop now
  while (1)
  {
    // processes incoming packets
    // all callbacks are generated from within the processInput calls
    srv->ZCom_processInput( eZCom_NoBlock );

    // outstanding data will be packed up and sent from here
    srv->ZCom_processOutput();

    // pause the program for a few milliseconds
    //ZoidCom::Sleep(10);
  }

  // delete the server object
  delete srv;

  // delete zoidcom
  delete zcom;

}
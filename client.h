/****************************************
* client.cpp
* simple client in one source file
*
* This file is part of the "Zoidcom Automated Networking System" application library.
* Copyright (C)2002-2004 by Jörg Rüppel. See documentation for copyright and licensing details.
*****************************************/

#include <zoidcom.h>


//
// the client class
//
vector3df cdata;
class Client : public ZCom_Control
{
protected:
public:
  // constructor - gets called when the client is created with new Client(...)
  Client()
  {
    // this will allocate the sockets and create local bindings
    if ( !ZCom_initSockets( true, 0, 0, 0 ) )
    {
      printf("Failed to initialize sockets!\n");
      // exit program immediately
      exit(255);
    }

    // string shown in log output
    ZCom_setDebugName("ZCOM_CLI");
  }

protected:
  // someone has replied to our broadcast
  void ZCom_cbDiscovered( const ZCom_Address & _addr, ZCom_BitStream &_reply )
  {
    printf("Broadcast reply from (%s): %s\n", _addr.toString(), _reply.getStringStatic());
  }

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
  bool ZCom_cbDiscoverRequest(const ZCom_Address &_addr, ZCom_BitStream &_request, ZCom_BitStream &_reply) {return false;}
};

//
// log output function - writes log from zoidcom to console
//


//
// main function - program starts here
//

int start_client()
{
  // initialize zoidcom with console logging function
  ZoidCom *zcom = new ZoidCom(logfunc);
  if (!zcom)
    return -1;
  std::string send="",temp;
  stringstream process;
  process<<cdata.X;
  process>>temp;
  send+=temp+" ";
   process<<cdata.Y;
  process>>temp;
  send+=temp+" ";
 process<<cdata.Z;
  process>>temp;
  send+=temp;
  if (!zcom->Init())
  {
   // printf("Problem initializing Zoidcom.\n");
    return -1;
  }
  
  // create client
  Client *cli = new Client( );

  // create broadcast address 
  ZCom_Address dst_udp;
  dst_udp.setPort( 8899 );

  // create broadcast data
  ZCom_BitStream *broadcast = new ZCom_BitStream;
  
  broadcast->addString(send.c_str());

  // send broadcast
 // printf("Broadcasting to port %d...\n", 8899);
  if (!cli->ZCom_Discover( dst_udp, broadcast ))
  {
  //  printf("Client: unable to send broadcast!\n");  
    zcom->Sleep(2000);
    exit(255);
  }

//  printf("Press CTRL+C to abort.\n");

  // zoidcom needs to get called regularly to get anything done so we enter the mainloop now
  // otherwise it wouldn't even start to connect
  while (1)
  {
    cdata=update();
	  // processes incoming packets
    // all callbacks are generated from within the processInput calls
    cli->ZCom_processInput( eZCom_NoBlock );

    // outstanding data will be packed up and sent from here
    cli->ZCom_processOutput();

    // pause the program for a few milliseconds
    ZoidCom::Sleep(10);
  }

  // delete the client object
  delete cli;

  // delete zoidcom
  delete zcom;

  return 0;
}

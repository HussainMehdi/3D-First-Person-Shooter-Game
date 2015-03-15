/****************************************
* zoidcom_address.h
* network address
*
* This file is part of the "Zoidcom Automated Networking System" application library.
* Copyright (C)2002-2007 by Joerg Rueppel. See documentation for copyright and licensing details.
*****************************************/

/** \file zoidcom_address.h
*/

#ifndef _ZOIDCOMADDRESS_H_
#define _ZOIDCOMADDRESS_H_

#include "zoidcom.h"

/*
** forwards
*/

class ZCom_Address_Private;

/* ===================================== */

/** @brief Zoidcom network address.
*/
class ZCOM_API ZCom_Address
{
private:
  ZCom_Address_Private *m_priv;
public:
  /// Constructor.
  ZCom_Address(void);
  /// Copy constructor.
  ZCom_Address(const ZCom_Address &);
  /// Destructor.
  ~ZCom_Address(void);

  /** @brief Set IP and port from string representation.
      @param _type Address type, only eZCom_AddressUDP currently supported by this function.
      @param _control_id Address control id. See also: \ref ZCom_Control::ZCom_setControlID(), \ref socketsharing
      @param _addr C-String in the form of a.b.c.d:port or hostname:port.
      @returns false if the address has invalid syntax

      The hostname will be resolved when the address is used to connect somewhere, or if resolveHostname() is
      called.
  */
  bool setAddress(eZCom_AddressType _type, zU8 _control_id, const char *_addr);


  /** @brief Returns string representation of IP-address in a static buffer.
      @returns Pointer to static buffer containing address in w.x.y.z:port format.

      \note Works only for IP addresses. If address has been set by hostname,
            resolveHostname() has to be called first.

      This function is threadsafe.
  */
  const char* getAddressIP(eZCom_GetIPAddressOption _with_port = eZCom_AddressWithPort) const;

  /** @brief Returns the previously set hostname in a static buffer.
      @returns Pointer to static buffer containing previously set hostname in hostname:port format.
      \note Returns NULL if address has not been set as a hostname string before.

      This function is threadsafe.
  */
  const char* getAddressHostname() const;

  /** @brief Get string representation of address.
      @returns Pointer to a static C-str. Result will be invalidated by next call to this function.

      Returns string in format [local]::port or [udp]:x.x.x.x:port or [udp]:hostname:port if no ip is available.

      This function is threadsafe.
  */
  const char* toString() const;

  /// Set IP address.
  void setIP(zU8 _a, zU8 _b, zU8 _c, zU8 _d);
  /// Set IP address (host byte order)
  void setIP(zU32 _ip);
  /// Set port.
  void setPort(zU16 _port);
  /// Set type.
  void setType(eZCom_AddressType _type);
  /// Set control id.
  void setControlID(zU8 _id);

  /** @brief Get port.
      @returns Port.
  */
  zU16 getPort(void) const;
  /** @brief Get IP as one 32bit value.
      @returns IP as 32bit value, access it as ((char*)getIP())[0-3]
  */
  zU32 getIP(void) const;
  /** @brief Get IP components separately.
      @param _pos 0-3, indicating which component should be returned.
      @returns Component of IP address.
  */
  zU8 getIP(zU8 _pos) const;
  /** @brief Get type.
      @returns Type of this address.
  */
  eZCom_AddressType getType(void) const;
  /** @brief Get control ID.
      @returns Control ID of this address.
  */
  zU8 getControlID(void) const;

  /** @brief Compare 2 addresses.
      @param _c Second address for comparison.
      @returns 'true' if addresses are equal.
  */
  bool operator==(const ZCom_Address &_c) const;
  /** @brief Assignment.
      @param _s Source address to copy from.
  */
  ZCom_Address& operator=(const ZCom_Address &_s);

  /** @brief Resolve previously set hostname to IP address.
      @param _async 'true' if you want the method to return immediately, 'false' if you want the method to return
             when the hostname is resolved or failed to resolve.
      @param _timeout Set timeout for _synchronous_ lookups in msecs. If _async is 'true', the _timeout parameter 
             will be ignored.
      @returns 'true' if _async==true and resolving could be started\n
               'true' if _async==false and resolving was successful\n
               'false' otherwise\n

      If _async is true, the application has to poll regularly until the result is available.
  */
  bool resolveHostname(bool _async, zU32 _timeout);

  /** @brief Check if async hostname resolution finished.
      @returns eZCom_HostnameResult
  */
  eZCom_HostnameResult checkHostname();

  // compute hashkey for address - internal
  zU32 computeHashKey(zU32 _max) const;
};


#endif
